#include <stdio.h>

#include "core.h"

void out(blang_var *string) {
	printf("%s", string->d.var_string);
}
