<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection_id = palo_init(HOST, PORT, USER, PASS);
	
	// create variables as follows
	$definition = "['Budget']=['Actual'] +['Actual']";
	$db_name = 'Demo';
	$cube_name = 'Sales';
	
	// create a rule
	$retval = palo_cube_rule_create(	
							  $connection_id, $db_name, 
							  $cube_name,
							  $definition
						 );
		
	// delete the rule we created above
	palo_cube_rule_delete(  $connection_id, $db_name,
							$cube_name,
							$retval['identifier']
						 );
	
	// disconnect the connection to server
    palo_disconnect($connection_id); 
	
?>
