#!/bin/bash

working=$(dirname "$0")
F=/tmp/ring_10421

cat > "$F.c"  << EOF
#include <string.h>
#include <stdio.h>

#include "base64.h"

char *data = (
#include "$1"
);

int main(){
	char *dec = base64_dec(data, strlen(data));

	printf(dec);
	free(dec);

	return 0;
}
EOF

cc -o "$F.out" "-I$working/../src" -I. "$F.c"

"$F.out"
