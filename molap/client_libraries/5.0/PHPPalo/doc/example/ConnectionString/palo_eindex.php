<?php
	 
	// define the constants
	define('CONN', 'LOCALHOST');	
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);
	
	// create the variables db_name and dimension
	$db_name = 'Demo';
	$dimension = 'Products';
	$element_name = 'Desktop L';
	$element_name_test_empty_string = 'Desktop LL';

	
	// Returns the position of Desktop L in dimension Products
	palo_eindex(CONN . '/' . $db_name, $dimension, $element_name);

	// Returns the position of Desktop L in dimension Products
	palo_eindex(CONN . '/' . $db_name, $dimension, $element_name, true);

	//Hide the error message if empty string parameter is set to true
	palo_eindex(CONN . '/' . $db_name, $dimension, 
	            $element_name_test_empty_string, true);

	//Show the error message if empty string parameter is set to false
	palo_eindex(CONN . '/' . $db_name, $dimension, 
	            $element_name_test_empty_string, false);
?>
