<?php
	
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);

	// create variables as follows
	$definition = "['Budget']=['Actual']";
	$cube_name = "Sales";
	$db_name = "Demo";
	
	// create a new cube rule using the given defintion
	palo_cube_rule_create(CONN . '/' . $db_name, $cube_name, $definition);
?>
