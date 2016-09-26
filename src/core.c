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

void free_blang_val(blang_val *var) {
	if (var->type == BLANG_VAL_STRING) {
		free(var->d.val_string);
	}

	free(var);
}
