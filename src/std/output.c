#include <stdio.h>

#include "core.h"

/**
 * signature: void output::print(string value);
 */
void output_print(helix_val *val) {
	printf("%s", val->d.val_string);
	free_helix_val(val);
}
