<?php
  
	error_reporting(E_ALL);

	define('CONN', 'LOCALHOST');
	define('HOST', 'localhost');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');

	$conn = palo_register_server(CONN, HOST, PORT, USER, PASS);


	$db = "demo";
	$cube = "Sales";
	$path = array("All Products","Europe","Year","2010","Variance","Units");
	$value = 5000;
	$area = array(array("All Products"),array("Europe"),array("Year"),
				  array("2010","2011"),array("Variance"),array("Units"));


	// goal seek with complete
	$rc = palo_goal_seek($conn, $db, $cube, $path, $value, PALO_GOALSEEK_COMPLETE);

	// goal seek with equal
	$rc = palo_goal_seek($conn, $db, $cube, $path, $value, 
						 PALO_GOALSEEK_EQUAL, $area);

	// goal seek with relative
	$rc = palo_goal_seek($conn, $db, $cube, $path, $value, 
						 PALO_GOALSEEK_RELATIVE, $area);

	echo "done"
?>
