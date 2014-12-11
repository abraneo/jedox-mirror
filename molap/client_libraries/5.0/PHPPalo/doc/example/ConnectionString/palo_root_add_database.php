<?php
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);
	
	// define the variable db_name
	$db_name = 'DemoTEST';
	$db_name = 'DemoTEST2';
	
	// add database DemoTEST as a Normal database
	palo_root_add_database(CONN . '/' . $db_name, 0);

	// add database DemoTEST2 as a Userinfo database
	palo_root_add_database(CONN . '/' . $db_name2, 3);
?>
