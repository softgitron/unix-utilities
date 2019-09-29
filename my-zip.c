#include <stdio.h>
#include <sys/mman.h>
#include "my-zip-lib.h"

#define TRUE 1
#define FALSE 0

/* Function definations */
void page_and_compress(MappedFile* mapped_file, RleList* output, long page_size, short final_file);
void compress(RleList* output, char letters[], long start_pos, long end_pos);

int main(int argc, char** argv) {
    MappedFile mapped_file;
    RleList output;
    long memory_amount;
    long page_size;
    short final_file = FALSE;
    /* Check there is argument, if not print help. */
    if (argc == 1) {
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }
    /* Zip all the files with rle */
    else {
        /* Get total memory amount. */
        memory_amount = init_maximum_memory();
        /* Prepare output Rle list. */
        rle_allocate(&output, INITIAL_MEMORY);
        /* Count maximum page_size. */
        page_size = (long)(memory_amount / sizeof(Rle*) - 1);
        /* page_size = 2; */
        /* Map files to memory and then compress. */
        for (int argument_number = 1; argument_number < argc; argument_number++) {
            mapped_file = map_file(argv[argument_number]);
            if (argument_number == argc -1) {
                final_file = TRUE;
            }
            /* Start compression progress with page support. */
            page_and_compress(&mapped_file, &output, page_size, final_file);
            /* Free handeled file from the memory. */
            munmap(mapped_file.data, mapped_file.size);
        }
        /* Free remaining memory. */
        rle_free(&output);
    }
    return 0;
}

void page_and_compress(MappedFile* mapped_file, RleList* output, long page_size, short final_file) {
    /* Read file in chunks if there is not enough output memory for the worst case scenario */
    long start = 0;
    long end = 0;
    while (1) {
        /* Calculate start and end position based on the page. */
        /* On the beginning start from zero. */
        if (end != 0) {
            start = end + 1;
        }
        else {
            start = 0;
        }
        if (page_size < mapped_file->size - end) {
            end = end + page_size;
        }
        else {
            end = mapped_file->size;
        }
        /* Start compression progress. */
        compress(output, mapped_file->data, start, end);
        if (end < mapped_file->size) {
            /* Write compressed data to stdout but leave last entry out since its not complete. */
            /* Check that there is actually data to be written. */
            if (output->length > 1) {
                fwrite(output->data, sizeof(Rle), output->length - 1, stdout);
            }
        }
        else {
            /* Write end results to buffer but leave one result out if not final file
            in case next file will have same letter that should be combined. */
            if (final_file != TRUE) {
            fwrite(output->data, sizeof(Rle), output->length - 1, stdout);
            }
            else {
                /* Write finally all bytes. */
                fwrite(output->data, sizeof(Rle), output->length, stdout);
            }
            break;
        }
    }
}

void compress(RleList* output, char letters[], long start_pos, long end_pos) {
    /* Compress given string with Run Length Encoding. */
    Rle element;
    char letter;
    int count;
    /* Continue from the last item if there is allready data in the output
       and start buffer from the beginning. */
    if (output->length > 0) {
        letter = output->data[output->length - 1].character;
        count = output->data[output->length - 1].count;
        output->length = 0;
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