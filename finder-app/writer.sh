#!/bin/sh
if [ -z "$1" ]; then
	echo "First argument not provided."
	exit 1
fi

if [ -z "$2" ]; then
	echo "Second argument not provided."
	exit 1
fi

path=$(dirname "$1")
mkdir -p "$path"
echo "$2" > $1
if [ $? -eq 0 ]; then
	exit 0
else
	echo "Error: File Creation Unsuccessful."
	exit 1
fi

