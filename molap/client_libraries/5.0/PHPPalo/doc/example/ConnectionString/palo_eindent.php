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
	$dimension = 'Products';	 	
	$element_name = 'Server Dual C'; 
	$element_name_test_empty_string = 'Server Dasfual Cgfg';
	 		
	// retrieves that the indentation of Products is 3
	palo_eindent(CONN . '/' . $db_name, $dimension, $element_name);  

	// retrieves that the indentation of Products is 3
	palo_eindent(CONN . '/' . $db_name, $dimension, $element_name, true);  

	// the empty string parameter hides the error, if set to true
	palo_eindent(CONN . '/' . $db_name, $dimension, 
	                  $element_name_test_empty_string, true);  

	// if empty string parameter is set to false the error will be shown
	palo_eindent(CONN . '/' . $db_name, $dimension, 
					  $element_name_test_empty_string, false);  
?>
	
