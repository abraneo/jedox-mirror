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
	$array_elements = array('2002', '2003', '2004');
	$array_element_test_empty_string = array('2023','2019');

	// delete elements '2002', '2003', '2004' in dimension Years
	palo_edelete_bulk(CONN . '/' . $db_name, $dimension, 
		$array_elements);

	// delete element '2002', '2003', '2004' in dimension Years
	palo_edelete_bulk(CONN . '/' . $db_name, $dimension, 
		$array_elements, true);

	// the empty string parameter hides the error, if set to true
	palo_edelete_bulk(CONN . '/' . $db_name, $dimension, 
		$array_element_test_empty_string, true);

	// if empty string parameter is set to false the error will be shown
	palo_edelete_bulk(CONN . '/' . $db_name, $dimension, 
		$array_element_test_empty_string, false);

?>
