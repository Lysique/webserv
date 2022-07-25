<html>
<head>
<title>Cookies</title>
</head>
<body>
<h1>Login session</h1>
<?php
  if (getenv('username'))
  {
      echo '<h2>Hello ' . getenv('username') . '<h/2>';
      ?>
      <form action="login.php" method="post">
      <input id='submit' type='submit' name='logout' value='logout'>
      <?php
  }
  else
  {
    ?>
    <form action="login.php" method="post">
    <input id='submit' type='submit' name='login' value='Login'>
    <input type="text" id="name" name="username">
    <?php
  }
?>
<h2>
<p><a href="other.php">Redirect to other login page</a></p>
</h2>
</body>
</html>