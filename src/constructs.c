#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "constructs.h"
#include "error.h"
#include "lexer.h"
#include "regex.h"
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

	// handle inverse operator
	//int inverse = source[chr] == '!' ? 1 : 0;

	//if (inverse) ++chr;

	// get the final value
	helix_val *val = evaluate_expression();

	// eat space between the left hand value and operator/brace
	eat_space();

	int result = helix_val_is_true(val);

	// handle inverse
	//if (inverse) {
	//	result = result == 0 ? 1 : 0;
	//}

	if (result) {
		// lex the true block
		++chr;
		lex();

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
	eat_space();

	int match;
	struct slre_cap cap[1];

	match = slre_match("^([A-Za-z0-9_]+)\\s*?\\(", source + chr, 32, cap, 1, 0);

	if (match >= 0) {
		// get the function name
		char *function_name = malloc(sizeof(char) * (cap->len + 1));

		// copy it in
		strncpy(function_name, cap->ptr, cap->len);
		function_name[cap->len] = '\0';

		int function_start;

		chr += strlen(function_name);

		// eat space between function name and paren
		eat_space();

		if (source[chr] == '(') {
			// optionally handle arguments
			++chr;++chr;
		}

		// eat space between args and lcurly
		eat_space();

		// push past lcurly
		++chr;

		function_start = chr;

		eat_braced_block();


	}
}

void con_var(void) {
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

	eat_space();
}

void con_while(void) {
	if (source[chr] != ' ') {
		HELIX_PARSE("Expected space after 'which'");
	}

	eat_space();

	int condition_chr_start = chr;
	int condition_line_start = line;

	// get the condition value
	helix_val *val = evaluate_expression();

	int result = helix_val_is_true(val);

	int block_chr_start = chr;

	while (result) {
		// push into the expression
		++chr;

		// lex the block
		lex();

		// reset the cursor to the beginning of the condition
		chr = condition_chr_start;
		line = condition_line_start;

		// ascertain the new condition value
		free_helix_val(val);
		val = evaluate_expression();
		result = helix_val_is_true(val);
	}

	chr = block_chr_start;
	eat_braced_block();
}

void handle_construct(const char *construct) {
	if (strcmp(construct, "out") == 0) {
		chr += 3;
		con_out();
	} else if (strcmp(construct, "if") == 0) {
		chr += 2;
		con_if();
	} else if (strcmp(construct, "fn") == 0) {
		chr += 2;
		con_fn();
	} else if (strcmp(construct, "while") == 0) {
		chr += 5;
		con_while();
	} else {
		HELIX_FATAL("Unknown keyword");
	}
}

helix_val *evaluate_expression(void) {
	int num_matches = 0;
	int match_len;

	int match;//remove
	struct slre_cap cap[1];//remove

	helix_val *lh_value = init_helix_val();

	// space
	if (source[chr] == ' ') {
		eat_space();
	}

	// handle string
	if (source[chr] == '\'') {
		helix_val_set_type(lh_value, HELIX_VAL_STRING);

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
					EXPAND_STRING_BY(lh_value->d.val_string, char, 1);
					strcat(lh_value->d.val_string, "\n");
					++chr;
					++chr;
					continue;
				}
			}

			EXPAND_STRING_BY(lh_value->d.val_string, char, 1);

			strncat(lh_value->d.val_string, source + chr, 1);
			++chr;
		}
	}

	// handle integer
	match = slre_match(LEXER_RE_INTEGERS, source + chr, 32, cap, 1, 0);

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
    char **matches = pcre_match(LEXER_RE_STD, source + chr, &num_matches);

	if (num_matches > 0) {
		match_len = strlen(matches[1]);

		// get the std name
		char *std = malloc(sizeof(char) * (match_len + 1));

		// copy it in
		strncpy(std, matches[1], match_len);
		std[match_len] = '\0';

		handle_std(std);

		free(std);
		free_pcre_matches(matches, num_matches);
	}

	// variable
	match = slre_match(LEXER_RE_VARIABLES, source + chr, 32, cap, 1, 0);

	if (match >= 0) {
		// get the variable name
		char *variable_name = malloc(sizeof(char) * (cap->len + 1));

		// copy it in
		strncpy(variable_name, cap->ptr, cap->len);
		variable_name[cap->len] = '\0';

		helix_val *lookup_val = hash_table_get(variable_name);

		if (lookup_val == NULL) {
			char error[1000];
			sprintf(error, "Variable '%s' has not been defined", variable_name);
			HELIX_FATAL(error);
		}

		helix_val_set_type(lh_value, lookup_val->type);

		if (lookup_val->type == HELIX_VAL_STRING) {
			lh_value->d.val_string =
				realloc(lh_value->d.val_string, sizeof(char) * (strlen(lh_value->d.val_string) + 1));
			strcpy(lh_value->d.val_string, lookup_val->d.val_string);
		} else if (lookup_val->type == HELIX_VAL_INT) {
			lh_value->d.val_int = lookup_val->d.val_int;
		} else if (lookup_val->type == HELIX_VAL_FLOAT) {
			lh_value->d.val_float = lookup_val->d.val_float;
		} else if (lookup_val->type == HELIX_VAL_BOOL) {
			lh_value->d.val_bool = lookup_val->d.val_bool;
		}

		// push past variable name
		chr += strlen(variable_name);
		eat_space();

		free(variable_name);
	}

	// concatenation?
	eat_space();
	if (source[chr] == '.') {
		++chr;

		// get whatever comes next
		helix_val *concatenated = evaluate_expression();

		// combine strings
		size_t length = strlen(concatenated->d.val_string);

		EXPAND_STRING_BY(lh_value->d.val_string, char, length);

		strcat(lh_value->d.val_string, concatenated->d.val_string);

		// discard the concatenated stuff
		free_helix_val(concatenated);
	}

	// comparison
	eat_space();
	int operator = 0;

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
	}

	if (operator != 0) {
		int result;

		// a comparison means the result is automatically a bool
		eat_space();

		helix_val *rh_value = evaluate_expression();

		char *tmp_lh_str;
		char *tmp_rh_str;

		if (operator == TOKEN_OPERATOR_SEQ) {
			if (lh_value->type != rh_value->type) {
				result = 0;
			} else {
				result = 0;
			}
		} else if (operator == TOKEN_OPERATOR_EQ) {
			tmp_lh_str = helix_val_as_string(lh_value);
			tmp_rh_str = helix_val_as_string(rh_value);

			result = strcmp(tmp_lh_str, tmp_rh_str) == 0 ? 1 : 0;

			free(tmp_lh_str);
			free(tmp_rh_str);
		} else if (operator == TOKEN_OPERATOR_SNEQ) {
			if (lh_value->type != rh_value->type) {
				result = 0;
			} else {
				result = 0;
			}
		} else if (operator == TOKEN_OPERATOR_NEQ) {
			tmp_lh_str = helix_val_as_string(lh_value);
			tmp_rh_str = helix_val_as_string(rh_value);

			result = strcmp(tmp_lh_str, tmp_rh_str) == 0 ? 0 : 1;

			free(tmp_lh_str);
			free(tmp_rh_str);
		}

		free_helix_val(rh_value);

		helix_val_set_type(lh_value, HELIX_VAL_BOOL);
		lh_value->d.val_bool = result;
	}

	return lh_value;
}
