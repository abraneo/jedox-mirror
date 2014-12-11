<?php
 	
 	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);
	
	//create variables as follows
	$db_name = 'Demo';
	$dimension_name = 'Products';	
	$element_name = 'Monitors'; 
	$element_name_test_empty_string = 'Minotors'; 
	
	// Count children elements from motherelement Monitors in dimension Products
	palo_echildcount(CONN . '/' . $db_name, $dimension_name, $element_name);

	//Hide the error message if empty string parameter is set to true
	palo_echildcount(CONN . '/' . $db_name, $dimension_name,
					 $element_name_test_empty_string, true);

	//Show the error message if empty string parameter is set to false
	palo_echildcount(CONN . '/' . $db_name, $dimension_name,
					 $element_name_test_empty_string, false);
?>

