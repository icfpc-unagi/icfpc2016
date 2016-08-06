<?php

require_once(dirname(__FILE__) . '/../library/api.php');

?><html>
<meta charset="UTF-8">
<title>Proposals</title>
<link rel="stylesheet" type="text/css" href="/style.css">
<body>
<h1>Proposals</h1>
<table>
<tr><td>Problem ID</td><td>Problem Size</td><td>Owner's Solution Size</td><td>Publish Time</td><td>Submission Time</td><td>Edit</td></tr>
<?php

foreach (Database::Select('
    SELECT
        `proposal_id`,
        `problem_id`,
        `proposal_note`,
        LENGTH(REPLACE(REPLACE(REPLACE(
            `proposal_problem`, "\n", ""), "\r", ""), " ", ""))
            AS `problem_size`,
        LENGTH(REPLACE(REPLACE(REPLACE(
            `proposal_solution`, "\n", ""), "\r", ""), " ", ""))
            AS `solution_size`,
        `proposal_submission`,
        `proposal_modified`
    FROM `proposal`
    ORDER BY `proposal_id`') AS $proposal) {
  echo '<tr>';
  if (is_null($proposal['problem_id'])) {
    echo '<td>-</td>';
  } else {
    echo '<td><a href="problem.php?problem_id=' . $proposal['problem_id'] .
       '">' . $proposal['problem_id'] . "</a></td>";
  }
  echo '<td>' . $proposal['problem_size'] . "</td>";
  echo '<td>' . $proposal['solution_size'] . "</td>";
  echo '<td>' . date('Y-m-d H:i:s', $proposal['proposal_id']) . "</td>";
  echo '<td>' . $proposal['proposal_submission'] . "</td>";
  echo '<td><a href="proposal.php?proposal_id=' . $proposal['proposal_id'] .
       '">Edit</a></td>';
  echo "</tr>\n";
}

?>
</table>
</body>
