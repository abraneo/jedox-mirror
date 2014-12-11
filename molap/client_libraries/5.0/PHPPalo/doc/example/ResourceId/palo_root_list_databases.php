<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id_id = palo_init( HOST, PORT, USER, PASS);	
	
	// List all databases available on the specified server
	palo_root_list_databases($connection_id_id); 

	
	// List all databases including the System database
	palo_root_list_databases($connection_id_id, true); 

	// List all databases including the System and UserInfo databases
	palo_root_list_databases($connection_id_id, true, true);

	// disconnect the connection to server
	palo_disconnect($connection_id_id); 	
?>
