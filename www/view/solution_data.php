<?php

require_once(dirname(__FILE__) . '/../library/api.php');

$snapshot = GetSnapshot();
$problem_id = GetParameter('problem_id');
foreach ($snapshot['problems'] as $problem) {
  if ($problem['problem_id'] == $problem_id) {
    break;
  }
}
if ($problem['problem_id'] != $problem_id) {
  $problem = NULL;
}

$solution_id = NULL;
if (isset($_GET['solution_id'])) {
  $solution_id = $_GET['solution_id'];
} else if (isset($_ENV['solution_id'])) {
  $solution_id = $_ENV['solution_id'];
} else {
  die('solution_id must be given.');
}

$data = Database::SelectCell('SELECT `solution_data` FROM `solution` WHERE `solution_id` = {solution_id}', ['solution_id' => $solution_id]);

if ($data === FALSE) {
  die('Solution is not found.');
}

?><html>
<meta charset="UTF-8">
<title>Solution</title>
<link rel="stylesheet" type="text/css" href="/style.css">
<body>
<h1>Figure</h1>
<table>
<tr><td>
<?php

function Draw($data) {
  $process = proc_open(
      'timeout 10s /alloc/global/bin/solution',
      [0 => ['pipe', 'r'],
       1 => ['pipe', 'w'],
       2 => ['pipe', 'w']],
      $pipes, NULL, NULL);
  if ($process === NULL) {
    echo 'Failed to run solution.';
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

Draw($data);

?>
</td><td>
<?php

function DrawProblem($data) {
  $process = proc_open(
      'timeout 10s /alloc/global/bin/problem --noshrink_viewbox',
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

if (!is_null($problem)) {
  $problem['data'] = GetBlob($problem['problem_spec_hash']);
  DrawProblem($problem['data']);
}

?>
</td><td>
<?php

function DrawValidation($problem, $solution) {
  $result = Execute([
      'command' => 'validate',
      'alsologtostderr' => 1,
      'show_figure' => 1,
      'problem_file' => $problem,
      'solution_file' => $solution]);
  echo $result['stdout'];
  if (strlen($result['stderr']) > 0) {
    echo "<h2>Standard Error</h2>\n";
    echo "<pre>" . htmlspecialchars(trim($result['stderr'])) . "</pre>\n";
  }
}

if (!is_null($problem)) {
  DrawValidation($problem['data'], $data);
}

?></td></tr></table>
<h1>Data</h1>
<textarea style="width:100%;height:300px;font-size:100%;font-family:monospace"><?php echo htmlspecialchars($data); ?></textarea>
</body>
