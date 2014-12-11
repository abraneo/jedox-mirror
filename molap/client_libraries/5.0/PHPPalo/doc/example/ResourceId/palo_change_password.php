<?php

	// create the variables pass and new_pass	
	$pass = "admin";
	$new_pass = "password";	
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init(HOST, PORT, USER, PASS);
	
	// changes old password with new one
	palo_change_password($connection_id, $pass, $new_pass);	
	
	// disconnect the connection to server
	palo_disconnect($connection_id); 
?>

