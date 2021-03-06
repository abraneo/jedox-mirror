<?php

// define the constants
define('CONN', 'LOCALHOST');
define('HOST', '127.0.0.1');
define('PORT', '7777');
define('USER', 'admin');
define('PASS', 'admin');

// registers a name for a PALO server
$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);

// create variables as follows
$db_name = 'Demo';
$dimension = 'Products';	

// Change type of element 'Desktop L' from numeric to string
palo_eupdate(CONN . '/' . $db_name, $dimension, 'Desktop L', 'S', null);

// replace all childrens of 'Monitors' with new ones
palo_eupdate(CONN . '/' . $db_name, $dimension, 'Monitors', 'C', 
	array(array('TFT Monitor XA', 1), array('TFT Monitor TL', 1)), 0);

// add more childrens to 'Monitors'...
palo_eupdate(CONN . '/' . $db_name, $dimension, 'Monitors', 'C', 
	array(array('Desktop L', -1), array('Desktop Pro', -3)), 1);

// delete the newly added elements from 'Monitors'
palo_eupdate(CONN . '/' . $db_name, $dimension, "Monitors", "C", 
	array('Desktop L','Desktop Pro'), 2)

?>