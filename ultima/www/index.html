<?php
   include("settings.php");

   echo "
      <html>
      <head>
      <title>".$shardTitle."</title>
      <link href='u4m.css' rel='stylesheet' type='text/css'>
      </head>

      <body class='u4m'>
      <center>

      <table class='u4m'>
      <tr><td class='u4m'>

      <div align='center'><img src='img/ankh.gif' width='56' height='87' alt=''></div>

      <h1 class='u4m'>".$shardTitle."</h1>
      <h2 class='u4m'>".$shardSubtitle."</h1>
      ";

   echo "<p />Game server address: $serverAddress";
   
   //display port
   if ($serverPort != "8888")
	   echo ":$serverPort";

   echo "<p />Server is: ";
   
   //Create a TCP/IP socket
   $socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
   if ($socket < 0)
   {
      echo "<font color='yellow'>status unknown</font>";
      //echo "socket_create() failed: reason: " . socket_strerror($socket) . "<br>";
   }
   else
   {

      @socket_set_nonblock($socket);
      @socket_connect($socket, $serverAddress, $serverPort);
      @socket_set_block($socket);

      switch(socket_select($r = array($socket), $w = array($socket), $f = array($socket), $timeout))
      {
         case 2: echo "<font color='red'>offline</font>"; break;
         case 1: echo "<font color='green'>online</font>"; break;
         case 0: echo "<font color='red'>offline</font>"; break;
      }
   }

?>

<p />
<li /><a href="reference/index.html">Player Reference</a>
<li /><a href="img/britannia.jpg" target="_blank"><i>The Lands of Britannia</i></a>
<li /><a href="history/index.html"><i>The History of Britannia</i></a>
<li /><a href="wisdom/index.html"><i>The Book of Mystic Wisdom</i></a>
<li /><a href="u4_client.zip">Download Ultima IV Multiplayer client</a>
<li /><a href="http://www.shatteredmoon.com/ultima">www.shatteredmoon.com</a>

</td></tr>
</table>

</center>

</body>
</html>