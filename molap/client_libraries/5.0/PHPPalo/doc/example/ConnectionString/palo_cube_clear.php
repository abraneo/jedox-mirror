<?php

	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');

	// registers a name for a PALO server   
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);

	// define the variables
	$db_name = "Demo";
	$cube_name = "Sales";

	// definition of the element list
	$element_list = array(
	   array('Desktop L'),
	   array('Germany', 'East'),
	   array('Jan'),
	   array('2002', '2004'),
	   array('Budget'),
	   array('Units')
	 );

	// clear the whole cube
	palo_cube_clear(CONN . '/' . $db_name, $cube_name);

	// clear only the given elements
	palo_cube_clear(CONN . '/' . $db_name, $cube_name, $element_list);
?>
