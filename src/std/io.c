#include <stdio.h>

#include "core.h"

void out(blang_val *string) {
	printf("%s", string->d.val_string);
}
