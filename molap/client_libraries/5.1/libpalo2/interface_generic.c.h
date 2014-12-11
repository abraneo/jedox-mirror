////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// Copyright (C) 2006-2014 Jedox AG
///
/// This program is free software; you can redistribute it and/or modify it
/// under the terms of the GNU General Public License (Version 2) as published
/// by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
/// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
/// more details.
///
/// You should have received a copy of the GNU General Public License along with
/// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
/// Place, Suite 330, Boston, MA 02111-1307 USA
///
/// If you are developing and distributing open source applications under the
/// GPL License, then you are free to use Palo under the GPL License.  For OEMs,
/// ISVs, and VARs who distribute Palo with their products, and do not license
/// and distribute their source code under the GPL, Jedox provides a flexible
/// OEM Commercial License.
///
/// @author
////////////////////////////////////////////////////////////////////////////////

#include "helper_generic.h.h"

#ifndef HAVE_INTERFACE_GENERIC_H
#define HAVE_INTERFACE_GENERIC_H

#include "helper.h"

/** @defgroup gen_funcs generic functions
 *  Charset independent function declarations.
 *
 *  To use the functions exported by libpalo2 you have to include "libpalo2.h" in your code.
 *  Then you have to initialize the library using either libpalo_init() or libpalo_init_r()().
 *
 *  The functions listed in the documentation use the placeholder CHAR_T for the character type. See the following section for more information.
 *
 *  The function suffixes have the following meanings:
 *      - _r():    This specifies a thread-safe function. All informations will be passed as parameters.
 *      - _a():    This specifies a funtion which expects strings in the encoding corresponding to the current locale. CHAR_T corresponds to 'char'.
 *      - _m():    This specifies a funtion which expects utf-8 encoded strings. CHAR_T corresponds to 'char'.
 *      - _w():    This specifies a funtion which expects wide character strings (hopefully unicode). CHAR_T corresponds to 'wchar_t'.
 *
 *  Combinations: _m_r(), _a_r(), _w_r()
 *	@{
 */

#ifndef LIBPALO
/* the following is defined only to allow easier documentation */
/** @brief
 *  Contains informations about a consolidation (child) element.
 */
struct dummy_consolidation_info {
	/// name of the consolidation element
	CHAR_T *name;
	/// type of the consolidation element
	enum libpalo_de_type type;
	/// consolidation factor
	double factor;
};

/* the following is defined only to allow easier documentation */
/** @brief
 *  Contains informations about a dimension element.
 */
struct dummy_element_info {
	/// name of the dimension element
	char *name;
	/// type of the dimension element
	enum libpalo_de_type type;
};
#endif
#endif

/* MT */

/** @defgroup gen_funcs_mt generic functions (thread-safe versions)
 *  @ingroup gen_funcs
 *  Thread-safe implementations.
 *	@{
 */

