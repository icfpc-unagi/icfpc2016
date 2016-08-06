<?php

require_once(dirname(__FILE__) . '/../library/api.php');

$snapshot = GetSnapshot();

if (is_null($snapshot)) {
  die('Invalid snapshot');
}

$problems = [];
foreach (Database::Select('
    SELECT
        `problem_id`,
        COUNT(*) AS `solution_count`,
        MAX(`solution_resemblance`) AS `solution_resemblance`
    FROM `solution`
    WHERE `solution_submission` IS NOT NULL
    GROUP BY `problem_id`') as $problem) {
  if ($problem['solution_resemblance'] == 1) {
    $problems[$problem['problem_id']]['solved'] = TRUE;
  }
}

foreach ($snapshot['problems'] as $problem) {
  $solved = 0;
  $resemblance = 0.0;
  foreach ($problem['ranking'] as $rank) {
    if ($rank['resemblance'] == 1) {
      $solved++;
    } else {
      $resemblance = max($resemblance, $rank['resemblance']);
    }
  }
  if ($solved >= 3) {
    $problems[$problem['problem_id']]['easy'] = TRUE;
  }
}

$problem_ids = [];
foreach ($problems as $problem_id => $problem) {
  if ($problem['easy'] && !$problem['solved']) {
    $problem_ids[] = $problem_id;
  }
}

if (count($problem_ids) > 0) {
  Database::Command('
      UPDATE `solution` SET `solution_lock` = UNIX_TIMESTAMP()
      WHERE `problem_id` IN (' . implode(', ', $problem_ids) . ') AND
            `solution_lock` > 1483196400');
  echo Database::AffectedRows() . "\n";
} else {
  echo "0\n";
}
