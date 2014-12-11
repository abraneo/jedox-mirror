<?php

/**
 * Palo PHP API
 *
 * @author		Marek Pikulski <marek.pikulski@jedox.com>
 * @author		Dominik Danehl <dominik.danehl@jedox.com>
 * @author      Erwin Zeiter <erwin.zeiter@jedox.com>
 * @author 		Zeljka Radosavljevic <zeljka.radosavljevic@jedox.com> 
 * @author 		Jiri Junek 
 * @copyright	Jedox AG 2006-2012
 * @version		4.0
 * @package		PHP-Jedox-Palo
 */
 
/**
  * Retrieves information about the extension.
  * @return Array Return array with informations about palo 
  * 
  *Example:     
  * {@example ./example/ResourceId/palo_version.php}
  *				
  *
 */
	function palo_version(){
		return $res;
	}

/**
  * Retrieves information about the latest error
  * @return Array Return array with informations about the latest error 
  * 
  *Example:     
  * {@example ./example/ResourceId/palo_error.php}
  *				
  *
 */
	function palo_error(){
		return $res;
	}

/**
  * Open a connection to a Palo Server
  *
  * @param string Name for the connection
  * @param string Address or hostname of the Palo server
  * @param int The port
  * @param string The username
  * @param string The password
  * @return mixed Returns a Palo link identifier on success, or error on failure. 
  *
 */
	function palo_init($host, $port, $username, $password){
		return $res;
	}

/**
  * Retrieves information about a PALO Server.
  *
  * @param mixed Connection_id
  * @return array array with information about the server
  *
  *Example:
  * {@example ./example/ResourceId/palo_server_info.php}    
  *      
 */
	function palo_server_info($connection_id){
		return $res;
	}

/**
  * Clears the error buffer.  
  *      
 */
	function palo_clear_error(){
	}

/**
  * Deletes all data or data in specified areas of a cube.
  * 
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be cleared    
  * @param array List of elements to be cleared. (optional)
  * @return bool TRUE on success or error on failure 
  *
  * Example for the code:
  * {@example ./example/ResourceId/palo_cube_clear.php}
  *   
  * 
*/
	function palo_cube_clear($connection_id, $database, $cube_name, $element_list){
		return $res;
	}

 /**
    * Deletes all elements in the specified dimension.
    *
    * @param mixed Connection_id
	* @param string Database name
    * @param string Name of the dimension to be cleared
    *
    * @return bool TRUE on success or error on failure
    * 
    *Example:
    * {@example ./example/ResourceId/palo_dimension_clear.php}
    *		    
  */

  function palo_dimension_clear($connection_id, $database, $dimension){
    return $res;
  }

/**
  * Adds a database to the specified server.
  *
  * @param mixed Connection_id
  * @param int type of the database (0=normal, 3=userinfo). default is 0.
  * @return bool TRUE on success or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_root_add_database.php}
*/
	function palo_root_add_database($connection_id, $type){
		return $res;
	}


/**
  * Deletes the specifed database.
  *
  * @param mixed Connection_id
  * @return bool TRUE on success or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_root_delete_database.php}
*/
	function palo_root_delete_database($connection_id){
		return $res;
	}

/**
  * Adds a cube to to the specified database.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be created
  * @param array Dimensions of the cube
  * @return bool TRUE on success or error on failure
  * 
  * Example:     
  * {@example ./example/ResourceId/palo_database_add_cube.php}  
*/
	function palo_database_add_cube($connection_id, $database, $cube_name, $cube_data){
		return $res;
	}

/**
  * Delets a cube from to the specified database.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be deleted
  * @return bool TRUE on success or error on failure
  * 
  *Example:     
  * {@example ./example/ResourceId/palo_database_delete_cube.php} 
*/
	function palo_database_delete_cube($connection_id, $database, $cube_name){
		return $res;
	}

/**
  * Load cube.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be loaded
  * @return bool TRUE on success or error on failure
  * 
  *Example:     
  * {@example ./example/ResourceId/palo_database_load_cube.php}
  *            
*/
	function palo_database_load_cube($connection_id, $database, $cube_name){
		return $res;
	}

/**
  * Unload cube.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be unloaded
  * @return bool TRUE on success or error on failure
  *
  *Example:
  * {@example ./example/ResourceId/palo_database_unload_cube.php}
*/
	function palo_database_unload_cube($connection_id, $database, $cube_name){
		return $res;
	}

/**
  * Adds a dimension to the specified database.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension to be created
  * @return bool TRUE on success or error on failure
  * 
  * Example:     
  * {@example ./example/ResourceId/palo_database_add_dimension.php}
*/
	function palo_database_add_dimension($connection_id, $database, $dimension_name){
		return $res;
	}

/**
  * Delets a dimension from the specified database.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension to be deleted
  * @return bool TRUE on success or error on failure
  * 
  *Example:     
  * {@example ./example/ResourceId/palo_database_delete_dimension.php}
*/
	function palo_database_delete_dimension($connection_id, $database, $dim_name){
		return $res;
	}

/**
  * Lists all children of an consolidated element (alias of palo_element_list_children).
  *
  * @param string Connection_id
  * @param string Database name
  * @param string Name of the dimension
  * @param string Name of the element to be searched
  * @return array List of associative arrays ("name", "type", "factor") or error.
  * 
  * Example:     
  * {@example ./example/ResourceId/palo_element_list_consolidation_elements.php}
  *      
*/
	function palo_element_list_consolidation_elements($connection_id, $database, $dimension_name, $element_name){
		return $res;
	}

