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

# Use nl and sort -nr to reverse lines, which is cross-platform
grep "^$TYPE" Resources/"${TYPE}s".txt | nl | sort -n | cut -f2- | \
awk -v obj="$OBJECT" 'tolower($0) ~ tolower("\\<"obj"\\>") {print NR-1, $0}'
