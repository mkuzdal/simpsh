/** @file: simpsh.c */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <fcntl.h>
#include <ucontext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

// Globals:

// Flags:
static int F_VERBOSE = 0;
static int F_PROFILE = 0;

// Return Status:
static int EXIT_STATUS = 0;

// Usage info
struct rusage usage;

double prevUserTime = 0.0;
double prevSystemTime = 0.0;

// Book-keeping:
int fd_count = 0;
int Nsubprocess = 0;
int Msubprocess = 1;

// Command struct:
typedef struct
{
    pid_t pid;
    char* cmd;
    char** args;
} command;

command** subprocesses;

// Signal Handler:
void catch_handler (int sig) {
    fprintf (stderr, "Signal %d caught\n", sig);
    perror ("");
    exit (sig);
}

// Helper Functions:
void closeAllFildes (int start, int N) {
    int ii = start;
    while (ii < N + 3) {
    	close(ii);
	ii++;
    }
}

void usageMessage (int who, int end) {
    if (getrusage (who, &usage) < 0) {
        fprintf (stderr, "--Profile Error : Unable to get usage information\n");
	perror ("");
        EXIT_STATUS = 1;
    }
    else {
        double totalUserTime = (double)usage.ru_utime.tv_sec + (double)usage.ru_utime.tv_usec/1000000;     
        double totalSystemTime = (double)usage.ru_stime.tv_sec + (double)usage.ru_stime.tv_usec/1000000;

	if (who == RUSAGE_SELF) {
	    double deltaUserTime = totalUserTime - prevUserTime;
            double deltaSystemTime = totalSystemTime - prevSystemTime;

            printf ("current command user time: %f\n", deltaUserTime);
            printf ("current command system time: %f\n", deltaSystemTime);
	   
            prevUserTime = totalUserTime;
            prevSystemTime = totalSystemTime;
        }
  	
	if (who == RUSAGE_CHILDREN || end) { 
            printf ("total user time: %f\n", totalUserTime);
            printf ("total system time: %f\n", totalSystemTime);
	}
     

	/*	printf ("maximum resident set size: %ld\n", (long int) usage.ru_maxrss);
	printf ("integral shared memory size: %ld\n", (long int) usage.ru_ixrss);
	printf ("integral unshared data size: %ld\n", (long int) usage.ru_idrss);
	printf ("integral unshared stack size: %ld\n", (long int) usage.ru_isrss);
	*/
    }
}

int is_active_fd (int fd) {
    return fcntl (fd, F_GETFD) != -1 || errno != EBADF;
}

