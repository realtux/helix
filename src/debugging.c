#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"

extern int stack_size;
extern stack_frame **stack;

void dump_full_stack(void) {
    for (int i = stack_size - 1; i >= 0; --i) {
        printf("* STACK FRAME %d\n", i);
        printf("****************************************\n");
        printf("* fn name: %s\n", stack[i]->name);
        printf("* char pos: %d\n", stack[i]->char_pos);
        printf("* line pos: %d\n", stack[i]->line_pos);
        printf("* arg count: %d\n", stack[i]->arg_count);
        printf("* has returned: %d\n", stack[i]->has_returned);
        printf("* local vars count: %d\n", stack[i]->local_vars->key_count);
        for (int x = stack[i]->local_vars->key_count - 1; x >= 0; --x) {
            printf("*   %s: %s\n",
                stack[i]->local_vars->keys[x],
                stack[i]->local_vars->vals[x]->d.val_string);
        }
        printf("* local fns count: %d\n", stack[i]->local_fns->key_count);
        for (int x = stack[i]->local_fns->key_count - 1; x >= 0; --x) {
            printf("*   %s: %s\n",
                stack[i]->local_fns->keys[x],
                stack[i]->local_fns->vals[x]);
        }
        printf("\n");
    }
}