/** @brief
 *  Retrieves a value from the PALO server
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *
 *  @param result will be set to the value of the referenced cell; use free_arg_palo_value_contents() to free resources associated with its contents
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the requested data
 *  @param cube name of the cube containing the requested data
 *  @param coordinates an array of strings specifying the cell containing the requested data
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(getdata)(libpalo_err *err, struct arg_palo_value *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array coordinates) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(getdata)(errs, result, so, convs, database, cube,  &coordinates), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves a "sub-cube" from the PALO server
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *
 *  @param result will be set to the values of the referenced cells; use free_arg_palo_value_array_contents() to free resources associated with its contents
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the requested data
 *  @param cube name of the cube containing the requested data
 *  @param coordinates an array of arrays of strings specifying the cells containing the requested data
 *  @param guess_coordinates_order has no function anymore, it is here just for compatibility
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(getdata_area)(libpalo_err *err, struct arg_palo_value_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array coordinates, palo_bool guess_coordinates_order) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(getdata_area)(errs, result, so, convs, database, cube, &coordinates), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves a "sub-cube" from the PALO server
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *
 *  @param result will be set to the values and coordinates of the referenced cells; use free_arg_dataset_array_contents() to free resources associated with its contents
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the requested data
 *  @param cube name of the cube containing the requested data
 *  @param coordinates an array of arrays of strings specifying the cells containing the requested data
 *  @param guess_coordinates_order has no function anymore, it is here just for compatibility
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(getdata_area2)(libpalo_err *err, struct arg_palo_dataset_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array coordinates, palo_bool guess_coordinates_order) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(getdata_area2)(errs, result, so, convs, database, cube, &coordinates), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


LIBEXPORT palo_err LIBPALO_FUNC_R(_getdata_multi)(struct errstack *errs, struct arg_palo_value_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_2d *coordinates) {
	palo_err pe;

	pe = PALO_SUFFIX(getdata_multi)(errs, result, so, convs, database, cube, coordinates);

	if (pe != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, pe, __FUNCTION__, __LINE__, __FILE__, NULL);
	}

	return PALO_SUCCESS;
}

/** @brief
 *  Retrieves the values of multiple cells from the PALO server in one call
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *
 *  @param result will be set to the values of the referenced cells; use free_arg_palo_value_array_contents() to free resources associated with its contents
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the requested data
 *  @param cube name of the cube containing the requested data
 *  @param coordinates a 2-dimensional array of strings (table) specifying the cells containing the requested data
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(getdata_multi)(libpalo_err *err, struct arg_palo_value_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_2d coordinates) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(getdata_multi)(errs, result, so, convs, database, cube, &coordinates), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Writes a value to the PALO server
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the target cell
 *  @param cube name of the cube containing the target cell
 *  @param ds specifies target cell and value
 *  @param mode specifies whether numbers can be written into multiple cells and how this should be done
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(setdata)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset ds, splash_mode mode) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(setdata)(errs, so, convs, database, cube, &ds, mode), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(setdata_extended)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset ds, splash_mode mode, unsigned short add, unsigned short eventprocessor, const struct arg_str_array_2d *locked_coordinates) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(setdata_extended)(errs, so, convs, database, cube, &ds, mode, add, eventprocessor, locked_coordinates), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

/** @brief
 *  Set the values of multiple cells from the PALO server in one call
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the requested data
 *  @param cube name of the cube containing the requested data
 *  @param dsa the coordinates and values of cells to be set
 *  @param mode the splash mode
 *  @param add Should the value be added to the existing value ?
 *  @param eventprocessor Should the eventprocessor be invoked if the cell is in a sensitive area ?
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(setdata_multi)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset_array dsa, splash_mode mode, unsigned short add, unsigned short eventprocessor) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(setdata_multi)(errs, so, convs, database, cube, &dsa, mode, add, eventprocessor), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(setdata_multi_extended)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset_array dsa, splash_mode mode, unsigned short add, unsigned short eventprocessor, const struct arg_str_array_2d *locked_coordinates) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(setdata_multi_extended)(errs, so, convs, database, cube, &dsa, mode, add, eventprocessor, locked_coordinates), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(init_ssl)(libpalo_err *err, struct conversions *convs, const CHAR_T *trustfile) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(PALO_SUFFIX(init_ssl)(errs, convs, trustfile), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Authenticates a user
 *
 *  This checks a users credentials. You need to authenticate before being able to perform any other operation on the server,
 *  otherwise the connection will be closed ungracefully. Normally, this function should be called immediately after palo_connect.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param username username to use
 *  @param pw the corresponding password
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(auth)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *username, const CHAR_T *pw) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(PALO_SUFFIX(auth_ssl)(errs, so, convs, username, pw, NULL, 0), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

/** @brief
 *  Authenticates a user using ssl
 *
 *  This checks a users credentials. You need to authenticate before being able to perform any other operation on the server,
 *  otherwise the connection will be closed ungracefully. Normally, this function should be called immediately after palo_connect.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param username username to use
 *  @param pw the corresponding password
 *  @param trustfile the trust with the list of trusted ca in pem format
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(auth_ssl)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *username, const CHAR_T *pw, const CHAR_T *trustfile) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(PALO_SUFFIX(auth_ssl)(errs, so, convs, username, pw, trustfile, 1), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(test_connection)(libpalo_err *err, struct conversions *convs, const CHAR_T *hostname, unsigned int service, const CHAR_T *username, const CHAR_T *pw, const CHAR_T *trustfile) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(PALO_SUFFIX(test_connection)(errs, convs, hostname, service, username, pw, trustfile), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(check_validity)(libpalo_err *err, struct sock_obj *so, struct conversions *convs) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(PALO_SUFFIX(check_validity)(errs, so, convs), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  "Pings" a server
 *
 *  You can use this function to check if a connection is working correctly.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(ping)(libpalo_err *err, struct sock_obj *so, struct conversions *convs) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(ping)(errs, so, convs), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Adds a new database to the PALO server
 *
 *  This function creates a new database which will be available immediately.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database to be created
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(root_add_database)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(root_add_database)(errs, so, convs, database), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Adds a new dimension to a database
 *
 *  This function creates a new dimension which will be available immediately.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database which will contain the new dimension
 *  @param dimension name of the dimension to be created
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_add_dimension)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_add_dimension)(errs, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Adds a new dimension to a database
 *
 *  This function creates a new dimension which will be available immediately.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database which will contain the new dimension
 *  @param dimension name of the dimension to be created
 *  @param type type of the dimension to be created (normal or user_info)
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_add_dimension2)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, dim_type type) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_add_dimension2)(errs, so, convs, database, dimension, type), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Renames a cube
 *
 *  This function renames a cube
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database which contains the cube to be renamed
 *  @param cube_oldname current name of the cube to be renamed
 *  @param cube_newname the new name of the cube
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_rename_cube)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube_oldname, const CHAR_T *cube_newname) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_rename_cube)(errs, so, convs, database, cube_oldname, cube_newname), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Renames a dimension
 *
 *  This function renames a dimension.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database which contains the dimension to be renamed
 *  @param dimension_oldname current name of the dimension to be renamed
 *  @param dimension_newname the new name of the dimension
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_rename_dimension)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension_oldname, const CHAR_T *dimension_newname) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_rename_dimension)(errs, so, convs, database, dimension_oldname, dimension_newname), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Adds a new cube to a database
 *
 *  This function creates a new cube which will be available immediately.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database which will contain the new cube
 *  @param cube name of the cube to be created
 *  @param dimensions an array of strings specifying the dimensions which the new cube will contain
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_add_cube)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array dimensions) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_add_cube)(errs, so, convs, database, cube, &dimensions), err, errs, __FUNCTION__, __FILE__, __LINE__);

}


/** @brief
 *  Adds a new cube to a database
 *
 *  This function creates a new cube which will be available immediately.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database which will contain the new cube
 *  @param cube name of the cube to be created
 *  @param dimensions an array of strings specifying the dimensions which the new cube will contain
 *  @param type type of the cube to be created (normal or user_info)
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_add_cube2)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array dimensions, cube_type type) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_add_cube2)(errs, so, convs, database, cube, &dimensions, type), err, errs, __FUNCTION__, __FILE__, __LINE__);

}


/** @brief
 *  Loads the specified database into memory.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database to be loaded
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_load)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_load)(errs, so, convs, database), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Shuts down the server
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(server_shutdown)(libpalo_err *err, struct sock_obj *so, struct conversions *convs) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(server_shutdown)(errs, so, convs), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Unloads the specified database from memory.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database to be unloaded
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_unload)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_unload)(errs, so, convs, database), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Unloads the specified database from memory and deletes any associated data.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database to be deleted
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_delete)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_delete)(errs, so, convs, database), err, errs, __FUNCTION__, __FILE__, __LINE__);
}



/** @brief
 *  Unloads the specified dimension from memory and deletes any associated data.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the dimension
 *  @param dimension name of the dimension to be deleted
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_delete)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(dimension_delete)(errs, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Empties the specified dimension.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the dimension
 *  @param dimension name of the dimension to be cleared
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_clear)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(dimension_clear)(errs, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Loads the specified cube into memory.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the cube
 *  @param cube name of the cube to be loaded
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_load)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cube_load)(errs, so, convs, database, cube), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

/** @brief
 *  Unloads the specified cube from memory.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the cube
 *  @param cube name of the cube to be unloaded
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_unload)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cube_unload)(errs, so, convs, database, cube), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Unloads the specified cube from memory and deletes any associated data.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the cube
 *  @param cube name of the cube to be deleted
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_delete)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cube_delete)(errs, so, convs, database, cube), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Commits all setdata operations (logged in .cube.log) to the cube data file (.cube.data).
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the cube
 *  @param cube name of the cube to be saved
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_commit_log)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cube_commit_log)(errs, so, convs, database, cube), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Saves the specified database to disk. You can use this to update an existing database.xml file.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database to be saved
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_save)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_save)(errs, so, convs, database), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the name of the n'th child of a consolidated element.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the name of the child element; use free_arg_str() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the consolidated (parent) dimension element
 *  @param n offset of the child element whose name should be returned (the first child element is at offset 1)
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(echildname)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, size_t n) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(echildname)(errs, result, so, convs, database, dimension, dimension_element, n), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the number of children of a consolidated element.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the number of child elements
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the consolidated (parent) dimension element
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(echildcount)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(echildcount)(errs, result, so, convs, database, dimension, dimension_element), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the number of parents of an element.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the number of child elements
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the child element
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eparentcount)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(eparentcount)(errs, result, so, convs, database, dimension, dimension_element), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the name of the n'th parent of an element.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the name of the parent element; use free_arg_str() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the child element
 *  @param n offset of the parent element whose name should be returned (the first parent element is at offset 1)
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eparentname)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, size_t n) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(eparentname)(errs, result, so, convs, database, dimension, dimension_element, n), err, errs, __FUNCTION__, __FILE__, __LINE__);

}


/** @brief
 *  Retrieves the number of elements in a dimension.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the number of dimension elements
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(ecount)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(ecount)(errs, result, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the name of the first element in a dimension.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the name of the first element in the dimension; use free_arg_str() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(efirst)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(efirst)(errs, result, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the position of an element in a dimension. The first element is located at position 1.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the position of the dimension element
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the dimension element whose position should be determined
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eindex)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(eindex)(errs, result, so, convs, database, dimension, dimension_element), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Checks if de_child is contained in the consolidated element de_parent.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will be set to zero, if de_child is not a child of de_parent, otherwise to a non-zero value
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param de_parent name of the consolidated element
 *  @param de_child name of the potential child element
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eischild)(libpalo_err *err, palo_bool *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_parent, const CHAR_T *de_child) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(eischild)(errs, result, so, convs, database, dimension, de_parent, de_child), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the name of the dimension element at the specified position.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the name of the dimension element; use free_arg_str() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param n position of the dimension element whose name should be returned (the first element is located at position 1)
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(ename)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, size_t n) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(ename)(errs, result, so, convs, database, dimension, n), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the name of the dimension containing the given elements.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the name of the dimension; use free_arg_str() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension_elements array containing the dimension element names
 *  @param should_be_unique flag specifying whether the function should check if there exist multiple dimensions containing all the dimension elements
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(edimension)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, struct arg_str_array dimension_elements, palo_bool should_be_unique) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(edimension)(errs, result, so, convs, database, &dimension_elements, should_be_unique), err, errs, __FUNCTION__, __FILE__, __LINE__);

}


/** @brief
 *  Retrieves the name of the cube dimension containing the given elements.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the name of the dimension; use free_arg_str() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param cube name of the cube
 *  @param dimension_elements array containing the dimension element names
 *  @param should_be_unique flag specifying whether the function should check if there exist multiple dimensions containing all the dimension elements
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(ecubedimension)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, struct arg_str_array dimension_elements, palo_bool should_be_unique) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(ecubedimension)(errs, result, so, convs, database, cube, &dimension_elements, should_be_unique), err, errs, __FUNCTION__, __FILE__, __LINE__);

}


/** @brief
 *  Retrieves the name of the succeeding element of a dimension_element.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the name of the succeeding dimension element; use free_arg_str() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the dimension element whose successor should be determined
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(enext)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(enext)(errs, result, so, convs, database, dimension, dimension_element), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the name of the preceding element of a dimension_element.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the name of the preceding dimension element; use free_arg_str() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the dimension element whose predecessor should be determined
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eprev)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(eprev)(errs, result, so, convs, database, dimension, dimension_element), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the type of a dimension element.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the type of the dimension element
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the dimension element whose type should be determined
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(etype)(libpalo_err *err, de_type *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(etype)(errs, result, so, convs, database, dimension, dimension_element), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the name of a sibling of a dimension element.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the name of the specified sibling of the dimension element
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the dimension element to start from
 *  @param n signed offset, relative to the specified dimension element
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(esibling)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, int n) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(esibling)(errs, result, so, convs, database, dimension, dimension_element, n), err, errs, __FUNCTION__, __FILE__, __LINE__);

}


/** @brief
 *  Retrieves the consolidation factor of the child element de_child within the consolidation element de_parent.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the consolidation factor
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param de_parent name of consolidated (parent) element
 *  @param de_child name of child element
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eweight)(libpalo_err *err, double *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_parent, const CHAR_T *de_child) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(eweight)(errs, result, so, convs, database, dimension, de_parent, de_child), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the level of an element in the consolidation hierarchy. Base elements are located at level 0.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the level of the dimension element int the consolidation hierarchy
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the dimension element whose level should be determined
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(elevel)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(elevel)(errs, result, so, convs, database, dimension, dimension_element), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the indentation level of an element in the consolidation hierarchy. The minimal indentation is 1.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the indentation level of the dimension element int the consolidation hierarchy
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the dimension element whose indentation level should be determined
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eindent)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(eindent)(errs, result, so, convs, database, dimension, dimension_element), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the maximal consolidation level in a dimension.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the depth of the deepest hierarchy in the dimension
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(etoplevel)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(etoplevel)(errs, result, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves a list of all databases on a server.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the list of database names; use free_arg_str_array_contents() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(root_list_databases)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(root_list_databases)(errs, result, so, convs), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves a list of all databases on a server with a special type.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the list of database names; use free_arg_str_array_contents() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param db_type the desired type
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(root_list_databases2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, int db_type) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(root_list_databases2)(errs, result, so, convs, db_type), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves a list of all cubes in a database.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the list of cube names; use free_arg_str_array_contents() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_list_cubes)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_list_cubes)(errs, result, so, convs, database), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves a list of all cubes in a database with a special type.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the list of cube names; use free_arg_str_array_contents() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param cube_type the desired type
 *  @param only_cubes_with_cells <> 0, if only cubes with cells should be returned
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_list_cubes2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, int cube_type, unsigned short int only_cubes_with_cells) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_list_cubes2)(errs, result, so, convs, database, cube_type, only_cubes_with_cells), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves a list of all dimensions in a database.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the list of dimension names; use free_arg_str_array_contents() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_list_dimensions)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_list_dimensions)(errs, result, so, convs, database), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves a list of all dimensions in a database with a special type.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the list of dimension names; use free_arg_str_array_contents() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dim_type the desired type
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_list_dimensions2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, int dim_type) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_list_dimensions2)(errs, result, so, convs, database, dim_type), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves a list of all dimensions in a cube.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the list of dimension names; use free_arg_str_array_contents() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param cube name of the cube
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_list_dimensions)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cube_list_dimensions)(errs, result, so, convs, database, cube), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves a list of all cubes which are using the specified dimension.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the cube names; use free_arg_str_array_contents() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_list_cubes)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(dimension_list_cubes)(errs, result, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves a list of all cubes which are using the specified dimension and has a special type.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the cube names; use free_arg_str_array_contents() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param cube_type the desired type
 *  @param only_cubes_with_cells <> 0, if only cubes with cells should be returned
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_list_cubes2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, int cube_type, unsigned short int only_cubes_with_cells) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(dimension_list_cubes2)(errs, result, so, convs, database, dimension, cube_type, only_cubes_with_cells), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves a list of all child elements of a consolidated element.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the list of child elements; use free_arg_consolidation_element_info_array_contents() to free associated memory
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the consolidated (parent) element
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(element_list_consolidation_elements)(libpalo_err *err, struct arg_consolidation_element_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(element_list_consolidation_elements)(errs, result, so, convs, database, dimension, dimension_element), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves a list of all elements in a dimension.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the list of dimension elements; use free_arg_dim_element_info_array_contents() to free associated memory
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_list_elements)(libpalo_err *err, struct arg_dim_element_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(dimension_list_elements)(errs, result, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves a list of all elements in a dimension with structur information.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the list of dimension elements with structur information; use free_arg_dim_element_info2_raw_array_contents() to free associated memory
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_list_elements2_raw)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(dimension_list_elements2_raw)(errs, result, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

/** @brief
 *  Retrieves a flat list of at most limit elements beginning at postion in a dimension with structur information.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the list of dimension elements with structur information; use free_arg_dim_element_info2_raw_array_contents() to free associated memory
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param start begin at this position
 *  @param limit at most limit elements
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_restricted_flat_list_elements)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, long start, long limit) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(dimension_restricted_flat_list_elements)(errs, result, so, convs, database, dimension, start, limit), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

/** @brief
 *  Retrieves a  list of at most limit top elements beginning at postion in a dimension with structur information.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the list of dimension elements with structur information; use free_arg_dim_element_info2_raw_array_contents() to free associated memory
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param start begin at this position
 *  @param limit at most limit elements
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_restricted_top_list_elements)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, long start, long limit) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(dimension_restricted_top_list_elements)(errs, result, so, convs, database, dimension, start, limit), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

/** @brief
 *  Retrieves a list of at most limit children elements beginning at postion in a dimension with structur information.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the list of dimension elements with structur information; use free_arg_dim_element_info2_raw_array_contents() to free associated memory
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param element_identifier identifier of parent
 *  @param start begin at this position
 *  @param limit at most limit elements
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_restricted_children_list_elements)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, long element_identifier, long start, long limit) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(dimension_restricted_children_list_elements)(errs, result, so, convs, database, dimension, element_identifier, start, limit), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

/** @brief
 *  Retrieves the type of the database.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the type of the database
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_database_type)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(get_database_type)(errs, result, so, convs, database), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the type of the dimension.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the type of the dimension
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_dimension_type)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(get_dimension_type)(errs, result, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the type of the cube.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the type of the cube
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param cube name of the cube
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_cube_type)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(get_cube_type)(errs, result, so, convs, database, cube), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the name of the associated attribute dimension.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the name of the atttribute dimension
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_attribute_dimension)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(get_attribute_dimension)(errs, result, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the name of the associated attribute cube.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the name of the atttribute cube
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_attribute_cube)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(get_attribute_cube)(errs, result, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the name of the associated rights cube.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the name of the rights cube
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_rights_cube)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(get_rights_cube)(errs, result, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Renames a dimension element. Any related data remains untouched.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param de_oldname old name of the dimension element
 *  @param de_newname new name of the dimension_element
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(erename)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_oldname, const CHAR_T *de_newname) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(erename)(errs, so, convs, database, dimension, de_oldname, de_newname), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Deletes a dimension element.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the dimension element to be deleted
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(edelete)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(edelete)(errs, so, convs, database, dimension, dimension_element), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Updates or adds a dimension element.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the new dimension element to be created
 *  @param mode operation mode
 *  @param type type of the new dimension element
 *  @param consolidation_elements list of child elements; if the new element is of any type other than libpalo_de_type_c this parameter should be an empty array (NULL might also be accepted in future API versions)
 *  @param append_consolidation_elements If a consolidated element is updated and this flag is true, the consolidation elements will be appended to the old ones.
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eadd_or_update)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, dimension_add_or_update_element_mode mode, de_type type, const struct arg_consolidation_element_info_array consolidation_elements, palo_bool append_consolidation_elements) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(eadd_or_update)(errs, so, convs, database, dimension, dimension_element, mode, type, &consolidation_elements, append_consolidation_elements), err, errs, __FUNCTION__, __FILE__, __LINE__);

}


LIBEXPORT libpalo_result LIBPALO_FUNC_R(element_create)(libpalo_err *err, struct arg_dim_element_info2_raw *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, de_type type, const struct arg_consolidation_element_info_array *consolidation_elements) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(element_create)(errs, result, so, convs, database, dimension, dimension_element, type, consolidation_elements), err, errs, __FUNCTION__, __FILE__, __LINE__);

}


/** @brief
 *  creates a bulk of elements.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param type type of the new dimension elements
 *  @param dim_elements name of the new dimension elements to be created
 *  @param consolidation_elements list of child elements; if the new elements s of any type other than libpalo_de_type_c this parameter should be an empty array
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(ecreate_bulk_simple)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, de_type type, const struct arg_str_array *dim_elements, const struct arg_consolidation_element_info_array_array *consolidation_elements) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(ecreate_bulk_simple)(errs, so, convs, database, dimension, type, dim_elements, consolidation_elements), err, errs, __FUNCTION__, __FILE__, __LINE__);

}

/** @brief
 *  Moves a dimension element inside a dimension.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param dimension_element name of the new dimension element to be moved
 *  @param new_position target position (0 is first)
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(emove)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, unsigned int new_position) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(emove)(errs, so, convs, database, dimension, dimension_element, new_position), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Exports several datasets from a cube.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param result will contains an array of palo datasets; use free_arg_getdata_export_result_contents() to free it
 *  @param database name of the database
 *  @param cube name of the cube
 *  @param export_area array of string arrays containing the elements whose values should be exported
 *  @param options options
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(getdata_export)(libpalo_err *err, struct arg_getdata_export_result *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array export_area, struct arg_getdata_export_options options) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(getdata_export)(errs, result, so, convs, database, cube, &export_area, &options), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Exports several datasets from a cube.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param result will contains an array of palo datasets; use free_arg_getdata_export_result_contents() to free it
 *  @param database name of the database
 *  @param cube name of the cube
 *  @param export_area array of string arrays containing the elements whose values should be exported
 *  @param options options
 *  @param use_rules If 1, then export rule based cell values
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(getdata_export_rule)(libpalo_err *err, struct arg_getdata_export_result *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array export_area, struct arg_getdata_export_options options, unsigned short use_rules) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(getdata_export_rule)(errs, result, so, convs, database, cube, &export_area, &options, use_rules), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Clears a partial cub or the whole cube.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param cube name of the cube
 *  @param elements array of string arrays describing the subcube to be cleared
 *  @param complete if =1 the whole cube will be cleared regardless of the content of elements
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_clear)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned short complete, const struct arg_str_array_array elements) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cube_clear)(errs, so, convs, database, cube, complete, &elements), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Copies values from one cell to another
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param cube name of the dimension
 *  @param from the source cell
 *  @param to the target cell
 *  @param withValue if 1 the paramter val is used
 *  @param val if withval=1 val is splashed to the target cell like the source cell
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cell_copy)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *from, const struct arg_str_array *to, unsigned short withValue, double val) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cell_copy)(errs, so, convs, database, cube, from, to, withValue, val), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(cell_copy_extended)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, cell_copy_mode mode, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *from, const struct arg_str_array *to, const struct arg_str_array_array *predict_area, const struct arg_str_array_2d *locked_coordinates, double* val, unsigned short use_rules) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cell_copy_extended)(errs, so, convs, mode, database, cube, from, to, predict_area, locked_coordinates, val, use_rules), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


LIBEXPORT libpalo_result LIBPALO_FUNC_R(cell_goalseek)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *path, double val) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cell_goalseek)(errs, so, convs, database, cube, path, val), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(cell_goalseek_extended)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *path, double val, goalseek_mode mode, const struct arg_str_array_array *goalseek_area) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cell_goalseek_extended)(errs, so, convs, database, cube, path, val, mode, goalseek_area), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

/** @brief
 *  get version info for the server
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param result will contains the version info
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(server_info)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, struct server_info *result) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(server_info)(errs, so, convs, result), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


LIBEXPORT libpalo_result LIBPALO_FUNC_R(api_version)(libpalo_err *err, struct api_info *result) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(PALO_SUFFIX(api_version)(errs, result), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves information about a database.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the information; use free_db_info_contents() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_info)(libpalo_err *err, struct arg_db_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(database_info)(errs, result, so, convs, database), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves information about a dimension.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the information; use free_dim_info_contents() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_info)(libpalo_err *err, struct arg_dim_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(dimension_info)(errs, result, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_info_simple)(libpalo_err *err, struct arg_dim_info_simple *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(dimension_info_simple)(errs, result, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

/** @brief
 *  Retrieves information about a element.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the information; use free_dim_element_info2_raw_contents() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param dimension name of the dimension
 *  @param element name of the element
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(element_info_simple)(libpalo_err *err, struct arg_dim_element_info2_raw *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *element) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(element_info_simple)(errs, result, so, convs, database, dimension, element), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves information about a cube.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param result will contain the information; use free_cube_info_contents() to free it
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param cube name of the cube
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_info)(libpalo_err *err, struct arg_cube_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cube_info)(errs, result, so, convs, database, cube), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


LIBEXPORT libpalo_result LIBPALO_FUNC_R(set_client_version)(libpalo_err *err, struct conversions *convs, const CHAR_T *client_version) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(PALO_SUFFIX(set_client_version)(errs, convs, client_version), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


LIBEXPORT libpalo_result LIBPALO_FUNC_R(is_admin_user)(libpalo_err *err, unsigned short int *retresult, struct sock_obj *so, struct conversions *convs) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(PALO_SUFFIX(is_admin_user)(errs, retresult, so, convs), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(is_gpu_server)(libpalo_err *err, unsigned short int *retresult, struct sock_obj *so, struct conversions *convs) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(PALO_SUFFIX(is_gpu_server)(errs, retresult, so, convs), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_rights)(libpalo_err *err, permission_type *retresult, struct sock_obj *so, struct conversions *convs, permission_art pa) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(PALO_SUFFIX(get_rights)(errs, retresult, so, convs, pa), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(set_user_password)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *user, const CHAR_T *password) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(set_user_password)(errs, so, convs, user, password), err, errs, __FUNCTION__, __FILE__, __LINE__);
}



/** @brief
 *  Change password.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param oldpassword old password
 *  @param newpassword new password
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */

