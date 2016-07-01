#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "error.h"
#include "lexer.h"
#include "std/constructs.h"
#include "std/io.h"

extern int line;
extern int chr;

extern char *source;

void handle_construct(const char *construct) {
	if (strcmp(construct, "out") == 0) {
		chr = chr + 3;
		out(evaluate_expression());
	}
}

blang_var *evaluate_expression(void) {
	blang_var *variable = malloc(sizeof(blang_var));
	variable->type = BLANG_VAR_STRING;
	variable->d.var_string = malloc(sizeof(char) * 1);
	variable->d.var_string[0] = '\0';

	// space
	if (source[chr] == ' ') {
		eat_space();
	}

	// string
	if (source[chr] == '\'') {
		++chr;

		infinite {
			// fail on newline
			if (source[chr] == '\n' || source[chr] == '\0') {
				BLANG_PARSE("Unterminated string");
			}

			// end string, accounting for escape
			if (source[chr] == '\'' && source[chr - 1] != '\\') {
				++chr;
				break;
			}

			// replace newline
			if (source[chr] == '\\') {
				// peek forward
				if (source[chr + 1] == 'n') {
					EXPAND_STRING_BY(variable->d.var_string, char, 1);

					strcat(variable->d.var_string, "\n");
					++chr;
					++chr;
					continue;
				}
			}

			EXPAND_STRING_BY(variable->d.var_string, char, 1);

			strncat(variable->d.var_string, source + chr, 1);
			++chr;
		}
	}

	eat_space();

	// concatenation?
	if (source[chr] == '.') {
		++chr;
		blang_var *concatenated = evaluate_expression();

		// combine strings
		size_t length = strlen(concatenated->d.var_string);
		EXPAND_STRING_BY(variable->d.var_string, char, length);

		strcat(variable->d.var_string, concatenated->d.var_string);
	}

	// newline means missing semi colon
	if (source[chr] == '\n' || source[chr] == '\0') {
		BLANG_PARSE("Unterminated expression");
	}

	return variable;
}
