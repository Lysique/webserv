<html>
<head>
<title>Hello PHP CGI</title>
</head>
<body>
<?php
  if (getenv('username'))
  {
      echo '<h2>Hello ' . getenv('username') . ', here is a beautifull dog !<h/2>';
      ?>
      <img src="Dog.png">
      <form action="other.php" method="post">
      <input id='submit' type='submit' name='logout' value='logout'>
      <?php
  }
  else
  {
    echo '<h1>You have to be logged in to see a beautifull dog !</h1>';
    ?>   
    <p><a href="login.php">Login</a></p>
    <?php
  }
?>
</body>
</html>
