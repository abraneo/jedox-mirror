<?php
  
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection_id_id = palo_init( HOST, PORT, USER, PASS);	

	// create the filter as follows
	$hfilter = palo_hfilter("Stationary PC's",FALSE,FALSE,2,NULL,NULL,NULL,2,3);
	$tfilter = palo_tfilter(array("D*"),false);
	$afilter = palo_afilter(array(2,1,"Price Per Unit",">500000"));
	$subcube = palo_subcube("Sales",NULL,"Germany","Jan","2005","Budget",
							"Gross Profit");
	$dfilter = palo_dfilter($subcube,array(">","5000"),5,NULL,NULL,0);
	$picklist = palo_picklist(array("Notebook SX","Notebook GT","Notebook LXC"
									,"Notebook TT","Notebook SL","Subnote SL",
									"Subnote XK"),0);
	$palo_sort = palo_sort(1,0,NULL,0,NULL,2,1);


	// subset function with all filter possibilities
	palo_subset($connection_id_id, "Demo", "Products", 1, NULL, $dfilter, $afilter,
				$picklist, $hfilter, $tfilter,$palo_sort);

	// disconnect the connection to server
	palo_disconnect($connection_id_id); 
?>
    