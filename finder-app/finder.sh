#!/bin/sh
if [ -z "$1" ]; then
	echo "First argument is not specified."
	exit 1
fi

if [ -z "$2" ]; then
	echo "Second argument is not specified."
	exit 1
fi

if [ -d "$1" ]; then
	fileCount=$(find $1 -type f | wc -l)
	searchCount=$(grep -r "$2" "$1" | wc -l)
	echo "The number of files are $fileCount and the number of matching lines are $searchCount"
else
	echo "$1 is not a directory."
	exit 1
fi
exit 0
