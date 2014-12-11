<?php
   
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);	
	
	// define the varables as follows
	$db_name = 'Demo';
	
	// retrieves all elements contained in the specified dimension 
	palo_dimension_list_elements2(CONN . '/' . $db_name, 'Years');
	
	// the same as above, but elements with insufficient rights will sorted out
	palo_dimension_list_elements2(CONN . '/' . $db_name, 'Years', true);
?>

