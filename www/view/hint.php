<?php

require_once(dirname(__FILE__) . '/../library/api.php');

$snapshot = GetSnapshot();

if (is_null($snapshot)) {
  die('Invalid snapshot');
}

$problem_id = GetParameter('problem_id');
if (is_null($problem_id)) {
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

$problem_data = GetBlob($problem['problem_spec_hash']);
$prefilter = Execute([
    'command' => 'prefilter',
    'stdin' => $problem_data]);
if ($prefilter['code'] !== 0) {
  die('Failed to run prefilter: ' . json_encode($prefilter));
}

$prefilter_data = $prefilter['stdout'];
$svg = Execute([
    'command' => 'problem',
    'filtered' => 1,
    'stdin' => $prefilter_data]);
if ($svg['code'] !== 0) {
  die('Failed to run problem: ' . json_encode($svg));
}

StartPage();

?>
<script src="http://ajax.googleapis.com/ajax/libs/jquery/1.8.3/jquery.min.js"></script>
<script>
var vertex = new Array();

$(document).ready(function() {
  $("circle").bind("click", function() {
    vertex.push($(this)[0].id.substr(1));
    var length = 0;
    var message = "";
    for (var i = 1; i < vertex.length; i++) {
      console.log(Math.hypot(
          $("#v" + vertex[i]).attr("cx") -
          $("#v" + vertex[i - 1]).attr("cx"),
          $("#v" + vertex[i]).attr("cy") -
          $("#v" + vertex[i - 1]).attr("cy")));
      length += Math.hypot(
          $("#v" + vertex[i]).attr("cx") -
          $("#v" + vertex[i - 1]).attr("cx"),
          $("#v" + vertex[i]).attr("cy") -
          $("#v" + vertex[i - 1]).attr("cy"));
    }
    if (length > 0) {
      message += "<div>Length: " + length + "</div>";
    }
    $("#message")[0].innerHTML = message;
    $("#output")[0].innerText =
        $("#data")[0].innerText + 
        vertex.length + "\n" +
        vertex.join(" ");
    $("#vertices")[0].innerText = vertex.length + "\n" + vertex.join(" ");
  });
});
</script>
<h1 class="page-header">Hint Maker</h1>
<div id="data" style="display:none"><?php echo htmlspecialchars($prefilter_data); ?></div>
<table class="layout"><tr><td>
<?php

echo $svg['stdout'];

?></td><td><div id="message"></div><pre id="vertices"></pre></td></tr></table>
<h1 class="page-header">Output</h1>
<textarea id="output" style="width:100%;height:400px;"></textarea>
