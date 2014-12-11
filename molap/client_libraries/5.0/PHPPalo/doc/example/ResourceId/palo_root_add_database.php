<?php
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id_id = palo_init( HOST, PORT, USER, PASS);	
	
	// define the variable db_name
	$db_name = 'DemoTEST';
	$db_name = 'DemoTEST2';
	
	// add database DemoTEST
	palo_root_add_database($connection_id_id, $db_name);

	// add database DemoTEST2 as a Userinfo database
	palo_root_add_database($connection_id_id, $db_name2, 3);

	// disconnect the connection to server
	palo_disconnect($connection_id_id); 
?>