LIBEXPORT libpalo_result LIBPALO_FUNC_R(change_password)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *oldpassword, const CHAR_T *newpassword) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(change_password)(errs, so, convs, oldpassword, newpassword), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  get version info for the libbrary
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param result will contains the version info
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(lib_version)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, struct api_info *result) {
	ERRSTACK_DEFINE_ONLY()

	result->major_version = 5;
	result->minor_version = 1;
	result->bugfix_version = 0;
	result->build_number = 7075;

	return _to_libpalo_result(PALO_SUCCESS, err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Add a rule for a cube
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param result will contain a rule info; use free_arg_rule_info_contents() to free it
 *  @param database name of the database
 *  @param cube name of the cube
 *  @param definition the defintion of the rule already csv encoded
 *  @param use_identifier Use identifier in textual representation of the rule in the result
 *  @param extern_id an extern id of the rule, can be NULL
 *  @param comment a comment for the rule, can be NULL
 *  @param activate if 0, the rule will be deactivated
 *  @param position: if 0, the rule will be added to the end of list (max_position+1)
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(rule_add)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const CHAR_T *definition, unsigned short use_identifier, const CHAR_T *extern_id, const CHAR_T *comment, unsigned short activate, double position) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(rule_add)(errs, result, so, convs, database, cube, definition, use_identifier, extern_id, comment, activate, position), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Modifies a rule for a cube
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param result will contain a rule info; use free_arg_rule_info_contents() to free it
 *  @param database name of the database
 *  @param cube name of the cube
 *  @param id the id of the rule
 *  @param definition the defintion of the rule already csv encoded
		   if NULL or empty, the definition will not be changed.
 *  @param use_identifier Use identifier in textual representation of the rule in the result
 *  @param extern_id an extern id of the rule, can be NULL
 *  @param comment a comment for the rule, can be NULL
 *  @param activate if 0, the rule will be deactivated
 *  @param position: if 0, the rule position will not change
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(rule_modify)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, long id, const CHAR_T *definition, unsigned short use_identifier, const CHAR_T *extern_id, const CHAR_T *comment, unsigned short activate, double position) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(rule_modify)(errs, result, so, convs, database, cube, id, definition, use_identifier, extern_id, comment, activate, position), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(rules_activate)(libpalo_err *err, struct arg_rule_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids, rule_activation_mode mode) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(rules_activate)(errs, result, so, convs, database, cube, len, ids, mode), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Get Information about a rule for a cube
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param result will contain a rule info; use free_arg_rule_info_contents() to free it
 *  @param database name of the database
 *  @param cube name of the cube
 *  @param id the id of the rule
 *  @param use_identifier Use identifier in textual representation of the rule in the result
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(rule_info)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, long id, unsigned short use_identifier) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(rule_info)(errs, result, so, convs, database, cube, id, use_identifier), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Retrieves the rule which is applied for a cell from the PALO server
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *
 *  @param result will be set to rule information for the referenced cell; use free_arg_rule_info_contents() to free resources associated with its contents
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the requested data
 *  @param cube name of the cube containing the requested data
 *  @param coordinates an array of strings specifying the cell containing the requested data
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cell_rule_info)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array coordinates) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cell_rule_info)(errs, result, so, convs, database, cube,  &coordinates), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  parse a rule for a cube
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param result will contain the XML representation; use free_arg_str_array_contents() to free it
 *  @param database name of the database
 *  @param cube name of the cube
 *  @param definition the defintion of the rule already csv encoded
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(rule_parse)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const CHAR_T *definition) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(rule_parse)(errs, result, so, convs, database, cube, definition), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Delete a rule for a cube
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database
 *  @param cube name of the cube
 *  @param identifier the identifier of the rule already to be deleted
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(rule_delete)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, long identifier) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(rule_delete)(errs, so, convs, database, cube, identifier), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(rules_delete)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(rules_delete)(errs, so, convs, database, cube, len, ids), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(rules_move)(libpalo_err *err, struct arg_rule_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids, double start_position, double below_position) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(rules_move)(errs, result, so, convs, database, cube, len, ids, start_position, below_position), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

