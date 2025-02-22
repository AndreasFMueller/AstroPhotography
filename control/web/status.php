<?php

if (isset($_GET['id'])) {
	$id = $_GET['id'];
} else {
	$id = -1;
}

// database connection
$mysqli = new mysqli("localhost", "snow", "blu33er", "snowstar");

function frac($x) {
	return $x - floor($x);
}

function formatangle($a, $precision) {
	$sign = ($a < 0) ? -1 : 1;
	$b = $sign * $a;
	$d = floor($b);
	$m = 60 * frac($b);
	$s = 60 * frac($m);
	$ss = frac($s);
	$s = floor($s);
	$m = floor($m);
	return sprintf("%s%02d:%02d:%02d", ($sign < 0) ? "-" : "+", $d, $m, $s);
}

function longitude($a) {
	if ($a < 0) {
		return "W" . substr(formatangle(-$a, 0), 1);
	} else {
		return "E" . substr(formatangle($a, 0), 1);
	}
}

function latitude($a) {
	if ($a < 0) {
		return "S" . substr(formatangle(-$a, 0), 1);
	} else {
		return "N" . substr(formatangle($a, 0), 1);
	}
}

$query = "select id, instrument, updatetime, avgguideerror, ".
	"ccdtemperature, lastimagestart, exposuretime, currenttaskid, ".
	"rightascension, declination, west, filter, longitude, latitude, ".
	"project, focus from statusupdate ";
if ($id < 0) {
	$query = $query . "order by id desc limit 1";
} else {
	$query = $query . "where id = " . $id;
}
$result = $mysqli->query($query);
$row = $result->fetch_assoc();
$id = $row["id"];
$previous = $id - 1;
$next = $id + 1;
$exposuretime = $row["exposuretime"];
if ($exposuretime < 10) {
	$exposuretime = 10;
}
$exposuretime = round($exposuretime);

switch ($row["filter"]) {
case 0:	$filtername = "L"; break;
case 1:	$filtername = "R"; break;
case 2:	$filtername = "G"; break;
case 3:	$filtername = "B"; break;
case 4:	$filtername = "H-&alpha;"; break;
case 5:	$filtername = "O-III"; break;
case 6:	$filtername = "S-II"; break;
}
?>
<html>
<head>
<title>Telescope status</title>
<?php
printf("<meta http-equiv=\"refresh\" content=\"%.0f\">", $row["exposuretime"]);
?>
<style>
body {
	background-color: #cc0000;
	font-family: verdana;
	font-size: 80%;
}
input.button {
	width: 100;
	font-weight: bold;
}
div.button {
	display: inline-block;
}
div.bottop {
	width: 350;
	text-align: center;
	color: #009900;
}
div.imagerow {
	display: box;
	width: 350;
	height: 320;
	padding: 0;
/*	border: 1px solid blue; */
	font-weight: bold;
}
div.eastwest {
	display: inline-block;
	height: 320;
	width: 10;
	color: #009900;
/*	border: 1px solid green; */
	text-align: center;
	font-weight: bold;
}
img {
	width: 320;
	height: 320;
	display: inline-block;
	vertical-align: middle;
}
</style>
</head>
<body>
<h1>Telescope status</h1>
<p>
<div class="button">
<form action="status.php">
<input type="submit" value="<< back <<" class="button"/>
<input type="hidden" name="id" value="<?= $previous ?>"/>
</form>
</div>
<div class="button">
<form action="status.php">
<input type="submit" value="last" class="button"/>
</form>
</div>
<div class="button">
<form action="status.php">
<input type="submit" value=">> forward >>" class="button"/>
<input type="hidden" name="id" value="<?= $next ?>"/>
</form>
</div>
</p>
<table>
<?php
printf("<tr><td>Instrument:</td><td>%s</td></tr>\n",
	$row["instrument"]);
printf("<tr><td>Project:</td><td>%s</td></tr>\n",
	$row["project"]);
printf("<tr><td>Time:</td><td>%s</td></tr>\n",
	$row["updatetime"]);
printf("<tr><td>Average guide error:</td><td>%.1f arcsec</td></tr>\n",
	$row["avgguideerror"]);
printf("<tr><td>CCD temperature</td><td>%.1f ºC</td></tr>\n",
	$row["ccdtemperature"]);
printf("<tr><td>Last exposure started:</td><td>%s</td></tr>\n",
	$row["lastimagestart"]);
printf("<tr><td>Exposure time:</td><td>%.3f sec</td></tr>\n",
	$row["exposuretime"]);
printf("<tr><td>Current task id:</td><td>%s</td></tr>\n",
	$row["currenttaskid"]);
printf("<tr><td>Target:</td><td>𝛼=%s   𝛿=%s</td></tr>\n",
	substr(formatangle($row["rightascension"], 1), 1),
	formatangle($row["declination"]), 0);
printf("<tr><td>Telescope position:</td><td>%s</td></tr>\n",
	($row["west"]) ? "west" : "east");
printf("<tr><td>Filter:</td><td>%s</td></tr>\n",
	$filtername);
printf("<tr><td>Focus position:</td><td>%s</td></tr>\n",
	$row["focus"]);
printf("<tr><td>Telescope Location:</td><td>%s    %s</td></tr>\n",
	longitude($row["longitude"]),
	latitude($row["latitude"]));

?>
</table>
<div class="bottop">
N
</div>
<div class="imagerow">
<div class="eastwest">E</div>
<?php
if ($id > 0) {
	printf("<img width=\"320\" align=\"middle\" src=\"sky.png.php?id=%d\" />", $id);
} else {
	printf("<img width=\"320\" align=\"middle\" src=\"sky.png.php\" />");
}
?>
<div class="eastwest">W</div>
</div>
<div class="bottop">
S
</div>
</div>
</body>
</html>
