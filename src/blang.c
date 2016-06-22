#include <stdio.h>
#include <stdlib.h>

#include "core.h"
#include "lexer.h"

int line_number = 1;
int chr = 0;

char *source;

int main(int argc, char **argv) {
    if (argc < 2) {
        AXIS_GENERAL("No input file found");
        return EXIT_FAILURE;
    }

    // get the file
    FILE *file = fopen(argv[1], "r");

    if (file == NULL) {
        AXIS_GENERAL("Couldn't open input file for reading");
        return EXIT_FAILURE;
    }

    long file_size;

    // get the source length
    fseek(file, 0L, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    // store the source
    source = malloc(file_size + 1);

    if (source == NULL) {
        AXIS_CORE("Allocation failed for source file");
        return EXIT_FAILURE;
    }

    // transfer file into source and close
    fread(source, file_size, 1, file);
    fclose(file);

    lex();

    // execution details
    printf("\n----\n");
    printf("Read %d lines", line_number);

    // clean up
    free(source);

    // always
    printf("\n");
    return EXIT_SUCCESS;
}