/** @brief
 *  List the rules for a cube
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param result will contains an array of rule infos; use free_arg_rule_info_array_contents() to free it
 *  @param database name of the database
 *  @param cube name of the cube
 *  @param use_identifier Use identifier in textual representation of the rule in the result
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(list_rules)(libpalo_err *err, struct arg_rule_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned short use_identifier) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(list_rules)(errs, result, so, convs, database, cube, use_identifier), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  list the rule functions
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param result will contain the rule functions in XML representation; use free_arg_str_array_contents() to free it
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(list_rulefunctions)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(list_rulefunctions)(errs, result, so, convs), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Set a lock on a subcube
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *
 *  @param result will be set to lock information; use free_arg_lock_info_contents() to free resources associated with its contents
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the requested data
 *  @param cube name of the cube containing the requested data
 *  @param coordinates an array of strings specifying the cell containing the requested data
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_lock)(libpalo_err *err, struct arg_lock_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array_array locked_area) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cube_lock)(errs, result, so, convs, database, cube, &locked_area), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  get a list of all locks of a cube
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *
 *  @param result will hold the list of all lock information; use free_arg_lock_info_contents() to free resources associated with its contents
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the requested data
 *  @param cube name of the cube containing the requested data
 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_locks)(libpalo_err *err, struct arg_lock_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cube_locks)(errs, result, so, convs, database, cube), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Rollback steps for a lock
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the requested data
 *  @param cube name of the cube containing the requested data
 *  @param lockid identifier of the lock
 *  @param steps number of steps to rollback (-1 means rollback all steps and release lock)

 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_rollback)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned long lockid, long steps) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cube_rollback)(errs, so, convs, database, cube, lockid, steps), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @brief
 *  Commit a lock
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *
 *  @param so reference to a struct sock_obj initialized by a preceding call to palo_connect
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *
 *  @param database name of the database containing the requested data
 *  @param cube name of the cube containing the requested data
 *  @param lockid identifier of the lock

 *
 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_commit)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned long lockid) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cube_commit)(errs, so, convs, database, cube, lockid), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_svs_info)(libpalo_err *err, struct arg_svs_info *result, struct sock_obj *so, struct conversions *convs) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(PALO_SUFFIX(get_svs_info)(errs, result, so, convs), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


LIBEXPORT libpalo_result LIBPALO_FUNC_R(event_lock_begin)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, CHAR_T *source, CHAR_T *AreaID) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(event_lock_begin)(errs, so, convs, source, AreaID), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


LIBEXPORT libpalo_result LIBPALO_FUNC_R(event_lock_end)(libpalo_err *err, struct sock_obj *so, struct conversions *convs) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(event_lock_end)(errs, so, convs), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_subset)(libpalo_err *err, struct arg_subset_result_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, int indent,
															struct arg_alias_filter_settings *alias,
															struct arg_field_filter_settings *field,
															struct arg_basic_filter_settings *basic,
															struct arg_data_filter_settings *data,
															struct arg_sorting_filter_settings *sorting,
															struct arg_structural_filter_settings *structural,
															struct arg_text_filter_settings *text) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(dimension_subset)(errs, result, so, convs, database, dimension, indent,alias, field, basic, data, sorting, structural, text), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(cell_drill_through)(libpalo_err *err, struct arg_str_array_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *coordinates, drillthrough_type mode) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(cell_drill_through)(errs, result, so, convs, database, cube, coordinates, mode), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(edelete_bulk)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const struct arg_str_array *dimension_elements) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(PALO_SUFFIX(edelete_bulk)(errs, so, convs, database, dimension, dimension_elements), err, errs, __FUNCTION__, __FILE__, __LINE__);
}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_convert)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, cube_type newtype) {
	ERRSTACK_DEFINE()

 	return _to_libpalo_result(PALO_SUFFIX(cube_convert)(errs, so, convs, database, cube, newtype), err, errs, __FUNCTION__, __FILE__, __LINE__);

}

LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_flat_count)(libpalo_err *err, size_t *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	ERRSTACK_DEFINE()

 	return _to_libpalo_result(PALO_SUFFIX(dimension_flat_count)(errs, retresult, so, convs, database, dimension), err, errs, __FUNCTION__, __FILE__, __LINE__);

}


static palo_err LIBPALO_FUNC_R(_connect)(struct errstack *errs, struct conversions *convs, const CHAR_T *hostname, const CHAR_T *service, struct sock_obj *so) {
	char *ahostname, *aservice;
	palo_err result;

	assert(hostname != NULL);
	assert(service != NULL);
	assert(so != NULL);
	assert(errs != NULL);

	result = STR_TO_LOCAL(convs, &ahostname, hostname);
	if (result != PALO_SUCCESS)
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);

	result = STR_TO_LOCAL(convs, &aservice, service);
	if (result != PALO_SUCCESS) {
		free(ahostname);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = _palo_connect_a(errs, ahostname, aservice, so);
	free(ahostname);
	free(aservice);
	if (result != PALO_SUCCESS)
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, NULL);

	return PALO_SUCCESS;
}


/*  @brief
 *  Establishes a connection with the server at hostname:servicename.
 *
 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free_contents()
 *  @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 *  @param hostname host name
 *  @param service service name (e.g. port)
 *  @param so pointer to the place that will be used to store connection specific data; call palo_disconnect() to close the connection and release connection specific data
 *
 *  @return returns NULL on success or a pointer to an error message on failure
 */
