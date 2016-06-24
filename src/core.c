#include <stdlib.h>

#include "core.h"

extern int stack_size;
extern stack_frame *stack;

void stack_push(stack_frame frame) {
	++stack_size;
	stack = realloc(stack, sizeof(stack_frame) * stack_size);
	stack[stack_size - 1] = frame;
}

void stack_destroy(void) {
	if (stack == NULL) return;

	free(stack);
}