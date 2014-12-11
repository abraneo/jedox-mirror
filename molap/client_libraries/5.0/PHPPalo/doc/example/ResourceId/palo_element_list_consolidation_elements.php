<?php
	
   	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server	
	$connection_id = palo_init( HOST, PORT, USER, PASS);	
		
	// define the varables as follows
	$db_name = 'Demo';
	$dimension_name = 'Products';	
	$element_name = 'Monitors'; 
	
	// list of associative arrays ("name", "type", "id", "factor"), or error.
	palo_element_list_consolidation_elements( $connection_id, $db_name,
									   $dimension_name, $element_name );

	// disconnect the connection to server
	palo_disconnect($connection_id); 

?>
