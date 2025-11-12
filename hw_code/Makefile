all: tests

tests: pteditor_test.c ../PTEditor/ptedit_header.h
	gcc -Os pteditor_test.c -std=gnu99 -o tests -fsanitize=address

a: gen_eviction_sets.c  ../PTEditor/ptedit_header.h
	# rm ./tests
	gcc -O0 gen_eviction_sets.c -std=gnu99 -o tests -fsanitize=address
	./tests

