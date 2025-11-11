all: tests

tests: pteditor_test.c ../PTEditor/ptedit_header.h
	gcc -Os pteditor_test.c -std=gnu99 -o tests -fsanitize=address
