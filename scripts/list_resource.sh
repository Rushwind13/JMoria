#!/bin/sh

# Use POSIX-compatible syntax for conditionals
if [ -z "$1" ]; then
  read -p "List which resource type? " TYPE
else
  TYPE="$1"
fi

# List entries in file order (matches runtime list order)
grep "^$TYPE" Resources/"${TYPE}s".txt | nl -v 0
