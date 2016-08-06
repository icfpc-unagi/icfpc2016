<?php

require_once(dirname(__FILE__) . '/../library/api.php');

function AddSolution() {
  $problem_id = GetParameterOrDie('problem_id');
  $solution = GetParameterOrDie('solution');
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

  $solution = FormatData($solution);

  if (trim($solution) == '') {
    Fail('Solution is empty');
  }

  $filter = Execute(['command' => 'postfilter', 'stdin' => $solution]);
  if ($filter['code'] != 0) {
    return $filter;
  }
  $solution = $filter['stdout'];

  $result = Execute([
      'command' => 'validate',
      'alsologtostderr' => 1,
      'problem_file' => $problem_data,
      'solution_file' => $solution]);

  if ($result['code'] != 0) {
    return $result;
  }

  $solution_id = Database::SelectCell('
      SELECT `solution_id` FROM `solution`
      WHERE `problem_id` = {problem_id} AND
            `solution_data` = {solution_data}
      ORDER BY `solution_id` LIMIT 1',
      ['problem_id' => $problem_id,
       'solution_data' => FormatData($solution)]);
  if ($solution_id) {
    $result['stdout'] = 'duplicated with solution_id=' . $solution_id;
    return $result;
  }

  Database::Command('INSERT INTO `solution`{solution}',
      ['solution' => [
          'problem_id' => $problem_id,
          'solution_data' => FormatData($solution)]]);

  return $result;
}

if (count(debug_backtrace()) == 0) {
  echo json_encode(AddSolution()) . "\n";
}
