#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "constructs.h"
#include "error.h"
#include "lexer.h"
#include "regex.h"

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
	frame->name = malloc(sizeof(char) * (function_len + 1));
	strcpy(frame->name, function_name);
	frame->local_vars = hash_table_init();
	frame->local_fns = hash_table_init_fn();
    frame->arg_count = 0;
    frame->has_returned = 0;

    // push to the opening paren
    chr += function_len;

    // and past it
    ++chr;

    while (source[chr] != ')') {
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

            // eat space between end val and comma or val and paren
            eat_space();

            // comma means new val, start over
            if (source[chr] == ',') {
                ++chr;
                eat_space();
                continue;
            }
        }

        // normal eval
        helix_val *arg = evaluate_expression();

        push_fn_arg(frame, arg);

        // eat space between end val and comma or val and paren
        eat_space();

        // comma means new val, start over
        if (source[chr] == ',') {
            ++chr;
            eat_space();
            continue;
        }
    }

    // push past paren
    ++chr;

    // store the line and chr of the place to return to after stack pop
    frame->char_pos = chr;
	frame->line_pos = line;

    // position cursor
    line = fn_line;
    chr = fn_chr;

    // rewind to args
    while (source[chr] != '(') --chr;
    ++chr;

    eat_space();

    // check for args
    if (source[chr] != ')') {
        int processed_args = 0;
        int num_matches = 0;
        int match_len;
        char **matches;

        infinite {
            matches = pcre_match(LEXER_RE_VARIABLES, source + chr, &num_matches);
            if (num_matches > 0) {
                match_len = strlen(matches[1]);

                // get the arg name
                char *arg_name = malloc(sizeof(char) * (match_len + 1));

                // copy it in
                strcpy(arg_name, matches[1]);
                arg_name[match_len] = '\0';

                free_pcre_matches(matches, num_matches);

                chr += match_len;

                eat_space();

                ++processed_args;

                // test if this value should go to local vars
                if (processed_args <= frame->arg_count) {
                    hash_table_add_by_table(frame, arg_name, frame->args[processed_args-1]);
                }

                if (source[chr] == ',') {
                    ++chr;
                    eat_space();
                    continue;
                }
            }

            if (source[chr] == ')') {
                break;
            } else {
                ++chr;
            }
        }
    }

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
