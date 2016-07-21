#!/bin/bash

test::ninestream_example() {
  local stdin="$(sub::tmpfile)"
  local stdout="$(sub::tmpfile)"

  echo ': $(( i = NINESTREAM_STREAM_ID ))' >>"${stdin}"
  echo ': $(( i *= i ))' >>"${stdin}"
  echo 'echo ${i}' >>"${stdin}"
  echo 'exit' >>"${stdin}"

  ninetan/ninestream \
      --master='ninetan/ninestream_example --alsologtostderr' \
      --worker='bash' --replicas=5 \
      --communicate \
      --alsologtostderr <"${stdin}" >"${stdout}"

  func::explode output $'\n' "$(sub::file_get_contents "${stdout}")"
  func::sort output
  EXPECT_EQ 'Stream 2 outputs 4'  "${output[0]}"
  EXPECT_EQ 'Stream 3 outputs 9'  "${output[1]}"
  EXPECT_EQ 'Stream 4 outputs 16' "${output[2]}"
  EXPECT_EQ 'Stream 5 outputs 25' "${output[3]}"
  EXPECT_EQ 'Stream 6 outputs 36' "${output[4]}"
}
