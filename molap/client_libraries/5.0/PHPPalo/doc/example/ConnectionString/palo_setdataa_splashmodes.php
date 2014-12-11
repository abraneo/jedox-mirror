<?php
	
	// set data as specified in array dimension without splashing
	palo_setdataa('20', 'SPLASH_MODE_NONE', CONN . '/' . $db_name,
				  $cube_name, $coordinates);
	
	// another sample to set the data without splashing
	palo_setdataa('20', false, CONN . '/' . $db_name, $cube_name, $coordinates);

	
	// overwrites existing value with new value
	palo_setdataa('20', 'SPLASH_MODE_DEFAULT', CONN . '/' . $db_name,
				  $cube_name, $coordinates);

	// another sample to overwrite existing value with new value
	palo_setdataa('|20', true, CONN . '/' . $db_name, $cube_name, $coordinates);


	// overwrites values on basis elements
	palo_setdataa('20', 'SPLASH_MODE_SET', CONN . '/' . $db_name,
				  $cube_name, $coordinates);

	// another sample to overwrite values on basis elements
	palo_setdataa('!20', true, CONN . '/' . $db_name, $cube_name, $coordinates);


	// adds value on basis elements
	palo_setdataa('20', 'SPLASH_MODE_ADD', CONN . '/' . $db_name, 
				  $cube_name, $coordinates);

	// another sample to add values on basis elements
	palo_setdataa('!!20', true, CONN . '/' . $db_name, $cube_name, $coordinates);

?>