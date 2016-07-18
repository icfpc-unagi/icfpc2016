#!/bin/bash

TARGET='ninetan/ninestream'
PID=''

setup() {
  local stdin="$(sub::tmpfile)"
  local stdout="$(sub::tmpfile)"
  mkfifo "${stdin}"
  mkfifo "${stdout}"
  "${TARGET}" <"${stdin}" >"${stdout}" &
  PID="$!"
  exec 4>"${stdin}"
  exec 3<"${stdout}"
}

call() {
  local line=''
  echo "$1" >&4
  IFS='' read line <&3
  echo "${line}"
}

test::ninestream_read() {
  setup
  EXPECT_STREQ 'OK 1' "$(call 'run 1 bash')"
  EXPECT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read 1 10')"
  EXPECT_STREQ 'OK' "$(call 'write 1 echo foo')"
  EXPECT_STREQ 'OK foo' "$(call 'read 1')"
  EXPECT_STREQ 'OK' "$(call 'write 1 exit')"
  EXPECT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read 1')"
  EXPECT_STREQ 'OK' "$(call 'exit')"
  wait "${PID}"
}
