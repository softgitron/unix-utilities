/* Needed by getline. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 10000
/* Bigger is better for systems with large memory.
   Might waste a lot of memory if set too big. */
#define MEMORY_MULTIPLIER 5

typedef struct String {
    /* Data itself */
    char* chars;
    /* Physical size of the string. */
    int size;
    /* Actual length of the string. */
    int length;
} String;

#pragma pack(push,1)
typedef struct Rle {
    /* Amount of characters. */
    int count;
    /* Character to be multiplied. */
    char character;
} Rle;
#pragma pack(pop)
/* https://stackoverflow.com/questions/40642765/how-to-tell-gcc-to-disable-padding-inside-struct
   makes struct follow size it should logically be. */

typedef struct RleList {
    /* Data itself */
    Rle* data;
    /* Physical size of the Rle list. */
    int size;
    /* Actual length of the Rle list. */
    int length;
} RleList;

FILE* open_file(char file_name[]) ;
String read_file(FILE* file);
RleList compress(char letters[], int start_pos, int end_pos);
void string_allocate(String* string, int initial_size);
void string_expand(String* string, int new_size);
void string_append(String* string1, char string2[]);
void string_combine(String* string1, String* string2);
void string_free(String* string);
void rle_allocate(RleList* rlelist, int initial_size);
void rle_expand(RleList* rlelist, int new_size);
void rle_append(RleList* rlelist, Rle* rle);
void rle_free(RleList* rlelist);

int main(int argc, char** argv) {
    FILE* file;
    String lines;
    string_allocate(&lines, BUFFER_SIZE);
    String new_lines;
    RleList output;
    /* Check there is argument, if not print help. */
    if (argc == 1) {
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }
    /* Zip all the files with rle */
    else {
        /* Read files to one large string. */
        for (int argument_number = 1; argument_number < argc; argument_number++) {
            file = open_file(argv[argument_number]);
            new_lines = read_file(file);
            fclose(file);
            /* Add latest file to big string. */
            string_combine(&lines, &new_lines);
            string_free(&new_lines);
        }
        /* Start compression progress. */
        output = compress(lines.chars, 0, lines.length);
        /* Write compressed data to stdout. */
        fwrite(output.data, sizeof(Rle), output.length, stdout);
    }
    /* Free left variables. */
    string_free(&lines);
    rle_free(&output);
    return 0;
}

FILE* open_file(char file_name[]) {
    /* Open filehandle and check it success. */
    FILE* file;
    if ((file = fopen(file_name, "rb")) == NULL) {
        /* Exit if the file read fails */
        printf("wgrep: cannot open file\n");
        exit(1);
    }
    return file;
}

String read_file(FILE* file) {
    /* Read file to long string quickly. */
    char buffer[BUFFER_SIZE];
    String lines;
    string_allocate(&lines, BUFFER_SIZE * MEMORY_MULTIPLIER);
    /* From fread man pages */
    clearerr(file);
    int read;
    while (feof(file) == 0) {
        /* Add always null to the end */
        read = fread(buffer, sizeof(char), BUFFER_SIZE - 1, file);
        buffer[read] = '\0';
        string_append(&lines, buffer);
    }
    return lines;
}

RleList compress(char letters[], int start_pos, int end_pos) {
    /* Compress given string with Run Length Encoding. */
    RleList output;
    Rle element;
    rle_allocate(&output, 100);
    char letter = letters[start_pos];
    int count = 1;
    /* Idea of the compress function is to calculate how many occurances of same letter is found.
       Function works by comparing current and previous letters. If there is a difference
       function will save amount and letter to the list of structs that can be directly written
       in binary mode. */
    for (int position = start_pos + 1; position <= end_pos; position++) {
        if (letters[position] != letter) {
            element.character = letter;
            element.count = count;
            /* Add struct to list of structs. */
            rle_append(&output, &element);
            count = 0;
            letter = letters[position];
        }
        count++;
    }
    return output;
}

void string_allocate(String* string, int initial_size) {
    /* Allocate new dynamic string. */
    string->size = initial_size;
    string->length = 0;
    if ((string->chars = (char*)calloc(initial_size, sizeof(char))) == NULL) {
        printf("Couldn't allocate memory.\n");
        exit(1);
    }
}

void string_expand(String* string, int new_size) {
    /* Make string double large of the requested size so it won't be expanded continously. */
    if ((string->chars = (char*)realloc(string->chars, new_size * sizeof(char) * MEMORY_MULTIPLIER)) == NULL) {
        printf("Couldn't allocate more memory.\n");
        exit(1);
    }
    /* Update new physical size. */
    string->size = new_size * MEMORY_MULTIPLIER;
}

void string_append(String* string1, char string2[]) {
    /* Add classical C-string to dynamic string. */
    int string2_length = strlen(string2);
    int new_length;
    new_length = string1->length + string2_length;
    /* Check that there is enough space in dynamic string. */
    if (new_length + 1 > string1->size) {
        string_expand(string1, new_length);
    }
    /* Combine strings to first string. */
    strncat(string1->chars, string2, string1->size);
    string1->length = new_length;
}

void string_combine(String* string1, String* string2) {
    /* Work basically as append function, but for two dynamic strings. */
    int new_length;
    new_length = string1->length + string2->length;
    if (new_length + 1 > string1->size) {
        string_expand(string1, new_length);
    }
    strncat(string1->chars, string2->chars, string1->size);
    string1->length = new_length;
}

void string_free(String* string) {
    /* Free dynamically allocated memory of the dynamic string. */
    free(string->chars);
}

void rle_allocate(RleList* rlelist, int initial_size) {
    /* Allocate new list of rle structs. */
    rlelist->size = initial_size;
    rlelist->length = 0;
    if ((rlelist->data = (Rle*)calloc(initial_size, sizeof(Rle))) == NULL) {
        printf("Couldn't allocate memory.\n");
        exit(1);
    }
}

void rle_expand(RleList* rlelist, int new_size) {
    /* Make RleList double large of the requested size so it won't be expanded continously. */
    if ((rlelist->data = (Rle*)realloc(rlelist->data, new_size * sizeof(Rle*) * MEMORY_MULTIPLIER)) == NULL) {
        printf("Couldn't allocate more memory.\n");
        exit(1);
    }
    /* Update new physical size. */
    rlelist->size = new_size * MEMORY_MULTIPLIER;
}

void rle_append(RleList* rlelist, Rle* rle) {
    /* Check that there is enough space for new Rle element */
    if (rlelist->length + 1 > rlelist->size) {
        rle_expand(rlelist, rlelist->length + 1);
    }
    /* Add Rle element to the list */
    rlelist->data[rlelist->length].character = rle->character;
    rlelist->data[rlelist->length].count = rle->count;
    rlelist->length++;
}

void rle_free(RleList* rlelist) {
    /* Free dynamically allocated memory of the rle list. */
    free(rlelist->data);
}