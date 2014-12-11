<?php
	
	// set data as specified in array dimension without splashing
	palo_setdata('20', 'SPLASH_MODE_NONE', $connection_id_id, $db_name,
				  $cube_name, $coordinate1, $coordinate2, $coordinate3, 
				  $coordinate4, $coordinate5, $coordinate6);
	
	// another sample to set the data without splashing
	palo_setdata('20', false, $connection_id_id, $db_name, $cube_name,
				 $coordinate1, $coordinate2, $coordinate3, 
				 $coordinate4, $coordinate5, $coordinate6);

	
	// overwrites existing value with new value
	palo_setdata('20', 'SPLASH_MODE_DEFAULT', $connection_id_id, $db_name,
				  $cube_name, $coordinate1, $coordinate2, $coordinate3, 
				  $coordinate4, $coordinate5, $coordinate6);
	
	
	// another sample to overwrite existing value with new value
	palo_setdata('|20', true, $connection_id_id, $db_name, $cube_name, 
				 $coordinate1, $coordinate2, $coordinate3,
				 $coordinate4, $coordinate5, $coordinate6);


	// overwrites values on basis elements
	palo_setdata('20', 'SPLASH_MODE_SET', $connection_id_id, $db_name,
				  $cube_name, $coordinate1, $coordinate2, $coordinate3,
				  $coordinate4, $coordinate5, $coordinate6);

	// another sample to overwrite values on basis elements
	palo_setdata('!20', true, $connection_id_id, $db_name, $cube_name,
				 $coordinate1, $coordinate2, $coordinate3, 
				 $coordinate4, $coordinate5, $coordinate6);


	// adds value on basis elements
	palo_setdata('20', 'SPLASH_MODE_ADD', $connection_id_id, $db_name, 
				  $cube_name, $coordinate1, $coordinate2, $coordinate3, 
				  $coordinate4, $coordinate5, $coordinate6);

	// another sample to add values on basis elements
	palo_setdata('!!20', true, $connection_id_id, $db_name, $cube_name, 
				 $coordinate1, $coordinate2, $coordinate3,
				 $coordinate4, $coordinate5, $coordinate6);

	// disconnect the connection to server
	palo_disconnect($connection_id_id); 
?>