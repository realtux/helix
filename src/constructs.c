#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "constructs.h"
#include "error.h"
#include "lexer.h"
#include "std.h"
#include "tpv/slre.h"

extern int line;
extern int chr;

extern char *source;

void con_out(helix_val *string) {
	printf("%s", string->d.val_string);
}

void con_if(void) {

}

void handle_construct(const char *construct) {
	if (strcmp(construct, "out") == 0) {
		chr = chr + 3;
		con_out(evaluate_expression());
	} else if (strcmp(construct, "if") == 0) {
		chr += 2;
		con_if();
	} else {
		HELIX_FATAL("Unknown keyword");
	}
}

helix_val *evaluate_expression(void) {
	int match;
	struct slre_cap cap[1];

	helix_val *value = malloc(sizeof(helix_val));
	value->type = HELIX_VAL_STRING;
	value->d.val_string = malloc(sizeof(char) * 1);
	value->d.val_string[0] = '\0';

	// space
	if (source[chr] == ' ') {
		eat_space();
	}

	// handle string
	if (source[chr] == '\'') {
		++chr;

		infinite {
			// fail on newline
			if (source[chr] == '\n' || source[chr] == '\0') {
				HELIX_PARSE("Unterminated string");
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
					EXPAND_STRING_BY(value->d.val_string, char, 1);
					strcat(value->d.val_string, "\n");
					++chr;
					++chr;
					continue;
				}
			}

			EXPAND_STRING_BY(value->d.val_string, char, 1);

			strncat(value->d.val_string, source + chr, 1);
			++chr;
		}
	}

	// handle integer
	match = slre_match(re_integers, source + chr, 32, cap, 1, 0);

	if (match >= 0) {
		infinite {
			// fail on newline
			if (source[chr] == '\n' || source[chr] == '\0') {
				HELIX_PARSE("Unterminated expression");
			}

			match = slre_match("[^1-9]", source + chr, 32, cap, 1, 0);

			if (match >= 0) break;

			++chr;
		}
	}

	// std call
	match = slre_match(re_std, source + chr, 32, cap, 1, 0);

	if (match >= 0) {
		// get the std name
		char *std = malloc(sizeof(char) * (cap->len + 1));

		// copy it in
		strncpy(std, cap->ptr, cap->len);
		std[cap->len] = '\0';

		handle_std(std);

		free(std);
	}

	eat_space();

	// concatenation?
	if (source[chr] == '.') {
		++chr;

		// get whatever comes next
		helix_val *concatenated = evaluate_expression();

		// combine strings
		size_t length = strlen(concatenated->d.val_string);

		EXPAND_STRING_BY(value->d.val_string, char, length);

		strcat(value->d.val_string, concatenated->d.val_string);

		// discard the concatenated stuff
		free_helix_val(concatenated);
	}

	// newline means missing semi colon
	if (source[chr] == '\n' || source[chr] == '\0') {
		HELIX_PARSE("Unterminated expression");
	}

	return value;
}
