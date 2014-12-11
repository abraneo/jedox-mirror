<?php       
  
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id_id = palo_init( HOST, PORT, USER, PASS);	

	// define the variable db_name
	$db_name = 'DemoTest';
	
	// delete database demoTest
	palo_root_delete_database($connection_id_id, $db_name);

	// disconnect the connection to server
	palo_disconnect($connection_id_id); 
?>