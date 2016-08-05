<?php

require_once(dirname(__FILE__) . '/../library/api.php');

$snapshot = GetSnapshot();

if (is_null($snapshot)) {
  die('Invalid snapshot');
}

$proposal_id = NULL;
if (isset($_GET['proposal_id'])) {
  $proposal_id = $_GET['proposal_id'];
} else if (isset($_ENV['proposal_id'])) {
  $proposal_id = $_ENV['proposal_id'];
} else {
  die('proposal_id must be given.');
}

$proposal = Database::SelectRow('
    SELECT * FROM `proposal`
    WHERE `proposal_id` = {proposal_id} LIMIT 1',
    ['proposal_id' => $proposal_id]);

if ($proposal['proposal_id'] != $proposal_id) {
  die('Proposal is not found.');
}

$data = NULL;
if (isset($_POST['data'])) {
  $data = $_POST['data'];
} else if (isset($_ENV['data'])) {
  $data = file_get_contents($_ENV['data']);
}

if (!is_null($data) && strlen(trim($data)) > 0) {
  $data = FormatData($data);
  Database::Command('
      UPDATE `proposal`
      SET
          `problem_id` = NULL,
          `proposal_solution` = {proposal_solution},
          `proposal_submission` = NULL,
          `proposal_lock` = 0
      WHERE `proposal_id` = {proposal_id}
      LIMIT 1',
      ['proposal_id' => $proposal_id,
       'proposal_data' => $data]);
}

?><html>
<meta charset="UTF-8">
<title>Proposal</title>
<link rel="stylesheet" type="text/css" href="/style.css">
<body>
<h1>Submit</h1>
<form action="?proposal_id=<?php echo $proposal_id; ?>" method="POST">
<textarea name="data" style="width:100%;height:200px"></textarea>
<input type="submit" value="Submit">
</form>
<?php

$data = Database::SelectCell('
    SELECT `proposal_solution`
    FROM `proposal`
    WHERE `proposal_id` = {proposal_id} LIMIT 1',
    ['proposal_id' => $proposal_id]);

if (!is_null($data)) {
?>
<h1>Figure</h1>
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

$data = Database::SelectCell('
    SELECT `proposal_solution`
    FROM `proposal`
    WHERE `proposal_id` = {proposal_id} LIMIT 1',
    ['proposal_id' => $proposal_id]);
Draw($data);

?>
<h1>Data</h1>
<textarea style="width:100%;height:300px;font-size:100%;font-family:monospace"><?php echo htmlspecialchars($data); ?></textarea>
<?php
}
?>
</body>
