<html>
<head>
<title>Hello PHP CGI</title>
</head>
<body>
<?php
// $first_name = htmlspecialchars($value['fname']);
// $last_name = htmlspecialchars($value['lname']);
echo getenv('SHELL');
echo "<h2>First Name " . getenv('fname')  . "</h2>";
echo "<h2>Last name " . getenv('lname')  . "</h2>";
echo "<h2>Second name " . getenv('second')  . "</h2>";
echo "<h2>Third name " . getenv('Third')  . "</h2>";
?>
</body>
</html>