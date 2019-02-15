#!/bin/bash

F=/tmp/ring_10421

echo "encoding $1"

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

cc -o encoder -I../src -I. "$F.c"

./encoder
