#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "lexer.h"

extern int line;
extern int chr;

extern char *source;

extern int stack_size;
extern stack_frame **stack;

void handle_fn_call(const char *function_name) {
    // retrieve function start pos
    char *pos = hash_table_get_fn(function_name);

    int separator_pos = 0;

    // find the colon separator
    while (pos[separator_pos] != ':') ++separator_pos;

    char *one = malloc(sizeof(char) * (separator_pos + 1));
    char *two = malloc(sizeof(char) * (strlen(pos) - separator_pos));

    strncpy(one, pos, separator_pos);
    one[separator_pos] = '\0';
    strcpy(two, pos + separator_pos + 1);

    // copy to normal ints
    int fn_line = atoi(one);
    int fn_chr = atoi(two);

    free(one);
    free(two);

    // add a new stack frame
    stack_frame *frame = malloc(sizeof(stack_frame));
	frame->char_pos = chr + strlen(function_name) + 3; // fix
	frame->line_pos = line;
	frame->name = malloc(sizeof(char) * (strlen(function_name) + 1));
	strcpy(frame->name, function_name);
	frame->local_vars = hash_table_init();
	frame->local_fns = hash_table_init_fn();
    frame->has_returned = 0;

    stack_push(frame);

    // position cursor
    line = fn_line;
    chr = fn_chr;

    lex();

    if (stack[stack_size - 1]->has_returned == 0) {
        // handle implicit return if return keyword was not encountered
        stack[stack_size-2]->return_val = init_helix_val();

        line = stack[stack_size-1]->line_pos;
        chr = stack[stack_size-1]->char_pos;

        stack_pop();
    }
}