int main (int argc, char** argv) {
    subprocesses = malloc (sizeof (command*) * Msubprocess);
    if (subprocesses == NULL) {
        fprintf (stderr, "Memory Allocation Error\n");
        perror ("");
        exit (-1);
    }

    opterr = 0;

    static struct option long_options[] = {
        { "append",	no_argument,		0,		'a'	},
	{ "cloexec", 	no_argument, 		0,		'c'	},
	{ "creat", 	no_argument,		0,		'C'	},
	{ "directory",	no_argument,		0,		'd'	},
	{ "dsync",	no_argument,		0,		'D'	},
	{ "excl",	no_argument,		0,		'e'	},
	{ "nofollow", 	no_argument,		0,		'n'	},
	{ "nonblock", 	no_argument,		0,		'N'	},
	{ "rsync", 	no_argument,		0,		'r'	},
	{ "sync", 	no_argument,		0,		's'	},
	{ "trunc", 	no_argument,		0,		't'	},  
	{ "rdonly",     required_argument,      0,              'R'     },
	{ "rdwr", 	required_argument,	0,		'y'	},
	{ "wronly", 	required_argument,	0,		'w'	},
	{ "pipe", 	no_argument,		0,		'p'	},
	{ "command", 	no_argument,    	0,		'm'	},
	{ "wait", 	no_argument,		0,		'W'	},
	{ "close", 	required_argument,	0,		'l'	},
	{ "verbose", 	no_argument,		&F_VERBOSE,	 1	},
	{ "profile", 	no_argument,		&F_PROFILE,	 1	},
	{ "abort",	no_argument,		0,		'A'	},
	{ "catch",	required_argument,	0,		'h'	},
	{ "ignore",	required_argument,	0,		'i'	},
	{ "default",	required_argument,	0,		'f'	},
	{ "pause",	no_argument,		0,		'P'	},
    	{ 0,		0,			0,		 0	}
    };
 
    int opt = 0;
    int long_index = 0;
    int oflags = 0;

    while ( (opt = getopt_long (argc, argv, "", long_options, &long_index)) != -1) {
      if (opt != '?') { 
        if (F_VERBOSE || F_PROFILE) {
	    printf ("--%s", long_options[long_index].name);
	    if (long_options[long_index].has_arg == required_argument) {
	    	printf (" %s", optarg);
	    }
	    if (long_options[long_index].val == 'm') {
		int arg_i = optind;
		while (arg_i < argc) {
		    if (argv[arg_i][0] == '-' && argv[arg_i][1] == '-')
			break;
		    printf (" %s", argv[arg_i]);
		    arg_i++;
		} 
	    }
	    printf ("\n");
	}

	switch (opt) {
	    case 'a': // append
	    {
		oflags |= O_APPEND;
		break;
	    }
	    case 'c': // cloexec
	    {
		oflags |= O_CLOEXEC;
		break;
	    }
	    case 'C': // creat
 	    {
		oflags |= O_CREAT;
		break;
	    }
	    case 'd': // directory
	    {
		oflags |= O_DIRECTORY;
		break;
	    }
	    case 'D': // dsync
	    {
		oflags |= O_DSYNC;
		break;
	    }
	    case 'e': // excl
	    {	
		oflags |= O_EXCL;
		break;
	    }
	    case 'n': // nofollow
  	    {
		oflags |= O_NOFOLLOW;
		break;
	    }
	    case 'N': // nonblock
	    {
		oflags |= O_NONBLOCK;
		break;
	    }
	    case 'r': // rsync
	    {
		oflags |= O_RSYNC;
		break;
	    }
	    case 's': // sync
	    {
		oflags |= O_SYNC;
		break;
	    }
	    case 't': // trunc
	    {
		oflags |= O_TRUNC;
		break;
	    }
            case 'R': // rdonly
	    {
		oflags |= O_RDONLY;
	        int fd = open (optarg, oflags);
		if (fd < 0) {
		    fprintf (stderr, "--rdonly Error : Unable to open the file `%s'\n", optarg);
		    perror ("");
		    EXIT_STATUS = 1;
		} 
		else {
		    if (dup2 (fd, fd_count + 3) < 0) {
		        fprintf (stderr, "--rdonly Error : Unable to open the file `%s'\n", optarg);
		        perror ("");
		        EXIT_STATUS = 1;
		    }  
		    if (fd != fd_count + 3) {
		        if (close (fd) < 0) {
			    fprintf (stderr, "--rdonly Error : Unable to open the file `%s'\n", optarg);
			    perror ("");
			    EXIT_STATUS = 1;
			}
		    }
		    fd_count++;		
		}
		oflags = 0;
		break;
	    }

	    case 'y': // rdwr
	    {
		oflags |= O_RDWR;
	        int fd = open (optarg, oflags);
		if (fd < 0) {
		    fprintf (stderr, "--rdwr Error : Unable to open the file `%s'\n", optarg);
		    perror ("");
		    EXIT_STATUS = 1;
		}
		else {
		    if (dup2 (fd, fd_count + 3) < 0) {
		        fprintf (stderr, "--rdwr Error : Unable to open the file `%s'\n", optarg);
		        perror ("");
		        EXIT_STATUS = 1;
		    }   
		    if (fd != fd_count + 3) {
		        if (close (fd) < 0) {
			    fprintf (stderr, "--rdwr Error : Unable to open the file `%s'\n", optarg);
			    perror ("");
			    EXIT_STATUS = 1;
		        }
		    }
		    fd_count++;		
		}
		oflags = 0;
		break;
	    }

	    case 'w': // wronly
	    {	
		oflags |= O_WRONLY;
		int fd = open (optarg, oflags);
		if (fd < 0) {
		    fprintf (stderr, "--wronly Error : Unable to open the file `%s'\n", optarg);
		    perror ("");	
		    EXIT_STATUS = 1;
		} 
		else {
		    if (dup2 (fd, fd_count + 3) < 0) {
		        fprintf (stderr, "--wronly Error : Unable to open the file `%s'\n", optarg);
		        perror ("");
		        EXIT_STATUS = 1;
		    }
		    if (fd != fd_count + 3) {
		        if (close (fd) < 0) {
		            fprintf (stderr, "--wronly Error : Unable to open the file `%s'\n", optarg);
			    perror ("");
			    EXIT_STATUS = 1;
		        }
		    }
		    fd_count++;
		}
		oflags = 0;
		break;
	    }

	    case 'p': // pipe
	    {		
		int pipefd[2];
		int pfd = pipe(pipefd);
		if (pfd < 0) {
		    fprintf (stderr, "--pipe Error : Unable to create pipe\n");
		    perror ("");
		    EXIT_STATUS = 1;
		}
		else {
		    if (dup2 (pipefd[0], fd_count + 3) < 0) {
		        fprintf (stderr, "--pipe Error : Unable to create pipe\n");
		        perror ("");
		        EXIT_STATUS = 1;
		    }
		    if (pipefd[0] != fd_count + 3) {
		      if (close (pipefd[0]) < 0) {
			  fprintf (stderr, "--pipe Error : Unable to create the pipe\n");
			  perror ("");
			  EXIT_STATUS = 1;
		      }
		    }
		    fd_count++;
		    if (dup2 (pipefd[1], fd_count + 3) < 0) {
		        fprintf (stderr, "--pipe Error : Unable to create the pipe\n");
		        perror ("");
		        EXIT_STATUS = 1;
		    }
		    if (pipefd[1] != fd_count + 3) {
		        if (close (pipefd[1]) < 0) {
			    fprintf (stderr, "--pipe Error : Unable to create the pipe\n");
			    perror ("");
			    EXIT_STATUS = 1;
		        }
		    }
		    fd_count++;
		}
		break;
	    }

	    case 'm': // command
	    {
		int bad_file = 0;
		int optindCopy = optind;
		while (optindCopy < argc) {
		    if (argv[optindCopy][0] == '-' && argv[optindCopy][1] == '-' )
		    	break;
		    
		    if (!is_active_fd (atoi (argv[optindCopy]) + 3) && (optindCopy - optind) >= 0 && (optindCopy - optind) <= 2) {
			fprintf (stderr, "--command Error : Invalid file descriptor %s\n", argv[optindCopy]);
			EXIT_STATUS = 1;
			bad_file = 1;
		    }
		    optindCopy++;
		}
		optindCopy--;	
		if (optindCopy - optind < 3) {
		    fprintf (stderr, "--command Error : Too few arguments\n");
		    EXIT_STATUS = 1;
		} 
		else if (!bad_file) { 
		    command* current_process = malloc (sizeof (command));
		    if (current_process == NULL) {
		        fprintf (stderr, "--command Error : Unable to allocate memory\n");
		        perror ("");
		        exit(-1);
		    }
		    current_process->cmd = argv[optind + 3];
		    current_process->args = malloc (sizeof (char*) * (optindCopy - optind - 1));   
		    if (current_process->args == NULL) {
		        fprintf (stderr, "--command Error : Unable to allocate memory\n");
		        perror ("");
		        exit (-1);
		    } 
		    else { 
		        current_process->args[0] = current_process->cmd;
		        int arg_i = 1;
		        while (arg_i < optindCopy - optind - 2) {
			    current_process->args[arg_i] = argv[optind + 3 + arg_i];
			    arg_i++;
		        }
		        current_process->args[arg_i] = NULL;
		        current_process->pid = fork ();
		        if (current_process->pid == -1) {
			    fprintf (stderr, "--command Error : Unable to create child process\n");
			    perror ("");
			    EXIT_STATUS = 1;
			    free (current_process);
		        }
		        else if (current_process->pid == 0) {	
			    dup2 (atoi (argv[optind + 0]) + 3, STDIN_FILENO);
			    dup2 (atoi (argv[optind + 1]) + 3, STDOUT_FILENO);
			    dup2 (atoi (argv[optind + 2]) + 3, STDERR_FILENO);
			    
			    free (subprocesses);
			    free (current_process);			
			    closeAllFildes (3, fd_count + 100);	

			    execvp (current_process->cmd, current_process->args);
		            _exit (-1);
		        }	
		        else {
			    if (Nsubprocess > Msubprocess) {
			        Msubprocess *= 2;
			        subprocesses = realloc (subprocesses, sizeof (command*) * Msubprocess);
				if (subprocesses == NULL) {
				    fprintf (stderr, "--command Error : Unable to allocate memory\n");
				    perror ("");
				    exit (-1);
				}
			    }
			    subprocesses[Nsubprocess] = current_process;
			    Nsubprocess++;
		        }
		    }
		}
		break;
	    }

	    case 'W': // wait
	    {	
		int exit_code;
		int id;
		int i = 0;
		int max_code = 0;

		closeAllFildes (3, fd_count);
		while (i < Nsubprocess) {
		    id = waitpid(-1, &exit_code, 0);
		    exit_code = WEXITSTATUS (exit_code);
		    printf ("%d ", exit_code);
		    if (exit_code > max_code) {
			max_code = exit_code;	    
		    }
		    int j = 0;
		    while (j < Nsubprocess) {
		        if (id == subprocesses[j]->pid) 
			    break;
			j++;
		    }
		    printf ("%s", subprocesses[j]->cmd);
		    int arg_i = 1;
		    while (subprocesses[j]->args[arg_i] != NULL) {
		    	printf (" %s", subprocesses[j]->args[arg_i]);
			arg_i++;
		    }	
		    printf ("\n"); 
		    i++;
		}
	        if (F_PROFILE) {
		    printf ("Parent:\n");
		    usageMessage (RUSAGE_SELF, 1);
		    printf ("Children:\n");
		    usageMessage (RUSAGE_CHILDREN, 1);
	        }
		free (subprocesses);
		if (EXIT_STATUS != 0)
		    exit (EXIT_STATUS);
		exit (max_code);
		break;
	    }	
	    case 'l': // close 
	    {	
		int fd = atoi (optarg);
		if (fd >= fd_count) {
		    fprintf (stderr, "--close Error : Invalid descriptor '%d'\n", fd);
		}
		else {
		    close(atoi (optarg) + 3);
		}
		break;
	    }
	    case 'A': // abort
	    {
	        raise (SIGSEGV);
	        break;
	    }
	    case 'h': // catch
	    {
	        signal (atoi (optarg), &catch_handler);
		break;
	    }
	    case 'i': // ignore
	    {
	        signal (atoi (optarg), SIG_IGN);
		break;
	    }
	    case 'f': // default
	    {
	        signal (atoi (optarg), SIG_DFL);
		break;
	    }	
	    case 'P': // pause
	    {	
	        if (pause () < 0) {
		  fprintf (stderr, "--pause Error\n");
		  perror ("");
		  EXIT_STATUS = 1;
	        }
		break;
	    }
	    case '?':
	    {
		break;
	    }
	}

	if (F_PROFILE) {
	    usageMessage (RUSAGE_SELF, 0);
	}
      }
    }
    free (subprocesses);
    closeAllFildes (3, fd_count);
    exit(EXIT_STATUS);
}
