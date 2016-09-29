#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "core.h"

extern int stack_size;
extern stack_frame **stack;

void stack_init(void) {
	stack = malloc(sizeof(stack_frame*) * stack_size);

	// set up the main frame
	stack_frame *frame = malloc(sizeof(stack_frame));
	frame->char_pos = 0;
	frame->line_pos = 1;
	frame->name = malloc(sizeof(char) * 5);
	strcpy(frame->name, "main");
	frame->local_vars = malloc(sizeof(helix_hash_table));
	frame->local_vars->key_count = 0;

	stack[stack_size - 1] = frame;
}

void stack_push(stack_frame *frame) {
	++stack_size;
	stack = realloc(stack, sizeof(stack_frame*) * stack_size);
	stack[stack_size - 1] = frame;
}

helix_val *stack_pop(void) {
	--stack_size;

	helix_val *val = stack[stack_size]->return_val;

	stack[stack_size] = NULL;
	stack = realloc(stack, sizeof(stack_frame*) * stack_size);

	return val == NULL ? NULL : val;
}

void stack_destroy(void) {
	if (stack == NULL) return;

	free(stack);
}

helix_val *init_helix_val(void) {
	helix_val *val = malloc(sizeof(helix_val));
	val->d.val_string = malloc(sizeof(char) * 1);
	val->d.val_string[0] = '\0';

	return val;
}

char *helix_val_as_string(helix_val *val) {
	char *final;

	if (val->type == HELIX_VAL_STRING) {
		// copy into final
		final = malloc(sizeof(char) * (strlen(val->d.val_string) + 1));
		strcpy(final, val->d.val_string);
		return final;
	} else if (val->type == HELIX_VAL_INT) {
		// convert the string from int
		char tmp[100];
		sprintf(tmp, "%d", val->d.val_int);

		// copy into final
		final = malloc(sizeof(char) * (strlen(tmp) + 1));
		strcpy(final, tmp);
		return final;
	}

	return NULL;
}

void free_helix_val(helix_val *val) {
	free(val->d.val_string);
	free(val);
}
