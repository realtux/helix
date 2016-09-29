#include <stdio.h>
#include <stdlib.h>

#include "core.h"

extern int line;

extern int stack_size;
extern stack_frame **stack;

void stack_trace(void) {
	printf("----\nStack Trace:\n");

	if (stack_size > 1) {
		int i;
		for (i = stack_size-1; i > 0; --i) {
			if (i == stack_size-1) {
				printf("  -> err:%d\n", line);
				printf("  -> %s():%d\n", stack[i]->name, stack[i]->line_pos);
			} else {
				printf("  -> %s():%d\n", stack[i]->name, stack[i]->line_pos);
			}
		}
	} else {
		printf("  -> err:%d\n", line);
	}

	exit(EXIT_FAILURE);
}
