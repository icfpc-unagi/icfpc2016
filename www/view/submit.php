<?php

require_once(dirname(__FILE__) . '/../library/api.php');

function FindSolution() {
  Database::Command('
      UPDATE `solution`
      SET `solution_id` = (@solution_id := `solution_id`),
          `solution_lock` = UNIX_TIMESTAMP() + 60
      WHERE
          `solution_lock` < UNIX_TIMESTAMP() AND
          `solution_submission` IS NULL
      ORDER BY `solution_lock`
      LIMIT 1');
  if (Database::AffectedRows() != 0) { return TRUE; }
  return FALSE;
}

function Submit($solution) {
  if (strlen(preg_replace(
          '%[\\s]*%', '', $solution['solution_data'])) <= 5000) {
    $response = SubmitSolution(
        $solution['problem_id'], $solution['solution_data']);
    if (is_null($response)) {
      die("Failed to submit.");
    }
  } else {
    $response = [
        'ok' => false,
        'solution_id' => $solution['solution_id'],
        'resemblance' => 0];
  }
  Database::Command('
      UPDATE `solution`
      SET `solution_resemblance` = {solution_resemblance},
          `solution_submission` = CURRENT_TIMESTAMP(),
          `solution_response` = {solution_response}
      WHERE `solution_id` = {solution_id}',
      ['solution_id' => $solution['solution_id'],
       'solution_resemblance' => $response['resemblance'],
       'solution_response' => json_encode($response)]);
}

if (!FindSolution()) {
  die('No submission is pending.');
}

Submit(Database::SelectRow('
    SELECT `solution_id`, `problem_id`, `solution_data`
    FROM `solution`
    WHERE `solution_id` = @solution_id'));
