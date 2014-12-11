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
	
	// defines coordinates for the target cells
	$coordinates1 = array(
		'Desktop L',
		'Germany',
		'Jan',
		'2004',
		'Budget',
		'Units'
	); 
	
	$coordinates2 = array(
		'Desktop Pro',
		'Germany',
		'Jan',
		'2004',
		'Budget',
		'Units'
	); 

	// Set value "100" to Desktop L in specified coordinates
	palo_setdata_bulk(CONN . '/' . $db_name, $cube_name, 
					  array($coordinates1), array("100"), "SPLASH_MODE_ADD"); 

	// Set value to Desktop L and to Desktop Pro as specified in arrays
	palo_setdata_bulk(CONN . '/' . $db_name, $cube_name,
					  array($coordinates1, $coordinates2), array("1111", "1234"), 
				  "SPLASH_MODE_ADD");

	// Set value to Desktop L and to Desktop Pro as specified in array
	palo_setdata_bulk(CONN . '/' . $db_name, $cube_name, 
	                  array($coordinates1, $coordinates2), array("1500", "2002"));

	
?>
