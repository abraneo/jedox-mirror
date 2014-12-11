<?php
	
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);	
	
	// List all databases available on the specified server
	palo_root_list_databases(CONN); 

	// List all databases including the System database
	palo_root_list_databases(CONN, true); 

	// List all databases including the System and UserInfo databases
	palo_root_list_databases(CONN, true, true); 
	
?>
