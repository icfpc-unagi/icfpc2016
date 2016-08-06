#!/bin/bash

set -e -u

"${BASH_SOURCE}.runfiles/iwiwi/prefilter" |
java -jar "${BASH_SOURCE}.runfiles/wata/wata_deploy.jar" |
"${BASH_SOURCE}.runfiles/iwiwi/postfilter1" |
"${BASH_SOURCE}.runfiles/iwiwi/postfilter2"
