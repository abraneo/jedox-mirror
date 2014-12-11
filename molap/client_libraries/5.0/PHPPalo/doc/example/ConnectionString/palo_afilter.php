<?php
  
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);
  $db_name = "Demo";

	// attribute filter removes elements, 
	// which do not match with the defined attribut rows
	palo_subset(CONN . '/' . $db_name,"Products",1,NULL,NULL,NULL,
	palo_afilter(array(2,1,"Price Per Unit",">500")),NULL,NULL,
	palo_sort(0,0,NULL,0,NULL,NULL,0));
?>
