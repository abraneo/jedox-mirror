<?php
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');

	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);	

	// create the variables db_name, cube_name and element
	$db_name = 'Demo';
	$dimension = 'Years';  
	$element = '2005';
	$element_test_empty_string = '2023';

	// delete element 2005 in dimension Years
	palo_edelete(CONN . '/' . $db_name, $dimension, 
		$element);

	// delete element 2005 in dimension Years
	palo_edelete(CONN . '/' . $db_name, $dimension, 
		$element, true);

	// the empty string parameter hides the error, if set to true
	palo_edelete(CONN . '/' . $db_name, $dimension, 
		$element_test_empty_string, true);

	// if empty string parameter is set to false the error will be shown
	palo_edelete(CONN . '/' . $db_name, $dimension, 
		$element_test_empty_string, false);
?>
