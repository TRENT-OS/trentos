#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# AStyle Checker
#
# Copyright (C) 2019-2010, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------
#
# *.astyle files are generated when file are not astyle compliant. Check if
# any such file exists in the workspace and fail in this case.
#

FILES=$(find . -name '*.astyle')

if [ ! -z "${FILES}" ]; then

    echo "ERROR: source is not astyle compliant, check: "

    for file in ${FILES}; do
        echo "  ${FILES}"
    done

    exit 1

fi

exit 0
