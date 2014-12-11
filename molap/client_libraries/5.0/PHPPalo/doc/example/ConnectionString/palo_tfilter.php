<?php
  
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);

	// Retrieve a subset with textfilter. Retrieve all products starts with "D"
	// (DOS filter kriteria)
	palo_subset(CONN.'/'."Demo","Products",1,NULL,
	palo_tfilter(array("D*"),false)); 

	// Retrieve a subset with textfilter. Retrieve all months ends with "n"
	//(Regex filter kriteria)
	palo_subset(CONN.'/'."Demo","Months",1,NULL,
	palo_tfilter(array("n$"),true));
?>