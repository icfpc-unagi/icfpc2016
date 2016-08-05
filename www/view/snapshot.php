<?php

require_once(dirname(__FILE__) . '/../library/api.php');

$snapshot = GetSnapshot();

if (is_null($snapshot)) {
  die('Invalid snapshot');
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
<title>Snapshot</title>
<link rel="stylesheet" type="text/css" href="/style.css">
<body>
<h1>Problems</h1>
<table>
<tr><td>ID</td><td>Owner</td><td>Problem Size</td><td>Owner's Solution Size</td><td>Solved</td><td>Solutions</td><td>Publish Time</td></tr>
<?php

foreach ($snapshot['problems'] as $problem) {
  $solved = 0;
  foreach ($problem['ranking'] as $rank) {
    if ($rank['resemblance'] == 1) {
      $solved++;
    }
  }
  echo '<tr>';
  echo '<td><a href="problem.php?problem_id=' . $problem['problem_id'] .
       '">' . $problem['problem_id'] . "</a></td>";
  echo '<td>' . htmlspecialchars($users[$problem['owner']]) . "</td>";
  echo '<td>' . $problem['problem_size'] . "</td>";
  echo '<td>' . $problem['solution_size'] . "</td>";
  echo '<td>' . $solved . '</td>';
  echo '<td><a href="solution.php?problem_id=' . $problem['problem_id'] .
       '">Soluions</a></td>';
  echo '<td>' . date('Y-m-d H:i:s', $problem['publish_time']) . "</td>";
  echo "</tr>\n";
}

?>
</table>
<h1>Leaderboard</h1>
<table>
<tr><td>Rank</td><td>Team name</td><td>Score</td></tr>
<?php

$rank = 1;
foreach ($snapshot['leaderboard'] as $user) {
  echo '<tr>';
  echo '<td>' . $rank . '</td>';
  echo '<td>' . htmlspecialchars($users[$user['username']]) .
       ' (' . $user['username'] . ")</td>";
  echo '<td>' . $user['score'] . "</td>";
  echo "</tr>\n";
  $rank++;
}

?>
</table>
</body>
