<?php
//
// sky.png.php -- produce an image
// 
// (c) 2018 Prof Dr Andreas Müller
//

if (isset($_GET['id'])) {
	$id = $_GET['id'];
} else {
	$id = -1;
}

// connect to the database
$mysqli = new mysqli("localhost", "snow", "blu33er", "snowstar");

$query = "select id, rightascension, declination, longitude, latitude, ".
	"exposuretime, unix_timestamp(lastimagestart) as 'lastimagestart' ".
	"from statusupdate ";
if ($id < 0) {
	$query = $query . "order by id desc limit 1";
} else {
	$query = $query . "where id = " . $id;
}
$result = $mysqli->query($query);
$row = $result->fetch_assoc();
$id = $row["id"];

// positions from database
$longitude = $row["longitude"];
$latitude = $row["latitude"];
$rightascension = $row["rightascension"];
$declination = $row["declination"];
$exposuretime = $row["exposuretime"];
if ($exposuretime < 10) {
	$exposuretime = 10;
}
$exposuretime = round($exposuretime);
$lastimagestart = round($row["lastimagestart"]);

// temporary file for the sky
$filename = tempnam("/var/tmp", "sky").".png";

$cmd = "/usr/local/astro/bin/astrosky --debug --cardinal --size=640".
	" --time=" . $lastimagestart .
	" --latitude=" . $latitude .
	" --longitude=" . $longitude .
	" --rightascension=" . $rightascension .
	" --declination=" . $declination . 
	" " . $filename;


ob_start();
system($cmd, $return_var);
if ($return_var) {
	header("Content-Type: text/plain");

	ob_end_flush();
	printf("command: %s", $cmd);
	exit;
}
ob_end_clean();

if (!($fp = fopen($filename, "rb"))) {
	printf("cannot open file");
}

header("Content-Type: image/png");
header("Content-Length: ".filesize($filename));
header("Refresh: ".$exposuretime);

fpassthru($fp);

fclose($fp);

exit;

?>
