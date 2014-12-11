<?php
  
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection_id_id = palo_init( HOST, PORT, USER, PASS);	

	// Retrieve a subset with textfilter. Retrieve all products starts with "D"
	// (DOS filter kriteria)
	palo_subset( $connection_id_id, "Demo", "Products", 1, NULL,
	palo_tfilter(array("D*"),false)); 

	// Retrieve a subset with textfilter. Retrieve all months ends with "n"
	//(Regex filter kriteria)
	palo_subset($connection_id_id, "Demo", "Months", 1, NULL,
	palo_tfilter(array("n$"),true));

	// disconnect the connection to server
	palo_disconnect($connection_id_id); 
?>