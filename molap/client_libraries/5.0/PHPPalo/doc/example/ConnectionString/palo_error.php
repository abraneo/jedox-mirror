<?php

	// define the constants
	define('CONN', 'LOCALHOST');	
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);	

	// create the variables db_name and cube_name	
	$db_name = 'Demo';
	$cube_name = 'Sales';		
	
	// return the specified data
	palo_data(CONN . '/' . $db_name, $cube_name,
			 'Desktop LLLLLLL', 'Germany', 'Jan', '2002', 'Budget', 'Units');		
	
	// call palo_error() to get the latest error message.
	palo_error();

	// call palo_error() again, now it returns no error
	// because the call above clears the error message after returning it
	palo_error();
?>

