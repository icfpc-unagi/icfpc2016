#!/bin/bash

cd "$(dirname "${BASH_SOURCE}")"

php fetch.php

bazel build -c opt //iwiwi:prefilter

bash summary.sh

pushd problems
for file in *.txt; do
  if [ ! -s ../problems_converted/$file ]; then
    ../../bazel-bin/iwiwi/prefilter < $file > ../problems_converted/$file
  fi
done
popd

git add problems/*.txt problems_converted/*.txt ../www/summary/*.png
