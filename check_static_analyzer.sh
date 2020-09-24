#!/bin/bash

cppcheck --project="$1" --output-file=cppcheck_output.txt \
    -i "`pwd`/OS-SDK/pkg/libs/os_network_stack/3rdParty/" \
    -i "`pwd`/OS-SDK/pkg/sdk-sel4-camkes/"

excluded=( 
    \#error
)

# Print the file so we have it in the Jenkins log

cat "cppcheck_output.txt"

# remove all lines from the output file which match elements in the 
# exclusion list

grep -v `printf '%s '"${excluded[@]/#/-e }"` "cppcheck_output.txt" > "cppcheck_output.txt.out"


# if there still are warnings left, fail the stage

if [[ $(wc -l < "cppcheck_output.txt.out") -gt 0 ]]; then
    exit 1
fi

exit 0