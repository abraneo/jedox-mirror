<?php
	
	// define the constanst	
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server
	$connection_id_id = palo_init(HOST, PORT, USER, PASS);
				
	// create the variables db_name and cube_name
	$db_name = 'Demo';
	$cube_name = 'Sales';
	
	// return specified data
	palo_dataav($connection_id_id, $db_name, $cube_name, 
				 array("All Products", 
				 array(5, 1, "Europe", "West", "East", "South", "North"), 
				 array(1, 5, "Year", "Qtr. 1", "Qtr. 2", "Qtr. 3", "Qtr. 4"), 
				 "2002", "Budget", "Units"));
	
	// disconnect the connection to server
	palo_disconnect($connection_id);
  ?>

