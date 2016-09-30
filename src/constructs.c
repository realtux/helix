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

extern int stack_size;
extern stack_frame **stack;

void con_out(void) {
	helix_val *val = evaluate_expression();

	if (val->type == HELIX_VAL_STRING) {
		printf("%s", val->d.val_string);
	} else if (val->type == HELIX_VAL_INT) {
		printf("%d", val->d.val_int);
	} else if (val->type == HELIX_VAL_FLOAT) {
		printf("%f", val->d.val_float);
	} else if (val->type == HELIX_VAL_BOOL) {
		printf("%s", val->d.val_bool == 0 ? "false" : "true");
	}
}

void con_if(void) {
	eat_space();

	helix_val *val_lh = evaluate_expression();

	eat_space();

	int operator;

	if (strncmp(source + chr, "===", 3) == 0) {
		operator = TOKEN_OPERATOR_SEQ;
		chr += 3;
	} else if (strncmp(source + chr, "==", 2) == 0) {
		operator = TOKEN_OPERATOR_EQ;
		chr += 2;
	} else if (strncmp(source + chr, "!==", 3) == 0) {
		operator = TOKEN_OPERATOR_SNEQ;
		chr += 3;
	} else if (strncmp(source + chr, "!=", 2) == 0) {
		operator = TOKEN_OPERATOR_NEQ;
		chr += 2;
	} else {
		HELIX_FATAL("Strange operator found");
	}

	eat_space();

	helix_val *val_rh = evaluate_expression();

	eat_space();

	if (source[chr] != '{') {
		HELIX_PARSE("Expected { after conditional");
	}

	int result;

	char *tmp_lh_str;
	char *tmp_rh_str;

	if (operator == TOKEN_OPERATOR_SEQ) {
		if (val_lh->type != val_rh->type) result = 0;
	} else if (operator == TOKEN_OPERATOR_EQ) {
		tmp_lh_str = helix_val_as_string(val_lh);
		tmp_rh_str = helix_val_as_string(val_rh);

		result = strcmp(tmp_lh_str, tmp_rh_str) == 0 ? 1 : 0;
	} else if (operator == TOKEN_OPERATOR_SNEQ) {
		if (val_lh->type != val_rh->type) result = 0;
	} else if (operator == TOKEN_OPERATOR_NEQ) {
		tmp_lh_str = helix_val_as_string(val_lh);
		tmp_rh_str = helix_val_as_string(val_rh);

		result = strcmp(tmp_lh_str, tmp_rh_str) == 0 ? 0 : 1;
	}

	free(tmp_lh_str);
	free(tmp_rh_str);

	if (result) {
		// lex the true block
		++chr;
		lex();

		// push past the rcurly
		++chr;

		// eat space between rcurly and possible else/elseif
		eat_space();

		if (strncmp(source + chr, "else", 4) == 0) {
			chr += 4;
			eat_braced_block();
		}
	} else {
		// eat the true block
		eat_braced_block();

		// eat space between rcurly and possible else/elseif
		eat_space();

		if (strncmp(source + chr, "else", 4) == 0) {
			chr += 4;

			eat_space();

			if (source[chr] != '{') {
				HELIX_PARSE("Expected { after conditional");
			}

			// lex the false block
			++chr;
			lex();
		}
	}
}

void con_fn(void) {

}

void con_var(void) {
	if (source[chr] != ' ') {
		HELIX_PARSE("Expected space after 'var'");
	}

	eat_space();

	char *var = malloc(1);
	var[0] = '\0';

	infinite {
		// fail on newline or endline
		if (source[chr] == '\n' || source[chr] == '\0') {
			HELIX_PARSE("Unterminated expression");
		}

		if (source[chr] == ' ' || source[chr] == '=') {
			eat_space();
			break;
		}

		EXPAND_STRING_BY(var, char, 1);
		strncat(var, source + chr, 1);
		++chr;
	}

	if (source[chr] != '=') {
		HELIX_PARSE("Expecting '=' after variable name");
	}

	++chr;

	eat_space();

	helix_val *val = evaluate_expression();

	hash_table_add(var, val);
}

void handle_construct(const char *construct) {
	if (strcmp(construct, "out") == 0) {
		chr = chr + 3;
		con_out();
	} else if (strcmp(construct, "if") == 0) {
		chr += 2;
		con_if();
	} else if (strcmp(construct, "fn") == 0) {
		chr += 2;
		con_fn();
	} else if (strcmp(construct, "var") == 0) {
		chr += 3;
		con_var();
	} else {
		HELIX_FATAL("Unknown keyword");
	}
}

helix_val *evaluate_expression(void) {
	int match;
	struct slre_cap cap[1];

	helix_val *val = init_helix_val();

	// space
	if (source[chr] == ' ') {
		eat_space();
	}

	// handle string
	if (source[chr] == '\'') {
		val->type = HELIX_VAL_STRING;

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
					EXPAND_STRING_BY(val->d.val_string, char, 1);
					strcat(val->d.val_string, "\n");
					++chr;
					++chr;
					continue;
				}
			}

			EXPAND_STRING_BY(val->d.val_string, char, 1);

			strncat(val->d.val_string, source + chr, 1);
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

		EXPAND_STRING_BY(val->d.val_string, char, length);

		strcat(val->d.val_string, concatenated->d.val_string);

		// discard the concatenated stuff
		free_helix_val(concatenated);
	}

	// newline means missing semi colon
	if (source[chr] == '\n' || source[chr] == '\0') {
		HELIX_PARSE("Unterminated expression");
	}

	return val;
}
