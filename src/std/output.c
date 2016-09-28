#include <stdio.h>

#include "core.h"

void output_print(helix_val *val) {
	printf("%s", val->d.val_string);
}

void output_reverse(helix_val *val) {
	printf("would be reversed");
}
