#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "error.h"
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

    int function_len = strlen(function_name);

    // add a new stack frame
    stack_frame *frame = malloc(sizeof(stack_frame));
	frame->char_pos = chr; // fix
	frame->line_pos = line;
	frame->name = malloc(sizeof(char) * (function_len + 1));
	strcpy(frame->name, function_name);
	frame->local_vars = hash_table_init();
	frame->local_fns = hash_table_init_fn();
    frame->has_returned = 0;

    // push to the opening paren
    chr += function_len;

    // and past it
    ++chr;

    // closure arg type
    if (source[chr] == '|') {
        // replace this with closure argument handling
        while (source[chr] != '|') ++chr;

        // eat space from end args to double arrow
        eat_space();

        if (source[chr] != '=' && source[chr + 1] != '>') {
            HELIX_PARSE("Expecting => after closure args");
        }

        // push past double arrow
        chr += 2;

        // eat space from double arrow to function open
        eat_space();

        int fn_s;

        fn_s = chr + 1;

        eat_braced_block();

        helix_val *arg = init_helix_val();
        arg->type = HELIX_VAL_FUNCTION;
        arg->d.val_fn_s = fn_s;

        push_fn_arg(frame, arg);
    }

    // eat space between last arg and end paren
    eat_space();

    // push past paren
    ++chr;

    // eat space between paren and opening brace
    eat_space();

    // push past brace
    ++chr;

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
