#!/bin/bash

TARGET='ninetan/ninestream'
PID=''

setup() {
  local stdin="$(sub::tmpfile)"
  local stdout="$(sub::tmpfile)"
  mkfifo "${stdin}"
  mkfifo "${stdout}"
  "${TARGET}" --alsologtostderr <"${stdin}" >"${stdout}" &
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
  ASSERT_STREQ 'OK 1' "$(call 'run 1 bash')"
  # bash output nothing, so it should exceed deadline.
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read 1 10')"
  ASSERT_STREQ 'OK 1' "$(call 'write 1 echo foo')"
  # "echo foo" should output "foo".
  ASSERT_STREQ 'OK 1 foo' "$(call 'read 1')"
  ASSERT_STREQ 'OK 1' "$(call 'write 1 exit')"
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read 1')"
  ASSERT_STREQ 'OK' "$(call 'exit')"
  wait "${PID}"
}

test::ninestream_readall() {
  setup
  ASSERT_STREQ 'OK 1 2 3' "$(call 'run 3 bash')"
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read -1 10')"
  ASSERT_STREQ 'OK 2' "$(call 'write 2 echo Stream 2')"
  ASSERT_STREQ 'OK 2 Stream 2' "$(call 'read -1')"
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read -1 10')"
  ASSERT_STREQ 'OK 1' "$(call 'write 1 echo Stream 1')"
  ASSERT_STREQ 'OK 1 Stream 1' "$(call 'read -1')"
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read -1 10')"
  ASSERT_STREQ 'OK 1 2 3' "$(call 'write -1 exit')"
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read -1')"
  ASSERT_STREQ 'OK' "$(call 'exit')"
  wait "${PID}"
}

test::ninestream_writeall() {
  setup
  ASSERT_STREQ 'OK 1 2 3' "$(call 'run 3 bash')"
  ASSERT_STREQ 'OK 1 2 3' "$(call 'write -1 echo foo')"
  ASSERT_STREQ 'OK 1 foo' "$(call 'read 1')"
  ASSERT_STREQ 'OK 2 foo' "$(call 'read 2')"
  ASSERT_STREQ 'OK 3 foo' "$(call 'read 3')"
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read -1 10')"
  ASSERT_STREQ 'OK 1 2 3' "$(call 'write -1 exit')"
  ASSERT_STREQ 'OK' "$(call 'exit')"
  wait "${PID}"
}

test::ninestream_kill() {
  setup
  ASSERT_STREQ 'OK 1 2 3 4' "$(call 'run 4 bash')"
  ASSERT_STREQ 'OK 4' "$(call 'write 4 exit')"
  sleep 0.05
  ASSERT_STREQ 'OK 1 2 3' "$(call 'write -1 echo foo')"
  ASSERT_STREQ 'OK' "$(call 'kill 2')"
  ASSERT_STREQ 'OK 1 foo' "$(call 'read 1')"
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read 2')"
  ASSERT_STREQ 'OK 3 foo' "$(call 'read 3')"
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read -1 10')"
  ASSERT_STREQ 'OK 1 3' "$(call 'list')"
  ASSERT_STREQ 'OK 1 3' "$(call 'write -1 echo foo')"
  ASSERT_STREQ 'OK' "$(call 'exit')"
  wait "${PID}"
}

test::ninestream_eof() {
  setup
  ASSERT_STREQ 'OK 1 2 3 4' "$(call 'run 4 bash')"
  ASSERT_STREQ 'OK 1 2 3 4' "$(call 'list')"
  ASSERT_STREQ 'OK 2' "$(call 'write 2 exit')"
  ASSERT_STREQ 'OK 4' "$(call 'write 4 echo foo')"
  ASSERT_STREQ 'OK 4' "$(call 'write 4 exit')"
  sleep 0.05
  ASSERT_STREQ 'OK 1 2 3 4' "$(call 'list')"
  ASSERT_STREQ 'OK 4 foo' "$(call 'read -1')"
  ASSERT_STREQ 'OK 1 3 4' "$(call 'list')"
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read -1 50')"
  ASSERT_STREQ 'OK 1 3' "$(call 'list')"
  ASSERT_STREQ 'OK' "$(call 'exit')"
  wait "${PID}"
}

test::ninestream_exec() {
  local stdin="$(sub::tmpfile)"
  local stdout="$(sub::tmpfile)"
  local line=''
  mkfifo "${stdin}"
  mkfifo "${stdout}"

  setup
  ASSERT_STREQ 'OK' \
      "$(call "exec { cat <${stdin} & }; cat >${stdout}; wait")"
  exec 4>"${stdin}"
  exec 3<"${stdout}"

  echo 'run 3 bash' >&4
  IFS='' read line <&3
  EXPECT_EQ 'OK 1 2 3' "${line}"

  echo 'write 2 echo foo' >&4
  IFS='' read line <&3
  EXPECT_EQ 'OK 2' "${line}"

  echo 'write -1 exit' >&4
  IFS='' read line <&3
  EXPECT_EQ 'OK 1 2 3' "${line}"

  echo 'read -1' >&4
  IFS='' read line <&3
  EXPECT_EQ 'OK 2 foo' "${line}"

  echo 'read -1' >&4
  IFS='' read line <&3
  EXPECT_EQ 'DEADLINE_EXCEEDED No ready stream.' "${line}"

  echo 'exit' >&4
  IFS='' read line <&3
  EXPECT_EQ 'OK' "${line}"

  wait "${PID}"
}
