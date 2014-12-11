<?php

	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// initiation of connection
	$connection_id_id = palo_init(HOST, PORT, USER, PASS);
  
  // receiving timestring from license
  $license_info = palo_license_info($connection_id_id);
  
  // define actuall date + 4 days      
  $expire_limit = time() + 4 * 24 * 3600;
 
  // if date of expire is < 4 days till expiration, then give message:
  //  else gives detailed Licenseinformations.   
  if( $license_info['expiredate'] <=  $expire_limit )
  {
       echo 'License is about to expire!';
  }
   else
  {
       echo '<br>Licensed to :' . $license_info['name'];
       echo '<br>Number of users :' . $license_info['users'];
       echo '<br>Expires on :' . date('Y-m-d', $license_info['expiredate'] );
       echo '<br>License type :' . $license_info['type'];
  }

?>