#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "core.h"
#include "error.h"

extern int line;
extern int chr;

extern char *source;

extern int stack_size;
extern stack_frame **stack;

void stack_init(void) {
	stack = malloc(sizeof(stack_frame *));

	// set up the main frame
	stack_frame *frame = malloc(sizeof(stack_frame));
	frame->char_pos = 0;
	frame->line_pos = 1;
	frame->name = malloc(sizeof(char) * 5);
	strcpy(frame->name, "main");
	frame->local_vars = hash_table_init();
	frame->local_fns = hash_table_init_fn();
	frame->arg_count = 0;
	frame->has_returned = 0;

	stack[0] = frame;
}

void stack_push(stack_frame *frame) {
	++stack_size;

	// check for max stack depth
	if (stack_size > 50) {
		--stack_size;
		HELIX_CORE("Max stack frame count of 50 reached");
	}

	stack = realloc(stack, sizeof(stack_frame *) * stack_size);
	stack[stack_size - 1] = frame;
}

void stack_pop(void) {
	// free the outgoing stack frame
	free(stack[stack_size - 1]);

	// reduce and resize
	--stack_size;
	stack = realloc(stack, sizeof(stack_frame *) * stack_size);
}

void stack_destroy(void) {
	if (stack == NULL) return;

	free(stack);
}

void push_fn_arg(stack_frame *frame, helix_val *arg) {
	if (frame->arg_count == 0) {
		frame->args = malloc(sizeof(helix_val *));
	} else {
		frame->args = realloc(frame->args, sizeof(helix_val *) * (frame->arg_count + 1));
	}

	frame->args[frame->arg_count] = arg;

	++frame->arg_count;
}

helix_hash_table *hash_table_init(void) {
	helix_hash_table *table = calloc(1, sizeof(helix_hash_table));
	table->key_count = 0;
	return table;
}

helix_hash_table_fn *hash_table_init_fn(void) {
	helix_hash_table_fn *table = calloc(1, sizeof(helix_hash_table_fn));
	table->key_count = 0;
	return table;
}

void hash_table_add(char *key, helix_val *val) {
	// check if the variable already exists, in which case it can be replaced
	helix_val **duplicate = hash_table_get_ref(key);

	if (duplicate != NULL) {
		*duplicate = val;
		return;
	}

	int s = stack_size - 1;
	int key_count = stack[s]->local_vars->key_count;

	++stack[s]->local_vars->key_count;

	// initial element check
	if (key_count == 0) {
		stack[s]->local_vars->keys = malloc(sizeof(char *) * 1);
		stack[s]->local_vars->vals = malloc(sizeof(helix_val *) * 1);
	} else {
		EXPAND_ARRAY_TO(stack[s]->local_vars->keys, char *, key_count + 1);
		EXPAND_ARRAY_TO(stack[s]->local_vars->vals, helix_val *, key_count + 1);
	}

	stack[s]->local_vars->keys[key_count] = key;
	stack[s]->local_vars->vals[key_count] = val;
}

void hash_table_add_by_table(stack_frame *frame, char *key, helix_val *val) {
	int key_count = frame->local_vars->key_count;

	++frame->local_vars->key_count;

	// initial element check
	if (key_count == 0) {
		frame->local_vars->keys = malloc(sizeof(char *) * 1);
		frame->local_vars->vals = malloc(sizeof(helix_val *) * 1);
	} else {
		EXPAND_ARRAY_TO(frame->local_vars->keys, char *, key_count + 1);
		EXPAND_ARRAY_TO(frame->local_vars->vals, helix_val *, key_count + 1);
	}

	frame->local_vars->keys[key_count] = key;
	frame->local_vars->vals[key_count] = val;
}

