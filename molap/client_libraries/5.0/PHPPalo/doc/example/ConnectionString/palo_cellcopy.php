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
	$from = array("All Products","Europe","Year","2010","Variance","Units");
	$to = array("All Products","Europe","Year","2011","Variance","Units");
	$value = 5000;
	$userules = true;

	// copy like with userules
	$rc = palo_cellcopy($conn, $db, $cube, $from, $to, $value, $userules);

	// copy with userules
	$rc = palo_cellcopy($conn, $db, $cube, $from, $to, $userules);

	// copy like with userules = false
	$rc = palo_cellcopy($conn, $db, $cube, $from, $to, $value);

	// copy with userules = false
	$rc = palo_cellcopy($conn, $db, $cube, $from, $to);

	echo "done"
?>