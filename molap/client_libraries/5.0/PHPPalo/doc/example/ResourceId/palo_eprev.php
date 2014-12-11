<?php
	
	// defines the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init( HOST, PORT, USER, PASS);	
	
	// create the variables db_name, cube_name, dimension_name and element
	$db_name = 'Demo';
	$cube_name = 'Sales'; 
	$dimension_name = 'Years';  
	$element_name = '2004';
	$dimension_name_test_empty_string = 'Yeadfrs'; 

	
	// retrieves the previous element of element 2004
	palo_eprev($connection_id, $db_name, $dimension_name, $element_name);

	// retrieves the previous element of element 2004
	palo_eprev($connection_id, $db_name, $dimension_name, $element_name, true);

	// the empty string parameter hides the error, if set to true
	palo_eprev($connection_id, $db_name, $dimension_name_test_empty_string,
	                 $element_name, true);

	// if empty string parameter is set to false the error will be shown
	palo_eprev($connection_id, $db_name, $dimension_name_test_empty_string,
	                 $element_name, false);

	// disconnect the connection to server
	palo_disconnect($connection_id); 
?>
