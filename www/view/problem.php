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

$users = [];
for ($id = 1; $id < 10; $id++) {
  $users["$id"] = "Contest Organizer Problem Set $id";
}
foreach ($snapshot['users'] as $user) {
  $users[$user['username']] = $user['display_name'];
}

ksort($users);

?><html>
<meta charset="UTF-8">
<title>Problem <?php echo $problem['problem_id']; ?></title>
<link rel="stylesheet" type="text/css" href="/style.css">
<body>
<h1>Problem Details</h1>
<table>
<?php
echo '<tr><td>ID</td><td>' . $problem['problem_id'] . "</td></tr>\n";
echo '<tr><td>Publish Time</td><td>' . date('Y-m-d H:i:s', $problem['publish_time']) . "</td></tr>\n";
echo '<tr><td>Owner</td><td>' . htmlspecialchars($users[$problem['owner']]) . "</td></tr>\n";
echo '<tr><td>Problem Size</td><td>' . $problem['problem_size'] . "</td></tr>\n";
echo '<tr><td>Solution Size</td><td>' . $problem['solution_size'] . "</td></tr>\n";
?>
</table>
<h1>Figure</h1>
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
    echo "<h2>Standard Error</h2>\n";
    echo "<pre>" . htmlspecialchars(trim($stderr)) . "</pre>\n";
  }
  $return_value = proc_close($process);
}

$data = GetBlob($problem['problem_spec_hash']);
Draw($data);

?>
<h1>Ranking</h1>
<table>
<tr><td>Rank</td><td>Resemblance</td><td>Size</td></tr>
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
</table>
<h1>Data</h1>
<textarea style="width:100%;height:300px;font-size:100%;font-family:monospace"><?php echo htmlspecialchars($data); ?></textarea>
</body>
