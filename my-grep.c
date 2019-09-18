/* Needed by getline. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* open_file(char file_name[]) ;
void search_and_print(FILE* file, char* search_term);

int main(int argc, char** argv) {
    FILE* file;
    /* Check there is argument, if not print help. */
    if (argc == 1) {
        printf("wgrep: searchterm [file ...]\n");
        return 1;
    }
    /* Use stdin instead of file if only one parameter. */
    else if (argc == 2) {
        file = stdin;
        search_and_print(file, argv[1]);
    }
    /* Search terms from files line by line */
    else {
        for (int argument_number = 2; argument_number < argc; argument_number++) {
            file = open_file(argv[argument_number]);
            search_and_print(file, argv[1]);
            fclose(file);
        }
    }
    return 0;
}

FILE* open_file(char file_name[]) {
    /* Open filehandle and check it success. */
    FILE* file;
    if ((file = fopen(file_name, "r")) == NULL) {
        /* Exit if the file read fails */
        printf("wgrep: cannot open file\n");
        exit(1);
    }
    return file;
}

void search_and_print(FILE* file, char* search_term) {
    char* line = NULL;
    size_t len = 0;
    /* Read file line by line and print line */
    while (getline(&line, &len, file) != -1) {
        /* Print line if it contains search term. */
        if (strstr(line, search_term) != NULL) {
            printf("%s", line);
        }
    }
    /* Free memory at the end as specified by getline manual. */
    free(line);
}