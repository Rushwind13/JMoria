#!/bin/sh
if [ -z "$1" ]; then
	read -p "Find which monster? " MONSTER
else
	MONSTER="$1"
fi

./util/find_resource.sh Monster "${MONSTER}"
