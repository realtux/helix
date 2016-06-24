#include <stdio.h>

#include "core.h"

extern int line;

extern int stack_size;
extern stack_frame *stack;

void stack_trace(void) {
	printf("----\nStack Trace:\n");

	if (stack_size > 0) {
		int i;
		for (i = stack_size; i >= 0; --i) {
			if (i == stack_size) {
				printf("  -> %s():%d\n", stack[i - 1].name, line);
			} else if (i == 0) {
				printf("  -> main:%d\n", stack[i - 1].line_pos);
			} else {
				printf("  -> %s():%d\n", stack[i - 1].name, stack[i - 1].line_pos);
			}
		}
	} else {
		printf("  -> main:%d\n", line);
	}
}
