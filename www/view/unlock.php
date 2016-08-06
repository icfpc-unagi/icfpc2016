<?php

require_once(dirname(__FILE__) . '/../library/api.php');

$solution_id = GetParameterOrDie('solution_id');

Database::Command('
    UPDATE `solution` SET `solution_lock` = UNIX_TIMESTAMP()
    WHERE `solution_id` = {solution_id} AND `solution_submission` IS NULL
    LIMIT 1',
    ['solution_id' => $solution_id]);

$problem_id = GetParameter('problem_id');
if (!is_null($problem_id)) {
  header("Location: solution.php?problem_id=$problem_id");
}
