<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server  
    $connection_id = palo_init(HOST, PORT, USER, PASS);

	// create variables as follows
	$db_name = 'Demo';	
	$cube_name = 'Sales';
	
	// retrieves excisting rules of the specified cube
	palo_cube_rules($connection_id, $db_name,$cube_name);


?>
