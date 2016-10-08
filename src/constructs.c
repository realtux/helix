#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "constructs.h"
#include "error.h"
#include "functions.h"
#include "lexer.h"
#include "regex.h"
#include "std.h"

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
		printf("%lld", val->d.val_int);
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

void con_fn_def(void) {
	int num_matches;
	int match_len;

	eat_space();

	char **matches = pcre_match(LEXER_RE_FN, source + chr, &num_matches);

	if (num_matches > 0) {
		match_len = strlen(matches[1]);

		// get the function name
		char *function_name = malloc(sizeof(char) * (match_len + 1));

		// copy it in
		strcpy(function_name, matches[1]);
		function_name[match_len] = '\0';

		free_pcre_matches(matches, num_matches);

		int function_line;
		int function_start;

		chr += strlen(function_name);

		// eat space between function name and paren
		eat_space();

		if (source[chr] == '(') {
			// push past arguments
			++chr;
			while (source[chr] != ')') ++chr;
			++chr;
		}

		// eat space between args and lcurly
		eat_space();

		if (source[chr] != '{') {
			HELIX_PARSE("Expecting '{' after function signature");
		}

		function_line = line;
		function_start = chr + 1;

		// push past the encloding function block
		eat_braced_block();

		char *line_pos = malloc(sizeof(char) * 64);
		line_pos[0] = '\0';

		sprintf(line_pos, "%d:%d", function_line, function_start);

		line_pos = realloc(line_pos, sizeof(char) * (strlen(line_pos) + 1));

		// store the function name and start pos
		hash_table_add_fn(function_name, line_pos);
	}
}

void con_var(void) {
	eat_space();

	char *var = malloc(sizeof(char) * 1);
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
		HELIX_PARSE("Expected space after 'while'");
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

void con_return(void) {
	eat_space();

	// get the return value
	helix_val *val = evaluate_expression();

	// add it to the stack frame beneath current
	stack[stack_size - 2]->return_val = val;
	stack[stack_size - 2]->has_returned = 1;

	// reset cursor
	line = stack[stack_size - 1]->line_pos;
	chr = stack[stack_size - 1]->char_pos;

	stack_pop();
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
		con_fn_def();
	} else if (strcmp(construct, "while") == 0) {
		chr += 5;
		con_while();
	} else if (strcmp(construct, "return") == 0) {
		chr += 6;
		con_return();
	} else {
		HELIX_FATAL("Unknown keyword");
	}
}

helix_val *evaluate_expression(void) {
	int num_matches = 0;
	int match_len;
	char **matches;

	helix_val *lh_value = init_helix_val();

	// space
	if (source[chr] == ' ') {
		eat_space();
	}

	// newline
	if (source[chr] == '\n') {
		++chr;
		++line;
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
	matches = pcre_match(LEXER_RE_INTEGERS, source + chr, &num_matches);
	if (num_matches > 0) {
		helix_val_set_type(lh_value, HELIX_VAL_INT);

		long long integer = atoi(matches[1]);
		lh_value->d.val_int = integer;

		chr += strlen(matches[1]);

		free_pcre_matches(matches, num_matches);
	}

	// std call
    matches = pcre_match(LEXER_RE_STD, source + chr, &num_matches);
	if (num_matches > 0) {
		match_len = strlen(matches[1]);

		// get the std name
		char *std = malloc(sizeof(char) * (match_len + 1));

		// copy it in
		strncpy(std, matches[1], match_len);
		std[match_len] = '\0';

		free_pcre_matches(matches, num_matches);
		handle_std(std);

		free(std);
	}

	// user function call
	matches = pcre_match(LEXER_RE_FN, source + chr, &num_matches);
	if (num_matches > 0) {
		match_len = strlen(matches[1]);

		// get the fn name
		char *fn = malloc(sizeof(char) * (match_len + 1));

		// copy it in
		strncpy(fn, matches[1], match_len);
		fn[match_len] = '\0';

		handle_fn_call(fn);

		// pass off return value
		lh_value = stack[stack_size - 1]->return_val;

		free(fn);
		free_pcre_matches(matches, num_matches);
	}

	// variable
	matches = pcre_match(LEXER_RE_VARIABLES, source + chr, &num_matches);
	if (num_matches > 0) {
		match_len = strlen(matches[1]);

		// get the variable name
		char *variable_name = malloc(sizeof(char) * (match_len + 1));

		// copy it in
		strncpy(variable_name, matches[1], match_len);
		variable_name[match_len] = '\0';

		helix_val *lookup_val = hash_table_get(variable_name);

		if (lookup_val == NULL) {
			HELIX_FATAL("Undefined variable");
		}

		helix_val_set_type(lh_value, lookup_val->type);

		if (lookup_val->type == HELIX_VAL_STRING) {
			lh_value->d.val_string =
				realloc(lh_value->d.val_string, sizeof(char) * (strlen(lookup_val->d.val_string) + 1));
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

	// closure arg type
	if (source[chr] == '|') {
		// push past start pipe
		++chr;

		// replace this with closure argument handling
		while (source[chr] != '|') ++chr;

		// push past end pipe
		++chr;

		// eat space from end args to double arrow
        eat_space();

        if (source[chr] != '=' && source[chr + 1] != '>') {
            HELIX_PARSE("Expecting => after closure args");
        }

        // push past double arrow
        chr += 2;

        // eat space from double arrow to function open
        eat_space();

		int fn_s;

		fn_s = chr;

		eat_braced_block();

		lh_value->type = HELIX_VAL_FUNCTION;
		lh_value->d.val_fn_s = fn_s;
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

	// addition
	eat_space();
	if (source[chr] == '+') {
		++chr;

		// get whatever comes next
		helix_val *added_val = evaluate_expression();

		if (added_val->type == HELIX_VAL_INT) {
			lh_value->d.val_int += added_val->d.val_int;
		}

		// discard the added stuff
		free_helix_val(added_val);
	}

	// subtraction
	eat_space();
	if (source[chr] == '-') {
		++chr;

		// get whatever comes next
		helix_val *subtracted_val = evaluate_expression();

		if (subtracted_val->type == HELIX_VAL_INT) {
			lh_value->d.val_int -= subtracted_val->d.val_int;
		}

		// discard the added stuff
		free_helix_val(subtracted_val);
	}

	// multiplication
	eat_space();
	if (source[chr] == '*') {
		++chr;

		// get whatever comes next
		helix_val *multiplied_val = evaluate_expression();

		if (multiplied_val->type == HELIX_VAL_INT) {
			lh_value->d.val_int *= multiplied_val->d.val_int;
		}

		// discard the added stuff
		free_helix_val(multiplied_val);
	}

	// division
	eat_space();
	if (source[chr] == '/') {
		++chr;

		// get whatever comes next
		helix_val *divided_val = evaluate_expression();

		if (divided_val->type == HELIX_VAL_INT) {
			lh_value->d.val_int /= divided_val->d.val_int;
		}

		// discard the added stuff
		free_helix_val(divided_val);
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
