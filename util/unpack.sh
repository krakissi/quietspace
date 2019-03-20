#!/bin/bash

working=$(dirname "$0")
ENC="$working/../src/assets_enc"
OUT="$working/../assets/out"

mkdir -p "$OUT"

for F in $(ls $ENC); do
	echo "unpacking $F..."
	"$working/decode.sh" "$ENC/$F" > "$OUT/$F"
done
