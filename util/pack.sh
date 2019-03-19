#!/bin/bash

working=$(dirname $0)
ENC=$working/../src/assets_enc
FROM=$working/../assets/from

if ! [ -e "$working/encoder" ]; then
	echo "Missing encoder! Run \"make encoder\" in the src directory."
	exit 1
fi

mkdir -p "$FROM"

for F in $(ls $FROM); do
	echo "packing $F..."
	"$working/encoder" "$FROM/$F" > "$ENC/$F"
done