/**
  * Lists all elements contained in the specified dimension (same as palo_dimension_list_elements, but with more details about each element).
  *
  * @param mixed Connection_id
  * @param string Name of the database
  * @param string Name of the dimension
  * @param bool If this is set to true, elements with insufficient rights will sorted out. default is false. (optional)
  * @return array List of elements or error on failure
  *
  * Example for a returned list of elements:
  *
  *<code>
  *array(
  *	array(
  *		'identifier' => 23,
  *		'name' => 'Europe',
  *		'type' => 'consolidated',
  *		'level' => 2,
  *		'indent' => 1,
  *		'depth' => 0,
  *		'position' => 23,
  *		'num_parents' => 2,
  *		'num_childen' => 4,
  *		'parents' => array(
  *			array (
  *				'identifier' => 1,
  *				'name' => 'foobar',
  *				'type' => 'consolidated',
  *			),
  *			..
  *		),
  *		'children' => array(
  *			array (
  *				'identifier' => 19,
  *				'name' => 'West',
  *				'type' => 'consolidated'
  *			),
  *			..
  *		)
  *	)
  *)
  *</code>
  *
  *Example for the code:
  * {@example ./example/ResourceId/palo_dimension_list_elements2.php}
*/
	function palo_dimension_list_elements2($connection_id, $database, $dimension_name, $filter){
		return $res;
	}

/**
  * Lists all elements contained in the specified dimension.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimenison
  * @param bool If this is set to true, elements with insufficient rights will sorted out. default is false. (optional)
  * @return array List of elements or error on failure
  * 
  *Example:     
  * {@example ./example/ResourceId/palo_dimension_list_elements.php}
  *      
*/
	function palo_dimension_list_elements($connection_id, $database, $dimension_name, $filter){
		return $res;
	}

/**
  * Lists all cubes using the specified dimension.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension
  * @return array List of cubes or error on failure
  * 
  *Example:     
  * {@example ./example/ResourceId/palo_dimension_list_cubes.php}
*/
	function palo_dimension_list_cubes($connection_id, $database, $dimension_name){
		return $res;
	}

/**
  * Lists all dimensions used by the specified cube.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be queried
  * @return array List of dimensions or error on failure
  *
  * Example:
  * {@example ./example/ResourceId/palo_cube_list_dimensions.php} 
*/
	function palo_cube_list_dimensions($connection_id, $database, $cube_name){
		return $res;
	}

/**
  * Lists all cubes in the specified database.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param int Type of cubes (optional) (0 = normal cubes, 1 = system cubes, 2 = attribute cubes)
  * @return array List of cubes or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_database_list_cubes.php}  
  *
  *        
*/
	function palo_database_list_cubes($connection_id, $database, $cube_type){
		return $res;
	}

/**
  * Lists all dimensions in the specified database.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param int Type of dimensions (optional) (0 = normal dimension, 1 = system dimension, 2 = attribute dimension)
  * @return array List of dimensions or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_database_list_dimensions.php}  
  *          
*/
	function palo_database_list_dimensions($connection_id, $database, $dimension_type){
		return $res;
	}

/**
  * Lists all databases available on the specified server.
  *
  * @param mixed Connection_id
  * @param bool show system database. true/false (false is default)
  * @param bool show userinfo databases. true/false (false is default)
  * @return array List of databases or error on failure
  * 
  * Example:
  * {@example ./example/ResourceId/palo_root_list_databases.php} 
*/
	function palo_root_list_databases($connection_id, $show_system, $show_userinfo){
		return $res;
	}

/**
  * Renames a dimension.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which should be renamed
  * @param string New name of the dimension
  * @return bool TRUE on success or error on failure
  * 
  * Example:
  * {@example ./example/ResourceId/palo_database_rename_dimension.php}              
*/
	function palo_database_rename_dimension($connection_id, $database, $dimension_name, $new_dimension_name){
		return $res;
	}

/**
  * Adds an element to the specified dimension.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension in which the element should be created
  * @param string Type of the new element (use 'N' for numeric, 'C' for consolidation or 'S' for string element)
  * @param string Name of the new element
  * @param string Name of the parent element (use an empty string, if the new element should not be a child element)
  * @param double Consolidation factor of the new element
  * @param int TRUE/1: delete elements; FALSE/0:don't delete elements; 2: only consolidated elements will be deleted 
  * @param boolean (optional) returns empty string in error case if true 
  * @return string Name of the element that is added
  * 
  *Example:
  * {@example ./example/ResourceId/palo_eadd.php} 
*/
	function palo_eadd($connection_id, $database, $dimension_name, $element_type, $element_name, $parent_element_name, $consolidation_factor, $clear,						   $empty_string){
		return $res;
	}

/**
  * Deletes an element from specified dimension and database.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension in which the element shoud be deleted
  * @param string Name of the element to be deleted
  * @param boolean (optional) returns empty string in error case if true 
  * @return bool TRUE on success or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_edelete.php}  
  *
  *        
*/
	function palo_edelete($connection_id, $database, $dimension, $element, $empty_string){
		return $res;
	}

/**
  * Deletes array of elements from specified dimension and database.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension in which the element shoud be deleted
  * @param array Name of the element to be deleted
  * @param boolean (optional) returns empty string in error case if true 
  * @return bool TRUE on success or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_edelete_bulk.php}  
  *
  *        
*/
	function palo_edelete_bulk($connection_id, $database, $dimension, $elements, $empty_string){
		return $res;
	}

/**
  * Moves an element to the specified position in a dimension.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param string Name of the element
  * @param int New position of the element
  * @return bool TRUE on success or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_emove.php}   
*/
	function palo_emove($connection_id, $database, $dimension_name, $element_name, $new_position){
		return $res;
	}

/**
  * Renames an element.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param string Name of the element which should be renamed
  * @param string New name of the element
  * @param boolean (optional) returns empty string in error case if true 
  * @return bool TRUE on success or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_erename.php}            
*/
	function palo_erename($connection_id, $database, $dimension_name, $element_name, $new_element_name, $emtpy_string){
		return $res;
	}

/**
  * Updates an element and changes the type of an element or adujsts the consolidation factor or the order of the child elements.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension in which the element should be updated
  * @param string Name of the the element
  * @param string Type of the new element (use 'N' for numeric, 'C' for consolidation or 'S' for string element)
  * @param array Array with child elements. Leave blank if you just want to change the type of an element.
  * @param int Decide what to do with the array of child elements (false/0=replace existing child elements list with the array, true/1=add the child elements array to existing list of child elements, 2=delete the child elements array from existing list of child elements)
  * @return bool TRUE on success or error on failure
  *   
  * Example for an array with consolidated elements:
  *
  * <code>
  * array(
  *   array(
  *     'foo',
  *     2.0
  *   ),
  *   array(
  *     'bar',
  *     -1.2
  *   ),
  *   ...
  * )
  *</code>  
  *  
  * Example: 
  * {@example ./example/ResourceId/palo_eupdate.php}     
  *    
*/
	function palo_eupdate($connection_id, $database, $dimension_name, $element_name, $element_type, $consolidated_elements, $mode){
		return $res;
	}

