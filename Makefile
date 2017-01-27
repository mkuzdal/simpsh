CC = gcc
CFLAGS = -g -Wall -Wextra

default: simpsh

simpsh: simpsh.c
	$(CC) $(CFLAGS) simpsh.c -o $@


check: simpsh test1a
	@-echo -e "\e[1m------------------------\e[0m"

test1a: test1 test2 test3 test4 test5 test6 test7 test8
test1b: test9
test1:
	@-echo -e "\e[1m----- TEST CASE 1: -----\n\e[0m"; \
	echo "test1" | cat > _0_.txt; \
	cat _0_.txt > _1_.txt; \
	./simpsh --rdonly _1_.txt > /dev/null 2>&1; \
	if [ $$? -eq 0 ] ; then \
		diff _1_.txt _0_.txt; \
		if [ $$? -eq 0 ] ; then \
			echo -e "     Test 1 \e[32;1mSuccess!\n\e[0m"; \
		else \
			echo "     Function output incorrect."; \
			echo -e "     Test 1 \e[31;1mFailed!\n\e[0m"; \
		fi \
	else \
		echo "     Simpsh did not exit with status 0."; \
		echo -e "     Test 1 \e[31;1mFailed!\n\e[0m"; \
	fi; \
	rm _0_.txt _1_.txt

test2:
	@-echo -e "\e[1m----- TEST CASE 2: -----\n\e[0m"; \
	./simpsh --rdonly _2_.txt > /dev/null 2>&1; \
	if [ $$? -eq 1 ] ; then \
		if [ -e "_2_.txt" ] ; then \
			echo "     File was created."; \
			echo -e "     Test 2 \e[31;1mFailed!\n\e[0m"; \
		else \
			echo -e "     Test 2 \e[32;1mSuccess!\n\e[0m"; \
		fi \
	else \
		echo "     Simpsh did not exit with status 1."; \
		echo -e "     Test 2 \e[31;1mFailed!\n\e[0m"; \
	fi

test3: 
	@-echo -e "\e[1m----- TEST CASE 3: -----\n\e[0m"; \
	echo "test3" | cat > _0_.txt; \
	cat _0_.txt > _3_.txt; \
	./simpsh --wronly _3_.txt > /dev/null 2>&1; \
	if [ $$? -eq 0 ] ; then \
		diff _3_.txt _0_.txt; \
		if [ $$? -eq 0 ] ; then \
			echo -e "     Test 3 \e[32;1mSuccess!\n\e[0m"; \
		else \
			echo "     Function output incorrect."; \
			echo -e "     Test 3 \e[31;1mFailed!\n\e[0m"; \
		fi \
	else \
		echo "     Simpsh did not exit with status 0."; \
		echo -e "     Test 3 \e[31;1mFailed!\n\e[0m"; \
	fi; \
	rm _0_.txt _3_.txt

test4:
	@-echo -e "\e[1m----- TEST CASE 4: -----\n\e[0m"; \
	touch _4_o.txt; \
	touch _4_e.txt; \
	echo "test4" | cat > _0_.txt; \
	cat _0_.txt > _4_.txt; \
	./simpsh --rdonly _4_.txt --wronly _4_o.txt --wronly _4_e.txt --command 0 1 2 cat > /dev/null 2>&1; \
	if [ $$? -eq 0 ] ; then \
		diff _4_o.txt _0_.txt; \
		if [ $$? -eq 0 ] ; then \
			echo -e "     Test 4 \e[32;1mSuccess!\n\e[0m"; \
		else \
			echo "     Function output incorrect."; \
			echo -e "     Test 4 \e[31;1mFailed!\n\e[0m"; \
		fi \
	else \
		echo "     Simpsh did not exit with status 0."; \
		echo -e "     Test 4 \e[31;1mFailed!\n\e[0m"; \
	fi; \
	rm _0_.txt _4_.txt _4_o.txt _4_e.txt

test5: 
	@-echo -e "\e[1m----- TEST CASE 5: -----\n\e[0m"; \
	touch _5_o.txt; \
	touch _5_e.txt; \
	echo "test5" | cat > _0_.txt; \
	cat _0_.txt > _5_.txt; \
	./simpsh --rdonly _5_.txt --wronly _5_o.txt --wronly _5_e.txt --command 0 1 2 wc > /dev/null 2>&1; \
	if [ $$? -eq 0 ] ; then \
		sleep 1; \
		wc < _0_.txt > _5_o_std.txt; \
		diff _5_o.txt _5_o_std.txt; \
		if [ $$? -eq 0 ] ; then \
			echo -e "     Test 5 \e[32;1mSuccess!\n\e[0m"; \
		else \
			echo "     Command output incorrect."; \
			echo -e "     Test 5 \e[31;1mFailed!\n\e[0m"; \
		fi; \
		rm _5_o_std.txt; \
	else \
		echo "     Simpsh did not exit with status 0."; \
		echo -e "     Test 5 \e[31;1mFailed!\n\e[0m"; \
	fi; \
	rm _0_.txt _5_.txt _5_o.txt _5_e.txt

