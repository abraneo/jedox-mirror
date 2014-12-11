<?php
	
	// define the constance
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init( HOST, PORT, USER, PASS);	
	
	// create the varables db_name, dimension, element_name, fatherelement
	$db_name = 'Demo';
	$dimension = 'Products';	 	
	$element_name = 'Desktop L';
	$parent_element = 'Stationary PC\'s';
	$parent_element_test_empty_string = 'Minotors';
	
	
	
	// check if the element Desktop L is a childelement of element Monitors
	palo_eischild($connection_id, $db_name, $dimension,  
				  $parent_element, $element_name);

	// check if the element Desktop L is a childelement of element Monitors
	palo_eischild($connection_id, $db_name, $dimension,  
				  $parent_element, $element_name, true);

	// Hide the error message if empty string parameter is set to true
	palo_eischild($connection_id, $db_name, $dimension,  
				  $parent_element_test_empty_string, $element_name, true);

	// Show the error message if empty string parameter is set to false
	palo_eischild($connection_id, $db_name, $dimension,  
				  $parent_element_test_empty_string, $element_name, false);

	// disconnect the connection to server
	palo_disconnect($connection_id); 

?>
