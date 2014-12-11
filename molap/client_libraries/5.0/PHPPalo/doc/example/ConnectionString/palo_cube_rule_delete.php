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
	$definition = "['Budget']=['Actual'] +['Actual']";
	$db_name = "Demo";
	$cube_name = "Sales";
	
	// create a rule
	$retval = palo_cube_rule_create(CONN . '/' . $db_name, $cube_name, $definition );
		
	// delete the rule we created above
	palo_cube_rule_delete( CONN . '/' . $db_name, $cube_name, $retval['identifier']);
?>
