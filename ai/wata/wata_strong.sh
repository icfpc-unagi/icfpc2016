#!/bin/bash

set -e -u

if [ -d "${BASH_SOURCE}.runfiles/__main__" ]; then
  BASE_DIR="${BASH_SOURCE}.runfiles/__main__"
else
  BASE_DIR="${BASH_SOURCE}.runfiles"
fi

"${BASE_DIR}/iwiwi/prefilter" |
java -jar "${BASE_DIR}/wata/wata_deploy.jar" -a StrongSolver |
"${BASE_DIR}/iwiwi/postfilter1" |
"${BASE_DIR}/iwiwi/postfilter2"
