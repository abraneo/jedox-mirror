<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init(HOST, PORT, USER, PASS);	
	
	// create the variables db_name, cube_name and element
	$db_name = 'Demo';
	$dimension = 'Years';  
	$element = '2005';
	$element_test_empty_string = '2023';
	
	// delete element 2005 in dimension Years
	palo_edelete($connection_id, $db_name, $dimension, 
		$element);

	// delete element 2005 in dimension Years
	palo_edelete($connection_id, $db_name, $dimension, 
		$element, true);

	// the empty string parameter hides the error, if set to true
	palo_edelete($connection_id, $db_name, $dimension, 
		$element_test_empty_string, true);

	// if empty string parameter is set to false the error will be shown
	palo_edelete($connection_id, $db_name, $dimension,	
		$element_test_empty_string, false);

	// disconnect the connection to server
	palo_disconnect($connection_id);
?>
