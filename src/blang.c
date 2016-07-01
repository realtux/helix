#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "blang.h"
#include "core.h"
#include "lexer.h"

#include "std/error.h"

char *source;

int line = 1;
int chr = 0;

int stack_size = 0;
stack_frame *stack;

int env_debug = false;

int main(int argc, char **argv) {
    // setup env
    const char *env_debug_value = getenv("DEBUG");

    // load up header declarations
    re_keywords = "^(out)";

    // timing
    clock_t begin;
    clock_t end;
    if (env_debug) {
        begin = clock();
    }

    if (env_debug_value != NULL && strcmp(getenv("DEBUG"), "1") == 0) {
        env_debug = true;
    }

    if (argc < 2) {
        BLANG_GENERAL("No input file found");
        return EXIT_FAILURE;
    }

    // get the file
    FILE *file = fopen(argv[1], "r");

    if (file == NULL) {
        BLANG_GENERAL("Couldn't open input file for reading");
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
        BLANG_CORE("Allocation failed for source file");
        return EXIT_FAILURE;
    }

    // transfer file into source and close
    fread(source, file_size, 1, file);
    fclose(file);

    lex();

    // clean up
    stack_destroy();
    free(source);

    // final debugging output, if any
    if (env_debug) {
        end = clock();

        // execution details
        printf("\n---->\n");
        printf("Lines Read: %d\n", line);
        printf("Execution Time: %0.4fs\n", (double) (end - begin) / CLOCKS_PER_SEC);
        printf("---->\n");
    }

    // always
    printf("\n");
    return EXIT_SUCCESS;
}