LIBEXPORT libpalo_result LIBPALO_FUNC_R(connect)(libpalo_err *err, struct conversions *convs, const CHAR_T *hostname, const CHAR_T *service, struct sock_obj *so) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(LIBPALO_FUNC_R(_connect)(errs, convs, hostname, service, so), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


static palo_err LIBPALO_LIBFUNC_R(_cache_add_request)(struct errstack *errs, libpalo_cache *c, struct sock_obj *so, struct conversions *convs, const CHAR_T *key, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *coordinates) {
	char *mkey = NULL;
	char *mdatabase = NULL;
	char *mcube = NULL;
	struct arg_str_array_m mcoordinates;
	size_t i;
	palo_err result;

	assert(c != NULL);
	assert(so != NULL);
	assert(key != NULL);
	assert(database != NULL);
	assert(cube != NULL);
	assert(coordinates != NULL);
	assert(errs != NULL);

	mcoordinates.a = NULL;
	mcoordinates.len = 0;

	if ((result = STR_TO_UTF8(convs, &mkey, key)) != PALO_SUCCESS
	        || (result = STR_TO_UTF8(convs, &mdatabase, database)) != PALO_SUCCESS
	        || (result = STR_TO_UTF8(convs, &mcube, cube)) != PALO_SUCCESS) {
		ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
		goto cleanup;
	}

	if ((mcoordinates.a = (char**)calloc(coordinates->len, sizeof(char*))) == NULL) {
		ERRSTACK_PREPARE(errs, PALO_ERR_NO_MEM, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_NO_MEM);
		goto cleanup;
	}
	mcoordinates.len = coordinates->len;

	for (i = 0; i < coordinates->len; i++) {
		if ((result = STR_TO_UTF8(convs, mcoordinates.a + i, coordinates->a[i])) != PALO_SUCCESS) {
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
			goto cleanup;
		}
	}

	result = _libpalo_cache_add_request_m_r(errs, c, so, convs, mkey, mdatabase, mcube, &mcoordinates);
	if (result != PALO_SUCCESS)
		ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, NULL);
	else {
		free(mkey);
		return PALO_SUCCESS;	 /* no other free()'s needed */
	}

cleanup:
	if (mkey != NULL)
		free(mkey);
	if (mdatabase != NULL)
		free(mdatabase);
	if (mcube != NULL)
		free(mcube);
	free_arg_str_array_contents_m(&mcoordinates);

	return result;
}


LIBEXPORT libpalo_result LIBPALO_LIBFUNC_R(cache_add_request)(libpalo_err *err, libpalo_cache *c, struct sock_obj *so, struct conversions *convs, const CHAR_T *key, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array coordinates) {
	ERRSTACK_DEFINE()

	return _to_libpalo_result(LIBPALO_LIBFUNC_R(_cache_add_request)(errs, c, so, convs, key, database, cube, &coordinates), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


static palo_err LIBPALO_LIBFUNC_R(_cache_get_result)(struct errstack *errs, libpalo_cache *c, struct conversions *convs, const CHAR_T *key, struct arg_palo_value *value) {
	char *mkey;
	palo_err result;
	struct arg_palo_value_m *mval;

	assert(c != NULL);
	assert(convs != NULL);
	assert(key != NULL);
	assert(value != NULL);
	assert(errs != NULL);

	result = STR_TO_UTF8(convs, &mkey, key);
	if (result != PALO_SUCCESS)
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);

	result = _libpalo_cache_get_result_m_r(errs, c, convs, mkey, &mval);
	free(mkey);
	if (result != PALO_SUCCESS)
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, NULL);

	if (mval->type == argPaloValueTypeStr) {
		result = UTF8_TO_STR(convs, &value->val.s, mval->val.s);
		if (result != PALO_SUCCESS)
			ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	} else if (mval->type == argPaloValueTypeDouble) {
		value->type = argPaloValueTypeDouble;
		value->val.d = mval->val.d;
	} else if (mval->type == argPaloValueTypeError) {
		ERRSTACK_RETURN(errs, PALO_ERR_CLIENT_SERVER_RETURNED_ERROR, __FUNCTION__, __LINE__, __FILE__, wcsdup(L"the server has returned an error: %d"), mval->val.err);
	} else {
		ERRSTACK_RETURN(errs, PALO_ERR_TYPE, __FUNCTION__, __LINE__, __FILE__, wcsdup(L"returned value has unknown type"));
	}

	value->type = mval->type;

	return PALO_SUCCESS;
}


LIBEXPORT libpalo_result LIBPALO_LIBFUNC_R(cache_get_result)(libpalo_err *err, libpalo_cache *c, struct conversions *convs, const CHAR_T *key, struct arg_palo_value *value) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(LIBPALO_LIBFUNC_R(_cache_get_result)(errs, c, convs, key, value), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


/** @ingroup gen_funcs
 *  @brief
 *  Formats's error information returned by library functions.
 *
 * @param convs reference to a struct conversions initialized by a preceding call to libpalo_init_r()
 * @param err pointer to the error descriptor
 *
 * @return pointer to the formatted string (use free_arg_str() to free it) or NULL if something went wrong
 */
LIBEXPORT CHAR_T *LIBPALO_LIBFUNC_R(err_get_string)(struct conversions *convs, const libpalo_err *err) {
	wchar_t *w;
	CHAR_T *s;
	palo_err result;

	w = errstack_errmsg_short(&err->errs, err->palo_error);
	if (w == NULL) {
		return NULL;
	}

	result = WCS_TO_STR(convs, &s, w);
	free(w);
	if (result != PALO_SUCCESS) {
		return NULL;
	}

	return s;
}


/** @} */

/** @defgroup gen_funcs_st generic functions (not thread-safe)
 *  @ingroup gen_funcs
 *  Functions which can be used in singlethreaded applications. Each of these functions is a wrapper for its corresponding thread-safe version. Follow the links for further documentation.
 *	@{
 */

/** @brief see thread-safe etoplevel() */
LIBEXPORT libpalo_result LIBPALO_FUNC(etoplevel)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(etoplevel)(err, result, so, &libpalo_convs, database, dimension);
}


/** @brief see thread-safe getdata_area() */
LIBEXPORT libpalo_result LIBPALO_FUNC(getdata_area)(libpalo_err *err, struct arg_palo_value_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array coordinates, palo_bool guess_coordinates_order) {
	return LIBPALO_FUNC_R(getdata_area)(err, result, so, &libpalo_convs, database, cube, coordinates, guess_coordinates_order);
}


/** @brief see thread-safe getdata_area2() */
LIBEXPORT libpalo_result LIBPALO_FUNC(getdata_area2)(libpalo_err *err, struct arg_palo_dataset_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array coordinates, palo_bool guess_coordinates_order) {
	return LIBPALO_FUNC_R(getdata_area2)(err, result, so, &libpalo_convs, database, cube, coordinates, guess_coordinates_order);
}


/** @brief see thread-safe getdata_multi() */
LIBEXPORT libpalo_result LIBPALO_FUNC(getdata_multi)(libpalo_err *err, struct arg_palo_value_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_2d coordinates) {
	return LIBPALO_FUNC_R(getdata_multi)(err, result, so, &libpalo_convs, database, cube, coordinates);
}


/** @brief see thread-safe elevel() */
LIBEXPORT libpalo_result LIBPALO_FUNC(elevel)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	return LIBPALO_FUNC_R(elevel)(err, result, so, &libpalo_convs, database, dimension, dimension_element);
}


/** @brief see thread-safe eindent() */
LIBEXPORT libpalo_result LIBPALO_FUNC(eindent)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	return LIBPALO_FUNC_R(eindent)(err, result, so, &libpalo_convs, database, dimension, dimension_element);
}


/** @brief see thread-safe eweight() */
LIBEXPORT libpalo_result LIBPALO_FUNC(eweight)(libpalo_err *err, double *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_parent, const CHAR_T *de_child) {
	return LIBPALO_FUNC_R(eweight)(err, result, so, &libpalo_convs, database, dimension, de_parent, de_child);
}


/** @brief see thread-safe dimension_delete() */
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_delete)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(dimension_delete)(err, so, &libpalo_convs, database, dimension);
}


/** @brief see thread-safe dimension_clear() */
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_clear)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(dimension_clear)(err, so, &libpalo_convs, database, dimension);
}


/** @brief see thread-safe esibling() */
LIBEXPORT libpalo_result LIBPALO_FUNC(esibling)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, int n) {
	return LIBPALO_FUNC_R(esibling)(err, result, so, &libpalo_convs, database, dimension, dimension_element, n);
}


/** @brief see thread-safe etype() */
LIBEXPORT libpalo_result LIBPALO_FUNC(etype)(libpalo_err *err, de_type *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	return LIBPALO_FUNC_R(etype)(err, result, so, &libpalo_convs, database, dimension, dimension_element);
}


/** @brief see thread-safe eprev() */
LIBEXPORT libpalo_result LIBPALO_FUNC(eprev)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	return LIBPALO_FUNC_R(eprev)(err, result, so, &libpalo_convs, database, dimension, dimension_element);
}


/** @brief see thread-safe enext() */
LIBEXPORT libpalo_result LIBPALO_FUNC(enext)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	return LIBPALO_FUNC_R(enext)(err, result, so, &libpalo_convs, database, dimension, dimension_element);
}


/** @brief see thread-safe eparentname() */
LIBEXPORT libpalo_result LIBPALO_FUNC(eparentname)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, int n) {
	return LIBPALO_FUNC_R(eparentname)(err, result, so, &libpalo_convs, database, dimension, dimension_element, n);
}


/** @brief see thread-safe eparentcount() */
LIBEXPORT libpalo_result LIBPALO_FUNC(eparentcount)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	return LIBPALO_FUNC_R(eparentcount)(err, result, so, &libpalo_convs, database, dimension, dimension_element);
}


/** @brief see thread-safe ename() */
LIBEXPORT libpalo_result LIBPALO_FUNC(ename)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, int n) {
	return LIBPALO_FUNC_R(ename)(err, result, so, &libpalo_convs, database, dimension, n);
}


/** @brief see thread-safe eischild() */
LIBEXPORT libpalo_result LIBPALO_FUNC(eischild)(libpalo_err *err, palo_bool *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_parent, const CHAR_T *de_child) {
	return LIBPALO_FUNC_R(eischild)(err, result, so, &libpalo_convs, database, dimension, de_parent, de_child);
}


/** @brief see thread-safe eindex() */
LIBEXPORT libpalo_result LIBPALO_FUNC(eindex)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	return LIBPALO_FUNC_R(eindex)(err, result, so, &libpalo_convs, database, dimension, dimension_element);
}


/** @brief see thread-safe efirst() */
LIBEXPORT libpalo_result LIBPALO_FUNC(efirst)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(efirst)(err, result, so, &libpalo_convs, database, dimension);
}


/** @brief see thread-safe ecount() */
LIBEXPORT libpalo_result LIBPALO_FUNC(ecount)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(ecount)(err, result, so, &libpalo_convs, database, dimension);
}


