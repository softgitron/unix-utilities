# unix-utilies

## What is inside this repo?
This repo consist of files related to operating systems & system programming course. There is 4 different kind of programs in this repo my-cat, my-grep, my-zip and my-unzip.

## Compilation
All programs are written in C and may be compiled with command `make program-name`

## My-cat
My-cat is simple implemention of the unix cat program. It can print content of the file to the commandline. Program may be tested by giving it files as an argument for example `./my-cat file1 file2`.

## My-grep
My-grep implements basic functionality of unix grep program. It can return lines that matches specific string query. Usage as follow `./my-grep search-term file`

## My-zip
My-zip implements RLE (run length encoding) efficiently for abitary large files. Usage as follow `./my-zip file1 file2 > output`.

### Working principles of my-zip
My zip works by first mapping input file to the memory and then going trough it in a paged manner. Page size is defined by fixed memory persentage that is defined in my-zip-lib. When the page is handeled it will be printed to stdout in a 5-byte format where first 4 bytes are allocated for length and last byte is allocated for compressed character. 

## My-unzip
My-unzip does reverse operation for my-zip. It converst compressed data back to text. Usage as follow `./my-unzip file1 file2 > output`

## Ostep-projects
All programs in this repository are implementations of the ostep-projects initial-utilities. Full program definations maybe found [here] (https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/initial-utilities)