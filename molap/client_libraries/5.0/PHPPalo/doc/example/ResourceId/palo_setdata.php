<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id_id = palo_init( HOST, PORT, USER, PASS);	
	
	// create the variables
	$db_name = 'Demo';
	$cube_name = 'Sales';
	
	// set specified data
	palo_setdata('12','false', $connection_id_id, $db_name,
				 $cube_name,'Desktop L','France','Jan','2009','Actual','Units');

	// disconnect the connection to server
	palo_disconnect($connection_id_id); 
?>