/** @brief see thread-safe echildcount() */
LIBEXPORT libpalo_result LIBPALO_FUNC(echildcount)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	return LIBPALO_FUNC_R(echildcount)(err, result, so, &libpalo_convs, database, dimension, dimension_element);
}


/** @brief see thread-safe echildname() */
LIBEXPORT libpalo_result LIBPALO_FUNC(echildname)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, int n) {
	return LIBPALO_FUNC_R(echildname)(err, result, so, &libpalo_convs, database, dimension, dimension_element, n);
}


/** @brief see thread-safe database_save() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_save)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database) {
	return LIBPALO_FUNC_R(database_save)(err, so, &libpalo_convs, database);
}


/** @brief see thread-safe cube_commit_log() */
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_commit_log)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube) {
	return LIBPALO_FUNC_R(cube_commit_log)(err, so, &libpalo_convs, database, cube);
}



/** @brief see thread-safe cube_unload() */
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_unload)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube) {
	return LIBPALO_FUNC_R(cube_unload)(err, so, &libpalo_convs, database, cube);
}


/** @brief see thread-safe cube_delete() */
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_delete)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube) {
	return LIBPALO_FUNC_R(cube_delete)(err, so, &libpalo_convs, database, cube);
}


/** @brief see thread-safe cube_load() */
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_load)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube) {
	return LIBPALO_FUNC_R(cube_load)(err, so, &libpalo_convs, database, cube);
}


/** @brief see thread-safe database_unload() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_unload)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database) {
	return LIBPALO_FUNC_R(database_unload)(err, so, &libpalo_convs, database);
}


/** @brief see thread-safe database_delete() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_delete)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database) {
	return LIBPALO_FUNC_R(database_delete)(err, so, &libpalo_convs, database);
}


/** @brief see thread-safe database_load() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_load)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database) {
	return LIBPALO_FUNC_R(database_load)(err, so, &libpalo_convs, database);
}


/** @brief see thread-safe server_shutdown() */
LIBEXPORT libpalo_result LIBPALO_FUNC(server_shutdown)(libpalo_err *err, struct sock_obj *so) {
	return LIBPALO_FUNC_R(server_shutdown)(err, so, &libpalo_convs);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(init_ssl)(libpalo_err *err, const CHAR_T *trustfile) {
	return LIBPALO_FUNC_R(init_ssl)(err, &libpalo_convs, trustfile);
}

/** @brief see thread-safe auth() */
LIBEXPORT libpalo_result LIBPALO_FUNC(auth)(libpalo_err *err, struct sock_obj *so, const CHAR_T *username, const CHAR_T *pw_hash) {
	return LIBPALO_FUNC_R(auth)(err, so, &libpalo_convs, username, pw_hash);
}

/** @brief see thread-safe auth_ssl() */
LIBEXPORT libpalo_result LIBPALO_FUNC(auth_ssl)(libpalo_err *err, struct sock_obj *so, const CHAR_T *username, const CHAR_T *pw_hash, const CHAR_T *trustfile) {
	return LIBPALO_FUNC_R(auth_ssl)(err, so, &libpalo_convs, username, pw_hash, trustfile);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(test_connection)(libpalo_err *err, const CHAR_T *hostname, unsigned int service, const CHAR_T *username, const CHAR_T *pw_hash, const CHAR_T *trustfile) {
	return LIBPALO_FUNC_R(test_connection)(err, &libpalo_convs, hostname, service, username, pw_hash, trustfile);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(check_validity)(libpalo_err *err, struct sock_obj *so) {
	return LIBPALO_FUNC_R(check_validity)(err, so, &libpalo_convs);
}

/** @brief see thread-safe ping() */
LIBEXPORT libpalo_result LIBPALO_FUNC(ping)(libpalo_err *err, struct sock_obj *so) {
	return LIBPALO_FUNC_R(ping)(err, so, &libpalo_convs);
}


/** @brief see thread-safe getdata() */
LIBEXPORT libpalo_result LIBPALO_FUNC(getdata)(libpalo_err *err, struct arg_palo_value *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array coordinates) {
	return LIBPALO_FUNC_R(getdata)(err, result, so, &libpalo_convs, database, cube, coordinates);
}


/** @brief see thread-safe setdata() */
LIBEXPORT libpalo_result LIBPALO_FUNC(setdata)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube,  const struct arg_palo_dataset ds, splash_mode mode) {
	return LIBPALO_FUNC_R(setdata)(err, so, &libpalo_convs, database, cube, ds, mode);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(setdata_extended)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube,  const struct arg_palo_dataset ds, splash_mode mode, unsigned short add, unsigned short eventprocessor, const struct arg_str_array_2d *locked_coordinates) {
	return LIBPALO_FUNC_R(setdata_extended)(err, so, &libpalo_convs, database, cube, ds, mode, add, eventprocessor, locked_coordinates);
}

/** @brief see thread-safe setdata_multi() */
LIBEXPORT libpalo_result LIBPALO_FUNC(setdata_multi)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset_array dsa, splash_mode mode, unsigned short add, unsigned short eventprocessor) {

	return LIBPALO_FUNC_R(setdata_multi)(err, so, &libpalo_convs, database, cube, dsa, mode, add, eventprocessor);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(setdata_multi_extended)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset_array dsa, splash_mode mode, unsigned short add, unsigned short eventprocessor, const struct arg_str_array_2d *locked_coordinates) {

	return LIBPALO_FUNC_R(setdata_multi_extended)(err, so, &libpalo_convs, database, cube, dsa, mode, add, eventprocessor, locked_coordinates);
}


/** @brief see thread-safe root_list_databases() */
LIBEXPORT libpalo_result LIBPALO_FUNC(root_list_databases)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so) {
	return LIBPALO_FUNC_R(root_list_databases)(err, result, so, &libpalo_convs);
}


/** @brief see thread-safe root_list_databases2() */
LIBEXPORT libpalo_result LIBPALO_FUNC(root_list_databases2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, int db_type) {
	return LIBPALO_FUNC_R(root_list_databases2)(err, result, so, &libpalo_convs, db_type);
}


/** @brief see thread-safe database_list_cubes() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_list_cubes)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, const CHAR_T *database) {
	return LIBPALO_FUNC_R(database_list_cubes)(err, result, so, &libpalo_convs, database);
}


/** @brief see thread-safe database_list_cubes2() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_list_cubes2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, const CHAR_T *database, int cube_type, unsigned short int only_cubes_with_cells) {
	return LIBPALO_FUNC_R(database_list_cubes2)(err, result, so, &libpalo_convs, database, cube_type, only_cubes_with_cells);
}


/** @brief see thread-safe database_list_dimensions() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_list_dimensions)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, const CHAR_T *database) {
	return LIBPALO_FUNC_R(database_list_dimensions)(err, result, so, &libpalo_convs, database);
}


/** @brief see thread-safe database_list_dimensions2() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_list_dimensions2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, const CHAR_T *database, int dim_type) {
	return LIBPALO_FUNC_R(database_list_dimensions2)(err, result, so, &libpalo_convs, database, dim_type);
}


/** @brief see thread-safe element_list_consolidation_elements() */
LIBEXPORT libpalo_result LIBPALO_FUNC(element_list_consolidation_elements)(libpalo_err *err, struct arg_consolidation_element_info_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	return LIBPALO_FUNC_R(element_list_consolidation_elements)(err, result, so, &libpalo_convs, database, dimension, dimension_element);
}


/** @brief see thread-safe dimension_list_elements() */
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_list_elements)(libpalo_err *err, struct arg_dim_element_info_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(dimension_list_elements)(err, result, so, &libpalo_convs, database, dimension);
}


/** @brief see thread-safe dimension_list_elements2_raw() */
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_list_elements2_raw)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(dimension_list_elements2_raw)(err, result, so, &libpalo_convs, database, dimension);
}

/** @brief see thread-safe dimension_restricted_flat_list_elements() */
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_restricted_flat_list_elements)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, long start, long limit) {
	return LIBPALO_FUNC_R(dimension_restricted_flat_list_elements)(err, result, so, &libpalo_convs, database, dimension, start, limit);
}

/** @brief see thread-safe dimension_restricted_top_list_elements() */
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_restricted_top_list_elements)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, long start, long limit) {
	return LIBPALO_FUNC_R(dimension_restricted_top_list_elements)(err, result, so, &libpalo_convs, database, dimension, start, limit);
}

/** @brief see thread-safe dimension_restricted_children_list_elements() */
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_restricted_children_list_elements)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, long element_identifier, long start, long limit) {
	return LIBPALO_FUNC_R(dimension_restricted_children_list_elements)(err, result, so, &libpalo_convs, database, dimension, element_identifier, start, limit);
}

/** @brief see thread-safe get_database_type() */

LIBEXPORT libpalo_result LIBPALO_FUNC(get_database_type)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database) {
	return LIBPALO_FUNC_R(get_database_type)(err, result, so, &libpalo_convs, database);
}


/** @brief see thread-safe get_dimension_type() */

LIBEXPORT libpalo_result LIBPALO_FUNC(get_dimension_type)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(get_dimension_type)(err, result, so, &libpalo_convs, database, dimension);
}


/** @brief see thread-safe get_cube_type() */

LIBEXPORT libpalo_result LIBPALO_FUNC(get_cube_type)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube) {
	return LIBPALO_FUNC_R(get_cube_type)(err, result, so, &libpalo_convs, database, cube);
}


/** @brief see thread-safe get_attribute_dimension() */

LIBEXPORT libpalo_result LIBPALO_FUNC(get_attribute_dimension)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(get_attribute_dimension)(err, result, so, &libpalo_convs, database, dimension);
}


/** @brief see thread-safe get_attribute_cube() */

LIBEXPORT libpalo_result LIBPALO_FUNC(get_attribute_cube)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(get_attribute_cube)(err, result, so, &libpalo_convs, database, dimension);
}


/** @brief see thread-safe get_rights_cube() */

LIBEXPORT libpalo_result LIBPALO_FUNC(get_rights_cube)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(get_rights_cube)(err, result, so, &libpalo_convs, database, dimension);
}


