#!/bin/bash

# $1 - path to cppcheck output file
# cppcheck outputs warnings with the filename attached to them

excluded_paths=( 
    sdk-sel4-camkes
    capdl_spec.c
)

if [ ! -f "$1" ]; then
    exit 0
fi

# Print the file so we have it in the Jenkins log

cat "$1"

# remove all lines from the output file which match elements in the 
# exclusion list

grep -v `printf '%s '"${excluded_paths[@]/#/-e }"` "$1" > "$1.out"


# if there still are warnings left, fail the stage

if [[ $(wc -l < "$1.out") -gt 0 ]]; then
    exit 1
fi

exit 0