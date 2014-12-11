<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection_id = palo_init(HOST, PORT, USER, PASS);
	
	// create variables as follows
	$definition = "['Budget']=['Actual']";
	$cube_name = 'Sales';
	$db_name = 'Demo';
	
	// create a new cube rule using the given defintion
	palo_cube_rule_create(	
							$connection_id, $db_name, 
							$cube_name,
							$definition
						 );
	
	// disconnect the connection to server
    palo_disconnect($connection_id); 
	
?>
