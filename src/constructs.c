#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "error.h"
#include "lexer.h"
#include "constructs.h"

extern int line;
extern int chr;

extern char *source;

void out(blang_val *string) {
	printf("%s", string->d.val_string);
}

void handle_construct(const char *construct) {
	if (strcmp(construct, "out") == 0) {
		chr = chr + 3;
		out(evaluate_expression());
	}
}

blang_val *evaluate_expression(void) {
	blang_val *variable = malloc(sizeof(blang_val));
	variable->type = BLANG_VAL_STRING;
	variable->d.val_string = malloc(sizeof(char) * 1);
	variable->d.val_string[0] = '\0';

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
					EXPAND_STRING_BY(variable->d.val_string, char, 1);

					strcat(variable->d.val_string, "\n");
					++chr;
					++chr;
					continue;
				}
			}

			EXPAND_STRING_BY(variable->d.val_string, char, 1);

			strncat(variable->d.val_string, source + chr, 1);
			++chr;
		}
	}

	eat_space();

	// concatenation?
	if (source[chr] == '.') {
		++chr;

		// get whatever comes next
		blang_val *concatenated = evaluate_expression();

		// combine strings
		size_t length = strlen(concatenated->d.val_string);
		EXPAND_STRING_BY(variable->d.val_string, char, length);

		strcat(variable->d.val_string, concatenated->d.val_string);

		// discard the concatenated stuff
		free_blang_val(concatenated);
	}

	// newline means missing semi colon
	if (source[chr] == '\n' || source[chr] == '\0') {
		BLANG_PARSE("Unterminated expression");
	}

	return variable;
}
