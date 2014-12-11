<?php
	
	// set data as specified in array dimension without splashing
	palo_setdataa('20', 'SPLASH_MODE_NONE', $connection_id, $db_name,
				  $cube_name, $coordinates);
	
	// another sample to set the data without splashing
	palo_setdataa('20', false, $connection_id, $db_name, $cube_name, $coordinates);

	
	// overwrites existing value with new value
	palo_setdataa('20', 'SPLASH_MODE_DEFAULT', $connection_id, $db_name,
				  $cube_name, $coordinates);

	// another sample to overwrite existing value with new value
	palo_setdataa('|20', true, $connection_id, $db_name, $cube_name, $coordinates);


	// overwrites values on basis elements
	palo_setdataa('20', 'SPLASH_MODE_SET', $connection_id, $db_name,
				  $cube_name, $coordinates);

	// another sample to overwrite values on basis elements
	palo_setdataa('!20', true, $connection_id, $db_name, $cube_name, $coordinates);


	// adds value on basis elements
	palo_setdataa('20', 'SPLASH_MODE_ADD', $connection_id, $db_name, 
				  $cube_name, $coordinates);

	// another sample to add values on basis elements
	palo_setdataa('!!20', true, $connection_id, $db_name, $cube_name, $coordinates);

	// disconnect the connection to server
	palo_disconnect($connection_id_id); 

?>