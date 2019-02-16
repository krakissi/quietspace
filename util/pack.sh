#!/bin/bash

ENC=../src/assets_enc
FROM=../assets/from

mkdir -p "$FROM"

for F in $(ls $FROM); do
	echo "packing $F..."
	./encode.sh "$FROM/$F" > "$ENC/$F"
done
