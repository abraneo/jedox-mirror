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
	$db_name = "Demo";
	$dimension = "Products";	
	$cube_name = "Sales";
	
	// parses the rule "['Budget']=['Actual']"
	palo_cube_rule_parse(CONN . '/' . $db_name, $cube_name, $definition);
?>
