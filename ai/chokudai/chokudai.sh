#!/bin/bash

set -e -u

if [ -d "${BASH_SOURCE}.runfiles/__main__" ]; then
  BASE_DIR="${BASH_SOURCE}.runfiles/__main__"
else
  BASE_DIR="${BASH_SOURCE}.runfiles"
fi

mono "${BASE_DIR}/chokudai/ICFPC2016.exe" |
"${BASE_DIR}/iwiwi/postfilter1" |
"${BASE_DIR}/iwiwi/postfilter2"
