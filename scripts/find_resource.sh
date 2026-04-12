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
grep "^$TYPE" Resources/"${TYPE}s".txt | \
awk -v obj="$OBJECT" 'tolower($0) ~ tolower("\\<"obj"\\>") {print NR-1, $0}'
