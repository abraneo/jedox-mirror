<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id_id = palo_init( HOST, PORT, USER, PASS);	


	// create array dimensions with coordinates
	$dimensions = array(
					    'Desktop L',
					    'Germany',
					    'Jan',
					    '2002',
					    'Budget',
					    'Units'
					   );
	
	// Exports data as specified in function
	palo_getdata_export($connection_id_id, 'Demo', 'Sales', TRUE, TRUE, 200,
						15000, 'gte', 'lt', 'and', 10, NULL, $dimensions);

	// Exports data as specified in function
	palo_getdata_export($connection_id_id, 'Demo', 'Sales', TRUE, TRUE, 
					 	">=200 AND <15000", 10, NULL, $dimensions);

	// disconnect the connection to server
	palo_disconnect($connection_id)_id; 
?>
