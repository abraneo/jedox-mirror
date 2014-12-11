<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init(HOST, PORT, USER, PASS);
	
	// create the variables db_name and cube_name
	$db_name = 'Demo';
	$cube_name = 'Sales';
	
	/* first, initalize the cache by calling palo_startcachcollect()
	now the following palo_datatc() calls are being collected */
	palo_startcachecollect();  		
	palo_datac($connection_id, $db_name, $cube_name, 'Desktop L', 'Germany', 
			  'Jan', '2002', 'Budget', 'Units');

	/* use the palo_endcachecollect() function to
	send all collected functions to the Palo server	*/		  
	palo_endcachecollect();
	
	/* now run every palo_datac() again to
	retrieve the result for each function call */
	palo_datac($connection_id, $db_name, $cube_name, 'Desktop L', 'Germany', 
			  'Jan', '2002', 'Budget', 'Units');  
	
	// disconnect the connection to server
	palo_disconnect($connection_id);
?>
