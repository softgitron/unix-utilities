compile-flags = -Wall -Werror -std=c99
# Taken help for debug variables
# https://stackoverflow.com/questions/1079832/how-can-i-configure-my-makefile-for-debug-and-release-builds

my-cat: my-cat.c
	gcc my-cat.c $(compile-flags) -o my-cat

my-grep: my-grep.c
	gcc my-grep.c $(compile-flags) -o my-grep

my-zip: my-zip-main.o my-zip-lib.o
	gcc my-zip-main.o my-zip-lib.o -o my-zip 

my-zip-main.o: my-zip.c
	gcc my-zip.c -c -o my-zip-main.o $(compile-flags)

my-zip-lib.o: my-zip-lib.c
	gcc my-zip-lib.c -c $(compile-flags)

my-unzip: my-unzip-main.o my-zip-lib.o
	gcc my-unzip-main.o my-zip-lib.o -o my-unzip 

my-unzip-main.o: my-unzip.c
	gcc my-unzip.c -c -o my-unzip-main.o $(compile-flags)

my-zip-debug: compile-flags += -DDEBUG -g -O0
my-zip-debug: my-zip

my-unzip-debug: compile-flags += -DDEBUG -g -O0
my-unzip-debug: my-unzip

my-grep-debug: compile-flags += -DDEBUG -g -O0
my-grep-debug: my-grep

my-cat-debug: compile-flags += -DDEBUG -g -O0
my-cat-debug: my-cat

clean:
	rm -rf *.o