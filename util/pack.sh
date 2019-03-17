#!/bin/bash

ENC=../src/assets_enc
FROM=../assets/from

working=$(dirname $0)

if ! [ -e "$working/encoder" ]; then
	echo "Missing encoder! Run \"make encoder\" in the src directory."
	exit 1
fi

mkdir -p "$FROM"

for F in $(ls $FROM); do
	echo "packing $F..."
	"$working/encoder" "$FROM/$F" > "$ENC/$F"
done
