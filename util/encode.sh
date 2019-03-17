#!/bin/bash

F=/tmp/ring_10421

cat > "$F.c"  << EOF
#include <string.h>
#include <stdio.h>

#include "base64.h"

char *data = (
#include "$1"
);

int main(){
	char *enc = base64_enc(data, strlen(data));

	base64_toquoted(enc, stdout);
	free(enc);

	return 0;
}
EOF

rm -f "$F.out"
cc -o "$F.out" -I../src -I. "$F.c"

"$F.out"
