#!/bin/sh
if [[ -z "$1" ]]; then
	read -p "Find which resource type? " TYPE
else
	TYPE="$1"
fi
if [[ -z "$2" ]]; then
	read -p "Find which item? " ITEM
else
	ITEM="$2"
fi
grep ^$TYPE Resources/${TYPE}s.txt | tac | nl -v $ITEM
