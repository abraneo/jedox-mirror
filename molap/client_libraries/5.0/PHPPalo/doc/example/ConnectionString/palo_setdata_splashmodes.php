<?php
	
	// set data as specified in array dimension without splashing
	palo_setdata('20', 'SPLASH_MODE_NONE', CONN . '/' . $db_name,
				  $cube_name, $coordinate1, $coordinate2, $coordinate3, 
				  $coordinate4, $coordinate5, $coordinate6);
	
	// another sample to set the data without splashing
	palo_setdata('20', false, CONN . '/' . $db_name, $cube_name,
				 $coordinate1, $coordinate2, $coordinate3, 
				 $coordinate4, $coordinate5, $coordinate6);

	
	// overwrites existing value with new value
	palo_setdata('20', 'SPLASH_MODE_DEFAULT', CONN . '/' . $db_name,
				  $cube_name, $coordinate1, $coordinate2, $coordinate3, 
				  $coordinate4, $coordinate5, $coordinate6);
	
	
	// another sample to overwrite existing value with new value
	palo_setdata('|20', true, CONN . '/' . $db_name, $cube_name, 
				 $coordinate1, $coordinate2, $coordinate3,
				 $coordinate4, $coordinate5, $coordinate6);


	// overwrites values on basis elements
	palo_setdata('20', 'SPLASH_MODE_SET', CONN . '/' . $db_name,
				  $cube_name, $coordinate1, $coordinate2, $coordinate3,
				  $coordinate4, $coordinate5, $coordinate6);

	// another sample to overwrite values on basis elements
	palo_setdata('!20', true, CONN . '/' . $db_name, $cube_name,
				 $coordinate1, $coordinate2, $coordinate3, 
				 $coordinate4, $coordinate5, $coordinate6);


	// adds value on basis elements
	palo_setdata('20', 'SPLASH_MODE_ADD', CONN . '/' . $db_name, 
				  $cube_name, $coordinate1, $coordinate2, $coordinate3, 
				  $coordinate4, $coordinate5, $coordinate6);

	// another sample to add values on basis elements
	palo_setdata('!!20', true, CONN . '/' . $db_name, $cube_name, 
				 $coordinate1, $coordinate2, $coordinate3,
				 $coordinate4, $coordinate5, $coordinate6);

?>