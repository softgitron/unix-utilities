my-cat: my-cat.c
	gcc my-cat.c -Wall -Werror -std=c99 -o my-cat

my-cat-debug: my-cat.c
	gcc my-cat.c -Wall -Werror -std=c99 -o my-cat -g

my-grep: my-grep.c
	gcc my-grep.c -Wall -Werror -std=c99 -o my-grep

my-grep-debug: my-grep.c
	gcc my-grep.c -Wall -Werror -std=c99 -o my-grep -g

my-zip: my-zip.c
	gcc my-zip.c -Wall -Werror -std=c99 -o my-zip

my-zip-debug: my-zip.c
	gcc my-zip.c -Wall -Werror -std=c99 -o my-zip -g