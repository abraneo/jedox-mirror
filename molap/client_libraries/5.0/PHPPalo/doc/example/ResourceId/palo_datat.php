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
			
	// return the data at the specified coordinates
	palo_datat($connection_id, $db_name, $cube_name, 'Desktop L', 'Germany', 
			  'Jan', '2002', 'Budget', 'Units');

	// disconnect the connection to server
	palo_disconnect($connection_id);
?>
