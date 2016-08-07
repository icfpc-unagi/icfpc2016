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

if (GetParameter('plain')) {
  header('Content-Type: text/plain');
  echo GetBlob($problem['problem_spec_hash']);
  exit();
}

$users = [];
for ($id = 1; $id < 10; $id++) {
  $users["$id"] = "Contest Organizer Problem Set $id";
}
foreach ($snapshot['users'] as $user) {
  $users[$user['username']] = $user['display_name'];
}

ksort($users);

if (!is_null(GetParameter('solution'))) {
  include_once(dirname(__FILE__) . '/../exec/add_solution.php');
  $result = AddSolution();
  if ($result['code'] != 0) {
    die('Failed to submit: ' . json_encode($result));
  }
  header('Location: problem.php?problem_id=' . $problem_id);
  die('Successfully submitted.');
}

StartPage();

$data = GetBlob($problem['problem_spec_hash']);

?>
<h1 class="page-header">Problem Details</h1>
<table class="layout"><tr><td>
<table class="table table-bordered">
<?php
echo '<tr><th>ID</th><td>' . $problem['problem_id'] . "</td></tr>\n";
echo '<tr><th>Publish Time</th><td>' . date('Y-m-d H:i:s', $problem['publish_time']) . "</th></tr>\n";
echo '<tr><th>Owner</th><td>' . htmlspecialchars($users[$problem['owner']]) . "</td></tr>\n";
echo '<tr><th>Problem Size</th><td>' . $problem['problem_size'] . "</td></tr>\n";
echo '<tr><th>Solution Size</th><td>' . $problem['solution_size'] . "</td></tr>\n";
?>
<tr><th>Data</th><td><textarea style="width:100%;height:100px;font-size:100%;font-family:monospace"><?php echo htmlspecialchars($data); ?></textarea></td></tr>
<tr><th>Links</th><td><a href="hint.php?problem_id=<?php echo $problem['problem_id']; ?>">Hint maker</a></td></tr>
</table>
</td><td style="padding:20px;vertical-algin:top;">
<?php

function Draw($data) {
  $process = proc_open(
      'timeout 10s /alloc/global/bin/problem',
      [0 => ['pipe', 'r'],
       1 => ['pipe', 'w'],
       2 => ['pipe', 'w']],
      $pipes, NULL, NULL);
  if ($process === NULL) {
    echo 'Failed to run problem.';
    return;
  }
  fwrite($pipes[0], str_replace("\r\n", "\n", $data));
  fclose($pipes[0]);
  echo stream_get_contents($pipes[1]);
  $stderr = stream_get_contents($pipes[2]);
  if (strlen($stderr) > 0) {
    echo "<h3>Standard Error</h3>\n";
    echo "<pre>" . htmlspecialchars(trim($stderr)) . "</pre>\n";
  }
  $return_value = proc_close($process);
}

Draw($data);

?>
</td></tr></table>
<h1 class="page-header">Submit</h1>
<form action="?problem_id=<?php echo $problem_id; ?>" method="POST">
<textarea name="solution" style="width:100%;height:200px"></textarea>
<input type="submit" value="Submit">
</form>
<h1 class="page-header">Ranking</h1>
<table class="layout"><tr><td style="width:65%">
<h3>Internal</h3>
<table class="table table-condensed table-striped">
  <tbody>
    <tr>
      <th>Solution ID</th>
      <th>AI Name</th>
      <th>Resemblance</th>
      <th>Solution Size</th>
      <th>Submission Time</th>
      <th>Creation Time</th>
    </tr>
<?php


foreach (Database::Select('
    SELECT
      `solution_id`,
      `problem_id`,
      `solution_ai`,
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
    echo '<tr class="pending">';
    echo '<td>' . $solution['solution_id'] .
         ' (<a href="unlock.php?solution_id=' . $solution['solution_id'] .
         '&problem_id=' . $solution['problem_id'] . '">Unlock</a>)</td>';
  } else {
    echo '<tr>';
    echo '<td>' . $solution['solution_id'] . '</td>';
  }
  echo '<td>' . htmlspecialchars($solution['solution_ai']) . '</td>';
  echo '<td>' . $solution['solution_resemblance'] . '</td>';
  echo '<td>' . $solution['solution_size'] .
       ' (<a href="solution_data.php?solution_id=' .
       $solution['solution_id'] . '&problem_id=' . $solution['problem_id'] .
       '">View</a>)</td>';
  echo '<td>' . $solution['solution_submission'] . '</td>';
  echo '<td>' . $solution['solution_created'] . '</td>';
  echo "</tr>\n";
}

?></table>
</td><td>
<h3>External</h3>
<table class="table table-condensed table-striped">
  <tbody>
    <tr>
      <th>Rank</th>
      <th>Resemblance</th>
      <th>Solution Size</th>
    </tr>
<?php

$rank = 1;
foreach ($problem['ranking'] as $user) {
  echo '<tr>';
  echo '<td>' . $rank . '</td>';
  echo '<td>' . $user['resemblance'] . "</td>";
  echo '<td>' . $user['solution_size'] . "</td>";
  echo "</tr>\n";
  $rank++;
}

?>
</td></tr></table>
