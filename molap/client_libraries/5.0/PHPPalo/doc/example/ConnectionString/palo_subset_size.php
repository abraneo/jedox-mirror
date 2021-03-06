<?php
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);

	// Retrieve the number of elements in subset 
	palo_subsetsize(CONN . '/' . "Demo", "Products",1,NULL,NULL,
	palo_tfilter(array("D*"),false),
	NULL,NULL,NULL,palo_sort(1,0,NULL,0,NULL,0,1));
?>