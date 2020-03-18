#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Publish documentation and reports to web server
#
# Copyright (C) 2019, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------

BRANCH_NAME=$1

rm -rf /var/www/${BRANCH_NAME}
mkdir -p /var/www/${BRANCH_NAME}
mv build-DOC/SEOS-API_doc-html /var/www/${BRANCH_NAME}/
mv build-DOC/SEOS-Projects_doc-html /var/www/${BRANCH_NAME}/
mv build-DOC/test_seos_libs /var/www/${BRANCH_NAME}/
