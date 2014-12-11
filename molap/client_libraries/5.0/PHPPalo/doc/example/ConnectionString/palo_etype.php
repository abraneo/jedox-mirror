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
	$dimension_name = 'Products';	 	
	$element_name = 'Server Dual C';
	$element_name_test_empty_string = 'Servdfger Dual C';
	
	// returns the type of element Server Dual C		
	palo_etype(CONN . '/' . $db_name, $dimension_name, $element_name);

	// returns the type of element Server Dual C		
	palo_etype(CONN . '/' . $db_name, $dimension_name, $element_name, true);

	// the empty string parameter hides the error, if set to true		
	palo_etype(CONN . '/' . $db_name, $dimension_name, 
			   $element_name_test_empty_string, true);

	// if empty string parameter is set to false the error will be shown		
	palo_etype(CONN . '/' . $db_name, $dimension_name, 
			   $element_name_test_empty_string, false);
?>
