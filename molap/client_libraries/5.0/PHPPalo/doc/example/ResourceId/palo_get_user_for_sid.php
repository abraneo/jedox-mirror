<?php
  
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection_id_id = palo_init(HOST, PORT, USER, PASS);	

	// read server info	
	$si = palo_server_info($connection_id_id);

	// read user name
	palo_get_user_for_sid($connection_id_id, $si['sid']);

	// disconnect
    palo_disconnect($connection_id_id); 
?>
