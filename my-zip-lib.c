/* Necessary libraries for helper functions. */
#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "my-zip-lib.h"

/* Overall memory amount that can be given to output usage. */
long memory_amount = 0;

long init_maximum_memory() {
    /* Get total memory amount and set it as global maximum */
    memory_amount = get_usable_memory();
    return memory_amount;
}

MappedFile map_file(char file_name[]) {
    
    /* Map memory in the file.
    Used https://stackoverflow.com/questions/20460670/reading-a-file-to-string-with-mmap
    as an example for the operation.
    Open filehandle and check it success. */
    int file;
    MappedFile mapped_file;
    struct stat file_info;
    if ((file = open(file_name, O_RDONLY)) == 0) {
        /* Exit if the file read fails */
        printf("wzip: cannot open file\n");
        exit(1);
    }
    /* Get size of the file */
    if (fstat(file, &file_info) != 0) {
        printf("wzip: Failed to get file information. Exiting...");
        exit(1);
    }
    mapped_file.size = file_info.st_size;
    /* Map file to the memory. */
    mapped_file.data = mmap(0, mapped_file.size, PROT_READ, MAP_PRIVATE, file, 0);
    if (mapped_file.data == MAP_FAILED) {
        printf("Failed to map file to memory. Exiting...");
        exit(1);
    }
    /* According to this link it's okay to close file after mapping.
    https://stackoverflow.com/questions/17490033/do-i-need-to-keep-a-file-open-after-calling-mmap-on-it */
    close(file);
    return mapped_file;
}

unsigned long long get_usable_memory() {
    /* https://stackoverflow.com/questions/2513505/how-to-get-available-memory-c-g/26639774 */
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return (long)(pages * page_size * MAX_MEMORY_PERCENTAGE);
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

void rle_expand(RleList* rlelist, int requested_size) {
    /* If requested size is more than maximum memory crash with out of memory error.
       This shouldn't never happen because of paging. */
    if (requested_size > memory_amount) {
        printf("Can't allocate more than defined max memory. Please contact developper.\n");
        exit(1);
    }
    /* Make RleList double large of the requested size so it won't be expanded continously.
    Match size of maximum available memory if the new size is too large */
    int new_size = requested_size * sizeof(Rle*) * MEMORY_MULTIPLIER;
    if (new_size > memory_amount / sizeof(Rle*)) {
        new_size = (int)(memory_amount / sizeof(Rle*));
    }
    if ((rlelist->data = (Rle*)realloc(rlelist->data, new_size)) == NULL) {
        printf("Couldn't allocate more memory.\n");
        exit(1);
    }
    /* Update new physical size in structs. */
    rlelist->size = new_size / sizeof(Rle*);
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

void string_allocate(String* string, int initial_size) {
    /* Allocate new list of rle structs. */
    string->size = initial_size;
    string->length = 0;
    if ((string->data = (char*)calloc(initial_size, sizeof(char))) == NULL) {
        printf("Couldn't allocate memory.\n");
        exit(1);
    }
}

void string_expand(String* string, int requested_size) {
    if (requested_size > memory_amount) {
        printf("Can't allocate more than defined max memory. Please contact developper.\n");
        exit(1);
    }
    int new_size = requested_size * sizeof(char*) * MEMORY_MULTIPLIER;
    if (new_size > memory_amount / sizeof(char*)) {
        new_size = (int)(memory_amount / sizeof(char*));
    }
    /* Make string double large of the requested size so it won't be expanded continously. */
    if ((string->data = (char*)realloc(string->data, new_size)) == NULL) {
        printf("Couldn't allocate more memory.\n");
        exit(1);
    }
    /* Update new physical size. */
    string->size = new_size / sizeof(char*);
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
    strncat(string1->data, string2, string1->size);
    string1->length = new_length;
}

void generate_chars(String* string, char character, int amount) {
    /* Expand string if needed for generated letters. */
    if (amount > string->size - string->length) {
        string_expand(string, string->length + amount);
    }
    /* Generate letters. */
    for (int point = string->length; point < amount + string->length; point++) {
        string->data[point] = character;
    }
    string->length += amount;
}

void string_free(String* string) {
    /* Free dynamically allocated memory of the dynamic string. */
    free(string->data);
}