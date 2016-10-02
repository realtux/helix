#ifndef CORE_H
#define CORE_H

#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#define true 1
#define false 0

#define infinite for(;;)

#define LEXER_RE_KEYWORDS "^(out|if|fn|while|for)\\s"
#define LEXER_RE_ASSIGNMENT "^([a-zA-Z0-9_]+)\\s*?="
#define LEXER_RE_INTEGERS "^([0-9]+)"
#define LEXER_RE_STD "^([a-zA-Z_0-9]+::[a-zA-Z_0-9]+)\\s*?\\("
#define LEXER_RE_FN "^([a-zA-Z_0-9]+)\\s*?\\("
#define LEXER_RE_VARIABLES "^([a-zA-Z0-9_]+)"

#define HELIX_VAL_INT 1
#define HELIX_VAL_FLOAT 2
#define HELIX_VAL_STRING 3
#define HELIX_VAL_BOOL 4
#define HELIX_VAL_DOUBLE 4

#define TOKEN_OPERATOR_SEQ 1 // ===
#define TOKEN_OPERATOR_EQ 2 // ==
#define TOKEN_OPERATOR_NEQ 3 // !=
#define TOKEN_OPERATOR_SNEQ 4 // !==
#define TOKEN_OPERATOR_LTE 5 // <=
#define TOKEN_OPERATOR_GTE 6 // >=
#define TOKEN_OPERATOR_LT 7 // >=
#define TOKEN_OPERATOR_GT 8 // >=

typedef struct {
    int type;
    union {
        int val_int;
        double val_double;
        float val_float;
        char *val_string;
        int val_bool;
        void *something;
    } d;
} helix_val;

typedef struct {
    int key_count;
    char **keys;
    helix_val **vals;
} helix_hash_table;

typedef struct {
    char *name;
    int char_pos;
    int line_pos;
    helix_hash_table *local_vars;
    helix_val *return_val;
} stack_frame;

#define EXPAND_STRING_BY(var, type, amt) var = realloc(var, sizeof(type) * (strlen(var) + amt + 1));
#define SHRINK_STRING_BY(var, type, amt) var = realloc(var, sizeof(type) * (strlen(var) + 1 - amt));
#define EXPAND_ARRAY_TO(var, type, amt) var = realloc(var, sizeof(type) * amt);
#define SHRINK_ARRAY_TO(var, type, amt) var = realloc(var, sizeof(type) * amt);

void stack_push(stack_frame *);
helix_val *stack_pop(void);
void stack_init(void);
void stack_destroy(void);
helix_hash_table *hash_table_init(void);
void hash_table_add(char*, helix_val *);
helix_val *hash_table_get(const char *);
helix_val **hash_table_get_ref(const char *);
helix_val *init_helix_val(void);
void helix_val_set_type(helix_val *, int);
char *helix_val_as_string(helix_val *);
int helix_val_is_true(helix_val *);
void free_helix_val(helix_val *);

#endif
