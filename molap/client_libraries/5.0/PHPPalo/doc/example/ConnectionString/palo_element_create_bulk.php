<?php
		
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);	
	
	// create the variables
	$database = 'Demo';
	$dimension = 'Years';	
	
	$c_elements = array('All Years');
	$n_elements = array('2002','2003','2004','2005','2006','2007','2008','2009');
	$weight = array(array(1,1,1,1,1,1,1,1));
	
	// clear the dimension
	palo_dimension_clear( CONN.'/'.$database, $dimension );

	// create the elements as defined in $n_elements
	palo_element_create_bulk( CONN.'/'.$database, $dimension, $n_elements, 'N' );	
	
	// create the C element 'All Years' with the elements in $n_elements as childrens
	palo_element_create_bulk( CONN.'/'.$database, $dimension, $c_elements, 'C', array($n_elements), $weight );	
?>