void hash_table_add_fn(char *key, char *val) {
	// check if the variable already exists, in which case it can be replaced
	char **duplicate = hash_table_get_fn_ref(key);

	if (duplicate != NULL) {
		*duplicate = val;
		return;
	}

	int s = stack_size - 1;
	int key_count = stack[s]->local_fns->key_count;

	++stack[s]->local_fns->key_count;

	// initial element check
	if (key_count == 0) {
		stack[s]->local_fns->keys = malloc(sizeof(char *) * 1);
		stack[s]->local_fns->vals = malloc(sizeof(char *) * 1);
	} else {
		EXPAND_ARRAY_TO(stack[s]->local_fns->keys, char *, key_count + 1);
		EXPAND_ARRAY_TO(stack[s]->local_fns->vals, char *, key_count + 1);
	}

	stack[s]->local_fns->keys[key_count] = key;
	stack[s]->local_fns->vals[key_count] = val;
}

helix_val *hash_table_get(const char *key) {
	int s = stack_size - 1;

	// check each frame downward
	while (s >= 0) {
		int key_count = stack[s]->local_vars->key_count;

		int pos = 0;

		infinite {
			if (pos == key_count) break;

			if (strcmp(key, stack[s]->local_vars->keys[pos]) == 0) {
				return stack[s]->local_vars->vals[pos];
			}

			++pos;
		}

		--s;
	}

	return NULL;
}

char *hash_table_get_fn(const char *key) {
	int s = stack_size - 1;

	// check each frame downward
	while (s >= 0) {
		int key_count = stack[s]->local_fns->key_count;

		int pos = 0;

		infinite {
			if (pos == key_count) break;

			if (strcmp(key, stack[s]->local_fns->keys[pos]) == 0) {
				return stack[s]->local_fns->vals[pos];
			}

			++pos;
		}

		--s;
	}

	return NULL;
}

helix_val **hash_table_get_ref(const char *key) {
	int s = stack_size - 1;

	// check each frame downward
	while (s >= 0) {
		int key_count = stack[s]->local_vars->key_count;
		int pos = 0;

		infinite {
			if (pos == key_count) return NULL;
			if (strcmp(key, stack[s]->local_vars->keys[pos]) == 0) {
				return &stack[s]->local_vars->vals[pos];
			}

			++pos;
		}

		--s;
	}

	return NULL;
}

char **hash_table_get_fn_ref(const char *key) {
	int s = stack_size - 1;

	// check each frame downward
	while (s >= 0) {
		int key_count = stack[s]->local_fns->key_count;
		int pos = 0;

		infinite {
			if (pos == key_count) return NULL;
			if (strcmp(key, stack[s]->local_fns->keys[pos]) == 0) {
				return &stack[s]->local_fns->vals[pos];
			}

			++pos;
		}

		--s;
	}

	return NULL;
}

helix_val *init_helix_val(void) {
	helix_val *val = malloc(sizeof(helix_val));

	if (val == NULL) {
		HELIX_FATAL("Failed to allocate memory for core value");
	}

	val->type = HELIX_VAL_NONE;
	val->d.val_string = calloc(1, sizeof(char));
	val->d.val_string[0] = '\0';

	return val;
}

void helix_val_set_type(helix_val *val, int new_type) {
	// check if string needs free
	if (val->type == HELIX_VAL_STRING && new_type != HELIX_VAL_STRING) {
		free(val->d.val_string);
	}

	val->type = new_type;
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
		sprintf(tmp, "%lld", val->d.val_int);

		// copy into final
		final = malloc(sizeof(char) * (strlen(tmp) + 1));
		strcpy(final, tmp);
		return final;
	}

	return NULL;
}

int helix_val_is_true(helix_val *val) {
	int result;

	if (val == NULL || val->type == HELIX_VAL_NONE) return 0;

	if (val->type != HELIX_VAL_BOOL) {
		if (val->type == HELIX_VAL_STRING) {
			result = strcmp(val->d.val_string, "") == 0 ? 0 : 1;
		} else if (val->type == HELIX_VAL_INT) {
			result = val->d.val_int == 0 ? 0 : 1;
		} else if (val->type == HELIX_VAL_FLOAT) {
			result = fabs(val->d.val_float - 0) < 0 ? 0 : 1;
		}
	} else {
		result = val->d.val_bool;
	}

	return result;
}

void free_helix_val(helix_val *val) {
	if (val->type == HELIX_VAL_STRING) free(val->d.val_string);
	free(val);
}
