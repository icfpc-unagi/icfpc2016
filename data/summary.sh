#!/bin/bash

cd "$(dirname "${BASH_SOURCE}")/problems"

generate() {
  ulimit -t 5
  ../../bin/problem --problem="${file}" | \
      convert -geometry 64x64 svg:- "${output}"
}

for file in *.txt; do
  id="${file%.txt}"
  output="../../www/summary/${id}.png"
  if [ ! -f "${output}" ]; then
    echo "Generating ${id}.png...";
    generate &
    wait
  fi
done
