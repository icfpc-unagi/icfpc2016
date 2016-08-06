<?php

require_once(dirname(__FILE__) . '/../library/api.php');

$snapshot = GetSnapshot();

if (is_null($snapshot)) {
  die('Invalid snapshot');
}

$problem_id = NULL;
if (isset($_GET['problem_id'])) {
  $problem_id = $_GET['problem_id'];
} else if (isset($_ENV['problem_id'])) {
  $problem_id = $_ENV['problem_id'];
} else {
  die('problem_id must be given.');
}

foreach ($snapshot['problems'] as $problem) {
  if ($problem['problem_id'] == $problem_id) {
    break;
  }
}

if ($problem['problem_id'] != $problem_id) {
  die('Unknown problem id');
}

if (!is_null(GetParameter('solution'))) {
  include_once(dirname(__FILE__) . '/../exec/add_solution.php');
  $result = AddSolution();
  if ($result['code'] != 0) {
    die('Failed to submit: ' . json_encode($result));
  }
}

?><html>
<meta charset="UTF-8">
<title>Solutions</title>
<link rel="stylesheet" type="text/css" href="/style.css">
<body>
<h1>Submit</h1>
<form action="?problem_id=<?php echo $problem_id; ?>" method="POST">
<textarea name="solution" style="width:100%;height:200px"></textarea>
<input type="submit" value="Submit">
</form>
<h1>Solutions for <?php echo $problem_id; ?></h1>
<table>
<tr><td>Solution ID</td><td>Problem ID</td><td>Resemblance</td><td>Data</td><td>Size</td><td>Submission Time</td><td>Created Time</td></tr>
<?php


foreach (Database::Select('
    SELECT
      `solution_id`,
      `problem_id`,
      `solution_resemblance`,
      `solution_submission`,
      LENGTH(REPLACE(REPLACE(REPLACE(
          `solution_data`, "\n", ""), "\r", ""), " ", "")) AS `solution_size`,
      `solution_lock`,
      `solution_created`
    FROM `solution`
    WHERE `problem_id` = {problem_id}
    ORDER BY
      `solution_resemblance` DESC,
      `solution_size`,
      `solution_submission` DESC,
      `solution_created` DESC',
    ['problem_id' => intval($problem_id)]) as $solution) {
  if ($solution['solution_lock'] > 1483196400 &&
      !$solution['solution_submission']) {
    echo '<tr style="background:#ccc">';
    echo '<td>' . $solution['solution_id'] .
         ' (<a href="unlock.php?solution_id=' . $solution['solution_id'] .
         '&problem_id=' . $solution['problem_id'] . '">Unlock</a>)</td>';
  } else {
    echo '<tr>';
    echo '<td>' . $solution['solution_id'] . '</td>';
  }
  echo '<td>' . $solution['problem_id'] . '</td>';
  echo '<td>' . $solution['solution_resemblance'] . '</td>';
  echo '<td><a href="solution_data.php?solution_id=' . $solution['solution_id'] . '">View</a></td>';
  echo '<td>' . $solution['solution_size'] . '</td>';
  echo '<td>' . $solution['solution_submission'] . '</td>';
  echo '<td>' . $solution['solution_created'] . '</td>';
  echo "</tr>\n";
}

?>
</table>
</body>
