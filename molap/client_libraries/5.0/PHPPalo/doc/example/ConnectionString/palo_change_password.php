<?php

	// create the variables pass and new_pass	
	$pass = "admin";
	$new_pass = "password";		

	// define the constants
	define('CONN', 'LOCALHOST');	
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', $pass);
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);
	
	// changes old password with new one
	palo_change_password($connection, $pass, $new_pass);	
	
?>

