<?php
 
 	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init(HOST, PORT, USER, PASS);	
	
	// create variables as follows
	$db_name = 'Demo';
	$dimension_name = 'Products';	
	$element_name = 'Monitors';
	// element name which does not exist
	$element_name_test_empty = 'Minotors';
			
	// retrieves the name of the 1st childelement of the consolidated 
	// fatherelement Monitors in dimension Products
	palo_echildname($connection_id, $db_name, $dimension_name,
	               $element_name,  1); 

	// retrieves the name of the 1st childelement of the consolidated 
	// fatherelement Monitors in dimension Products
	palo_echildname($connection_id, $db_name, $dimension_name, 
	               $element_name,  1, true); 

	// the empty string parameter hides the error, if set to true
	palo_echildname($connection_id, $db_name, $dimension_name, 
	               $element_name_test_empty,  1, true); 

	// if empty string parameter is set to false the error will be shown
	palo_echildname($connection_id, $db_name, $dimension_name,
	               $element_name_test_empty,  1, false); 
	
	// disconnect the connection to server
	palo_disconnect($connection_id);
?>
