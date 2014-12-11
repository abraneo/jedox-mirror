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
	$dimension_name = 'Products';
	// element name which does not exist
	$element_name_test_empty_string = 'Products234';
	
	
	
	// the empty string parameter hides the error, if true
	palo_ecount(CONN . '/' . $db_name, $element_name_test_empty_string, true);

	// if the empty string parameter is set to false the error
	// will be shown
	palo_ecount(CONN . '/' . $db_name, $element_name_test_empty_string, false);
	
	// return the number of elements in specified dimension	
	palo_ecount(CONN . '/' . $db_name, $dimension_name, true);
	
	// return the number of elements in specified dimension	
	palo_ecount(CONN . '/' . $db_name, $dimension_name);
	
?>

