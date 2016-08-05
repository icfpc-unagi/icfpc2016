<?php

require_once(dirname(__FILE__) . '/../library/api.php');

function Call() {
  $problem_id = GetParameterOrDie('problem_id');
  $snapshot = GetSnapshot();

  foreach ($snapshot['problems'] as $problem) {
    if ($problem['problem_id'] == $problem_id) {
      break;
    }
  }

  if ($problem['problem_id'] != $problem_id) {
    Fail("$problem_id is not found");
  }

  $problem_data = GetBlob($problem['problem_spec_hash']);
  if (is_null($problem_data)) {
    Fail('Failed to fetch problem data', '500 Internal Server Error');
  }

  return Execute([
      'command' => 'problem',
      'alsologtostderr' => 1,
      'stdin' => $problem_data]);
}

if (php_sapi_name() == 'cli') {
  print_r(Call());
}
