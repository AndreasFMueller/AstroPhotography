<html>
<head>
<title>Update</title>
</head>
<body>
<!--
 
  update.php 

  (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil

-->
<?php
// database connection
$mysqli = new mysqli("localhost", "snow", "blu33er", "snowstar");

// get the post variables
$instrument = $_POST['instrument'];
$updatetime = $_POST['updatetime'];
$avgguideerror = $_POST['avgguideerror'];

$ccdtemperature = $_POST['ccdtemperature'];
$lastimagestart = $_POST['lastimagestart'];
$exposuretime = $_POST['exposuretime'];
$currenttaskid = $_POST['currenttaskid'];

$telescopeRA = $_POST['telescopeRA'];
$telescopeDEC = $_POST['telescopeDEC'];
$west = $_POST['west'];
if ($west == "yes") {
	$west = 1;
} else {
	$west = 0;
}
$filter = $_POST['filter'];
$observatoryLONG = $_POST['observatoryLONG'];
$observatoryLAT = $_POST['observatoryLAT'];
$project = $_POST['project'];
$focus = $_POST['focus'];


// send the update to the database
$query = "insert into statusupdate(instrument, updatetime, avgguideerror, ".
	"ccdtemperature, lastimagestart, exposuretime, currenttaskid, ".
	"rightascension, declination, west, filter, longitude, latitude, ".
	"project, focus) ".
	"values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
if (!($stmt = $mysqli->prepare($query))) {
	echo "prepare failed: " . $mysqli->connect_error;
}
if (!$stmt->bind_param("ssddsdiddiiddsi", $instrument, $updatetime, $avgguideerror,
	$ccdtemperature, $lastimagestart, $exposuretime, $currenttaskid,
	$telescopeRA, $telescopeDEC, $west, $filter, 
	$observatoryLONG, $observatoryLAT, $project, $focus)) {
	echo "Binding failed: " . $stmt->error;
}
if (!$stmt->execute()) {
	echo "Execute failed: " . $stmt->error;
}

$stmt->close();

?>
</body>
</html>
