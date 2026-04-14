#!/bin/sh
# Ensure POSIX-compatible syntax for conditionals
if [ -z "$1" ]; then
    read -p "Find which resource type? " TYPE
else
    TYPE="$1"
fi
if [ -z "$2" ]; then
    read -p "Find which object? " OBJECT
else
    OBJECT="$2"
fi

# List entries in file order (matches runtime list order)
# Show the index, header line, and full definition block for each match
FILE="Resources/${TYPE}s.txt"
grep "^$TYPE" "$FILE" | \
awk -v obj="$OBJECT" 'tolower($0) ~ tolower("\\<"obj"\\>") {print NR-1, $0}' | \
while read -r idx header; do
    echo "$idx $header"
    # Extract the block between the header line and the closing }
    awk -v hdr="$header" '
        $0 == hdr { found=1; next }
        found && /^\{/ { inside=1; next }
        found && inside && /^\}/ { exit }
        found && inside { print }
    ' "$FILE"
    echo ""
done
