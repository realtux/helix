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
	frame->local_vars = hash_table_init();
	frame->return_val = init_helix_val();

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

helix_hash_table *hash_table_init(void) {
	helix_hash_table *table = malloc(sizeof(helix_hash_table));
	table->key_count = 0;
	return table;
}

void hash_table_add(char *key, helix_val *val) {
	int key_count = stack[stack_size-1]->local_vars->key_count;
	int s = stack_size - 1;

	++stack[s]->local_vars->key_count;

	// initial element check
	if (key_count == 0) {
		stack[s]->local_vars->keys = malloc(sizeof(char *) * 1);
		stack[s]->local_vars->vals = malloc(sizeof(helix_val *) * 1);
	} else {
		EXPAND_ARRAY_TO(stack[stack_size - 1]->local_vars->keys, char *, key_count + 1)
	}

	stack[s]->local_vars->keys[key_count] = key;
	stack[s]->local_vars->vals[key_count] = val;
}

helix_val *hash_table_get(const char *key) {
	int s = stack_size - 1;
	int key_count = stack[s]->local_vars->key_count;

	int pos = 0;

	infinite {
		if (pos == key_count) return NULL;

		if (strcmp(key, stack[s]->local_vars->keys[pos]) == 0) {
			return stack[s]->local_vars->vals[pos];
		}
	}
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
