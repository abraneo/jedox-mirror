<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init(HOST, PORT, USER, PASS);	
	
	// create the variables db_name and dimension
	$db_name = 'Demo';
	$dimension_name = 'Products';	
	$dimension_name_test_empty_string = 'Pooooducts';	
	
	//Retrieves the first element from the dimension Products	
	palo_efirst($connection_id, $db_name, $dimension_name);  

	//Retrieves the first element from the dimension Products	
	palo_efirst($connection_id, $db_name, $dimension_name, true);  

	//Hide the error message if empty string parameter is set to true
	palo_efirst($connection_id, $db_name, $dimension_name_test_empty_string, true);  

	//Show the error message if empty string parameter is set to false
	palo_efirst($connection_id, $db_name, $dimension_name_test_empty_string, false); 
	
	// disconnect the connection to server
	palo_disconnect($connection_id);
?>
 
