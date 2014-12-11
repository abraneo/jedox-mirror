<?php
	
	// define the constants 
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);
	
	// create the variables as follows
	$db_name = 'Demo';
	$dimension_name = 'Products';	
	$element_name = 'Desktop L';   
	 		
	// moves element Desktop L to the specified position in dimension Products
	palo_emove(CONN . '/' . $db_name, $dimension_name, $element_name, 2);  	  

?>
