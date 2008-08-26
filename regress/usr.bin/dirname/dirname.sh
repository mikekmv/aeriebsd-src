#!/bin/sh

test_dirname()
{
	echo "Testing \"$1\""
	result=`dirname "$1" 2>&1`
	if [ "$result" != "$2" ]; then
		echo "Expected \"$2\", but got \"$result\""
		exit 1
	fi
}

test_dirname "/" "/"
test_dirname "//" "/"
test_dirname "/usr/bin/" "/usr"
test_dirname "//usr//bin//" "//usr"
test_dirname "usr" "."
test_dirname "\"\"" "."
test_dirname "/usr" "/"
test_dirname "/usr/bin" "/usr"
test_dirname "usr/bin" "usr"
test_dirname ""	"."
test_dirname "/./" "/"
test_dirname "///usr//bin//" "///usr"
