<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id_id = palo_init(HOST, PORT, USER, PASS);	
	
	// create the variables db_name and cube_name
	$db_name = 'Demo';
	$cube_name = 'Sales';
	
	/* create the array dimension with the coordinates
	the last one is missing - we will add it later */
	$dimension1 = array(
					   	'Desktop L',
					   	'Germany',
					   	'Jan',
					   	'2002',
					   	'Budget',
					   	'Units'
					   );
	
	$dimension2 = array(
					   	'Desktop L',
					   	'Germany',
					   	'Jan',
					   	'2002',
					   	'Budget',
					   	'Gross Profit'
					   );   			
	
	/* first, initalize the cache by calling palo_startcachcollect()
	now the following palo_dataatc() calls are being collected */
	palo_startcachecollect();

	palo_dataatc($connection_id_id, $db_name, $cube_name, $dimension1);
	palo_dataatc($connection_id_id, $db_name, $cube_name, $dimension2);

	/* use the palo_endcachecollect() function to
	send all collected functions to the Palo server	*/	
	palo_endcachecollect();
	
	/* now run every palo_dataatc() again to
	retrieve the result for each function call */
	palo_dataatc($connection_id_id, $db_name, $cube_name, $dimension1);
	palo_dataatc($connection_id_id, $db_name, $cube_name, $dimension2);
	
	// disconnect the connection to server
    palo_disconnect($connection_id_id); 
	
	
?>

