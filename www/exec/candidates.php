<?php

require_once(dirname(__FILE__) . '/../library/api.php');

function Call() {
  $solution_ai = GetParameterOrDie('solution_ai');

  $snapshot = GetSnapshot();
  $problem_ids = [];
  foreach ($snapshot['problems'] as $problem) {
    if ($problem['owner'] == '42') {
      continue;
    }
    $problem_ids[$problem['problem_id']] = TRUE;
  }

  foreach (Database::Select('
      SELECT `problem_id`,
             MAX(`solution_resemblance`) AS `solution_resemblance`
      FROM `solution`
      GROUP BY `problem_id`') as $solution) {
    if ($solution['solution_resemblance'] == 1) {
      unset($problem_ids[$solution['problem_id']]);
    }
  }

  foreach (Database::Select('
      SELECT `problem_id`,
             MAX(`solution_resemblance`) AS `solution_resemblance`
      FROM `solution`
      WHERE `solution_ai` = {solution_ai}
      GROUP BY `problem_id`',
      ['solution_ai' => $solution_ai]) as $solution) {
    unset($problem_ids[$solution['problem_id']]);
  }

  $problem_ids = array_keys($problem_ids);
  shuffle($problem_ids);

  echo implode("\n", $problem_ids) . "\n";
}

if (count(debug_backtrace()) == 0) {
  print_r(Call());
}
