<?php

	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init(HOST, PORT, USER, PASS);
	
	// create the variables db_name and cube_name	
	$db_name = 'Demo';
	$cube_name = 'Sales';		
	
	// return the specified data
	palo_data($connection_id, $db_name, $cube_name,
			 'Desktop LLLL', 'Germany', 'Jan', '2002', 'Budget', 'Units');		
	
	// call palo_error() to get the latest error message.
	palo_error();

	// call palo_error() again, now it returns no error
	// because the call above clears the error message after returning it
	palo_error();

	// disconnect the connection to server
    palo_disconnect($connection_id); 
?>

