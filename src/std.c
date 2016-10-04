#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "constructs.h"
#include "error.h"
#include "lexer.h"

#include "std/output.h"
#include "std/thread.h"

extern int line;
extern int chr;

extern char *source;

void handle_std(const char *namespace) {
	// eat up the namespace and method name
	while (source[chr] != '(') ++chr;

	// and the lparen
	++chr;

	if (strcmp(namespace, "output::print") == 0) {
		output_print(evaluate_expression());
	} else if (strcmp(namespace, "thread::new") == 0) {
		thread_new(evaluate_expression());
	} else {
		char *suffix = " is not a function";
		char *message = malloc(sizeof(char) * (strlen(namespace) + strlen(suffix) + 1));

		strcpy(message, namespace);
		strcat(message, suffix);

		HELIX_WARNING(message);

		free(message);
	}

	eat_space();

	// rparen
	++chr;

}
