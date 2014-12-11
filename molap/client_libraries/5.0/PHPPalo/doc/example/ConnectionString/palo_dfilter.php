<?php
  
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);

	// Retrieve a subset of dimension elements with data filter
	palo_subset(CONN . "/" . "Demo","Products",1,NULL,palo_dfilter(
	palo_subcube("Sales",NULL,"Germany","Jan","2002","Budget","Units"),
	array(">","50000")));
?>
