#!/bin/bash

./ninetan/http/server_example --alsologtostderr --port=8001 &
sub::atexit "kill -TERM $!"

test::server_example_get() {
  local actual="$(sub::tmpfile)"
  local expected="$(sub::tmpfile)"
  local output=(
      'method: "GET"'
      'path: "/"'
      'version: "HTTP/1.1"'
      'header {'
      '  key: "user-agent"'
      '  value: "ServerTest"'
      '}'
      'header {'
      '  key: "host"'
      '  value: "localhost:8001"'
      '}'
      'header {'
      '  key: "accept"'
      '  value: "*/*"'
      '}'
  )
  sub::implode $'\n' output > "${expected}"
  curl 'http://localhost:8001/' -A 'ServerTest' >"${actual}"
  diff "${expected}" "${actual}"
}
