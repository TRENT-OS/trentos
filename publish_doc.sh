#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Publish documentation and reports to web server
#
# Copyright (C) 2019, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------

DOC_ROOT=/var/www

BRANCH_NAME=$1
DOC_DIR=${DOC_ROOT}/seos_tests/${BRANCH_NAME}

rm -rf ${DOC_DIR}
mkdir -p ${DOC_DIR}

# ToDo: should better copy and not move
mv build-DOC/SEOS-Projects_doc-html ${DOC_DIR}/
