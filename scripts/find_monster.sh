#!/bin/sh
if [ -z "$1" ]; then
	read -p "Find which monster? " MONSTER
else
	MONSTER="$1"
fi

./scripts/find_resource.sh Monster "${MONSTER}"
