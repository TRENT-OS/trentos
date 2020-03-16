#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Publish documentation and reports to web server
#
# Copyright (C) 2019, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------

SCRIPT_DIR=$(cd `dirname $0` && pwd)
BRANCH_NAME=$(cd ${SCRIPT_DIR} && git rev-parse --abbrev-ref HEAD)

rm -rf /var/www/${BRANCH_NAME}
mkdir -p /var/www/${BRANCH_NAME}
mv ${SCRIPT_DIR}/build-DOC/SEOS-API_doc-html /var/www/${BRANCH_NAME}/
mv ${SCRIPT_DIR}/build-DOC/SEOS-Projects_doc-html /var/www/${BRANCH_NAME}/
mv ${SCRIPT_DIR}/build-DOC/test_seos_libs /var/www/${BRANCH_NAME}/
