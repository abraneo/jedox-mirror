<?php

// create the constants
define('CONN', 'LOCALHOST');
define('HOST', '127.0.0.1');
define('PORT', '7777');
define('USER', 'admin');
define('PASS', 'admin');

// registers a name for a PALO server
$connection_id_id = palo_register_server(HOST, PORT, USER, PASS);

//a boolean argument to set palo_uni_code to true or false.
$bool =true;

// activates the usage of unicode.
palo_use_unicode($bool);

// create the variables as follows
$db_name = 'Demo';
$dimension = 'Regions';    
      
    // clear the dimension 'Years' and add a numeric element
    // with 'All Years' as parent
    // if parent doesn´t exists, it will be created
    palo_eadd($connection_id_id, $db_name, $dimension, 'N', 'München', 'Regions', 1, 1);

?>