/**
  * Returns the amount of child elements of the specified elements.
  *
  * @param string Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param string Name of the consolidated element which children should be counted
  * @param boolean (optional) returns empty string in error case if true
  * @return mixed Amount of the child elements or error on failure
  * 
  * Example:
  * {@example ./example/ResourceId/palo_echildcount.php} 
  *        
*/
	function palo_echildcount($connection_id, $database, $dimension_name, $element_name, $empty_string){
		return $res;
	}

/**
  * Retrieves the name of the n'th child of an consolidated element.
  *
  * @param resource Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param string Name of the element
  * @param int Number of the cild element to retrieve
  * @param boolean (optional) returns empty string in error case if true 
  * @return string Name of the child element
  *   
  * Example:
  * {@example ./example/ResourceId/palo_echildname.php}
*/
	function palo_echildname($connection_id, $database, $dimension_name, $element_name, $child_number, $empty_string){
		return $res;
	}

/**
  * Returns the amount of elements in the specified dimension.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension in which should be counted
  * @param boolean (optional) returns empty string in error case if true 
  * @return int Number of elements or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_ecount.php}
*/
	function palo_ecount($connection_id, $database, $dimension_name, $emtpy_string){
		return $res;
	}

/**
  * Retrieves the first element from the specified dimension.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param boolean (optional) returns empty string in error case if true 
  * @return mixed Name of the element or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_efirst.php}
  *
  *      
*/
	function palo_efirst($connection_id, $database, $dimension_name, $empty_string){
		return $res;
	}

/**
  * Returns the position in the dimension of the specified elements
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param string Name of the element
  * @param boolean (optional) returns empty string in error case if true 
  * @return int Position of the element or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_eindex.php}
  *      
*/
	function palo_eindex($connection_id, $database, $dimension_name, $element_name, $empty_string){
		return $res;
	}

/**
  * Checks if the specified element is a child of the other specified element.
  *
  * @param string Connection_id
  * @param string Database name
  * @param string Name of the dimension
  * @param string Name of the parent element
  * @param string Name of the Element
  * @param boolean (optional) returns empty string in error case if true 
  * @return bool TRUE, if the parent element contains the specified child element
  * 
  *Example:
  * {@example ./example/ResourceId/palo_eischild.php}         
*/
	function palo_eischild($connection_id, $database, $dimension_name, $parent_element, $element, $empty_string){
		return $res;
	}

/**
  * Retrieves the level of an element in the consolidation hierarchy. The minimal indention is 1.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param string Name of the element whose level should be retrieved
  * @param boolean (optional) returns empty string in error case if true 
  * @return mixed Level of indention or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_elevel.php}
*/
	function palo_elevel($connection_id, $database, $dimension_name, $element_name, $empty_string){
		return $res;
	}

/**
  * Retrieves the indentation level of an element in the consolidation hierarchy. The minimal indention is 1.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param string Name of the element whose indentation level should be retrieved
  * @param boolean (optional) returns empty string in error case if true 
  * @return mixed Level of indention or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_eindent.php}
  *       
*/
	function palo_eindent($connection_id, $database, $dimension_name, $element_name, $empty_string){
		return $res;
	}

/**
  * Retrieves the element at the specified position.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param mixed Position of the element to retrieve (int) or element name (string). The latter version simply returns the passed string.
  * @return mixed Name of the element or error on failure
  *
  * Note: if you want to use 0 as starting position instead of 1,
  * you must add the following to the end of your php.ini
  *
  * <code>
  * jedox.phppalo.start_index=0
  * </code>
  * 
  *Example:
  * {@example ./example/ResourceId/palo_ename.php}
  *      
*/
	function palo_ename($connection_id, $database, $dimension_name, $position){
		return $res;
	}

/**
  * Retrieves the amount of parents of the specified element.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param string Name of the element which parent elements should be counted
  * @param boolean (optional) returns empty string in error case if true 
  * @return mixed Amount of the parent elements or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_eparentcount.php} 
  *      
*/
	function palo_eparentcount($connection_id, $database, $dimension_name, $element_name, $empty_string){
		return $res;
	}

/**
  * Retrieves the n'th parent name of the specified element.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param string Name of the element which parent element should be retrieved
  * @param int Number of the parent to retrieve
  * @param boolean (optional) returns empty string in error case if true 
  * @return mixed Name of the element or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_eparentname.php}  
*/
	function palo_eparentname($connection_id, $database, $dimension_name, $element_name, $parent_number, $empty_string){
		return $res;
	}

/**
  * Retrieves the previous element.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param string Name of the element which previous element should be retrieved
  * @param boolean (optional) returns empty string in error case if true 
  * @return mixed Name of the previous element or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_eprev.php} 
*/
	function palo_eprev($connection_id, $database, $dimension_name, $element_name, $emtpy_string){
		return $res;
	}

/**
  * Retrieves the next element.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param string Name of the element, which next element should be retrieved
  * @param boolean (optional) returns empty string in error case if true 
  * @return mixed Name of the next element or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_enext.php}
  *      
*/
	function palo_enext($connection_id, $database, $dimension_name, $element_name, $empty_string){
		return $res;
	}

/**
  * Returns the n'th sibling of an element.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param string Name of the element which sibling should be retrieved
  * @param int Offset of the silbing (can be negative)
  * @param boolean (optional) returns empty string in error case if true 
  * @return mixed Name of the n'th sibling or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_esibling.php}
  *          
*/
	function palo_esibling($connection_id, $database, $dimension_name, $element_name, $sibling_number, $empty_string){
		return $res;
	}

