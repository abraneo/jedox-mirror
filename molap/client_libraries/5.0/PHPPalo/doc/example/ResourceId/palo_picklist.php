<?php
  
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection_id_id = palo_init( HOST, PORT, USER, PASS);	
	
	// Add defined elements in palo_picklist at the beginning of the 
	// subset with data filter (elements with vlaue greater than 50000)
	echo "<pre>";
	print_r(palo_subset($connection_id_id, "Demo","Products",1,NULL,
	palo_picklist(array("Desktop High XL","Desktop Pro XL"),0),
	palo_dfilter(palo_subcube("Sales",NULL,"Germany","Jan","2002",
				 "Budget","Units"),array(">","50000"))));

	// disconnect the connection to server
	palo_disconnect($connection_id_id); 
?>