<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init( HOST, PORT, USER, PASS);	

	// create the variables db_name, dimension, element and fatherelement
	$db_name = 'Demo';
	$dimension_element = 'Products';	 	
	$child_element = 'TFT Monitor TL';    
	$consolidation_element = 'Monitors'; 
	$consolidation_element_test_empty_string = 'Minotors';
	 
	/* Returns the consolidation factor of element 
	TFT Monitor TL consolidated with element Monitors */	
	palo_eweight($connection_id, $db_name, $dimension_element, 
				 $consolidation_element, $child_element);   

	/* Returns the consolidation factor of element 
	TFT Monitor TL consolidated with element Monitors */	
	palo_eweight($connection_id, $db_name, $dimension_element, 
				 $consolidation_element, $child_element, true);  

	// the empty string parameter hides the error, if set to true
	palo_eweight($connection_id, $db_name, $dimension_element, 
				 $consolidation_element_test_empty_string, $child_element, true);  

	// if empty string parameter is set to false the error will be shown
	palo_eweight($connection_id, $db_name, $dimension_element, 
				 $consolidation_element_test_empty_string, $child_element, false);
				 
	// disconnect the connection to server
	palo_disconnect($connection_id); 
?>

