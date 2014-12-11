<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection_id = palo_init(HOST, PORT, USER, PASS);
	
	// create variables as follows
	$definition = "['Budget']= ['Budget']+['Actual']";
	$new_definition = "['Budget']=['Actual']";
	$db_name = 'Demo';
	$cube_name = 'Sales';
	$comment = 'Change the definition';
	
	// create a new rule
	$id = palo_cube_rule_create($connection_id, $db_name, $cube_name, $definition);

	// modify the definition and comment of the just now created rule
	palo_cube_rule_modify($connection_id, $db_name, $cube_name, $id['identifier'],
	                     $new_definition, $id['extern_id'], $comment, true);
	
	
	// disconnect the connection to server
    palo_disconnect($connection_id); 
	

?>
