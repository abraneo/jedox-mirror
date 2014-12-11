<?php
	
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);


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
	palo_getdata_export(CONN . '/' . 'Demo', 'Sales', TRUE, TRUE, 200,
						15000, 'gte', 'lt', 'and', 10, NULL, $dimensions);

	// Exports data as specified in function
	palo_getdata_export(CONN . '/' . 'Demo', 'Sales', TRUE, TRUE, 
					 	">=200 AND <15000", 10, NULL, $dimensions);
?>
