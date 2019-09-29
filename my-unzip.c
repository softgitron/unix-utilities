#include <stdio.h>
#include <sys/mman.h>
#include "my-zip-lib.h"

/* Function definations */
void buffer_and_uncompress(MappedFile mapped_file, String buffer, long buffer_size);
void print_buffer(String* string);

int main(int argc, char** argv) {
    MappedFile mapped_file;
    long memory_amount;
    String buffer;
    long buffer_size;
    /* Check there is argument, if not print help. */
    if (argc == 1) {
        printf("wunzip: file1 [file2 ...]\n");
        return 1;
    }
    /* Unzip all the files with rle */
    else {
        /* Get total memory amount. */
        memory_amount = init_maximum_memory();
        /* Count maximum buffer_size. */
        buffer_size = (long)(memory_amount / sizeof(Rle*) - 1);
        /* Prepare buffer. */
        string_allocate(&buffer, INITIAL_MEMORY);
        /* Map files to memory and then uncompress. */
        for (int argument_number = 1; argument_number < argc; argument_number++) {
            mapped_file = map_file(argv[argument_number]);
            /* Start compression progress with page support. */
            buffer_and_uncompress(mapped_file, buffer, buffer_size);
            /* Free handeled file from the memory. */
            munmap(mapped_file.data, mapped_file.size);
        }
        string_free(&buffer);
    }
    return 0;
}

void buffer_and_uncompress(MappedFile mapped_file, String buffer, long buffer_size) {
    int* length_p;
    char* character_p;
    int length;
    char character;
    /* Go two 5 bytes at a time as defined in format. */
    for (int byte = 0; byte < mapped_file.size; byte += 5) {
        /* Get length and character info using pointer arithmetrics. */
        length_p = (int*)(mapped_file.data + byte);
        character_p = (char*)(mapped_file.data + byte + 4);
        length = *length_p;
        character = *character_p;
        /* If amount of amount of character to be printed is greater than buffer
           generate character in chunks. */
        if (length > buffer_size) {
            print_buffer(&buffer);
            for (int page = 0; page < length; page += buffer_size) {
                if (length - page < buffer_size) {
                    generate_chars(&buffer, character, length - page);
                }
                else {
                    generate_chars(&buffer, character, buffer_size);
                }
                print_buffer(&buffer);
            }
        }
        /* If there is smaller than buffer amounts of character but buffer is full
           print buffer first and then generate characters. */
        else if (buffer.length + length > buffer_size) {
            print_buffer(&buffer);
            generate_chars(&buffer, character, length);
        }
        /* If there is still room in the buffer, just generate characters. */
        else {
            generate_chars(&buffer, character, length);
        }
    }
    /* If there is character left in the buffer print them. */
    if (buffer.length > 0) {
        print_buffer(&buffer);
    }
}

void print_buffer(String* string) {
    /* Little helper function for buffer printing. */
    fwrite(string->data, sizeof(char), string->length, stdout);
    string->length = 0;
}