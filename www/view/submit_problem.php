<?php

require_once(dirname(__FILE__) . '/../library/api.php');

function FindProposal() {
  Database::Command('
      UPDATE `proposal`
      SET `proposal_id` = (@proposal_id := `proposal_id`),
          `proposal_lock` = UNIX_TIMESTAMP() + 60
      WHERE
          `proposal_lock` < UNIX_TIMESTAMP() AND
          `proposal_solution` IS NOT NULL AND
          `proposal_submission` IS NULL
      ORDER BY `proposal_lock`
      LIMIT 1');
  if (Database::AffectedRows() != 0) { return TRUE; }
  return FALSE;
}

function Submit($proposal) {
  $response = SubmitProblem(
      $proposal['proposal_id'], $proposal['proposal_solution']);
  if (is_null($response)) {
    die("Failed to submit.");
  }
  if (!$response['ok']) {
    $response['problem_id'] = NULL;
    $problem = NULL;
  } else {
    $problem = GetBlob($response['problem_spec_hash']);
  }
  Database::Command('
      UPDATE `proposal`
      SET `problem_id` = {problem_id},
          `proposal_submission` = CURRENT_TIMESTAMP(),
          `proposal_problem` = {proposal_problem},
          `proposal_response` = {proposal_response}
      WHERE `proposal_id` = {proposal_id}',
      ['proposal_id' => $proposal['proposal_id'],
       'problem_id' => $response['problem_id'],
       'proposal_problem' => $problem,
       'proposal_response' => json_encode($response)]);
}

if (!FindProposal()) {
  die('No proposal is pending.');
}

Submit(Database::SelectRow('
    SELECT `proposal_id`, `proposal_solution`
    FROM `proposal`
    WHERE `proposal_id` = @proposal_id'));
