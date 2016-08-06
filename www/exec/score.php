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

  if ($problem['owner'] == '42') {
    echo "1\n";
    return;
  }

  echo Database::SelectCell('
      SELECT MAX(`solution_resemblance`)
      FROM `solution`
      WHERE `problem_id` = {problem_id}',
      ['problem_id' => $problem_id]) . "\n";
}

if (count(debug_backtrace()) == 0) {
  print_r(Call());
}
