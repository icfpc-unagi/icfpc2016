<?php

require_once(dirname(__FILE__) . '/../library/api.php');

function Score($solution) {
  if (!$solution) {
    die('No submission to score is pending.');
  }

  $snapshot = GetSnapshot();
  $problem_id = $solution['problem_id'];
  foreach ($snapshot['problems'] as $problem) {
    if ($problem['problem_id'] == $problem_id) {
      break;
    }
  }
  if ($problem['problem_id'] != $problem_id) {
    die('Unknown problem id');
  }

  $resemblance = 0;
  if (strlen(preg_replace(
          '%[^\\s]*%', '', $solution['solution_data'])) <= 5000) {
    $response = Execute([
        'command' => 'score',
        'arg0_file' => GetBlob($problem['problem_spec_hash']),
        'arg1_file' => $solution['solution_data']]);
    if (is_null($response)) {
      die("Failed to submit.");
    }
    if (is_numeric(trim($response['stdout']))) {
      $resemblance = floatval(trim($response['stdout']));
    }
  } else {
    $resemblance = 0;
  }
  echo "Solution " . $solution['solution_id'] . ': ' . $resemblance . "\n";
  Database::Command('
      UPDATE `solution`
      SET `solution_resemblance` = {solution_resemblance}
      WHERE `solution_id` = {solution_id} AND
            `solution_submission` IS NULL',
      ['solution_id' => $solution['solution_id'],
       'solution_resemblance' => $resemblance]);
}

for ($i = 0; $i < 3; $i++) {
  Score(Database::SelectRow('
      SELECT `solution_id`, `problem_id`, `solution_data`
      FROM `solution`
      WHERE `solution_resemblance` IS NULL
      LIMIT 1'));
}
