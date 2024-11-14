#!/bin/bash

if [[ -z "$1" ]]; then
  read -p "List which resource type? " TYPE
else
  TYPE="$1"
fi

grep ^$TYPE Resources/${TYPE}s.txt | tac | nl -v 0