/**
  * Returns the maximal consolidation level in a dimension (consolidation depth).
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension
  * @return mixed Maximal consolidation level or error on failure
  * 
  *Example:
  * {@example ./example/ResourceId/palo_etoplevel.php}   
*/
	function palo_etoplevel($connection_id, $database, $dimension_name){
		return $res;
	}

/**
  * Returns the type of the specified element.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the element
  * @param string Name of the element which type should be retrieved
  * @param boolean (optional) returns empty string in error case if true 
  * @return mixed Type of the element as string or error on failure
  * Types are: 'numeric', 'string', 'consolidated', 'rule'
  * 
  *Example:
  * {@example ./example/ResourceId/palo_etype.php}    
*/
	function palo_etype($connection_id, $database, $dimension_name, $element_name, $empty_string){
		return $res;
	}

/**
  * Returns the consolidation factor of the child element in a consolidation element.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the dimension which contains the elements
  * @param string Name of the consolidation element
  * @param string Name of the child element
  * @param boolean (optional) returns empty string in error case if true 
  * @return mixed Factor of consolidation or error on failure
  * 
  *Example: 
  * {@example ./example/ResourceId/palo_eweight.php} 
*/	
	function palo_eweight($connection_id, $database, $dimension_name, $consolidation_element, $child_element, $emtpy_string){
		return $res;
	}

/**
 * Prepare to collect (cache) DATAC requests.
 *
 * Succeeding DATAC requests will be cached.
 *  Please look at funciton palo_dataac to see how palo_startcachecollect works
 * 
 *
 * @return bool TRUE on success or error on failure
*/
	function palo_startcachecollect(){
		return $res;
	}

/**
  * Returns the data at the specified coordinates or error if the data could not be fetched
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be queried
  * @param array Coordinates of the data to be fetched
  * @return mixed String or double value or error
  * 
  *Example:
  * {@example ./example/ResourceId/palo_dataa.php}         
 */
	function palo_dataa($connection_id, $database, $cube_name, $coordinates){
		return $res;
	}

/**
  * Returns the data at the specified coordinates or error if the data could not be fetched. Breaks output every 255 characters.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be queried
  * @param array Coordinates of the data to be fetched
  * @return mixed Array with string parts or double value or error
  * 
  *Example:
  * {@example ./example/ResourceId/palo_dataat.php} 
  *                   
 */
	function palo_dataat($connection_id, $database, $cube_name, $coordinates){
		return $res;
	}

/**
  * Returns the data at the specified coordinates or error if the data could not be fetched (caching version).
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be queried
  * @param array Coordinates of the data to be fetched
  * @return mixed String or double value or error if cache is in return mode. #NA or error if cache is in collect mode.
  * 
  * Example:     
  * {@example ./example/ResourceId/palo_dataac.php} 
  *
  *                    
 */
	function palo_dataac($connection_id, $database, $cube_name, $coordinates){
		return $res;
	}

/**
  * Returns the data at the specified coordinates or error if the data could not be fetched (caching version). Breaks output every 255 characters.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be queried
  * @param array Coordinates of the data to be fetched
  * @return mixed Array with string parts or double value or error if cache is in return mode. #NA or error if cache is in collect mode.
  * 
  * Example:     
  * {@example ./example/ResourceId/palo_dataatc.php} 
  *            
 */
	function palo_dataatc($connection_id, $database, $cube_name, $coordinates){
		return $res;
	}

/**
  * Returns the data at the specified coordinates or error if the data could not be fetched
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be queried
  * @param string coordinates of the data to be fetched
  *
  * @return mixed String or double value or error
  *   
  * Example:     
  * {@example ./example/ResourceId/palo_data.php} 
  *                            
 */
	function palo_data($connection_id, $database, $cube_name, $coordinate1){
		return $res;
	}

/**
  * Returns the data at the specified coordinates or error if the data could not be fetched. Breaks output every 255 characters.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be queried
  * @param string Coordinates of the data to be fetched
  *
  * @return mixed Array with string parts or double value or error
  * 
  *Example:     
  * {@example ./example/ResourceId/palo_datat.php} 
  *     
 */
	function palo_datat($connection_id, $database, $cube_name, $coordinate1){
		return $res;
	}

/**
  * Returns the data at the specified coordinates or error if the data could not be fetched (caching version).
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be queried
  * @param array Coordinates of the data to be fetched
  *
  * @return mixed String or double value or error if cache is in return mode. #NA or error it cache is in collect mode.
  *
  * Example:     
  * {@example ./example/ResourceId/palo_datac.php}
  *          
 */
	function palo_datac($connection_id, $database, $cube_name, $coordinate1){
		return $res;
	}

/**
  * Returns the data at the specified coordinates or error if the data could not be fetched (caching version). Breaks output every 255 characters.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be queried
  * @param string Coordinates of the data to be fetched
  * @return mixed Array with string parts or double value or error if cache is in return mode. #NA or error it cache is in collect mode.
  *
  * Example:     
  * {@example ./example/ResourceId/palo_datatc.php}
 */
	function palo_datatc($connection_id, $database, $cube_name, $coordinate1){
		return $res;
	}

/**
  * Sets data at the specified coordinates
  *
  * 
  * @param mixed The value (string or double)
  * @param mixed Indicates if you want to use splashing (bool) or specifies the splash mode (string: "SPLASH_MODE_NONE",                                "SPLASH_MODE_DEFAULT","SPLASH_MODE_SET" or "SPLASH_MODE_ADD").
  * @param mixed Connection_id
  * @param string Name of the cube
  * @param array Coordinates of the value to be set
  *
  * @return return float if the element is of type numeric or string if the element is of type string 
  * 
  *Example:     
  * {@example ./example/ResourceId/palo_setdataa.php}
 */
	function palo_setdataa($value, $splash, $connection_id, $cube_name, $dimensions){
		return $res;
	}

