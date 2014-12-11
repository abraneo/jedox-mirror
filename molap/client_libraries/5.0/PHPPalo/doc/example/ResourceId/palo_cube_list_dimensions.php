<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server 
	$connection_id = palo_init(HOST, PORT, USER, PASS);	
	
	// define the variables as follows
	$db_name = 'Demo';
	$cube_name = 'Sales';  
	
	// listing all dimensions by using the cube Sales
	palo_cube_list_dimensions($connection_id, $db_name, $cube_name);
	
	// disconnect the connection to server
    palo_disconnect($connection_id); 	  
?>


