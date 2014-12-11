<?php
    // define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
   	
	// registers a name for a PALO server
	$connection_id = palo_init(HOST, PORT, USER, PASS);
	
	// create variables db_name and cube_name
	$db_name = 'Demo';
	$cube_name = 'SalesTest';
	
	// create array cube_data
	$cube_data = array(
						 'Products',
						 'Regions',
						 'Months',
						 'Years',
						 'Datatypes',
						 'Measures'
					  );
	
	// add cube SalesTest
	palo_database_add_cube($connection_id, $db_name, $cube_name, $cube_data);

	// disconnect the connection to server
	palo_disconnect($connection_id);
?>
