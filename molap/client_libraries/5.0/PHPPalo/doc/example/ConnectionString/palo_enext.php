<?php
	
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);	
	
	// create the variables db_name, dimension and element
	$db_name = 'Demo';
	$dimension_name = 'Years';	 	
	$element_name = '2006'; 
	// element name which does not exist
	$element_name_test_empty_string = '2060';
	
	// retrieves the next elment  
	palo_enext(CONN . '/' . $db_name, $dimension_name, $element_name);
	
	// retrieves the next elment  
	palo_enext(CONN . '/' . $db_name, $dimension_name, $element_name); 	

	// the empty string parameter hides the error, if set to true 
	palo_enext(CONN . '/' . $db_name, $dimension_name, 
	           $element_name_test_empty_string, true); 	

	// if empty string parameter is set to false the error will be shown
	palo_enext(CONN . '/' . $db_name, $dimension_name, 
	           $element_name_test_empty_string, false); 	
?>
