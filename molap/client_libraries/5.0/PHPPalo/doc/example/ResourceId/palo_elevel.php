<?php
	
	// defines the constants 
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init( HOST, PORT, USER, PASS);	
	
	// create variables as follows
	$db_name = 'Demo';
	$dimension_name = 'Products';	 	
	$element_name = 'Monitors'; 
	$element_name_test_empty_string = 'Monitasdfors'; 
	
	// Retrieves the level of the element Monitors in the consolidation hierarchy	
	palo_elevel($connection_id, $db_name, $dimension_name,  $element_name);


	// the empty string parameter hides the error, if set to true
	palo_elevel($connection_id, $db_name, $dimension_name,  
	                  $element_name_test_empty_string, true);

	// if empty string parameter is set to false the error will be shown
	palo_elevel($connection_id, $db_name, $dimension_name, 
                      $element_name_test_empty_string, false);

	// disconnect the connection to server
	palo_disconnect($connection_id); 
?>
