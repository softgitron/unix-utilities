#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1000

void read_and_print(char file_name[]);

int main(int argc, char** argv) {
    /* Start to print files one by one */
    for (int argument_number = 1; argument_number < argc; argument_number++) {
        read_and_print(argv[argument_number]);
    }
    return 0;
}

void read_and_print(char file_name[]) {
    char buffer[BUFFER_SIZE];
    FILE* file;
    if ((file = fopen(file_name, "r")) == NULL) {
        /* Exit if the file read fails */
        printf("wcat: cannot open file\n");
        exit(1);
    }
    /* Read until end of file. */
    while ((fgets(buffer, BUFFER_SIZE, file)) != NULL) {
        printf("%s", buffer);
    }
    fclose(file);
}