/* Percentage of system memory that can be used at most */
#define MAX_MEMORY_PERCENTAGE 0.1
#define MEMORY_MULTIPLIER 2
#define INITIAL_MEMORY 10000

typedef struct MappedFile {
    /* Data itself */
    void* data;
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

typedef struct String {
    /* Data itself */
    char* data;
    /* Physical size of the string */
    int size;
    /* Virtual size of the string */
    int length;
} String;

/* Function definations */
long init_maximum_memory();
FILE* open_file(char file_name[]);
MappedFile map_file(char file_name[]);
unsigned long long get_usable_memory();

void rle_allocate(RleList* rlelist, int initial_size);
void rle_expand(RleList* rlelist, int requested_size);
void rle_append(RleList* rlelist, Rle* rle);
void rle_free(RleList* rlelist);

void string_allocate(String* string, int initial_size);
void string_expand(String* string, int requested_size);
void string_append(String* string1, char string2[]);
void generate_chars(String* string, char character, int amount);
void string_free(String* string);