/** @brief see thread-safe cube_list_dimensions() */
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_list_dimensions)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube) {
	return LIBPALO_FUNC_R(cube_list_dimensions)(err, result, so, &libpalo_convs, database, cube);
}


/** @brief see thread-safe dimension_list_cubes() */
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_list_cubes)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(dimension_list_cubes)(err, result, so, &libpalo_convs, database, dimension);
}


/** @brief see thread-safe dimension_list_cubes2() */
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_list_cubes2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, int cube_type, unsigned short int only_cubes_with_cells) {
	return LIBPALO_FUNC_R(dimension_list_cubes2)(err, result, so, &libpalo_convs, database, dimension, cube_type, only_cubes_with_cells);
}


/** @brief see thread-safe erename() */
LIBEXPORT libpalo_result LIBPALO_FUNC(erename)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_oldname, const CHAR_T *de_newname) {
	return LIBPALO_FUNC_R(erename)(err, so, &libpalo_convs, database, dimension, de_oldname, de_newname);
}


/** @brief see thread-safe edelete() */
LIBEXPORT libpalo_result LIBPALO_FUNC(edelete)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	return LIBPALO_FUNC_R(edelete)(err, so, &libpalo_convs, database, dimension, dimension_element);
}


/** @brief see thread-safe eadd_or_update() */
LIBEXPORT libpalo_result LIBPALO_FUNC(eadd_or_update)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, dimension_add_or_update_element_mode mode, de_type type, const struct arg_consolidation_element_info_array consolidation_elements, palo_bool append_consolidation_elements) {
	return LIBPALO_FUNC_R(eadd_or_update)(err, so, &libpalo_convs, database, dimension, dimension_element, mode, type, consolidation_elements, append_consolidation_elements);
}



LIBEXPORT libpalo_result LIBPALO_FUNC(element_create)(libpalo_err *err, struct arg_dim_element_info2_raw *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, de_type type, const struct arg_consolidation_element_info_array *consolidation_elements) {
	return LIBPALO_FUNC_R(element_create)(err, result, so, &libpalo_convs, database, dimension, dimension_element, type, consolidation_elements);
}

/** @brief see thread-safe ecreate_bulk_simple() */
LIBEXPORT libpalo_result LIBPALO_FUNC(ecreate_bulk_simple)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, de_type type, const struct arg_str_array *dim_elements, const struct arg_consolidation_element_info_array_array *consolidation_elements) {
	return LIBPALO_FUNC_R(ecreate_bulk_simple)(err, so, &libpalo_convs, database, dimension, type, dim_elements, consolidation_elements);
}


/** @brief see thread-safe emove() */
LIBEXPORT libpalo_result LIBPALO_FUNC(emove)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, unsigned int new_position) {
	return LIBPALO_FUNC_R(emove)(err, so, &libpalo_convs, database, dimension, dimension_element, new_position);
}


/** @brief see thread-safe edimension() */
LIBEXPORT libpalo_result LIBPALO_FUNC(edimension)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, struct arg_str_array dimension_elements, palo_bool should_be_unique) {
	return LIBPALO_FUNC_R(edimension)(err, result, so, &libpalo_convs, database, dimension_elements, should_be_unique);
}


/** @brief see thread-safe ecubedimension() */
LIBEXPORT libpalo_result LIBPALO_FUNC(ecubedimension)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, struct arg_str_array dimension_elements, palo_bool should_be_unique) {
	return LIBPALO_FUNC_R(ecubedimension)(err, result, so, &libpalo_convs, database, cube, dimension_elements, should_be_unique);
}


/** @brief see thread-safe root_add_database() */
LIBEXPORT libpalo_result LIBPALO_FUNC(root_add_database)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database) {
	return LIBPALO_FUNC_R(root_add_database)(err, so, &libpalo_convs, database);
}


/** @brief see thread-safe database_add_dimension() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_add_dimension)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(database_add_dimension)(err, so, &libpalo_convs, database, dimension);
}


/** @brief see thread-safe database_add_dimension2() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_add_dimension2)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, dim_type type) {
	return LIBPALO_FUNC_R(database_add_dimension2)(err, so, &libpalo_convs, database, dimension, type);
}


/** @brief see thread-safe database_rename_cube() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_rename_cube)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube_oldname, const CHAR_T *cube_newname) {
	return LIBPALO_FUNC_R(database_rename_cube)(err, so, &libpalo_convs, database, cube_oldname, cube_newname);
}


/** @brief see thread-safe database_rename_dimension() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_rename_dimension)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension_oldname, const CHAR_T *dimension_newname) {
	return LIBPALO_FUNC_R(database_rename_dimension)(err, so, &libpalo_convs, database, dimension_oldname, dimension_newname);
}


/** @brief see thread-safe database_add_cube() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_add_cube)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array dimensions) {
	return LIBPALO_FUNC_R(database_add_cube)(err, so, &libpalo_convs, database, cube, dimensions);
}


/** @brief see thread-safe database_add_cube2() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_add_cube2)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array dimensions, cube_type type) {
	return LIBPALO_FUNC_R(database_add_cube2)(err, so, &libpalo_convs, database, cube, dimensions, type);
}


/** @brief see thread-safe getdata_export() */
LIBEXPORT libpalo_result LIBPALO_FUNC(getdata_export)(libpalo_err *err, struct arg_getdata_export_result *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array export_area, struct arg_getdata_export_options options) {
	return LIBPALO_FUNC_R(getdata_export)(err, result, so, &libpalo_convs, database, cube, export_area, options);
}


/** @brief see thread-safe getdata_export_rule() */
LIBEXPORT libpalo_result LIBPALO_FUNC(getdata_export_rule)(libpalo_err *err, struct arg_getdata_export_result *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array export_area, struct arg_getdata_export_options options, unsigned short use_rules) {
	return LIBPALO_FUNC_R(getdata_export_rule)(err, result, so, &libpalo_convs, database, cube, export_area, options, use_rules);
}


/** @brief see thread-safe cube_clear() */
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_clear)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, unsigned short complete, const struct arg_str_array_array elements) {
	return LIBPALO_FUNC_R(cube_clear)(err, so, &libpalo_convs, database, cube, complete, elements);
}


/** @brief see thread-safe cell_copy() */
LIBEXPORT libpalo_result LIBPALO_FUNC(cell_copy)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *from, const struct arg_str_array *to, unsigned short withValue, double val) {
	return LIBPALO_FUNC_R(cell_copy)(err, so, &libpalo_convs, database, cube, from, to, withValue, val);
}


LIBEXPORT libpalo_result LIBPALO_FUNC(cell_copy_extended)(libpalo_err *err, struct sock_obj *so, cell_copy_mode mode, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *from, const struct arg_str_array *to, const struct arg_str_array_array *predict_area, const struct arg_str_array_2d *locked_coordinates, double* val, unsigned short use_rules) {
	return LIBPALO_FUNC_R(cell_copy_extended)(err, so, &libpalo_convs, mode, database, cube, from, to, predict_area, locked_coordinates, val, use_rules);
}


LIBEXPORT libpalo_result LIBPALO_FUNC(cell_goalseek)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *path, double val) {
	return LIBPALO_FUNC_R(cell_goalseek)(err, so, &libpalo_convs, database, cube, path, val);
}


LIBEXPORT libpalo_result LIBPALO_FUNC(cell_goalseek_extended)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *path, double val, goalseek_mode mode, const struct arg_str_array_array *goalseek_area) {
	return LIBPALO_FUNC_R(cell_goalseek_extended)(err, so, &libpalo_convs, database, cube, path, val, mode, goalseek_area);
}


/** @brief see thread-safe database_info() */
LIBEXPORT libpalo_result LIBPALO_FUNC(database_info)(libpalo_err *err, struct arg_db_info *result, struct sock_obj *so, const CHAR_T *database) {
	return LIBPALO_FUNC_R(database_info)(err, result, so, &libpalo_convs, database);
}


/** @brief see thread-safe dimension_info() */
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_info)(libpalo_err *err, struct arg_dim_info *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(dimension_info)(err, result, so, &libpalo_convs, database, dimension);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_info_simple)(libpalo_err *err, struct arg_dim_info_simple *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
	return LIBPALO_FUNC_R(dimension_info_simple)(err, result, so, &libpalo_convs, database, dimension);
}

/** @brief see thread-safe element_info_simple() */
LIBEXPORT libpalo_result LIBPALO_FUNC(element_info_simple)(libpalo_err *err, struct arg_dim_element_info2_raw *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *element) {
	return LIBPALO_FUNC_R(element_info_simple)(err, result, so, &libpalo_convs, database, dimension, element);
}

