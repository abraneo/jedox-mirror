<?php
  
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);

	// read server info	
	$si = palo_server_info(CONN);

	// read user name
	palo_get_user_for_sid(CONN, $si['sid']);

	// disconnect
    palo_remove_connection(CONN); 
?>
