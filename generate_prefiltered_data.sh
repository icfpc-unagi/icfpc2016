#!/bin/sh
set -e

cd iwiwi
make
cd ../

git pull
cd data/problems
for I in *.txt;
do
    ../../iwiwi/bin/prefilter < $I > ../problems_converted/$I;
    git add ../problems_converted/$I
done
git commit -m "Converted problem datag"
git push