/** @brief see thread-safe cube_info() */
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_info)(libpalo_err *err, struct arg_cube_info *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube) {
	return LIBPALO_FUNC_R(cube_info)(err, result, so, &libpalo_convs, database, cube);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(set_client_version)(libpalo_err *err, const CHAR_T *client_version) {
	return LIBPALO_FUNC_R(set_client_version)(err, &libpalo_convs, client_version);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(is_admin_user)(libpalo_err *err, unsigned short int *retresult, struct sock_obj *so) {
	return LIBPALO_FUNC_R(is_admin_user)(err, retresult, so, &libpalo_convs);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(is_gpu_server)(libpalo_err *err, unsigned short int *retresult, struct sock_obj *so) {
	return LIBPALO_FUNC_R(is_gpu_server)(err, retresult, so, &libpalo_convs);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(get_rights)(libpalo_err *err, permission_type *retresult, struct sock_obj *so, permission_art pa) {
	return LIBPALO_FUNC_R(get_rights)(err, retresult, so, &libpalo_convs, pa);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(set_user_password)(libpalo_err *err,  struct sock_obj *so, const CHAR_T *user, const CHAR_T *password) {
	return LIBPALO_FUNC_R(set_user_password)(err, so, &libpalo_convs, user, password);
}

/** @brief see thread-safe change_password() */
LIBEXPORT libpalo_result LIBPALO_FUNC(change_password)(libpalo_err *err,  struct sock_obj *so, const CHAR_T *oldpassword, const CHAR_T *newpassword) {
	return LIBPALO_FUNC_R(change_password)(err, so, &libpalo_convs, oldpassword, newpassword);
}


/** @brief see thread-safe version() */
LIBEXPORT libpalo_result LIBPALO_FUNC(server_info)(libpalo_err *err, struct sock_obj *so, struct server_info *result) {
	return LIBPALO_FUNC_R(server_info)(err, so, &libpalo_convs, result);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(api_version)(libpalo_err *err, struct api_info *result) {
	return LIBPALO_FUNC_R(api_version)(err, result);
}


/** @brief see thread-safe lib_version() */
LIBEXPORT libpalo_result LIBPALO_FUNC(lib_version)(libpalo_err *err, struct sock_obj *so, struct api_info *result) {
	return LIBPALO_FUNC_R(lib_version)(err, so, &libpalo_convs, result);
}


/** @brief see thread-safe rule_add() */
LIBEXPORT libpalo_result LIBPALO_FUNC(rule_add)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const CHAR_T *definition, unsigned short use_identifier, const CHAR_T *extern_id, const CHAR_T *comment, unsigned short activate, double position) {
	return LIBPALO_FUNC_R(rule_add)(err, result, so, &libpalo_convs, database, cube, definition, use_identifier, extern_id, comment, activate, position);
}


/** @brief see thread-safe rule_modify() */
LIBEXPORT libpalo_result LIBPALO_FUNC(rule_modify)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, long id, const CHAR_T *definition, unsigned short use_identifier, const CHAR_T *extern_id, const CHAR_T *comment, unsigned short activate, double position) {
	return LIBPALO_FUNC_R(rule_modify)(err, result, so, &libpalo_convs, database, cube, id, definition, use_identifier, extern_id, comment, activate, position);
}


LIBEXPORT libpalo_result LIBPALO_FUNC(rules_activate)(libpalo_err *err, struct arg_rule_info_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids, rule_activation_mode mode) {
	return LIBPALO_FUNC_R(rules_activate)(err, result, so, &libpalo_convs, database, cube, len, ids, mode);
}

/** @brief see thread-safe rule_info() */
LIBEXPORT libpalo_result LIBPALO_FUNC(rule_info)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, long id, unsigned short use_identifier) {
	return LIBPALO_FUNC_R(rule_info)(err, result, so, &libpalo_convs, database, cube, id, use_identifier);
}


/** @brief see thread-safe cell_rule_info() */
LIBEXPORT libpalo_result LIBPALO_FUNC(cell_rule_info)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array coordinates) {
	return LIBPALO_FUNC_R(cell_rule_info)(err, result, so, &libpalo_convs, database, cube, coordinates);
}


/** @brief see thread-safe rule_parse() */
LIBEXPORT libpalo_result LIBPALO_FUNC(rule_parse)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const CHAR_T *definition) {
	return LIBPALO_FUNC_R(rule_parse)(err, result, so, &libpalo_convs, database, cube, definition);
}


/** @brief see thread-safe rule_delete() */
LIBEXPORT libpalo_result LIBPALO_FUNC(rule_delete)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, long identifier) {
	return LIBPALO_FUNC_R(rule_delete)(err, so, &libpalo_convs, database, cube, identifier);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(rules_delete)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids) {
	return LIBPALO_FUNC_R(rules_delete)(err, so, &libpalo_convs, database, cube, len, ids);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(rules_move)(libpalo_err *err, struct arg_rule_info_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids, double start_position, double below_position) {
	return LIBPALO_FUNC_R(rules_move)(err, result, so, &libpalo_convs, database, cube, len, ids, start_position, below_position);
}

/** @brief see thread-safe list_rules */
LIBEXPORT libpalo_result LIBPALO_FUNC(list_rules)(libpalo_err *err, struct arg_rule_info_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, unsigned short use_identifier) {
	return LIBPALO_FUNC_R(list_rules)(err, result, so, &libpalo_convs, database, cube, use_identifier);
}


/** @brief see thread-safe list_rulefunctions() */
LIBEXPORT libpalo_result LIBPALO_FUNC(list_rulefunctions)(libpalo_err *err, CHAR_T **result, struct sock_obj *so) {
	return LIBPALO_FUNC_R(list_rulefunctions)(err, result, so, &libpalo_convs);
}


/** @brief see thread-safe lock() */
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_lock)(libpalo_err *err, struct arg_lock_info *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array_array locked_area) {
	return LIBPALO_FUNC_R(cube_lock)(err, result, so, &libpalo_convs, database, cube, locked_area);
}


/** @brief see thread-safe locks() */
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_locks)(libpalo_err *err, struct arg_lock_info_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube) {
	return LIBPALO_FUNC_R(cube_locks)(err, result, so, &libpalo_convs, database, cube);
}


/** @brief see thread-safe rollback() */
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_rollback)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, unsigned long lockid, long steps) {
	return LIBPALO_FUNC_R(cube_rollback)(err, so, &libpalo_convs, database, cube, lockid, steps);
}


/** @brief see thread-safe commit() */
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_commit)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, unsigned long lockid) {
	return LIBPALO_FUNC_R(cube_commit)(err, so, &libpalo_convs, database, cube, lockid);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(get_svs_info)(libpalo_err *err, struct arg_svs_info *result, struct sock_obj *so) {
	return LIBPALO_FUNC_R(get_svs_info)(err, result, so, &libpalo_convs);
}


LIBEXPORT libpalo_result LIBPALO_FUNC(event_lock_begin)(libpalo_err *err, struct sock_obj *so, CHAR_T *source, CHAR_T *AreaID) {
	return LIBPALO_FUNC_R(event_lock_begin)(err, so, &libpalo_convs, source, AreaID);
}


LIBEXPORT libpalo_result LIBPALO_FUNC(event_lock_end)(libpalo_err *err, struct sock_obj *so) {
	return LIBPALO_FUNC_R(event_lock_end)(err, so, &libpalo_convs);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_subset)(libpalo_err *err, struct arg_subset_result_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, int indent,
														struct arg_alias_filter_settings *alias,
														struct arg_field_filter_settings *field,
														struct arg_basic_filter_settings *basic,
														struct arg_data_filter_settings *data,
														struct arg_sorting_filter_settings *sorting,
														struct arg_structural_filter_settings *structural,
														struct arg_text_filter_settings *text) {
	return LIBPALO_FUNC_R(dimension_subset)(err, result, so, &libpalo_convs, database, dimension, indent, alias, field, basic, data, sorting, structural, text);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(cell_drill_through)(libpalo_err *err, struct arg_str_array_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *coordinates, drillthrough_type mode) {
	return LIBPALO_FUNC_R(cell_drill_through)(err, result, so, &libpalo_convs, database, cube, coordinates, mode);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(edelete_bulk)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const struct arg_str_array *dimension_elements) {
	return LIBPALO_FUNC_R(edelete_bulk)(err, so, &libpalo_convs, database, dimension, dimension_elements);
}


LIBEXPORT libpalo_result LIBPALO_FUNC(cube_convert)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, cube_type newtype) {
	return LIBPALO_FUNC_R(cube_convert)(err, so, &libpalo_convs, database, cube, newtype);
}

LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_flat_count)(libpalo_err *err, size_t *retresult, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension) {
 	return LIBPALO_FUNC_R(dimension_flat_count)(err, retresult, so, &libpalo_convs, database, dimension);
}


LIBEXPORT libpalo_result LIBPALO_LIBFUNC(cache_add_request)(libpalo_err *err, libpalo_cache *c, struct sock_obj *so, const CHAR_T *key, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array coordinates) {
	return LIBPALO_LIBFUNC_R(cache_add_request)(err, c, so, &libpalo_convs, key, database, cube, coordinates);
}


LIBEXPORT libpalo_result LIBPALO_LIBFUNC(cache_get_result)(libpalo_err *err, libpalo_cache *c, const CHAR_T *key, struct arg_palo_value *value) {
	return LIBPALO_LIBFUNC_R(cache_get_result)(err, c, &libpalo_convs, key, value);
}


LIBEXPORT libpalo_result LIBPALO_FUNC(connect)(libpalo_err *err, const CHAR_T *hostname, const CHAR_T *service, struct sock_obj *so) {
	return LIBPALO_FUNC_R(connect)(err, &libpalo_convs, hostname, service, so);
}


LIBEXPORT CHAR_T *LIBPALO_LIBFUNC(err_get_string)(const libpalo_err *err) {
	return LIBPALO_LIBFUNC_R(err_get_string)(&libpalo_convs, err);
}


LIBEXPORT const struct arg_str_array LIBPALO_LIBFUNC(make_arg_str_array)(size_t num_strs, const CHAR_T * const *strs) {
	struct arg_str_array sa;

	sa.len = num_strs;
	sa.a = (CHAR_T **)strs;

	return sa;
}


LIBEXPORT const struct arg_str_array_array LIBPALO_LIBFUNC(make_arg_str_array_array)(size_t num_arrays, const struct arg_str_array *arrays) {
	struct arg_str_array_array sa;

	sa.len = num_arrays;
	sa.a = (struct arg_str_array*)arrays;

	return sa;
}


LIBEXPORT const struct arg_consolidation_element_info_array LIBPALO_LIBFUNC(make_arg_consolidation_element_info_array)(size_t num_ceis, const struct arg_consolidation_element_info *ceis) {
	struct arg_consolidation_element_info_array ceia;

	ceia.len = num_ceis;
	ceia.a = (struct arg_consolidation_element_info*)ceis;

	return ceia;
}


LIBEXPORT const struct arg_palo_value LIBPALO_LIBFUNC(make_arg_palo_value)(const CHAR_T *str_val, double dbl_val) {
	struct arg_palo_value pv;

	if (str_val != NULL) {
		pv.type = argPaloValueTypeStr;
		pv.val.s = (CHAR_T*)str_val;
	} else {
		pv.type = argPaloValueTypeDouble;
		pv.val.d = dbl_val;
	}

	return pv;
}


LIBEXPORT const struct arg_palo_dataset LIBPALO_LIBFUNC(make_arg_palo_dataset)(const struct arg_str_array coordinates, const struct arg_palo_value value) {
	struct arg_palo_dataset ds;

	ds.coordinates = coordinates;
	ds.value = value;

	return ds;
}


/** @} */
/** @} */

#include "helper_generic.c.h"
