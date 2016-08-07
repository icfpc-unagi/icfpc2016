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
<title>Snapshot at <?php
echo date('Y-m-d H:i:s', $snapshot['snapshot_time']);
?></title>
<link rel="stylesheet" type="text/css" href="/style.css">
<body>
<h1>Problems at <?php
echo date('Y-m-d H:i:s', $snapshot['snapshot_time']);
?></h1>
<div><?php
$compact = !is_null(GetParameter('compact'));
if ($compact) {
  echo '[<a href="?">Full</a>]';
} else {
  echo '[<a href="?compact=1">Compact</a>]';
}
?></div>
<table>
<tr><td>ID</td><td>Owner</td><td>Problem Size</td><td>Owner's Solution Size</td><td>Solved</td><td>Solutions</td><td>Publish Time</td></tr>
<?php

$solutions = [];
foreach (Database::Select('
    SELECT
        `problem_id`,
        COUNT(*) AS `solution_count`,
        MAX(`solution_resemblance`) AS `solution_resemblance`
    FROM `solution`
    WHERE `solution_submission` IS NOT NULL
    GROUP BY `problem_id`') as $problem) {
  $solutions[$problem['problem_id']]['submitted'] = $problem;
}
foreach (Database::Select('
    SELECT
        `problem_id`,
        COUNT(*) AS `solution_count`,
        MAX(`solution_resemblance`) AS `solution_resemblance`
    FROM `solution`
    GROUP BY `problem_id`') as $problem) {
  $solutions[$problem['problem_id']]['all'] = $problem;
}

foreach ($snapshot['problems'] as $problem) {
  $solved = 0;
  $resemblance = 0.0;
  foreach ($problem['ranking'] as $rank) {
    if ($rank['resemblance'] == 1) {
      $solved++;
    } else {
      $resemblance = max($resemblance, $rank['resemblance']);
    }
  }
  $solution = $solutions[intval($problem['problem_id'])];
  if ($compact) {
    if ($solution['all']['solution_resemblance'] == 1) continue;
    if ($problem['owner'] == '42') continue;
  }
  echo '<tr>';
  echo '<td><a href="problem.php?problem_id=' . $problem['problem_id'] .
       '">' . $problem['problem_id'] . "</a></td>";
  echo '<td>' . htmlspecialchars($users[$problem['owner']]) . "</td>";
  echo '<td>' . $problem['problem_size'] . "</td>";
  echo '<td>' . $problem['solution_size'] . "</td>";
  if ($solved > 0) {
    echo '<td>' . $solved . '</td>';
  } else {
    echo '<td>(' . $resemblance . ')</td>';
  }
  echo '<td><a href="problem.php?problem_id=' . $problem['problem_id'] . '">';
  if (!isset($solution['all']['solution_count'])) {
    echo 'No';
  } else if ($solution['submitted']['solution_resemblance'] == 1.0) {
    echo '<b>Solved</b>';
  } else if ($solution['all']['solution_resemblance'] == 1.0) {
    echo '<b style="color:#888">Solved</b>';
  } else if ($solution['submitted']['solution_resemblance'] ==
             $solution['all']['solution_resemblance']) {
    echo $solution['submitted']['solution_resemblance'];
  } else if (is_null($solution['all']['solution_resemblance'])) {
    echo '??????';
  } else {
    echo '<span style="color:#888">(' .
         $solution['all']['solution_resemblance'] . ')</span>';
  }
  echo '</a></td>';
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
