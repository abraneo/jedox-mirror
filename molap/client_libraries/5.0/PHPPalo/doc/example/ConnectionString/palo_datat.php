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
			
	// return the data at the specified coordinates
	palo_datat(CONN . '/' . $db_name, $cube_name, 'Desktop L', 'Germany', 
			  'Jan', '2002', 'Budget', 'Units');			  
?>
