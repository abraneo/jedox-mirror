<?php
  
	error_reporting(E_ALL);

	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection_id_id = palo_init(HOST, PORT, USER, PASS);


	$db = "demo";
	$cube = "Sales";
	$path = array("All Products","Europe","Year","2010","Variance","Units");
	$value = 5000;
	$area = array(array("All Products"),array("Europe"),array("Year"),
				  array("2010","2011"),array("Variance"),array("Units"));


	// goal seek with complete
	$rc = palo_goal_seek($connection_id_id, $db, $cube, $path,
						 $value, PALO_GOALSEEK_COMPLETE);

	// goal seek with equal
	$rc = palo_goal_seek($connection_id_id, $db, $cube, $path,
						 $value, PALO_GOALSEEK_EQUAL, $area);

	// goal seek with relative
	$rc = palo_goal_seek($connection_id_id, $db, $cube, $path,
						 $value, PALO_GOALSEEK_RELATIVE, $area);

	echo "done"
?>
