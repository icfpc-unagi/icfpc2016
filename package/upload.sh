#!/bin/bash

set -e -u

if [ "${CIRCLE_BUILD_NUM:-none}" != 'none' ]; then
  VERSION="r${CIRCLE_BUILD_NUM}"
else
  VERSION="${USER}"
fi

aws s3 cp --recursive . "s3://ninebinary/${VERSION}"
