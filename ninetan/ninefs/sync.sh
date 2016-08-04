#!/bin/bash

rsync -a --delete --exclude='.git' --delete-excluded \
    ~/"github/" ~/"Dropbox/ICFPC2015/github/"