/**
  * Sets data at the specified coordinates
  *
  * @param mixed The value (string or double)
  * @param mixed Indicates if you want to use splashing (bool) or specifies the splash mode (string: "SPLASH_MODE_NONE","SPLASH_MODE_DEFAULT", "SPLASH_MODE_SET" or "SPLASH_MODE_ADD").
  * @param mixed Connection_id
  * @param string Name of the cube
  * @param string First coordinate more can follow ($coordinate1, $coordinate2,.., $coordinate14)
  * @return bool TRUE on success or error on failure
  *
  *Example:     
  * {@example ./example/ResourceId/palo_setdata.php}
  *
  *<ul>
  *<li>short description: Sets or changes the value of cube cell
  *<li>long description: The server calculates the list of all base element for a numeric path,
  *<li>which contains one or more consolidate element.
  *<li>The given value will be distributed among these base paths using the following method: 
  *<li>no splashing: will result in an error message 
  *<li>default:
  *<li><li>value = 0.0:
  *<li>clear all base paths
  *<li>value <> 0.0 and old_value = 0.0:
  *<li>compute the splash value according to the weights of the path and set this 
  *<li>value to all base paths.
  *<li>value <> 0.0 and old_value <> 0.0:
  *<li>scale all value so that the sum is the new value.
  *<li>add: add given value to all base paths
  *<li>set: set all base paths to the given value
  *<li>(0=no splashing, 1=default, 2=add, 3=set)                              
  *</ul>           
 */
	function palo_setdata($value, $splash, $connection_id, $cube_name, $dimensions, $coordinate1){
		return $res;
	}

/**
  * Exports complete cube or subcube depending selected parameters
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be queried
  * @param bool  if true empty cells (0 or "") will be ignored
  * @param bool if true only base elements will be exported
  * @param double Lower limit of values which should be exported
  * @param double Upper limit of values which should be exported
  * @param string Operator for the lower limit (Use "gt" for "greater than" "gte" for "greater or equal", "lt" for "less than", "lte" for "less or      equal", "eq" for "qual" or "neq" for "not equal"
  * @param string Operator for the upper limit
  * @param string Compare mode (User "and", "or" or "xor");
  * @param int Maximal amount of rows to be exported
  * @param array Array specifying the first path to be returned. (Useful if there are more results than max_rows.)
  * @param array Subcube to be exported (optional)
  * @param bool set this to true to use rules. default is false (optional)
  *
  * @return array array with data records 
  *
  * Example of a subcube:
  *
  * <code>
  * $dimension = array(
  *   array('Desktop L'),
  *   array('Germany', 'East'),
  *   array('Jan'),
  *   array('2002', '2004'),
  *   array('Budget'),
  *   array('Units')
  * );
  * </code>
  *
  * Example of returned cube data:
  *
  * <code>
  * array(
  *   array(
  *     'value' => 12.34,
  *     'path' => array(
  *       'Desktop L',
  *       'Germany',
  *       'Jan',
  *       '2002',
  *       'Budget',
  *       'Cost of Sales'
  *     )
  *   ),
  *   ...
  * )
  * </code>
  * 
  *Example for the code:    
  * {@example ./example/ResourceId/palo_getdata_export.php}
  *
*/
  function palo_getdata_export($connection_id, $database, $cube_name, $ignore_empty, $base_only, $lower_limit, $upper_limit, $lower_operator, $upper_operator, $compare_mode, $max_rows, $first_path, $subcube, $use_rules){
    return $res;
  }

/**
  * Returns a complete data view (-> "cube slice")
  *
  * The arrays used by this function represent matrices. They look all like the following:
  *	array(long number_of_rows, long number_of_columns, value_11, value_12, ..., value_21, value_22, ...)
  * Where value_ij is the value at the i-th row and j-th column.
  *
  * @param mixed Connection_id
  * @param string Database name
  * @param string Name of the cube to be queried
  * @param mixed Coordinates of the data to be fetched
  * 
  * @return mixed specially structured array of cell values (string, double or error)
  *
  * Example:
  * {@example ./example/ResourceId/palo_datav.php}
  *   
 */
	function palo_datav($connection_id, $database, $cube_name, $coordinate1){
		return $res;
	}


/**
  * Creates a new rule for the specified coordinates
  *
  * @param string Connection_id
  * @param string Database name
  * @param string Database name
  * @param string Name of the cube to be queried
  * @param string defintinon retrieves constructed rule
  * 
  * @return array array with the consructed rule 
  *  
  * Example:     
  * {@example ./example/ResourceId/palo_cube_rule_create.php}                       
 */
 
 	function palo_cube_rule_create($connection_id, $database, $cube_name, $definition){
		return $res;
	}
	
	
/**
  * Parses the definition of the rule and verifies it
  *
  * @param string Connection_id
  * @param string Database name
  * @param string Name of the cube to be queried
  * @param string defintinon retrieves constructed rule
  * 
  * @return Empty string or error on failure
  * 
  *          
  * Example:     
  * {@example ./example/ResourceId/palo_cube_rule_parse.php}                       
 */
 
 	function palo_cube_rule_parse($connection_id, $database, $cube_name, $definition){
		return $res;
	}
	
	
/**
  * Retrieves excisting rules
  *
  * @param string Connection_id
  * @param string Database name
  * @param string Name of the cube to be queried
  * 
  * @return array list of rules
  *  
  * Example:     
  * {@example ./example/ResourceId/palo_cube_rules.php}                       
 */
 
 	function palo_cube_rules($connection_id, $database, $cube_name){
		return $res;
	}
	
	/**
  * Delete a given rule
  *
  * @param string Connection_id
  * @param string Database name
  * @param string Name of the cube to be queried
  * @param string id contains function to construct rules
  *
  * @return boolean true on success or false on failure 
  *  
  * Example:     
  * {@example ./example/ResourceId/palo_cube_rule_delete.php}                       
 */
 
 	function palo_cube_rule_delete($connection_id, $database, $cube_name, $rule){
		return $res;
	}


/**
  * Sets the values at the specified coordinates in dimension
  *
  * @param string Connection_id
  * @param string Database name
  * @param string Cube name
  * @param array Coordinates of the value to be set
  * @param array value to be set
  * @param mixed Indicates if you want to use splashing (bool) or specifies the splash mode (string: "SPLASH_MODE_NONE", "SPLASH_MODE_DEFAULT",         "SPLASH_MODE_SET" or "SPLASH_MODE_ADD").
  * @return boolean true on success or false on failure  
  *
  *
  * Example:     
  * {@example ./example/ResourceId/palo_setdata_bulk.php}                       
 */
 
 	function palo_setdata_bulk($connection_id, $database, $cube, $coordinates, $value, $splash){

		return $res;
	}

