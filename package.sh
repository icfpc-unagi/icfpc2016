#!/bin/bash

set -e -u

OUTPUT="${TMPDIR}/binary.tgz"
if [ -f "${OUTPUT}" ]; then
  rm "${OUTPUT}"
fi

tar zcfh "${OUTPUT}" ./

UNAME="$(uname -s | tr '[A-Z]' '[a-z]')"
HASH="$(git rev-parse --short HEAD)"
SUFFIX='-master'
if [ "$(git status --short)" != '' ]; then
  SUFFIX="-${USER}"
fi

KEY="${UNAME}-${HASH}${SUFFIX}"
aws s3 cp --quiet "${OUTPUT}" "s3://ninecluster/binary/${KEY}"
echo "${KEY}"
