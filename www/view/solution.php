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

$data = NULL;
if (isset($_POST['data'])) {
  $data = $_POST['data'];
} else if (isset($_ENV['data'])) {
  $data = file_get_contents($_ENV['data']);
}

if (!is_null($data) && strlen(trim($data)) > 0) {
  Database::Command('INSERT INTO `solution`{solution}',
      ['solution' => [
          'problem_id' => $problem_id,
          'solution_data' => trim($data) . "\n"]]);
}

?><html>
<meta charset="UTF-8">
<title>Solutions</title>
<link rel="stylesheet" type="text/css" href="/style.css">
<body>
<h1>Submit</h1>
<form action="?problem_id=<?php echo $problem_id; ?>" method="POST">
<textarea name="data" style="width:100%;height:200px"></textarea>
<input type="submit" value="Submit">
</form>
<h1>Solutions for <?php echo $problem_id; ?></h1>
<table>
<tr><td>Solution ID</td><td>Problem ID</td><td>Resemblance</td><td>Data</td><td>Submission Time</td><td>Created Time</td></tr>
<?php

foreach (Database::Select('
    SELECT
      `solution_id`,
      `problem_id`,
      `solution_resemblance`,
      `solution_submission`,
      `solution_created`
    FROM `solution`
    WHERE `problem_id` = {problem_id}
    ORDER BY `solution_submission` DESC, `solution_created` DESC',
    ['problem_id' => intval($problem_id)]) as $solution) {
  echo '<tr>';
  echo '<td>' . $solution['solution_id'] . '</td>';
  echo '<td>' . $solution['problem_id'] . '</td>';
  echo '<td>' . $solution['solution_resemblance'] . '</td>';
  echo '<td>N/A</td>';
  echo '<td>' . (is_null($solution['solution_submission'])
                 ? 'N/A'
                 : date('Y-m-d H:i:s', $solution['solution_submission'])) .
       '</td>';
  echo '<td>' . date('Y-m-d H:i:s', $solution['solution_created']) . '</td>';
  echo "</tr>\n";
}

?>
</table>
</body>
