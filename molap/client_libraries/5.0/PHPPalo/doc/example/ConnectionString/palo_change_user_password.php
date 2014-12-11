<?php

	// create the variables pass and new_pass	
	$otheruser = "poweruser";
	$new_pass = "password";		

	// define the constants
	define('CONN', 'LOCALHOST');	
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);
	
	// changes password of other user to new one
	palo_change_user_password($connection, $otheruser, $new_pass);	
	
	// disconnect the connection to server
	palo_disconnect($connection_id); 
?>

