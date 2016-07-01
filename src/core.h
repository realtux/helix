#ifndef CORE_H
#define CORE_H

#define BLANG_GENERAL(msg) printf("Blang General: %s\n", msg);
#define BLANG_CORE(msg) \
    printf("Blang Core: %s at line %d\n", msg, line); \
    stack_trace();
#define BLANG_FATAL(msg) \
    printf("Blang Fatal: %s at line %d\n", msg, line); \
    stack_trace();
#define BLANG_PARSE(msg) \
    printf("Blang Parse: %s at line %d\n", msg, line); \
    stack_trace();
#define BLANG_WARNING(msg) \
    printf("Blang Warning: %s at line %d\n", msg, line); \
    stack_trace();

#define true 1
#define false 0

#define infinite for(;;)

const char *re_keywords;

#define BLANG_VAR_INT 1
#define BLANG_VAR_FLOAT 2
#define BLANG_VAR_STRING 3
#define BLANG_VAR_BOOL 4

typedef struct {
    int type;
    union {
        int var_int;
        double var_float;
        char *var_string;
        int var_bool;
        void *something;
    } d;
} blang_var;

typedef struct {
    char *name;
    int char_pos;
    int line_pos;
    union {
        int return_int;
        double return_float;
        char *return_string;
        int return_bool;
        void *something;
    } return_val;
} stack_frame;

#define EXPAND_STRING_BY(var, type, amt) var = realloc(var, sizeof(type) * (strlen(var) + amt + 1));
#define SHRINK_STRING_BY(var, type, amt) var = realloc(var, sizeof(type) * (strlen(var) + 1 - amt));

void stack_push(stack_frame);
void stack_destroy(void);

#endif