/**
  * Returns true if connection can be established.
  *
  * @param mixed connection object returned by palo_init
  *
  * @return mixed true on success or error on failure  
  *
  * Example:     
  * {@example ./example/ResourceId/palo_ping.php}                       
 */
 
 	function palo_ping($connection_id);

		return $res;
	}


/**
  * Lists all children of an consolidated element (alias of palo_element_list_consolidation_elements).
  *
  * @param string Connection_id
  * @param string Database name
  * @param string Name of the dimension
  * @param string Name of the element to be searched
  * @return array List of associative arrays ("name", "type", "factor") or error.
  *
  * @return array array which contains name, type and weight of the element  
  *
  * Example:     
  * {@example ./example/ResourceId/palo_element_list_children.php}                       
 */
 
 	function palo_element_list_children($connection_id, $database, $dimension, $element){

		return $res;
	}

/**
  * Retrieve a subset of dimension elements
  *
  * @param string connection
  * @param string Database name
  * @param string name of the dimension
  * @param int Entry of 1 or 2 or 3 controls the kind of indent. If empty the view is flat
  * @param string Alternative name for an element that defined as an attribute (optional)
  * @param array Result of DFILTER, AFILTER, PICKLIST, HFILTER, TFILTER or SORT in order as how mentioned (optional)
  *
  * @return array array which contains indent, name and alias of the element  
  *
  * Example:     
  * {@example ./example/ResourceId/palo_subset.php}                       
 */
 
 	function palo_subset($connection_id, $database, $dimension, $indent, $alias, $filter){

		return $res;
	}

/**
  * Creates a given amount of elements
  *
  * @param string connection
  * @param string Database name
  * @param string name of the dimension
  * @param array Array of Arrays with the elements to create
  * @param string Type of the new element (use 'N' for numeric, 'C' for consolidation or 'S' for string element).
  * @param array Array of Arrays with the childrens for the new elements
  * @param array Array of Arrays with the weight for the new elements
  *
  * @return mixed true on success, error message in case of an error.
  *
  * Example:     
  * {@example ./example/ResourceId/palo_element_create_bulk.php}                       
 */
 
 	function palo_element_create_bulk($connection_id, $database, $dimension, $element, $type, $children, $weight){

		return $res;
	}

/**
  * A data value will be investigated for each element of a subset. This value can be from type of numeric or string
  * @param string palo_subcube is necessary for dfilter
  * @param array Designation of criteria for each element defined values. Possible operators are <,>,=
  * @param int Optional. In case an integer value TOP is passed, only a number of TOP elements with the highest values are displayed. If the argument is left blank, the elements will not be restricted
  * @param int Numerical values between 1 and 99 only those elements will be selected, if their cumulated value results in a total value that is below the marginal value. The evaluation starts with the highest element
  * @param int Numerical value between 1 and 99. If e.g. 20 is entered all elements which are cumulated until &lt; = 20% will be evaluated. The evaluation starts with the least value
  * @param int Operators: 0/empty = SUM, 1=ALL, 2=AVERAGE, 3=MAXIMUM,4=any, 5=MINIMUM, 7=TEXT 
  * @param boolean blank/false = extracted cell content with Enterprise Rules. True = Enterprise Rules not to be applied
  *
  * @return array array which contains name, type and weight of the element  
  *
  * Example:  
  * {@example ./example/ResourceId/palo_dfilter.php}                       
 */

 	function palo_dfilter($subcube, $operations, $top, $upper_percentage, $lower_percentage, $cell_operator, $no_rules){

		return $res;
	}

 /**
  * @param resource connection
  * @param string name of the database
  * @param string name of the cube
  * @param int id of the rule which should be modified
  * @param string The definition of the rule.
  * @param string Information about the extern id (can be empty).
  * @param string Comment to the rule.
  * @param boolean if true rule is activate, if false rule is deactivated
  *
  * @return array array which contains name, type and weight of the element  
  *
  * Example:     
  * {@example ./example/ResourceId/palo_cube_rule_modify.php}                       
 */

 	function palo_cube_rule_modify($connection_id, $database, $cube, $identifier, $new_definition, $extern_id, $comment, $active){

		return $res;
	}

/**
  * Retrieve the length of a subset of dimension elements
  *
  * @param string string Connection_id
  * @param string Database name
  * @param string name of the dimension
  * @param int Entry of 1 or 2 or 3 controls the kind of indent. If empty the view is flat
  * @param string Alternative name for an element that defined as an attribute (optional)
  * @param array Result of DFILTER, AFILTER, PICKLIST, HFILTER, TFILTER or SORT in order as how mentioned (optional)
  *
  * @return int retrieve the length of a subset of dimension elements 
  *
  * Example:     
  * {@example ./example/ResourceId/palo_subset_size.php}                       
 */

 	function palo_subset_size($connection_id, $database, $element, $indent, $alias, $filter){

		return $res;
	}

/**
  * Adds elements to the subset which can not deleted by no way. 
    Further more a bulk can be defined, that attend as a filter
  *
  * @param mixed Adds elements to the subset which can not be deleted by no way. Further more a bulk can be defined, that attend as a filter
  * @param int 0/empty = Elements will be add at the beginning of the subset, 1 = Elements will be add at the end of the subset, 2 = Elements will be add merged to the subset, 3 = Only the elements, which were created under defintion will be shown
  *
  * @return array array with information about the element defined in picklist
  *
  * Example:     
  * {@example ./example/ResourceId/palo_picklist.php}                       
 */

	function palo_picklist(array $Definition, $Type){

		return $res;
	}

/**
  * Elements their name match with specified string model, will be shown, the rest will be removed
  *
  * @param mixed >DOS filter criteria. For example provoke that all elements which do not beginn with "D" or do not include a "k" will be removed. The question mark (? provokes the opposite of the Wildcard character (*)
  * @param int Extended filter criteria. For example provoke that all elements which do not end with an "X" or do not include a "s" or a "d" will be shown
  *
  * @return array array with the specified elements
  *
  * Example:     
  * {@example ./example/ResourceId/palo_tfilter.php}                       
 */
	function palo_tfilter(array $Regex, $Extended){

		return $res;
	}

