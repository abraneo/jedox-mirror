<?php
	
	// define the constants	 
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);

	// create the variables db_name, dimension
	$db_name = 'Demo';
	$dimension_name = 'Products';	
	 		
	// retrieves the element at the 2nd position of dimension Products
	palo_ename(CONN . '/' . $db_name, $dimension_name, 2);
?>
