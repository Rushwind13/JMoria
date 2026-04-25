#!/bin/sh
if [ -z "$1" ]; then
	read -p "Find which item? " ITEM
else
	ITEM="$1"
fi

./util/find_resource.sh Item "${ITEM}"
