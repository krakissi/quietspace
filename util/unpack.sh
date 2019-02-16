#!/bin/bash

ENC=../src/assets_enc
OUT=../assets/out

mkdir -p "$OUT"

for F in $(ls $ENC); do
	echo "unpacking $F..."
	./decode.sh "$ENC/$F" > "$OUT/$F"
done