/**
  * Sort the elements of a subset in a given order
  *
  * @param int 0/empty Flat hierarchy - default 1 shows the child elements, which were removed from the subset. 2 Cuts the view at this position 
  * @param int Grade for 0/empty = Definition, 1 = Data, 2 = Text, 3 = Alias
  * @param int Sorting of terms, whose been created by attributes. This filter should not be used with \"Criteria\"
  * @param int This function just works with Criteria and Whole together. 0/empty = no action, 1 = sorting of basic elements only, 2 = sort only consolidated elements
  * @param int This function works with Criteria and Whole only. If not empty, it will sort on the given level
  * @param int Reverse the sorting. 0/empty = no action, 1= Consolidations below, 2 = total, 3= 1 and 2 combined
  * @param int Show duplicate values, 0/empty = hide duplicates, 1 show duplicates
  *
  * @return array array with information about the filter
  *
  * Example:     
  * {@example ./example/ResourceId/palo_sort.php}                       
 */
	function palo_sort($whole, $criteria, $attribute, $type_limitation, $level_element, $reverse, $show_duplicates){
		return $res;
	}

/**
  * retrieve information about the specified cube
  *
  * @param string Connection_id
  * @param string Database name
  * @param string name of the cube
  *
  * @return array array with information about the cube
  *
  * Example:     
  * {@example ./example/ResourceId/palo_cube_info.php}                       
 */

	function palo_cube_info($connection_id, $database, $cube){

		return $res;
	}


/**
  * retrieve information about the specified dimension
  *
  * @param string name of the host/name of the database
  * @param string name of the dimension
  *
  * @return array array with information about the dimension
  *    
  *
  * Example:     
  * {@example ./example/ResourceId/palo_dimension_info.php}
  *
  *  
  *An Example how the array looks like    
  *<code>
  *array(10) {
  *   [0]=>  string(2) "12"
  *   [1]=>  string(8) "Products"
  *   [2]=>  string(2) "30"
  *   [3]=>  string(1) "2"
  *   [4]=>  string(1) "3"
  *   [5]=>  string(1) "2"
  *   [6]=>  string(1) "0"
  *   [7]=>  string(2) "13"
  *   [8]=>  string(2) "11"
  *   [9]=>  string(2) "12"
  *         }     
  *</code>     
  *
  *  Description:  
  *  <ul>
  *  <li>[0](dimension => identifier => Identifier of the dimension)
  *  <li>[1](name_dimension => string => Name of the dimension)
  *  <li>[2](number_elements => integer => Number of elements)
  *  <li>[3](maximum_level => integer => Maximum level of the dimension)
  *  <li>[4](maximum_indent => integer => Maximum indent of the dimension)
  *  <li>[5](maximum_depth => integer => Maximum depth of the dimension)
  *  <li>[6](type => identyfier => Type of dimension)
  *  <li>(0 = normal, 1 = system, 2 = attribute, 3 = user info)
  *  <li>[7](attributes_dimension =>  identyfier => 
  *  <li>Identifier of the attributes dimension of a normal dimension
  *  <li>or the identifier of the normal dimension associated to a attributes dimension.)
  *  <li>[8](attributes_cube => identyfier => Identifier of the attributes cube.)
  *  <li>(only for normal dimensions)
  *  <li>[9](rights_cube => identyfier => Identifier of the rights cube.)
  *  <li>(only for normal dimensions)
  *  </ul>          
  */

	function palo_dimension_info($connection_id, $dimension){

		return $res;
	}


/**
  * retrieve information about the specified dimension
  *
  * @param string Selection of elements, which are above ELEMENT in dimensionaly hierarchy. Empty = level under ELEMENT. If ABOVE = TRUE the higher-level elements will be handle
  * @param string Works only with element. TRUE = All elements in higher-level hierarchy will be handled. FALSE/empty = All elements in less-level hierarchy will be handled
  * @param boolean Works only wiht element. TRUE:  Element will not be shown. FALSE or empty: Element will be shown
  * @param int Works only with Element. 1 = basic elements will be blanked out. 2 = consolidated elements will be blanked out. if empty elements will not be blanked out.
  * @param string Revolve count is required else error will be thrown. This criteria do not work with Element. Elements, which are not at level of REVOLVE ELEMENT will be removed
  * @param string Defines the length of the revolving list (without REVOLVE-arguments possible). List respond the entered number
  * @param int Revolve count is required else error will be thrown. Shows elements with higher level (REVOLVE ADD = 2). With less level (REVOLVE ADD = 1)
  * @param string Beginning of the hierarchical choice at level of this element (if empty, no hierarchical choice)
  * @param string The end of the hierarchical choice at level of this element (if empty, until the top element)
  *
  * @return array array with information about the dimension
  *
  * Example:     
  * {@example ./example/ResourceId/palo_hfilter.php}                       
 */

	function palo_hfilter($element, $Above, $exclusive, $hide, $revolve_elem, $revolve_count, $revolve_add, $level_start, $level_end){

		return $res;
	}

/**
  * retrieve information about the specified dimension
  *
  * @param array array contains: number of rows, number of columns and the definition for the field-filter. Supported operators for numeric criteria are: ">", "<" and "="
  *
  * @return array array with information about the filter
  *
  * Example:     
  * {@example ./example/ResourceId/palo_afilter.php}                       
 */

	function palo_afilter($row_column_advanced_filter_matrix){

		return $res;
	}

/**
  * retrieve the id for the given cube
  *
  * @param string Connection_id
  * @param string Database name
  * @param string name of the cube
  *
  * @return long id of the cube
  *              
  * Example:     
  * {@example ./example/ResourceId/palo_get_cube_id.php}   

 */

	function palo_get_cube_id($connection_id, $database, $cube_name){
		return $res;
	}