test6:
	@-echo -e "\e[1m----- TEST CASE 6: -----\n\e[0m"; \
	touch _6_o.txt; \
	touch _6_e.txt; \
	echo "test6" | cat > _0_.txt; \
	cat _0_.txt > _6_.txt; \
	./simpsh --rdonly _6_.txt --wronly _6_o.txt --wronly _6_e.txt --coxxx --command 0 1 2 cat > /dev/null 2>&1; \
	if [ $$? -eq 0 ] ; then \
		sleep 1; \
		diff _6_o.txt _0_.txt; \
		if [ $$? -eq 0 ] ; then \
			echo -e "     Test 6 \e[32;1mSuccess!\n\e[0m"; \
		else \
			echo "     Function did not continue to the next option."; \
			echo -e "     Test 6 \e[31;1mFailed!\n\e[0m"; \
		fi \
	else \
		echo "     Simpsh did not exit with status 0."; \
		echo -e "     Test 6 \e[31;1mFailed!\n\e[0m"; \
	fi; \
	rm _0_.txt _6_.txt _6_o.txt _6_e.txt

test7:
	@-echo -e "\e[1m----- TEST CASE 7: -----\n\e[0m"; \
	touch _7_i.txt; \
	touch _7_o.txt; \
	touch _7_e.txt; \
	touch _7_o_m.txt; \
	touch _7_v_ne.txt; \
	touch _7_v_e.txt; \
	./simpsh --rdonly _7_i.txt --wronly _7_o.txt --wronly _7_e.txt --verbose --command 0 1 2 sleep 0.01 > _7_o_m.txt 2> /dev/null; \
	if [ $$? -eq 0 ] ; then \
		cat _7_o_m.txt | grep "sleep" > _7_v_ne.txt; \
		cat _7_o_m.txt | grep "only" > _7_v_e.txt; \
		if [ -s _7_v_ne.txt ] ; then \
			if [ -s _7_v_e.txt ] ; then \
				echo "     Function printed options other than verbose and command"; \
				echo -e "     Test 7 \e[31;1mFailed!\n\e[0m"; \
			else \
				echo -e "     Test 7 \e[32;1mSuccess!\n\e[0m";	\
			fi \
		else \
			echo "     Function did not print command."; \
			echo -e "     Test 7 \e[31;1mFailed!\n\e[0m"; \
		fi \
	else \
		echo "     Simpsh did not exit with status 0."; \
		echo -e "     Test 7 \e[31;1mFailed!\n\e[0m"; \
	fi; \
	rm _7_i.txt _7_o.txt _7_e.txt _7_o_m.txt _7_v_ne.txt _7_v_e.txt

test8:
	@-echo -e "\e[1m----- TEST CASE 8: -----\n\e[0m"; \
	touch _8_i.txt; \
	touch _8_o.txt; \
	touch _8_e.txt; \
	touch _8_e_o.txt; \
	./simpsh --rdonly _8_i.txt --wronly _8_o.txt --wronly _8_e.txt --command 2 4 6 ls 2> _8_e_o.txt; \
	if [ $$? -eq 1 ] ; then \
		if [ -s "_8_e_o.txt" ] ; then \
			echo -e "     Test 8 \e[32;1mSuccess!\n\e[0m"; \
		else \
			echo "     No error message sent."; \
			echo -e "     Test 8 \e[31;1mFailed!\n\e[0m"; \
		fi \
	else \
		echo "     Simpsh did not exit with status 0."; \
		echo -e "     Test 8 \e[31;1mFailed!\n\e[0m"; \
	fi; \
	rm _8_i.txt _8_o.txt _8_e.txt _8_e_o.txt


dist: lab1-matthewkuzdal.tar.gz
submission_files = README Makefile simpsh.c
lab1-matthewkuzdal.tar.gz: $(submission_files)
	tar cf - --transform='s|^|lab1-matthewkuzdal/|' $(submission_files) | gzip -9 >$@

clean:
	rm -rf simpsh
