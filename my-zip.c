/* Needed by getline. */
#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* Percentage of system memory that can be used at most */
#define MAX_MEMORY_PERCENTAGE 0.02
#define MEMORY_MULTIPLIER 2
#define INITIAL_MEMORY 10000

typedef struct MappedFile {
    /* Data itself */
    char* data;
    /* Physical size of the file */
    int size;
} MappedFile;

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
    long size;
    /* Actual length of the Rle list. */
    long length;
} RleList;

/* Overall memory amount that can be given to output usage. */
long memory_amount;

/* Function definations */
FILE* open_file(char file_name[]) ;
MappedFile map_file(char file_name[]);
unsigned long long get_usable_memory();
void compress(RleList* output, char letters[], long start_pos, long end_pos);
void rle_allocate(RleList* rlelist, int initial_size);
void rle_expand(RleList* rlelist, int requested_size);
void rle_append(RleList* rlelist, Rle* rle);
void rle_free(RleList* rlelist);

int main(int argc, char** argv) {
    MappedFile mapped_file;
    RleList output;
    long page_size;
    /* Check there is argument, if not print help. */
    if (argc == 1) {
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }
    /* Zip all the files with rle */
    else {
        /* Get total memory amount. */
        memory_amount = get_usable_memory();
        /* Prepare output Rle list. */
        rle_allocate(&output, INITIAL_MEMORY);
        /* Count maximum page_size. */
        page_size = (long)(memory_amount / sizeof(Rle*) - 1);
        //page_size = 2;
        /* Read files to one large string. */
        for (int argument_number = 1; argument_number < argc; argument_number++) {
            mapped_file = map_file(argv[argument_number]);
            /* Read file in chunks if there is not enough output memory for the worst case scenario */
            long start = 0;
            long end = 0;
            while (1) {
                /* Calculate start and end position based on page. */
                /* On the beginning start from zero. */
                if (end != 0) {
                    start = end + 1;
                }
                else {
                    start = 0;
                }
                if (page_size < mapped_file.size - end) {
                    end = end + page_size;
                }
                else {
                    end = mapped_file.size;
                }
                /* Move last element to first if needed. */
                if (output.length > 0) {
                    output.data[0].character = output.data[output.length - 1].character;
                    output.data[0].count = output.data[output.length - 1].count;
                    output.length = 1;
                }
                /* Start compression progress. */
                compress(&output, mapped_file.data, start, end);
                if (end < mapped_file.size) {
                    /* Write compressed data to stdout but leave last entry out since its not complete. */
                    /* Check that there is actually data to be written. */
                    if (output.length > 1) {
                        fwrite(output.data, sizeof(Rle), output.length - 1, stdout);
                    }
                }
                else {
                    /* Write end results to buffer but leave one result out if not final file
                    incase next file will have same letter that should be combined. */
                    if (argument_number != argc -1) {
                        fwrite(output.data, sizeof(Rle), output.length - 1, stdout);
                    }
                    else {
                        /* Write finally all bytes. */
                        fwrite(output.data, sizeof(Rle), output.length, stdout);
                    }
                    break;
                }
            }
            /* Free unnecessary memory. */
            munmap(mapped_file.data, mapped_file.size);
        }
        /* Free remaining memory. */
        rle_free(&output);
    }
    return 0;
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

void compress(RleList* output, char letters[], long start_pos, long end_pos) {
    /* Compress given string with Run Length Encoding. */
    Rle element;
    char letter;
    int count;
    /* Continue from the last item if there is allready data in the output. */
    if (output->length > 0) {
        letter = output->data[output->length - 1].character;
        count = output->data[output->length - 1].count;
        output->length--;
    }
    else {
        letter = letters[start_pos];
        count = 0;
    }
    /* Idea of the compress function is to calculate how many occurances of same letter is found.
       Function works by comparing current and previous letters. If there is a difference
       function will save amount and letter to the list of structs that can be directly written
       in binary mode. */
    for (int position = start_pos; position <= end_pos; position++) {
        if (letters[position] != letter) {
            element.character = letter;
            element.count = count;
            /* Add struct to list of structs. */
            rle_append(output, &element);
            count = 0;
            letter = letters[position];
        }
        count++;
    }
    /* If final character is not null append so far found character to output.
       This is needed in paged situation. */
    if (letters[end_pos] != '\0') {
        element.character = letters[end_pos];
        element.count = count;
        rle_append(output, &element);
    }
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