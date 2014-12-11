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
	$dimension_name = 'Products';	
	$element_name = 'Monitors'; 
	
	// list of associative arrays ("name", "type", "id", "factor"), or error.
	palo_element_list_consolidation_elements( CONN . '/' . $db_name,
									   $dimension_name, $element_name );

?>
