#!/bin/bash

source "$(dirname "${BASH_SOURCE}")/../../bin/imosh" || exit 1
eval "${IMOSH_INIT}"

exec {LOCK}>>"$(dirname "${BASH_SOURCE}")/lock"
if ! flock -w 1 "${LOCK}"; then
  echo 'This script is already running, so exiting...' >&2
  exit
fi

while :; do
  pushd ~/github
  git pull
  popd
  bash "$(dirname "${BASH_SOURCE}")/sync.sh"
  {
    echo 'HTTP/1.0 200 OK'
    echo 'Content-Type: text/html'
    echo
    echo 'OK'
    sleep 10
  } | nc -l 18080
done &

~/.dropbox-dist/dropboxd

sub::exit
