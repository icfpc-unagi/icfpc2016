#!/bin/bash

set -e -u

java -jar "${BASH_SOURCE}.runfiles/wata/wata_deploy.jar"
