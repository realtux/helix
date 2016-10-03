#ifndef CONSTRUCTS_H
#define CONSTRUCTS_H

void con_out(void);
void con_if(void);
void con_fn(void);
void con_var(void);
void con_while(void);
void con_return(void);
void handle_construct(const char *);
helix_val *evaluate_expression(void);

#endif