/**
  * retrieve the name for the given cube
  *
  * @param string Connection_id
  * @param string Database name
  * @param long id of the cube
  *
  * @return string name of the cube
  *              
  * Example:     
  * {@example ./example/ResourceId/palo_get_cube_name.php}   
 */

	function palo_get_cube_name($connection_id, $database, $cube_id){
		return $res;
	}

/**
  * retrieve the name for the given dimension
  *
  * @param string Connection_id
  * @param string Database name
  * @param long id of the dimension
  *
  * @return string name of the dimension
  *              
  * Example:     
  * {@example ./example/ResourceId/palo_get_dimension_name.php}   
 */

	function palo_get_dimension_name($connection_id, $database, $dimension_id){
		return $res;
	}

/**
  * retrieve the id for the given dimension
  *
  * @param string Connection_id
  * @param string Database name
  * @param string name of the dimension
  *
  * @return long id of the dimension
  *              
  * Example:     
  * {@example ./example/ResourceId/palo_get_dimension_id.php}   
 */

	function palo_get_dimension_id($connection_id, $database, $dimension_name){
		return $res;
	}

/**
  * retrieve the id for the given element
  *
  * @param string Connection_id
  * @param string Database name
  * @param string name of the dimension
  * @param string name of the element
  *
  * @return long id of the element
  *              
  * Example:     
  * {@example ./example/ResourceId/palo_get_element_id.php}   
 */

	function palo_get_element_id($connection_id, $database, $dimension_name, $element_name){
		return $res;
	}

/**
  * retrieve the name for the given element
  *
  * @param string Connection_id
  * @param string Database name
  * @param string name of the dimension
  * @param long id of the element
  *
  * @return name name of the element
  *              
  * Example:     
  * {@example ./example/ResourceId/palo_get_element_name.php}   
 */

	function palo_get_element_name($connection_id, $database, $dimension_name, $element_id){
		return $res;
	}
	

  /**
  * retrieve the expirationdate of the license
  * 
  * @param string name of the host/name of the database
  *
  * @return string string with information about the license
  *              
  * Example:     
  * {@example ./example/ResourceId/palo_license_info.php}   
 */

	function palo_license_info($connection_id){
		return $res;
		}
		
		
  /**
  * Activates Unicode so that for example umlauts can be used.
  * 
  *Example with unicode: München, Rätsel
  *
  *With using palo_use_unicode the names will be shown correctly.
  *     
  * @param boolean activation parameter.
  *                   
  * Example:     
  * {@example ./example/ResourceId/palo_use_unicode.php}   
 */

	function palo_use_unicode($bool){
		return $res;
	}		
	/**
  * Returns a complete data view (-> "cube slice")
  *
  * gives the same result as datav, but does not use a matrix as Argument,
  * but an Array with the given parameters.    
  *
  * @param string "connection_name/database_name"
  * @param string Name of the cube to be queried
  * @param mixed Coordinates of the data to be fetched
  * 
  * @return mixed specially structured array of cell values (string, double or error)
  *
  * Example:
  * {@example ./example/ResourceId/palo_dataav.php}
  *   
  */
	function palo_dataav($connection_id, $cube_name, $coordinate1){
		return $res;
	}
	
	/**
  * Changes password.
  *
  * @param mixed Connection_id
  * @param string password of user which we want to changed
  * @param string New password for the user
  * @return bool TRUE on success or error on failure
  * 
  * Example:
  * {@example ./example/ResourceId/palo_change_password.php}              
  */
	function palo_change_password($connectionressource, $oldpassword, $newpassword){
		return $res;
	}
		/**
  * Value added to the target cell and all sibling cells in specified dimension are changed, 
  * in a way that parent cell stay unchanged
  *
  * @param string connection
  * @param string name of the database
  * @param string name of the cube
  * @param array coordinates of the value to be set
  * @param string value to be set
  * @param string allocation you can use three different type of allocation. (type=0 or "PALO_GOALSEEK_COMPLETE", type=1 or "PALO_GOALSEEK_EQUAL" and type=2 or "PALO_GOALSEEK_RELATIVE").
  * @param array area where value will be reallocated. (optional)
  * @return bool TRUE on success or error on failure
  * 
  * Example:
  * {@example ./example/ResourceId/palo_goal_seek.php}              
  */
	function palo_goal_seek($connection, $db_name, $cube_name, $coordinates, $value, $allocation, $area){
		return $res;
	}
		/**
  * Copying value from one cell to defined cell
  *
  * @param string connection
  * @param string name of the database
  * @param string name of the cube
  * @param array from coordinates of the value which will be copied
  * @param array to coordinates of the value to be set
  * @param string value to be set (optional)
  * @param bool userule use rule-calculated values from source area. Default is false. (optional) 
  * @return bool TRUE on success or error on failure
  * 
  * Example:
  * {@example ./example/ResourceId/palo_cellcopy.php}              
  */
	function palo_cellcopy($connection, $db_name, $cube_name, $from, $to, $value, $userrule){
		return $res;
	}
	/**
  * Returns user name for specified sid.
  *
  * @param string sid
  * @return string user name
  * 
  * Example:
  * {@example ./example/ResourceId/palo_get_user_for_sid.php}              
  */
	function palo_get_user_for_sid($sid){
		return $res;
	}
	/**
  * Returns groups for specified sid.
  *
  * @param string sid
  * @return array Array of groups
  * 
  * Example:
  * {@example ./example/ResourceId/palo_get_groups_for_sid.php}              
  */
	function palo_get_groups_for_sid($sid){
		return $res;
	}
	/**
  * Changes password of specified user.
  *
  * @param mixed Connection_id
  * @param string User whose password should be changed
  * @param string New password for the user
  * @return bool TRUE on success or error on failure
  * 
  * Example:
  * {@example ./example/ResourceId/palo_change_user_password.php}              
  */
	function palo_change_user_password($connection_id, $username, $password){
		return $res;
	}
	/**
  * Sets description of the client application in next OLAP sessions.
  *
  * @param string Description of the client application.
  * 
  * Example:
  * {@example ./example/ResourceId/palo_set_client_description.php}              
  */
	function palo_set_client_description($description){
		return $res;
	}
?>
