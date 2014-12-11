<?php
	
	// create the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init(HOST, PORT, USER, PASS);	
	
	// create the variables as follows
	$db_name = 'Demo';
	$dimension = 'Years';
		
	// clear all C elements and add element '2009' without a parent
	palo_eadd($connection_id, $db_name, $dimension, 'N', '2009', '', 1, 2);

	// clear the dimension 'Years' and add a numeric element with 'All Years' as parent
	// if parent doesn´t exists, it will be created
	palo_eadd($connection_id, $db_name, $dimension, 'N', '2010', 'All Years', 1, 1);

	// add another element and delete nothing
	palo_eadd($connection_id, $db_name, $dimension, 'N', '2011', '', 1, 0);

	// disconnect the connection to server
	palo_disconnect($connection_id);
?>

