<?php
	
	// define the constants 
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);

	// create the variables as follows
	$db_name = 'Demo';
	$dimension_name = 'Products';	
	$element_name_test_emty_string = 'Desktopasd L';
	$new_element_name = 'Desktop XXXL';    
	$element_name = 'Desktop L';
	 		
	// rename element Desktop L into Desktop XXL
	palo_erename(CONN . '/' . $db_name, $dimension_name,
				 $element_name, $new_element_name);

	// rename element Desktop L into Desktop XXL
	palo_erename(CONN . '/' . $db_name, $dimension_name, 
				 $element_name, $new_element_name, true);

	// the empty string parameter hides the error, if set to true
	palo_erename(CONN . '/' . $db_name, $dimension_name,
				 $element_name_test_emty_string, $new_element_name, true);

	// if empty string parameter is set to false the error will be shown 
	palo_erename(CONN . '/' . $db_name, $dimension_name,
				 $element_name_test_emty_string, $new_element_name, false);
?>
