#!/bin/sh

# Use POSIX-compatible syntax for conditionals
if [ -z "$1" ]; then
  read -p "List which resource type? " TYPE
else
  TYPE="$1"
fi

# Use `nl`, `sort -nr`, and `cut -f2-` to reverse lines in a cross-platform way
grep "^$TYPE" Resources/"${TYPE}s".txt | nl | sort -nr | cut -f2- | nl -v 0
