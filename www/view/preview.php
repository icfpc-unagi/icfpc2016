<?php

require_once(dirname(__FILE__) . '/../library/api.php');

$solution = GetParameter('solution');

?><html>
<meta charset="UTF-8">
<title>Solutions</title>
<link rel="stylesheet" type="text/css" href="/style.css">
<body>
<h1>Preview Solution</h1>
<form action="?" method="POST">
<textarea name="solution" style="width:100%;height:200px"><?php
if (!is_null($solution)) {
  echo htmlspecialchars($solution);
}
?></textarea>
<input type="submit" value="Preview">
</form>
<h1>Figure</h1>
<?php

if (!is_null($solution)) {
  $result = Execute(['command' => 'solution',
                     'alsologtostderr' => 1,
                     'stdin' => $solution]);
  if ($result['code'] == 0) {
    echo $result['stdout'];
    unset($result['stdout']);
  }

  echo '<h2>Other Information</h2><pre>';
  print_r($result);
  echo '</pre>';
}

?>
</body>
