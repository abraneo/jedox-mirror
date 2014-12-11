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
	
	// create the array dimension with the coordinates
	$dimension = array(
					   	'Desktop L',
					   	'Germany',
					   	'Jan',
					   	'2002',
					   	'Budget',
					   	'Units'
					   );       
	
	// returns the data at specified coordinates in array dimension
	palo_dataat($connection_id, $db_name, $cube_name, $dimension);
	
	// disconnect the connection to server
    palo_disconnect($connection_id); 
	
	
?>

