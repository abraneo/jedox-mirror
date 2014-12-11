<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init( HOST, PORT, USER, PASS);	
	
	// create the variables db_name, dimension and element
	$db_name = 'Demo';
	$dimension_name = 'Products';	 	
	$element_name = 'Server Dual C';
	$element_name_test_empty_string = 'Servsghser Dual C';
	
	// retrieves the amount of parents of the 'element Server Dual C'
	palo_eparentcount($connection_id, $db_name, $dimension_name, $element_name);   

	// the empty string parameter hides the error, if set to true
	palo_eparentcount($connection_id, $db_name, $dimension_name,
					  $element_name_test_empty_string, true); 

	// if empty string parameter is set to false the error message will be shown
	palo_eparentcount($connection_id, $db_name, $dimension_name,
					  $element_name_test_empty_string, false);  

	// disconnect the connection to server
	palo_disconnect($connection_id); 
?>
