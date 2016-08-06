<?php

require_once(dirname(__FILE__) . '/../library/api.php');

$snapshot = GetSnapshot();

if (is_null($snapshot)) {
  die('Invalid snapshot');
}

$problems = [];
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
  $problems[$problem['problem_id']]['solved'] = $solved;
  $problems[$problem['problem_id']]['resemblance'] = $resemblance;
}

$approx_solutions = Database::Select('
    SELECT * FROM (
      SELECT
        `problem_id`,
        `solution_resemblance`,
        `solution_id`,
        MAX(`solution_submission`) AS `solution_submission`
      FROM (
        SELECT
          `problem_id`,
          MAX(`solution_resemblance`) AS `solution_resemblance`
        FROM `solution`
        WHERE `solution_resemblance` < 1
        GROUP BY `problem_id`) AS `a`
      NATURAL JOIN (
        SELECT
          `solution_id`,
          `problem_id`,
          `solution_resemblance`,
          `solution_submission`
        FROM `solution`) AS `b`
      GROUP BY `problem_id`) AS `c`');
$all_solutions = Database::Select('
    SELECT * FROM (
      SELECT
        `problem_id`,
        `solution_resemblance`,
        `solution_id`,
        MAX(`solution_submission`) AS `solution_submission`
      FROM (
        SELECT
          `problem_id`,
          MAX(`solution_resemblance`) AS `solution_resemblance`
        FROM `solution`
        GROUP BY `problem_id`) AS `a`
      NATURAL JOIN (
        SELECT
          `solution_id`,
          `problem_id`,
          `solution_resemblance`,
          `solution_submission`
        FROM `solution`) AS `b`
      GROUP BY `problem_id`) AS `c`');

foreach ($approx_solutions as $solution) {
  $problems[$solution['problem_id']]['approx'] = $solution;
}
foreach ($all_solutions as $solution) {
  $problems[$solution['problem_id']]['all'] = $solution;
}

$solution_ids = [];
foreach ($problems as $problem) {
  // Highest score is submitted.
  if (!is_null($problem['all']['solution_submission'])) {
    continue;
  }
  // If the problem is easy, then submit it.
  if ($problem['solved'] >= 3) {
    $solution_ids[] = $problem['all']['solution_id'];
    continue;
  }
  // Highest approx score is submitted.
  if (!is_null($problem['approx']['solution_submission'])) {
    continue;
  }
  $solution_ids[] = $problem['approx']['solution_id'];
}

foreach ($solution_ids as $key => $value) {
  if (!$value) unset($solution_ids[$key]);
}

Database::Command('
    UPDATE `solution`
    SET `solution_lock` = UNIX_TIMESTAMP()
    WHERE `solution_id` IN (' . implode(', ', $solution_ids) . ') AND
          `solution_lock` > 1483196400');
echo Database::AffectedRows() . "\n";

