#!/bin/bash
if [[ -z "$1" ]]; then
	read -p "Find which resource type? " TYPE
else
	TYPE="$1"
fi
if [[ -z "$2" ]]; then
	read -p "Find which object? " OBJECT
else
	OBJECT="$2"
fi

grep ^$TYPE Resources/${TYPE}s.txt | tac| awk -v obj="$OBJECT" 'tolower($0) ~ tolower("\\<"obj"\\>") {print NR-1, $0}'
