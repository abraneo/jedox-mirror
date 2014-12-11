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

palo_err PALO_SUFFIX(set_element_info2_raw)(wchar_t **errmsg, struct arg_dim_element_info2_raw *retresult, struct conversions *convs, const jedox::palo::ELEMENT_INFO &ei) {
	palo_err result = PALO_SUCCESS;
	size_t j, vsize;

	result = UTF8_TO_STR(convs, &(retresult->name), ei.nelement.c_str());
	if (result != PALO_SUCCESS) {
		*errmsg = MSG_ENCODING_FAILED;
		return result;
	}
	
	result = number2types(ei.type, &(retresult->type));
	if (result != PALO_SUCCESS) {
		*errmsg = MSG_TYPE_CONVERSION_FAILED;
		return result;
	}

	retresult->identifier = ei.element;
	retresult->position = ei.position;
	retresult->level = ei.level;
	retresult->indent = ei.indent;
	retresult->depth = ei.depth;

	vsize = ei.parents.size();

	if (vsize > 0) {
		retresult->parents = (struct arg_parent_info_raw*)calloc(vsize, sizeof(struct arg_parent_info_raw));
		if (retresult->parents == NULL) {
			*errmsg = MSG_PALO_ERR_NO_MEM;
			return PALO_ERR_NO_MEM;
		}

		for (j = 0;j < vsize;j++) {
			(retresult->parents + j)->identifier = ei.parents[j];
		}
	} else {
		retresult->parents = NULL;
	}

	retresult->num_parents = vsize;

	vsize = ei.children.size();

	if (vsize > 0) {
		retresult->children = (struct arg_child_info_raw*)calloc(vsize, sizeof(struct arg_child_info_raw));
		if (retresult->children == NULL) {
			*errmsg = MSG_PALO_ERR_NO_MEM;
			return result;
		}

		for (j = 0;j < vsize;j++) {
			(retresult->children + j)->identifier = ei.children[j];
			(retresult->children + j)->factor = ei.weights[j];
		}
	} else {
		retresult->children = NULL;
	}

	retresult->num_children = vsize;

	return PALO_SUCCESS;

}


palo_err PALO_SUFFIX(database_info)(struct errstack *errs, struct arg_db_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *utf8_db = NULL;

	retresult->name = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		DATABASE_INFO dbinfo = (*(so->myServer))[utf8_db].getCacheDataCopy();

		retresult->id = dbinfo.database;
		retresult->number_dimensions = dbinfo.number_dimensions;
		retresult->number_cubes = dbinfo.number_cubes;

		switch (dbinfo.status) {

			case DATABASE_INFO::UNLOADED :
				retresult->status = unloaded_db;
				break;

			case DATABASE_INFO::LOADED :
				retresult->status = loaded_db;
				break;

			case DATABASE_INFO::CHANGED :
				retresult->status = changed_db;
				break;

			case DATABASE_INFO::LOADING :
				retresult->status = loading_db;
				break;

			default:
				retresult->status = unknown_db_status;
				break;

		}

		switch (dbinfo.status) {

			case DATABASE_INFO::NORMAL :
				retresult->type = normal_db;
				break;

			case DATABASE_INFO::SYSTEM :
				retresult->type = system_db;
				break;

			case DATABASE_INFO::USERINFO :
				retresult->type = user_info_db;
				break;

			default:
				retresult->type = unknown_db_type;
				break;

		}

		result = UTF8_TO_STR(convs, &(retresult->name), dbinfo.ndatabase.c_str());
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
		}

	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(dimension_info)(struct errstack *errs, struct arg_dim_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *name = NULL;

	retresult->name = NULL;
	retresult->assoc_dimension = NULL;
	retresult->attribut_cube = NULL;
	retresult->rights_cube = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;


	result = STR_TO_UTF8(convs, &name, dimension);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_dimension(name);
	free(name);
	name = NULL;


	try {
		DIMENSION_INFO diminfo = (*(so->myServer))[utf8_db].dimension[utf8_dimension].getCacheDataCopy();

		retresult->id = diminfo.dimension;
		retresult->number_elements = diminfo.number_elements;
		retresult->maximum_level = diminfo.maximum_level;
		retresult->maximum_indent = diminfo.maximum_indent;
		retresult->maximum_depth = diminfo.maximum_depth;

		switch (diminfo.type) {

			case DIMENSION_INFO::NORMAL :
				retresult->type = normal_dim;
				break;

			case DIMENSION_INFO::SYSTEM :
				retresult->type = system_dim;
				break;

			case DIMENSION_INFO::ATTRIBUTE :
				retresult->type = attribut_dim;
				break;

			case DIMENSION_INFO::USERINFO :
				retresult->type = user_info_dim;
				break;

			case DIMENSION_INFO::SYSTEM_ID :
				retresult->type = system_id_dim;
				break;

			default:
				retresult->type = unknown_dim_type;
				break;

		}

		result = UTF8_TO_STR(convs, &(retresult->name), diminfo.ndimension.c_str());
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
		}

		if ((diminfo.assoc_dimension != -1) && (result == PALO_SUCCESS)) {
			result = UTF8_TO_STR(convs, &(retresult->assoc_dimension), (*(so->myServer))[utf8_db].dimension[diminfo.assoc_dimension].getCacheData().ndimension.c_str());
			if (result != PALO_SUCCESS) {
				errmsg = MSG_ENCODING_FAILED;
			}
		}

		if ((diminfo.attribute_cube != -1) && (result == PALO_SUCCESS)) {
			result = UTF8_TO_STR(convs, &(retresult->attribut_cube), (*(so->myServer))[utf8_db].cube[diminfo.attribute_cube].getCacheData().ncube.c_str());
			if (result != PALO_SUCCESS) {
				errmsg = MSG_ENCODING_FAILED;
			}
		}

		if ((diminfo.rights_cube != -1) && (result == PALO_SUCCESS)) {
			result = UTF8_TO_STR(convs, &(retresult->rights_cube), (*(so->myServer))[utf8_db].cube[diminfo.rights_cube].getCacheData().ncube.c_str());
			if (result != PALO_SUCCESS) {
				errmsg = MSG_ENCODING_FAILED;
			}
		}

	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		free(retresult->name);
		retresult->name = NULL;
		free(retresult->assoc_dimension);
		retresult->assoc_dimension = NULL;
		free(retresult->attribut_cube);
		retresult->attribut_cube = NULL;
		free(retresult->rights_cube);
		retresult->rights_cube = NULL;
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;

}


palo_err PALO_SUFFIX(dimension_info_simple)(struct errstack *errs, struct arg_dim_info_simple *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	retresult->name = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, dimension);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_dimension(name);
	free(name);
	name = NULL;

	try {
		DIMENSION_INFO diminfo = (*(so->myServer))[utf8_db].dimension[utf8_dimension].getCacheDataCopy();

		retresult->id = diminfo.dimension;
		retresult->number_elements = diminfo.number_elements;
		retresult->maximum_level = diminfo.maximum_level;
		retresult->maximum_indent = diminfo.maximum_indent;
		retresult->maximum_depth = diminfo.maximum_depth;

		switch (diminfo.type) {

			case DIMENSION_INFO::NORMAL :
				retresult->type = normal_dim;
				break;

			case DIMENSION_INFO::SYSTEM :
				retresult->type = system_dim;
				break;

			case DIMENSION_INFO::ATTRIBUTE :
				retresult->type = attribut_dim;
				break;

			case DIMENSION_INFO::USERINFO :
				retresult->type = user_info_dim;
				break;

			case DIMENSION_INFO::SYSTEM_ID :
				retresult->type = system_id_dim;
				break;

			default:
				retresult->type = unknown_dim_type;
				break;

		}

		retresult->assoc_dimension_id = diminfo.assoc_dimension;
		retresult->attribut_cube_id = diminfo.attribute_cube;
		retresult->rights_cube_id = diminfo.rights_cube;

		result = UTF8_TO_STR(convs, &(retresult->name), diminfo.ndimension.c_str());
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
		}

	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		free(retresult->name);
		retresult->name = NULL;
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(element_info_simple)(struct errstack *errs, struct arg_dim_element_info2_raw *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *element) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *name = NULL ;

	retresult->name = NULL;
	retresult->type = de_n;
	retresult->identifier = -1;
	retresult->position = 0;
	retresult->level = 0;
	retresult->indent = 0;
	retresult->depth = 0;
	retresult->num_parents = 0;
	retresult->parents = NULL;
	retresult->num_children = 0;
	retresult->children = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, dimension);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}
	std::string utf8_dim(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, element);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}
	std::string utf8_element(name);
	free(name);
	name = NULL;

	try {
		result = PALO_SUFFIX(set_element_info2_raw)(&errmsg, retresult, convs, (*(so->myServer))[utf8_db].dimension[utf8_dim][utf8_element].getCacheDataCopy());
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		PALO_SUFFIX(free_arg_dim_element_info2_raw_contents)(retresult);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(cube_info)(struct errstack *errs, struct arg_cube_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *name = NULL;
	CHAR_T **strs = NULL;
	size_t vsize = 0, vsize2 = 0, i;

	retresult->name = NULL;
	retresult->dimensions.len = 0;
	retresult->dimensions.a = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;

	try {
		CUBE_INFO cubeinfo; 
		(*(so->myServer))[utf8_db].cube[utf8_cube].ActualInfo(cubeinfo);

		retresult->id = cubeinfo.cube;
		retresult->number_dimensions = cubeinfo.number_dimensions;
		retresult->number_cells = cubeinfo.number_cells;
		retresult->number_filled_cells = cubeinfo.number_filled_cells;

		switch (cubeinfo.status) {

			case CUBE_INFO::UNLOADED :
				retresult->status = unloaded_cube;
				break;

			case CUBE_INFO::LOADED :
				retresult->status = loaded_cube;
				break;

			case CUBE_INFO::CHANGED :
				retresult->status = changed_cube;
				break;

			default:
				retresult->status = unknown_cube_status;
				break;

		}

		switch (cubeinfo.type) {

			case CUBE_INFO::NORMAL :
				retresult->type = normal_cube;
				break;

			case CUBE_INFO::SYSTEM :
				retresult->type = system_cube;
				break;

			case CUBE_INFO::ATTRIBUTE :
				retresult->type = (CheckForSpecialCubes::isPropertyCube(cubeinfo.ncube)) ? system_cube : attribut_cube;
				break;

			case CUBE_INFO::USERINFO :
				retresult->type = user_info_cube;
				break;

			case CUBE_INFO::GPU :
				retresult->type = gpu_cube;
				break;

			default:
				retresult->type = unknown_cube_type;
				break;

		}

		result = UTF8_TO_STR(convs, &(retresult->name), cubeinfo.ncube.c_str());
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
		}

		if (result == PALO_SUCCESS) {

			vsize = cubeinfo.dimensions.size();
			if (vsize > 0) {
				strs = (CHAR_T **)calloc(vsize, sizeof(CHAR_T*));
				if (strs != NULL) {
					for (i = 0;i < vsize;i++) {
						vsize2 = i;
						result = UTF8_TO_STR(convs, strs + i, (*(so->myServer))[utf8_db].dimension[cubeinfo.dimensions[i]].getCacheData().ndimension.c_str());
						if (result != PALO_SUCCESS) {
							free(retresult->name);
							retresult->name = NULL;
							errmsg = MSG_ENCODING_FAILED;
							break;
						}
					}
					vsize2 = 0;
					if (result == PALO_SUCCESS) {
						retresult->dimensions = LIBPALO_LIBFUNC(make_arg_str_array)(vsize, strs);
					}
				} else {
					result = PALO_ERR_NO_MEM;
					errmsg = MSG_PALO_ERR_NO_MEM;
				}
			} else {
				result = PALO_ERR_EMPTY_CUBE;
				errmsg = MSG_PALO_ERR_EMPTY_CUBE;
			}
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	for (i = 0; i < vsize2;i++) {
		free(*(strs + i));
	}

	if (vsize2 > 0) {
		free(strs);
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;

}


palo_err PALO_SUFFIX(cube_list_dimensions)(struct errstack *errs, struct arg_str_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *utf8_db = NULL, *utf8_cube = NULL;
	CHAR_T **strs = NULL;
	size_t vsize = 0, vsize2 = 0, i;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		DIMENSION_LIST dimensions = (*(so->myServer))[utf8_db].cube[utf8_cube].getCacheData().dimensions;

		vsize = dimensions.size();
		if (vsize > 0) {
			strs = (CHAR_T **)calloc(vsize, sizeof(CHAR_T*));
			if (strs != NULL) {
				for (i = 0;i < vsize;i++) {
					vsize2 = i;
					result = UTF8_TO_STR(convs, strs + i, (*(so->myServer))[utf8_db].dimension[dimensions[i]].getCacheData().ndimension.c_str());
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				}
				vsize2 = 0;
				if (result == PALO_SUCCESS) {
					*retresult = LIBPALO_LIBFUNC(make_arg_str_array)(vsize, strs);
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		} else {
			result = PALO_ERR_EMPTY_CUBE;
			errmsg = MSG_PALO_ERR_EMPTY_CUBE;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	for (i = 0; i < vsize2;i++) {
		free(*(strs + i));
	}

	if (vsize2 > 0) {
		free(strs);
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;

}


palo_err PALO_SUFFIX(database_list_dimensions2)(struct errstack *errs, struct arg_str_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, int dim_type) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL;
	CHAR_T **strs = NULL;
	size_t vsize = 0, vsize2 = 0, i;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		vector<string> dimnames;

		dimnames.reserve((*(so->myServer))[utf8_db].getCacheData().number_dimensions);

		std::unique_ptr<DimensionsCache::CacheIterator> it = (*(so->myServer))[utf8_db].dimension.getIterator();

		while (!(*it).end()) {
			if ((**it).type == dim_type) {
				dimnames.push_back((**it).ndimension);
			}
			++*it;
		}

		vsize = dimnames.size();
		if (vsize > 0) {
			strs = (CHAR_T **)calloc(vsize, sizeof(CHAR_T*));
			if (strs != NULL) {
				for (i = 0;i < vsize;i++) {
					vsize2 = i;
					result = UTF8_TO_STR(convs, strs + i, dimnames[i].c_str());
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				}
				vsize2 = 0;
				if (result == PALO_SUCCESS) {
					*retresult = LIBPALO_LIBFUNC(make_arg_str_array)(vsize, strs);
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		} else {
			retresult->len = 0;
			retresult->a = NULL;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	for (i = 0; i < vsize2;i++) {
		free(*(strs + i));
	}

	if (vsize2 > 0) {
		free(strs);
	}

	free(utf8_db);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(database_list_dimensions)(struct errstack *errs, struct arg_str_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL;
	CHAR_T **strs = NULL;
	size_t vsize = 0, vsize2 = 0, i;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		vector<string> dimnames;

		dimnames.reserve((*(so->myServer))[utf8_db].getCacheData().number_dimensions);

		std::unique_ptr<DimensionsCache::CacheIterator> it = (*(so->myServer))[utf8_db].dimension.getIterator();

		while (!(*it).end()) {
			dimnames.push_back((**it).ndimension);
			++*it;
		}

		vsize = dimnames.size();
		if (vsize > 0) {
			strs = (CHAR_T **)calloc(vsize, sizeof(CHAR_T*));
			if (strs != NULL) {
				for (i = 0;i < vsize;i++) {
					vsize2 = i;
					result = UTF8_TO_STR(convs, strs + i, dimnames[i].c_str());
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				}
				vsize2 = 0;
				if (result == PALO_SUCCESS) {
					*retresult = LIBPALO_LIBFUNC(make_arg_str_array)(vsize, strs);
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		} else {
			retresult->len = 0;
			retresult->a = NULL;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	for (i = 0; i < vsize2;i++) {
		free(*(strs + i));
	}

	if (vsize2 > 0) {
		free(strs);
	}

	free(utf8_db);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(database_list_cubes2)(struct errstack *errs, struct arg_str_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, int cube_type, unsigned short int only_cubes_with_cells) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL;
	CHAR_T **strs = NULL;
	size_t vsize = 0, vsize2 = 0, i;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		vector<string> cubenames;

		cubenames.reserve((*(so->myServer))[utf8_db].getCacheData().number_cubes);
		std::unique_ptr<CubesCache::CacheIterator> it = (*(so->myServer))[utf8_db].cube.getIterator();
		bool doadd;

		while (!(*it).end()) {
			if ((only_cubes_with_cells == 0) || ((**it).number_cells > 0))  {
				if (cube_type == system_cube) {
					doadd = ((**it).type == cube_type) || ((((cube_type_tag)((**it).type)) == attribut_cube) && CheckForSpecialCubes::isPropertyCube((**it).ncube));
				} else {
					if (cube_type == attribut_cube) {
						doadd = ((**it).type == cube_type) && !CheckForSpecialCubes::isPropertyCube((**it).ncube);
					} else {
						doadd = ((**it).type == cube_type);
					}
				}

				if (doadd) {
					cubenames.push_back((**it).ncube);
				}
			}
			++*it;
		}

		vsize = cubenames.size();
		if (vsize > 0) {
			strs = (CHAR_T **)calloc(vsize, sizeof(CHAR_T*));
			if (strs != NULL) {
				for (i = 0;i < vsize;i++) {
					vsize2 = i;
					result = UTF8_TO_STR(convs, strs + i, cubenames[i].c_str());
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				}
				vsize2 = 0;
				if (result == PALO_SUCCESS) {
					*retresult = LIBPALO_LIBFUNC(make_arg_str_array)(vsize, strs);
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		} else {
			retresult->len = 0;
			retresult->a = NULL;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	for (i = 0; i < vsize2;i++) {
		free(*(strs + i));
	}

	if (vsize2 > 0) {
		free(strs);
	}

	free(utf8_db);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(database_list_cubes)(struct errstack *errs, struct arg_str_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL;
	CHAR_T **strs = NULL;
	size_t vsize = 0, vsize2 = 0, i;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		vector<string> cubenames;
		cubenames.reserve((*(so->myServer))[utf8_db].getCacheData().number_cubes);

		std::unique_ptr<CubesCache::CacheIterator> it = (*(so->myServer))[utf8_db].cube.getIterator();

		while (!(*it).end()) {
			cubenames.push_back((**it).ncube);
			++*it;
		}

		vsize = cubenames.size();
		if (vsize > 0) {
			strs = (CHAR_T **)calloc(vsize, sizeof(CHAR_T*));
			if (strs != NULL) {
				for (i = 0;i < vsize;i++) {
					vsize2 = i;
					result = UTF8_TO_STR(convs, strs + i, cubenames[i].c_str());
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				}
				vsize2 = 0;
				if (result == PALO_SUCCESS) {
					*retresult = LIBPALO_LIBFUNC(make_arg_str_array)(vsize, strs);
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		} else {
			retresult->len = 0;
			retresult->a = NULL;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	for (i = 0; i < vsize2;i++) {
		free(*(strs + i));
	}

	if (vsize2 > 0) {
		free(strs);
	}

	free(utf8_db);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;

}


palo_err PALO_SUFFIX(dimension_list_cubes2)(struct errstack *errs, struct arg_str_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, int cube_type, unsigned short int only_cubes_with_cells) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL;
	CHAR_T **strs = NULL;
	size_t vsize = 0, vsize2 = 0, i;
	long id;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {

		DIMENSION_LIST dimensions;
		vector<string> cubenames;

		cubenames.reserve((*(so->myServer))[utf8_db].getCacheData().number_cubes);

		id = (*(so->myServer))[utf8_db].dimension[utf8_dim].getCacheData().dimension;

		std::unique_ptr<CubesCache::CacheIterator> it = (*(so->myServer))[utf8_db].cube.getIterator();
		bool found;

		while (!(*it).end()) {
			if ((only_cubes_with_cells == 0) || ((**it).number_cells > 0))  {
				if (cube_type == system_cube) {
					found = ((**it).type == cube_type) || ((((cube_type_tag)((**it).type)) == attribut_cube) && CheckForSpecialCubes::isPropertyCube((**it).ncube));
				} else {
					if (cube_type == attribut_cube) {
						found = ((**it).type == cube_type) && !CheckForSpecialCubes::isPropertyCube((**it).ncube);
					} else {
						found = ((**it).type == cube_type);
					}
				}

				if (found) {
					dimensions.resize((**it).dimensions.size());
					dimensions = (**it).dimensions;
					found = false;
					vsize = dimensions.size();

					for (i = 0; (i < vsize) && !found; i++) {
						found = (dimensions[i] == id);
					}

					if (found) {
						cubenames.push_back((**it).ncube);
					}
				}
			}
			++*it;
		}
		vsize = cubenames.size();
		if (vsize > 0) {
			strs = (CHAR_T **)calloc(vsize, sizeof(CHAR_T*));
			if (strs != NULL) {
				for (i = 0;i < vsize;i++) {
					vsize2 = i;
					result = UTF8_TO_STR(convs, strs + i, cubenames[i].c_str());
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				}
				vsize2 = 0;
				if (result == PALO_SUCCESS) {
					*retresult = LIBPALO_LIBFUNC(make_arg_str_array)(vsize, strs);
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		} else {
			retresult->len = 0;
			retresult->a = NULL;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	for (i = 0; i < vsize2;i++) {
		free(*(strs + i));
	}

	if (vsize2 > 0) {
		free(strs);
	}

	free(utf8_db);
	free(utf8_dim);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(dimension_list_cubes)(struct errstack *errs, struct arg_str_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL;
	CHAR_T **strs = NULL;
	size_t vsize = 0, vsize2 = 0, i;
	long id;
	unsigned short found;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		DIMENSION_LIST dimensions;

		vector<string> cubenames;
		cubenames.reserve((*(so->myServer))[utf8_db].getCacheData().number_cubes);

		id = (*(so->myServer))[utf8_db].dimension[utf8_dim].getCacheData().dimension;

		std::unique_ptr<CubesCache::CacheIterator> it = (*(so->myServer))[utf8_db].cube.getIterator();

		while (!(*it).end()) {
			dimensions.resize((**it).dimensions.size());
			dimensions = (**it).dimensions;
			found = 0;
			vsize = dimensions.size();

			for (i = 0; (i < vsize) && (found == 0); i++) {
				found = (dimensions[i] == id) ? 1 : 0;
			}

			if (found == 1) {
				cubenames.push_back((**it).ncube);
			}
			++*it;
		}

		vsize = cubenames.size();
		if (vsize > 0) {
			strs = (CHAR_T **)calloc(vsize, sizeof(CHAR_T*));
			if (strs != NULL) {
				for (i = 0;i < vsize;i++) {
					vsize2 = i;
					result = UTF8_TO_STR(convs, strs + i, cubenames[i].c_str());
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				}
				vsize2 = 0;
				if (result == PALO_SUCCESS) {
					*retresult = LIBPALO_LIBFUNC(make_arg_str_array)(vsize, strs);
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		} else {
			retresult->len = 0;
			retresult->a = NULL;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	for (i = 0; i < vsize2;i++) {
		free(*(strs + i));
	}

	if (vsize2 > 0) {
		free(strs);
	}

	free(utf8_db);
	free(utf8_dim);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(dimension_list_elements)(struct errstack *errs, struct arg_dim_element_info_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL;
	size_t elemcount = 0, pos = 0;

	retresult->a = NULL;
	retresult->len = 0;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		Dimension dim = (*(so->myServer))[utf8_db].dimension[utf8_dim];

		elemcount = dim.getCacheData().number_elements;

		if (elemcount > 0) {
			retresult->a = (struct arg_dim_element_info*)calloc(elemcount, sizeof(struct arg_dim_element_info));
			retresult->len = elemcount;

			if (retresult->a != NULL) {

				std::unique_ptr<DimensionCache::CacheIterator> it = dim.getIterator();

				while (!(*it).end()) {
					pos = (**it).position;

					result = UTF8_TO_STR(convs, &((retresult->a + pos)->name), (**it).nelement.c_str());
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
					result = number2types((**it).type, &((retresult->a + pos)->type));
					if (result != PALO_SUCCESS) {
						break;
					}

					++*it;
				}

			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		}

	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);

	if (result != PALO_SUCCESS) {
		PALO_SUFFIX(free_arg_dim_element_info_array_contents)(retresult);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(root_list_databases2)(struct errstack *errs, struct arg_str_array *retresult, struct sock_obj *so, struct conversions *convs, int db_type) {
	palo_err result = PALO_SUCCESS;
	size_t i, j, vsize = 0, vsize2 = 0;
	wchar_t *errmsg = NULL;
	CHAR_T **strs = NULL;

	retresult->len = 0;
	retresult->a = NULL;

	try {
		vector<string> dbnames;

		std::unique_ptr<ServerCache::CacheIterator> it = (*(so->myServer)).getIterator();

		while (!(*it).end()) {
			if (((**it).type == db_type) && (((**it).status == DATABASE_INFO::LOADED) || ((**it).status == DATABASE_INFO::CHANGED))) {
				dbnames.push_back((**it).ndatabase);
			}
			++*it;
		}

		vsize = dbnames.size();
		if (vsize > 0) {
			strs = (CHAR_T **)calloc(vsize, sizeof(CHAR_T*));
			if (strs != NULL) {
				for (i = 0;i < vsize;i++) {
					vsize2 = i;
					result = UTF8_TO_STR(convs, strs + i, dbnames[i].c_str());
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				}
				vsize2 = 0;
				if (result == PALO_SUCCESS) {
					*retresult = LIBPALO_LIBFUNC(make_arg_str_array)(vsize, strs);
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		} else {
			retresult->len = 0;
			retresult->a = NULL;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	for (j = 0; j < vsize2;j++) {
		free(*(strs + j));
	}

	if (vsize2 > 0) {
		free(strs);
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;

}


palo_err PALO_SUFFIX(root_list_databases)(struct errstack *errs, struct arg_str_array *retresult, struct sock_obj *so, struct conversions *convs) {
	palo_err result = PALO_SUCCESS;
	size_t i, j, vsize = 0, vsize2 = 0;
	wchar_t *errmsg = NULL;
	CHAR_T **strs = NULL;

	retresult->len = 0;
	retresult->a = NULL;

	try {
		vector<string> dbnames;

		std::unique_ptr<ServerCache::CacheIterator> it = (*(so->myServer)).getIterator();

		while (!(*it).end()) {
			if (((**it).status == DATABASE_INFO::LOADED) || ((**it).status == DATABASE_INFO::CHANGED)) {
				dbnames.push_back((**it).ndatabase);
			}
			++*it;
		}

		vsize = dbnames.size();
		if (vsize > 0) {
			strs = (CHAR_T **)calloc(vsize, sizeof(CHAR_T*));
			if (strs != NULL) {
				for (i = 0;i < vsize;i++) {
					vsize2 = i;
					result = UTF8_TO_STR(convs, strs + i, dbnames[i].c_str());
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				}
				vsize2 = 0;
				if (result == PALO_SUCCESS) {
					*retresult = LIBPALO_LIBFUNC(make_arg_str_array)(vsize, strs);
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		} else {
			retresult->len = 0;
			retresult->a = NULL;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	for (j = 0; j < vsize2;j++) {
		free(*(strs + j));
	}

	if (vsize2 > 0) {
		free(strs);
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;

}


palo_err PALO_SUFFIX(erename)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_oldname, const CHAR_T *de_newname) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_elem = NULL, *utf8_newname = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_elem, de_oldname);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_newname, de_newname);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		free(utf8_elem);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].dimension[utf8_dim][utf8_elem].rename(utf8_newname);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_elem);
	free(utf8_newname);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(edelete)(struct errstack *errs,  struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_elem = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_elem, dimension_element);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].dimension[utf8_dim][utf8_elem].destroy();
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_elem);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(emove)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, unsigned int new_position) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_elem = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_elem, dimension_element);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].dimension[utf8_dim][utf8_elem].move(new_position);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_elem);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(eadd_or_update)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, dimension_add_or_update_element_mode mode, de_type type, const struct arg_consolidation_element_info_array *consolidation_elements, palo_bool append_consolidation_elements) {
	palo_err result = PALO_SUCCESS, result2 = PALO_SUCCESS;
	wchar_t *errmsg = NULL, *w_dimname = NULL, *w_elemname = NULL;
	char *name = NULL;
	ELEMENT_INFO::TYPE ltype;
	short dnf_dim = 0, dnf_elem = 0;
	size_t i, vsize = consolidation_elements->len;
	vector<string> children(vsize);
	vector<double> weights(vsize);

	bool found = false;

	result = types2number(type, &ltype);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, NULL);
	}

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, dimension);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_dim(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, dimension_element);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_elem(name);
	free(name);
	name = NULL;

	for (i = 0; i < vsize; i++) {
		result = STR_TO_UTF8(convs, &name, (consolidation_elements->a + i)->name);
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
			break;
		}
		children[i] = name;
		free(name);
		weights[i] = (consolidation_elements->a + i)->factor;
	}

	if (result == PALO_SUCCESS) {
		try {
			found = (*(so->myServer))[utf8_db].dimension[utf8_dim].Exists(utf8_elem);

			if (!found) {
				if (mode == dimension_add_or_update_element_mode_update) {
					result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
				}
			} else {
				if (mode == dimension_add_or_update_element_mode_force_add) {
					result = PALO_ERR_DIMENSION_ELEMENT_EXISTS;
				}
			}

			if (result == PALO_SUCCESS) {
				if (!found) {
					(*(so->myServer))[utf8_db].dimension[utf8_dim].createElement(utf8_elem, ltype, children, weights);
				} else {
					if (append_consolidation_elements == PALO_TRUE) {
						(*(so->myServer))[utf8_db].dimension[utf8_dim][utf8_elem].append(children, weights);
					} else {
						if (mode != dimension_add_or_update_element_mode_add) {
							(*(so->myServer))[utf8_db].dimension[utf8_dim][utf8_elem].replace(ltype, children, weights);
						}
					}
				}
			}
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			result = PALO_ERR_SYSTEM;
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
		}
	}

	if (result != PALO_SUCCESS) {
		if (result == PALO_ERR_DIM_ELEMENT_NOT_FOUND) {
			result2 = STR_TO_WCS(convs, &w_dimname, dimension);
			if (result2 != PALO_SUCCESS) {
				w_dimname = const_cast<wchar_t *>(L"");
			}
			result2 = STR_TO_WCS(convs, &w_elemname, dimension_element);
			if (result2 != PALO_SUCCESS) {
				dnf_elem = 1;
				w_elemname = const_cast<wchar_t *>(L"");
			}

			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, wcsdup(L"cannot update element \"%ls\" in dimension \"%ls\": element not found"), w_elemname, w_dimname);
		} else {
			if (result == PALO_ERR_DIMENSION_ELEMENT_EXISTS) {
				result2 = STR_TO_WCS(convs, &w_dimname, dimension);
				if (result2 != PALO_SUCCESS) {
					dnf_dim = 1;
					w_dimname = const_cast<wchar_t *>(L"");
				}
				result2 = STR_TO_WCS(convs, &w_elemname, dimension_element);
				if (result2 != PALO_SUCCESS) {
					dnf_elem = 1;
					w_elemname = const_cast<wchar_t *>(L"");
				}

				ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, wcsdup(L"cannot add element \"%ls\" to dimension \"%ls\": element already exists"), w_elemname, w_dimname);
			} else {
				ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
			}
			if (dnf_dim == 0) {
				free(w_dimname);
			}
			if (dnf_elem == 0) {
				free(w_elemname);
			}

		}
		return result;
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(element_create)(struct errstack *errs, struct arg_dim_element_info2_raw *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, de_type type, const struct arg_consolidation_element_info_array *consolidation_elements) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;
	ELEMENT_INFO::TYPE ltype;
	size_t i, vsize = consolidation_elements->len;
	vector<string> children(vsize);
	vector<double> weights(vsize);

	result = types2number(type, &ltype);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, NULL);
	}

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, dimension);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_dim(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, dimension_element);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_elem(name);
	free(name);
	name = NULL;

	for (i = 0; i < vsize; i++) {
		result = STR_TO_UTF8(convs, &name, (consolidation_elements->a + i)->name);
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
			break;
		}
		children[i] = name;
		free(name);
		weights[i] = (consolidation_elements->a + i)->factor;
	}

	if (result == PALO_SUCCESS) {
		try {
			ELEMENT_INFO ei;
			(*(so->myServer))[utf8_db].dimension[utf8_dim].createElement(ei, utf8_elem, ltype, children, weights);
			result = PALO_SUFFIX(set_element_info2_raw)(&errmsg, retresult, convs, ei);
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			result = PALO_ERR_SYSTEM;
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
		}
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(element_list_consolidation_elements)(struct errstack *errs, struct arg_consolidation_element_info_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	palo_err result = PALO_SUCCESS, result2 = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;
	struct arg_consolidation_element_info *ceis = NULL;
	size_t vsize = 0, vsize2 = 0, i;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, dimension);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_dim(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, dimension_element);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_elem(name);
	free(name);
	name = NULL;


	try {
		const Dimension& dim = (*(so->myServer))[utf8_db].dimension[utf8_dim];
		const ELEMENT_INFO& ei = const_cast<Dimension&>(dim)[utf8_elem].getCacheData();
		vsize = ei.children.size();

		if (vsize > 0) {
			ceis = (arg_consolidation_element_info*)calloc(vsize, sizeof(struct arg_consolidation_element_info));
			if (ceis != NULL) {
				long id;
				vsize2 = 0;
				for (i = 0;i < vsize;i++) {
					id = ei.children[i];
					if (const_cast<Dimension&>(dim).Exists(id)) {
						const ELEMENT_INFO& child = const_cast<Dimension&>(dim)[id].getCacheData();
						result = UTF8_TO_STR(convs, &((ceis + vsize2)->name), child.nelement.c_str());
						if (result != PALO_SUCCESS) {
							errmsg = MSG_ENCODING_FAILED;
							break;
						}
						result = number2types(child.type, &((ceis + vsize2)->type));
						if (result != PALO_SUCCESS) {
								break;
						}
						(ceis + vsize2)->factor = ei.weights[i];
						vsize2++;
					}
				}

				if (vsize2 == 0) {
					free(ceis);
				} else {
					if (vsize2 < vsize) {
						ceis = (arg_consolidation_element_info*)realloc(ceis, vsize2 * sizeof(struct arg_consolidation_element_info));
					}
				}

				if (result == PALO_SUCCESS) {
					*retresult = LIBPALO_LIBFUNC(make_arg_consolidation_element_info_array)(vsize2, ceis);
					vsize2 = 0;
				}
			} else {
				errmsg = MSG_PALO_ERR_NO_MEM;
				result = PALO_ERR_NO_MEM;
			}
		} else {
			retresult->len = 0;
			retresult->a = NULL;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	for (i = 0; i < vsize2;i++) {
		free((ceis + i)->name);
	}

	if (vsize2 > 0) {
		free(ceis);
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;

}

palo_err PALO_SUFFIX(etoplevel)(struct errstack *errs, unsigned int *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *utf8_db = NULL, *utf8_dim = NULL;
	*retresult = 0;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		*retresult = (*(so->myServer))[utf8_db].dimension[utf8_dim].getCacheData().maximum_level;
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(elevel)(struct errstack *errs, unsigned int *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_elem = NULL;

	*retresult = 0;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_elem, dimension_element);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		*retresult = (*(so->myServer))[utf8_db].dimension[utf8_dim][utf8_elem].getCacheData().level;
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_elem);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(etype)(struct errstack *errs, de_type *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_elem = NULL;

	*retresult = de_unknown;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_elem, dimension_element);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		result = number2types((*(so->myServer))[utf8_db].dimension[utf8_dim][utf8_elem].getCacheData().type, retresult);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_elem);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(eprev)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_elem = NULL;
	size_t pos = 0;
	size_t vsize = 0, i;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_elem, dimension_element);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;

		vector<ELEMENT_INFO> elems;

		elems.reserve((*(so->myServer))[utf8_db].dimension[utf8_dim].getCacheData().number_elements);

		std::unique_ptr<DimensionCache::CacheIterator> it = (*(so->myServer))[utf8_db].dimension[utf8_dim].getIterator();

		while (!(*it).end()) {
			elems.push_back(**it);
			++*it;
		}

		vsize = elems.size();

		if (vsize > 0) {
			size_t index;

			if (FindDimElementIndex(elems, utf8_elem, &index) == PALO_TRUE) {
				pos = elems[index].position;

				if (pos > 0) {
					unsigned long curpos, bestpos = 0;
					size_t index = -1;

					pos--;
					for (i = 0;i < vsize;i++) {
						curpos = elems[i].position;
						if (curpos == pos) {
							index = i;
							break;
						} else if ((curpos < pos) && (curpos >= bestpos)) {
							index = i;
							bestpos = curpos; 
						}
					}

					if (index != -1) {
						result = UTF8_TO_STR(convs, retresult, elems[index].nelement.c_str());
						if (result != PALO_SUCCESS) {
							errmsg = MSG_ENCODING_FAILED;
						}
					}

				} else {
					result = PALO_ERR_INV_OFFSET;
				}
			} else {
				result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
			}
		} else {
			result = PALO_ERR_DIM_EMPTY;
			errmsg = MSG_PALO_ERR_DIM_EMPTY;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_elem);

	if (result != PALO_SUCCESS) {
		if (result == PALO_ERR_INV_OFFSET) {
			ERRSTACK_PREPARE(errs, PALO_ERR_INV_OFFSET, __FUNCTION__, __LINE__, __FILE__, MSG_INVALID_INDEX, pos, -1);
		} else {
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
		}
		return result;
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(enext)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_elem = NULL;
	size_t pos = 0;
	size_t vsize = 0, i;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_elem, dimension_element);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;


		vector<ELEMENT_INFO> elems;

		elems.reserve((*(so->myServer))[utf8_db].dimension[utf8_dim].getCacheData().number_elements);

		std::unique_ptr<DimensionCache::CacheIterator> it = (*(so->myServer))[utf8_db].dimension[utf8_dim].getIterator();

		while (!(*it).end()) {
			elems.push_back(**it);
			++*it;
		}

		vsize = elems.size();

		if (vsize > 0) {
			size_t index;

			if (FindDimElementIndex(elems, utf8_elem, &index) == PALO_TRUE) {
				pos = elems[index].position;
				if (pos < vsize - 1) {
					unsigned long curpos, bestpos=(unsigned long)-1;
					size_t index = -1;

					pos++;
					for (i = 0;i < vsize;i++) {
						curpos = elems[i].position;
						if (curpos == pos) {
							index = i;
							break;
						} else if ((curpos > pos) && (curpos <= bestpos)) {
							index = i;
							bestpos = curpos; 
						}
					}

					if (index != -1)
					{
						result = UTF8_TO_STR(convs, retresult, elems[index].nelement.c_str());
						if (result != PALO_SUCCESS) {
							errmsg = MSG_ENCODING_FAILED;
						}
					}

				} else {
					result = PALO_ERR_INV_OFFSET;
				}
			} else {
				result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
			}
		} else {
			result = PALO_ERR_DIM_EMPTY;
			errmsg = MSG_PALO_ERR_DIM_EMPTY;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_elem);

	if (result != PALO_SUCCESS) {
		if (result == PALO_ERR_INV_OFFSET) {
			ERRSTACK_PREPARE(errs, PALO_ERR_INV_OFFSET, __FUNCTION__, __LINE__, __FILE__, MSG_INVALID_INDEX , pos, + 1);
		} else {
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
		}
		return result;
	}

	return PALO_SUCCESS;

}


palo_err PALO_SUFFIX(get_database_type)(struct errstack *errs, unsigned int *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		*retresult = (*(so->myServer))[utf8_db].getCacheData().type;
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(get_dimension_type)(struct errstack *errs, unsigned int *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		*retresult = (*(so->myServer))[utf8_db].dimension[utf8_dim].getCacheData().type;
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(get_cube_type)(struct errstack *errs, unsigned int *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		*retresult = (*(so->myServer))[utf8_db].cube[utf8_cube].getCacheData().type;
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(get_attribute_dimension)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL;
	long id;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		id = (*(so->myServer))[utf8_db].dimension[utf8_dim].getCacheData().dimension;
		id = (*(so->myServer))[utf8_db].dimension[id].getCacheData().assoc_dimension;
		result = UTF8_TO_STR(convs, retresult, (*(so->myServer))[utf8_db].dimension[id].getCacheData().ndimension.c_str());
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(get_attribute_cube)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL;
	long id;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		id = (*(so->myServer))[utf8_db].dimension[utf8_dim].getCacheData().dimension;
		id = (*(so->myServer))[utf8_db].dimension[id].getCacheData().attribute_cube;
		result = UTF8_TO_STR(convs, retresult, (*(so->myServer))[utf8_db].cube[id].getCacheData().ncube.c_str());
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
		}

	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(get_rights_cube)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL;
	long id;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		id = (*(so->myServer))[utf8_db].dimension[utf8_dim].getCacheData().dimension;
		id = (*(so->myServer))[utf8_db].dimension[id].getCacheData().rights_cube;
		result = UTF8_TO_STR(convs, retresult, (*(so->myServer))[utf8_db].dimension[id].getCacheData().ndimension.c_str());
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
		}

	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(ename)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, size_t n) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;

		std::unique_ptr<DimensionCache::CacheIterator> it = (*(so->myServer))[utf8_db].dimension[utf8_dim].getIterator();

		while (!(*it).end()) {
			if ((**it).position == n - 1) {
				result = UTF8_TO_STR(convs, retresult, (**it).nelement.c_str());
				if (result != PALO_SUCCESS) {
					errmsg = MSG_ENCODING_FAILED;
				}
				break;
			}
			++*it;
		}

	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);

	if (result != PALO_SUCCESS) {
		if (result == PALO_ERR_INV_OFFSET) {
			ERRSTACK_PREPARE(errs, PALO_ERR_INV_OFFSET, __FUNCTION__, __LINE__, __FILE__, wcsdup(L"invalid index: %u"), n);
		} else {
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
		}
		return result;
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(eindex)(struct errstack *errs, unsigned int *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_elem = NULL;

	*retresult = 0;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_elem, dimension_element);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		// We add 1 to get compatibility with palo 1.0c
		*retresult = (*(so->myServer))[utf8_db].dimension[utf8_dim][utf8_elem].getCacheData().position + 1;
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_elem);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(efirst)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
		unsigned long curpos, bestpos=(unsigned long)-1;
		long id = -1;

		std::unique_ptr<DimensionCache::CacheIterator> it = (*(so->myServer))[utf8_db].dimension[utf8_dim].getIterator();

		while (!(*it).end()) {
			curpos = (**it).position;

			if (curpos == 0) {
				id = (**it).element;
				break;
			} else if (curpos <= bestpos){
				id = (**it).element;
				bestpos = curpos; 
			}
			++*it;
		}

		if (id != -1) {
			result = UTF8_TO_STR(convs, retresult, (*(so->myServer))[utf8_db].dimension[utf8_dim][id].getCacheData().nelement.c_str());
			if (result != PALO_SUCCESS) {
				errmsg = MSG_ENCODING_FAILED;
			}
		}

	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(ecount)(struct errstack *errs, unsigned int *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL;
	*retresult = 0;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		*retresult = (*(so->myServer))[utf8_db].dimension[utf8_dim].getCacheData().number_elements;
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(eparentcount)(struct errstack *errs, unsigned int *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_elem = NULL;

	*retresult = 0;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_elem, dimension_element);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		*retresult = (*(so->myServer))[utf8_db].dimension[utf8_dim][utf8_elem].getCacheData().number_parents;
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_elem);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(echildcount)(struct errstack *errs, unsigned int *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_elem = NULL;

	*retresult = 0;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_elem, dimension_element);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		*retresult = (*(so->myServer))[utf8_db].dimension[utf8_dim][utf8_elem].getCacheData().number_children;
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_elem);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(cube_commit_log)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].cube[utf8_cube].save();
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(cube_delete)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].cube[utf8_cube].destroy();
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(cube_unload)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].cube[utf8_cube].unload();
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(cube_load)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].cube[utf8_cube].load();
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(dimension_delete)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].dimension[utf8_dim].destroy();
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(database_delete)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].destroy();
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(database_unload)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].unload();
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(database_load)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *utf8_db = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].load();
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(server_shutdown)(struct errstack *errs, struct sock_obj *so, struct conversions *convs) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	try {
		(*(so->myServer)).shutdown();
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(database_rename_cube)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube_oldname, const CHAR_T *cube_newname) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL, *utf8_newname = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube_oldname);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_newname, cube_newname);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_cube);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].cube[utf8_cube].rename(utf8_newname);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_cube);
	free(utf8_newname);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(database_rename_dimension)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension_oldname, const CHAR_T *dimension_newname) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_newname = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension_oldname);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_newname, dimension_newname);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].dimension[utf8_dim].rename(utf8_newname);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_newname);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(database_add_dimension2)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, dim_type type) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].createDimension(utf8_dim, type);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(database_add_dimension)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	return PALO_SUFFIX(database_add_dimension2)(errs, so, convs, database, dimension, normal_dim);
}


palo_err PALO_SUFFIX(database_add_cube2)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *dimensions,  cube_type type) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	char *utf8_db = NULL, *utf8_cube = NULL;
	size_t i, vsize = dimensions->len;
	vector<string> dimnames(vsize);

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	for (i = 0; i < vsize; i++) {
		result = STR_TO_UTF8(convs, &name, *(dimensions->a + i));
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
			break;
		}
		dimnames[i] = name ;
		free(name);
	}

	try {
		if (result == PALO_SUCCESS) {
			(*(so->myServer))[utf8_db].createCube(utf8_cube, dimnames, type);
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(database_add_cube)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *dimensions) {
	return PALO_SUFFIX(database_add_cube2)(errs, so, convs, database, cube, dimensions, normal_cube);
}


palo_err PALO_SUFFIX(root_add_database)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer)).createDatabase(utf8_db);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(ping)(struct errstack *errs, struct sock_obj *so, struct conversions *convs) {
	palo_err result = PALO_SUCCESS;
	try {
		(*(so->myServer)).ping();
	} catch (const SocketException&) {
		result = PALO_ERR_NETWORK;
	} catch (const std::exception&) {
		result = PALO_ERR_SYSTEM;
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_NETWORK;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, NULL);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(init_ssl)(struct errstack *errs, struct conversions *convs, const CHAR_T *trustfile) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char  *loc_trustfile = NULL;

	if (trustfile != NULL) {
		result = STR_TO_LOCAL(convs, &loc_trustfile, trustfile);
		if (result != PALO_SUCCESS) {
			ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
		}
	}

	try {
		std::string TrustFile(std::string((loc_trustfile == NULL) ? "" : loc_trustfile));
		checkedInitSSL(TrustFile);
	} catch (const PaloServerException& exp) {
		result = PALO_ERR_NEWCODES(exp.code());
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		errmsg = MSG_CONNECT_FAILED;
		result = PALO_ERR_SYSTEM;
#endif
	}

	free(loc_trustfile);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(auth_ssl)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *username, const CHAR_T *pw_hash, const CHAR_T *trustfile, unsigned short use_prefered_ssl) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *utf8_user = NULL, *utf8_pass = NULL;

	result = PALO_SUFFIX(init_ssl)(errs, convs, trustfile);

	if ((username == NULL) || (mystrlen(username) == 0)) {
		errmsg = wcsdup(L"no username given");
		result = PALO_ERR_AUTH;
	} else {
		result = STR_TO_UTF8(convs, &utf8_user, username);
		if (result != PALO_SUCCESS) {
			ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
		}

		result = STR_TO_UTF8(convs, &utf8_pass, pw_hash);
		if (result != PALO_SUCCESS) {
			free(utf8_user);
			ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
		}

		try {
			std::string key;
			so->myServer = JedoxXLHelper::getInstance().getServer(so->hostname, so->port, utf8_user, utf8_pass, key, (use_prefered_ssl) ? Https: Http).get();
			updateso(so, key);
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			result = PALO_ERR_NEWCODES(exp.code());
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			errmsg = MSG_CONNECT_FAILED;
			result = PALO_ERR_SYSTEM;
#endif
		}
	}

	if (result != PALO_SUCCESS) {
		free(utf8_user);
		free(utf8_pass);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	so->username = utf8_user;
	so->pw = utf8_pass;

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(auth)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *username, const CHAR_T *pw_hash) {
	return PALO_SUFFIX(auth_ssl)(errs, so, convs, username, pw_hash, NULL, 0);
}

palo_err PALO_SUFFIX(check_validity)(struct errstack *errs, struct sock_obj *so, struct conversions *convs) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	try {
		std::string key;
		so->myServer = JedoxXLHelper::getInstance().getServer(so->hostname, so->port, so->username, so->pw, key, Https).get();
		updateso(so, key);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		result = PALO_ERR_NEWCODES(exp.code());
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		errmsg = MSG_CONNECT_FAILED;
		result = PALO_ERR_SYSTEM;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(database_save)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].save();
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(eindent)(struct errstack *errs, unsigned int *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_elem = NULL;

	*retresult = 0;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_elem, dimension_element);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		*retresult = (*(so->myServer))[utf8_db].dimension[utf8_dim][utf8_elem].getCacheData().indent;
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_elem);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(eweight)(struct errstack *errs, double *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_parent, const CHAR_T *de_child) {
	palo_err result = PALO_SUCCESS, result2 = PALO_SUCCESS;
	wchar_t *errmsg = NULL, *w_dimname = NULL, *w_elemname = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_parent = NULL, *utf8_child = NULL;
	size_t vsize = 0, i;
	short dnf_dim = 0, dnf_elem = 0;
	CHAR_T *dimension_element = NULL;
	long id;

	*retresult = 0;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_parent, de_parent);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_child, de_child);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		free(utf8_parent);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}
	try {
		vector<ELEMENT_INFO> elems;

		elems.reserve((*(so->myServer))[utf8_db].dimension[utf8_dim].getCacheData().number_elements);

		std::unique_ptr<DimensionCache::CacheIterator> it = (*(so->myServer))[utf8_db].dimension[utf8_dim].getIterator();

		while (!(*it).end()) {
			elems.push_back(**it);
			++*it;
		}

		vsize = elems.size();

		if (vsize > 0) {
			size_t pindex, cindex;

			if (FindDimElementIndex(elems, utf8_parent, &pindex) == PALO_TRUE) {
				if (FindDimElementIndex(elems, utf8_child, &cindex) == PALO_TRUE) {
					vsize = elems[pindex].children.size();
					id = elems[cindex].element;
					result = PALO_ERR_DIM_ELEMENT_NOT_CHILD;
					for (i = 0;i < vsize;i++) {
						if (elems[pindex].children[i] == id) {
							*retresult = elems[pindex].weights[i];
							result = PALO_SUCCESS;
							break;
						}
					}
				} else {
					dimension_element = (CHAR_T*)de_child;
					result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
				}
			} else {
				dimension_element = (CHAR_T*)de_parent;
				result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
			}
		} else {
			result = PALO_ERR_DIM_EMPTY;
			errmsg = MSG_PALO_ERR_DIM_EMPTY;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_parent);
	free(utf8_child);

	if (result != PALO_SUCCESS) {
		if (result == PALO_ERR_DIM_ELEMENT_NOT_FOUND) {
			result2 = STR_TO_WCS(convs, &w_dimname, dimension);
			if (result2 != PALO_SUCCESS) {
				dnf_dim = 1;
				w_dimname = const_cast<wchar_t *>(L"");
			}
			result2 = STR_TO_WCS(convs, &w_elemname, dimension_element);
			if (result2 != PALO_SUCCESS) {
				dnf_elem = 1;
				w_elemname = const_cast<wchar_t *>(L"");
			}
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_DIM_ELEMENT_NOT_FOUND, w_elemname, w_dimname);
		} else {
			if (result == PALO_ERR_DIM_ELEMENT_NOT_CHILD) {
				result2 = STR_TO_WCS(convs, &w_dimname, de_parent);
				if (result2 != PALO_SUCCESS) {
					dnf_dim = 1;
					w_dimname = const_cast<wchar_t *>(L"");
				}
				result = STR_TO_WCS(convs, &w_elemname, de_child);
				if (result2 != PALO_SUCCESS) {
					dnf_elem = 1;
					w_elemname = const_cast<wchar_t *>(L"");
				}
				ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, wcsdup(L"\"%ls\" is not a child of \"%ls\""), w_elemname, w_dimname);
			} else {
				ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
			}
			if (dnf_dim == 0) {
				free(w_dimname);
			}
			if (dnf_elem == 0) {
				free(w_elemname);
			}
		}
		return result;
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(eischild)(struct errstack *errs,  palo_bool *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_parent, const CHAR_T *de_child) {
	palo_err result = PALO_SUCCESS, result2 = PALO_SUCCESS;
	wchar_t *errmsg = NULL, *w_dimname = NULL, *w_elemname = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_parent = NULL, *utf8_child = NULL;
	size_t vsize = 0, i;
	short dnf_dim = 0, dnf_elem = 0;
	CHAR_T *dimension_element = NULL;
	long id;

	*retresult = PALO_FALSE;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_parent, de_parent);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_child, de_child);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		free(utf8_parent);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}
	try {
		vector<ELEMENT_INFO> elems;

		elems.reserve((*(so->myServer))[utf8_db].dimension[utf8_dim].getCacheData().number_elements);

		std::unique_ptr<DimensionCache::CacheIterator> it = (*(so->myServer))[utf8_db].dimension[utf8_dim].getIterator();

		while (!(*it).end()) {
			elems.push_back(**it);
			++*it;
		}

		vsize = elems.size();

		if (vsize > 0) {

			size_t pindex, cindex;

			if (FindDimElementIndex(elems, utf8_parent, &pindex) == PALO_TRUE) {
				if (FindDimElementIndex(elems, utf8_child, &cindex) == PALO_TRUE) {
					vsize = elems[pindex].children.size();
					id = elems[cindex].element;
					for (i = 0;i < vsize;i++) {
						if (elems[pindex].children[i] == id) {
							*retresult = PALO_TRUE;
							break;
						}
					}
				} else {
					dimension_element = (CHAR_T*)de_child;
					result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
				}
			} else {
				dimension_element = (CHAR_T*)de_parent;
				result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
			}
		} else {
			result = PALO_ERR_DIM_EMPTY;
			errmsg = MSG_PALO_ERR_DIM_EMPTY;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_parent);
	free(utf8_child);

	if (result != PALO_SUCCESS) {
		if (result == PALO_ERR_DIM_ELEMENT_NOT_FOUND) {
			result2 = STR_TO_WCS(convs, &w_dimname, dimension);
			if (result2 != PALO_SUCCESS) {
				dnf_dim = 1;
				w_dimname = const_cast<wchar_t *>(L"");
			}
			result2 = STR_TO_WCS(convs, &w_elemname, dimension_element);
			if (result2 != PALO_SUCCESS) {
				dnf_elem = 1;
				w_elemname = const_cast<wchar_t *>(L"");
			}
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_DIM_ELEMENT_NOT_FOUND, w_elemname, w_dimname);
			if (dnf_dim == 0) {
				free(w_dimname);
			}
			if (dnf_dim == 0) {
				free(w_elemname);
			}
		} else {
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
		}
		return result;
	}

	return PALO_SUCCESS;

}


palo_err PALO_SUFFIX(eparentname)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, size_t n) {
	palo_err result = PALO_SUCCESS;
	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_elem = NULL;
	wchar_t *errmsg = NULL;

	*retresult = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_elem, dimension_element);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		ELEMENT_LIST parents = (*(so->myServer))[utf8_db].dimension[utf8_dim][utf8_elem].getCacheData().parents;
		n--;

		if ((n < parents.size()) && (n >= 0)) {
			result = UTF8_TO_STR(convs, retresult, (*(so->myServer))[utf8_db].dimension[utf8_dim][parents[n]].getCacheData().nelement.c_str());
			if (result != PALO_SUCCESS) {
				errmsg = MSG_ENCODING_FAILED;
			}
		} else {
			result = PALO_ERR_INV_ARG;
		}

	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_elem);

	if (result != PALO_SUCCESS) {
		if (result == PALO_ERR_INV_ARG) {
			ERRSTACK_PREPARE(errs, PALO_ERR_INV_ARG, __FUNCTION__, __LINE__, __FILE__, MSG_INVALID_OFFSET, ++n);
		} else {
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
		}
		return result;
	}

	return result;
}


palo_err PALO_SUFFIX(echildname)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, size_t n) {
	palo_err result = PALO_SUCCESS;
	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_elem = NULL;
	wchar_t *errmsg = NULL;

	*retresult = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_elem, dimension_element);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		ELEMENT_LIST children = (*(so->myServer))[utf8_db].dimension[utf8_dim][utf8_elem].getCacheData().children;
		n--;

		if ((n < children.size()) && (n >= 0)) {
			result = UTF8_TO_STR(convs, retresult, (*(so->myServer))[utf8_db].dimension[utf8_dim][children[n]].getCacheData().nelement.c_str());
			if (result != PALO_SUCCESS) {
				errmsg = MSG_ENCODING_FAILED;
			}
		} else {
			result = PALO_ERR_INV_ARG;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_elem);

	if (result != PALO_SUCCESS) {
		if (result == PALO_ERR_INV_ARG) {
			ERRSTACK_PREPARE(errs, PALO_ERR_INV_ARG, __FUNCTION__, __LINE__, __FILE__, MSG_INVALID_OFFSET, ++n);
		} else {
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
		}
		return result;
	}
	return result;
}


palo_err PALO_SUFFIX(esibling)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, int n) {
	palo_err result = PALO_SUCCESS, result2 = PALO_SUCCESS;
	wchar_t *errmsg = NULL, *w_dimname = NULL, *w_elemname = NULL;
	char *utf8_db = NULL, *utf8_dim = NULL, *utf8_elem = NULL;
	size_t vsize = 0;
	short dnf_dim = 0, dnf_elem = 0;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_elem, dimension_element);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		free(utf8_dim);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}
	try {
		if (n == 0) {

			if ((*(so->myServer))[utf8_db].dimension[utf8_dim].Exists(utf8_elem)) {
				*retresult = mystrdup(dimension_element);
				if (*retresult != NULL) {
					result = PALO_SUCCESS;
				} else {
					result = PALO_ERR_NO_MEM;
				}
			} else {
				result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
			}
		} else {

			vector<ELEMENT_INFO> elems;
			elems.reserve((*(so->myServer))[utf8_db].dimension[utf8_dim].getCacheData().number_elements);

			std::unique_ptr<DimensionCache::CacheIterator> it = (*(so->myServer))[utf8_db].dimension[utf8_dim].getIterator();

			while (!(*it).end()) {
				elems.push_back(**it);
				++*it;
			}

			vsize = elems.size();

			if (vsize > 0) {

				size_t index, sindex;
				if (FindDimElementIndex(elems, utf8_elem, &index) == PALO_TRUE) {
					result = getDimElementSiblingIndex(&errmsg, &sindex, elems, index, n);
					if (result == PALO_SUCCESS) {
						result = UTF8_TO_STR(convs, retresult, elems[sindex].nelement.c_str());
						if (result != PALO_SUCCESS) {
							errmsg = MSG_ENCODING_FAILED;
						}
					}
				} else {
					result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
				}
			} else {
				result = PALO_ERR_DIM_EMPTY;
				errmsg = MSG_PALO_ERR_DIM_EMPTY;
			}
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);
	free(utf8_elem);

	if (result != PALO_SUCCESS) {
		if (result == PALO_ERR_DIM_ELEMENT_NOT_FOUND) {
			result2 = STR_TO_WCS(convs, &w_dimname, dimension);
			if (result2 != PALO_SUCCESS) {
				dnf_dim = 1;
				w_dimname = const_cast<wchar_t *>(L"");
			}
			result2 = STR_TO_WCS(convs, &w_elemname, dimension_element);
			if (result2 != PALO_SUCCESS) {
				dnf_elem = 1;
				w_elemname = const_cast<wchar_t *>(L"");
			}
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_DIM_ELEMENT_NOT_FOUND, w_elemname, w_dimname);
			if (dnf_dim == 0) {
				free(w_dimname);
			}
			if (dnf_elem == 0) {
				free(w_elemname);
			}
		} else {
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
		}
		return result;
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(edimension)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, struct arg_str_array *dimension_elements, palo_bool should_be_unique) {
	palo_err result = PALO_SUCCESS, result2 = PALO_SUCCESS;
	palo_bool res = PALO_FALSE;
	wchar_t *errmsg = NULL, *w_dim1name = NULL, *w_dim2name = NULL;
	char *utf8_db = NULL , *utf8_elem = NULL;
	size_t vsize = 0, i, num = dimension_elements->len;
	size_t j = 0;
	short dnf_dim1 = 0, dnf_dim2 = 0;

	vector<DIMENSION_INFO> dims;
	DIMENSION_INFO dim;
	dim.dimension = -1;			 // marker for undefined

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {

		dims.reserve((*(so->myServer))[utf8_db].getCacheData().number_dimensions);

		std::unique_ptr<DimensionsCache::CacheIterator> it = (*(so->myServer))[utf8_db].dimension.getIterator();

		while (!(*it).end()) {
			dims.push_back(**it);
			++*it;
		}

		vsize = dims.size();

		if (vsize > 0) {

			size_t index;

			vector<ELEMENT_INFO> elems;

			std::unique_ptr<DimensionCache::CacheIterator> it;

			for (i = 0; i < vsize; i++) {
				it = (*(so->myServer))[utf8_db].dimension[dims[i].dimension].getIterator();

				while (!(*it).end()) {
					elems.push_back(**it);
					++*it;
				}
				
				for (j = 0; j < num; j++) {
					result = STR_TO_UTF8(convs, &utf8_elem, dimension_elements->a[j]);
					if (result == PALO_SUCCESS) {
						res = FindDimElementIndex(elems, utf8_elem, &index);
						free(utf8_elem);
						if (res == PALO_FALSE) {
							break;
						}
					} else {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				}
				if (result != PALO_SUCCESS) {
					break;
				}
				if (j == num) {
					/* all dimension elements have been found */
					if ((should_be_unique == PALO_TRUE) && (dim.dimension != -1)) {
						/* result is not unique */
						result = PALO_ERR_NAME_NOT_UNIQUE;
						break;
					} else {
						dim = dims[i];
						if (should_be_unique == PALO_FALSE) {
							break;
						}
					}
				}
				elems.clear();
			}

			if (result != PALO_ERR_NAME_NOT_UNIQUE) {
				if (dim.dimension != -1) {
					result = UTF8_TO_STR(convs, retresult, dim.ndimension.c_str());
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
					}
				} else {
					errmsg = MSG_PALO_ERR_DIM_NOT_ALL_ELEMENTS_FOUND;
					result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
				}
			}
		} else {
			errmsg = MSG_PALO_ERR_DIM_NOT_ALL_ELEMENTS_FOUND;
			result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);

	if (result != PALO_SUCCESS) {
		if (result == PALO_ERR_NAME_NOT_UNIQUE) {
			result2 = utf82wcs(convs, &w_dim1name, dim.ndimension.c_str());
			if (result2 != PALO_SUCCESS) {
				dnf_dim1 = 1;
				w_dim1name = const_cast<wchar_t *>(L"");
			}
			result2 = utf82wcs(convs, &w_dim2name, dims[j].ndimension.c_str());
			if (result2 != PALO_SUCCESS) {
				dnf_dim2 = 1;
				w_dim2name = const_cast<wchar_t *>(L"");
			}
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_DUPLICATES_IN_DIMS, w_dim1name, w_dim2name);
			if (dnf_dim1 == 0) {
				free(w_dim1name);
			}
			if (dnf_dim2 == 0) {
				free(w_dim2name);
			}
		} else {
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
		}
		return result;
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(ecubedimension)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, struct arg_str_array *dimension_elements, palo_bool should_be_unique) {
	palo_err result = PALO_SUCCESS, result2 = PALO_SUCCESS;
	palo_bool res = PALO_FALSE;
	wchar_t *errmsg = NULL, *w_dim1name = NULL, *w_dim2name = NULL;
	char *name = NULL, **utf8_elems;
	DIMENSION_LIST dims;
	DIMENSION_INFO dim;
	DIMENSION_INFO dim2;
	size_t vsize = 0, i, num = dimension_elements->len;
	size_t j = 0;
	short dnf_dim1 = 0, dnf_dim2 = 0;

	dim.dimension = -1;			 // marker for undefined

#ifdef ENCODING_M
	name = const_cast<char *>(database);
#else
	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}
#endif
	std::string utf8_db(name);
#ifndef ENCODING_M
	free(name);
#endif
	name = NULL;

#ifdef ENCODING_M
	name = const_cast<char *>(cube);
#else
	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}
#endif
	std::string utf8_cube(name);
#ifndef ENCODING_M
	free(name);
#endif
	name = NULL;

	utf8_elems = (char **)calloc(num, sizeof(char*));
	if (utf8_elems == NULL) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_NO_MEM);
	}

	for (j = 0; j < num; j++) {
		result = STR_TO_UTF8(convs, utf8_elems+j, dimension_elements->a[j]);
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
			break;
		}
	}

	if (result == PALO_SUCCESS)
	{
		try {

			dims = (*(so->myServer))[utf8_db].cube[utf8_cube].getCacheData().dimensions;
			vsize = dims.size();

			if (vsize > 0) {

				bool found;

				for (i = 0; i < vsize; i++) {

					for (j = 0; j < num; j++) {
						found = (*(so->myServer))[utf8_db].dimension[dims[i]].Exists(*(utf8_elems+j));
						if (!found){
							break;
						}
					}
					if (j == num) {
						// all dimension elements have been found
						if ((should_be_unique == PALO_TRUE) && (dim.dimension != -1)) {
							/// result is not unique
							dim2 = (*(so->myServer))[utf8_db].dimension[ dims[i] ].getCacheData();
							result = PALO_ERR_NAME_NOT_UNIQUE;
							break;
						} else {
							dim = (*(so->myServer))[utf8_db].dimension[ dims[i] ].getCacheData();
							if (should_be_unique == PALO_FALSE) {
								break;
							}
						}
					}
				}
				if (result != PALO_ERR_NAME_NOT_UNIQUE) {
					if (dim.dimension != -1) {
						result = UTF8_TO_STR(convs, retresult, dim.ndimension.c_str());
						if (result != PALO_SUCCESS) {
							errmsg = MSG_ENCODING_FAILED;
						}
					} else {
						errmsg = MSG_PALO_ERR_DIM_NOT_ALL_ELEMENTS_FOUND;
						result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
					}
				}
			} else {
				errmsg = MSG_PALO_ERR_DIM_NOT_ALL_ELEMENTS_FOUND;
				result = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
			}
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			result = PALO_ERR_SYSTEM;
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
		}
	}

	for (j = 0; j < num; j++) {
		free(utf8_elems+j);
	}

	if (result != PALO_SUCCESS) {
		if (result == PALO_ERR_NAME_NOT_UNIQUE) {
			result2 = utf82wcs(convs, &w_dim1name, dim.ndimension.c_str());
			if (result2 != PALO_SUCCESS) {
				dnf_dim1 = 1;
				w_dim1name = const_cast<wchar_t *>(L"");
			}
			result2 = utf82wcs(convs, &w_dim2name, dim2.ndimension.c_str());
			if (result2 != PALO_SUCCESS) {
				dnf_dim2 = 1;
				w_dim2name = const_cast<wchar_t *>(L"");
			}
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_DUPLICATES_IN_DIMS, w_dim1name, w_dim2name);
			if (dnf_dim1 == 0) {
				free(w_dim1name);
			}
			if (dnf_dim2 == 0) {
				free(w_dim2name);
			}
		} else {
			ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
		}
		return result;
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(dimension_clear)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *utf8_db = NULL, *utf8_dim = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_dim, dimension);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].dimension[utf8_dim].clear();
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_dim);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(getdata)(struct errstack *errs, struct arg_palo_value *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *coordinates) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL, *name;

	size_t i, vsize = coordinates->len;
	vector<string> elements(vsize);

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	vsize = coordinates->len;
	for (i = 0; i < vsize; i++) {
		result = STR_TO_UTF8(convs, &name, *(coordinates->a + i));
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
			break;
		}
		elements[i] = name ;
		free(name);
	}

	if (result == PALO_SUCCESS) {
		try {
			CELL_VALUE cv;
			(*(so->myServer))[utf8_db].cube[utf8_cube].CellValue(cv, elements);

			result = number2valtypes(cv.type, &(retresult->type));
			if (result == PALO_SUCCESS) {
				switch (retresult->type) {
					case argPaloValueTypeDouble:
						retresult->val.d = cv.val.d;
						break;
					case argPaloValueTypeStr:
						result = UTF8_TO_STR(convs, &(retresult->val.s), cv.val.s.c_str());
						if (result != PALO_SUCCESS) {
							errmsg = MSG_ENCODING_FAILED;
						}
						break;
					default:
						result = PALO_ERR_TYPE;
						break;
				}
			}
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			result = PALO_ERR_SYSTEM;
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
		}
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(getdata_area)(struct errstack *errs, struct arg_palo_value_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array *coordinates) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL, *name = NULL;
	size_t i, j, vsize = coordinates->len, vsize2;

	vector<string> elems;
	vector<vector<string> > area(vsize);

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	for (i = 0; i < vsize; i++) {
		vsize2 = (coordinates->a + i)->len;
		elems.resize(vsize2);
		for (j = 0; j < vsize2; j++) {
			result = STR_TO_UTF8(convs, &name, *((coordinates->a + i)->a + j));
			if (result != PALO_SUCCESS) {
				errmsg = MSG_ENCODING_FAILED;
				break;
			}
			elems[j] = name;
			free(name);
		}
		if (result != PALO_SUCCESS) {
			break;
		}
		area[i] = elems;
	}

	if (result == PALO_SUCCESS) {
		try {
			vector<CELL_VALUE_PATH> cvp = (*(so->myServer))[utf8_db].cube[utf8_cube].CellArea(area);

			vsize = cvp.size();

			retresult->a = (struct arg_palo_value*)calloc(vsize, sizeof(struct arg_palo_value));
			if (retresult->a != NULL) {
				retresult->len = vsize;
				for (i = 0; i < vsize; i++) {
					result = number2valtypes(cvp[i].type, &((retresult->a + i)->type));
					if (result == PALO_SUCCESS) {
						switch ((retresult->a + i)->type) {
							case argPaloValueTypeDouble:
								(retresult->a + i)->val.d = cvp[i].val.d;
								break;
							case argPaloValueTypeStr:
								result = UTF8_TO_STR(convs, &((retresult->a + i)->val.s), cvp[i].val.s.c_str());
								if (result != PALO_SUCCESS) {
									errmsg = MSG_ENCODING_FAILED;
								}
								break;
							default:
								result = PALO_ERR_TYPE;
								break;
						}
					}
					if (result != PALO_SUCCESS) {
						retresult->len = i;
						PALO_SUFFIX(free_arg_palo_value_array_contents)(retresult);
						break;
					}
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			result = PALO_ERR_SYSTEM;
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
		}
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;

}


palo_err PALO_SUFFIX(getdata_area2)(struct errstack *errs, struct arg_palo_dataset_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array *coordinates) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL, *name = NULL;
	size_t vsize = coordinates->len, vsize2, i, j, numdim;
	vector<string> elems;
	vector<vector<string> > area(vsize);

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	for (i = 0; i < vsize; i++) {
		vsize2 = (coordinates->a + i)->len;
		elems.resize(vsize2);
		for (j = 0; j < vsize2; j++) {
			result = STR_TO_UTF8(convs, &name, *((coordinates->a + i)->a + j));
			if (result != PALO_SUCCESS) {
				errmsg = MSG_ENCODING_FAILED;
				break;
			}
			elems[j] = name;
			free(name);
		}
		if (result != PALO_SUCCESS) {
			break;
		}
		area[i] = elems;
	}

	if (result == PALO_SUCCESS) {
		try {

			vector<CELL_VALUE_PATH> cvp = (*(so->myServer))[utf8_db].cube[utf8_cube].CellArea(area);
			DIMENSION_LIST dimids = (*(so->myServer))[utf8_db].cube[utf8_cube].getCacheData().dimensions;

			vsize = cvp.size();
			numdim = dimids.size();

			retresult->len = vsize;
			retresult->a = (struct arg_palo_dataset*)calloc(retresult->len, sizeof(struct arg_palo_dataset));
			if (retresult->a != NULL) {

				for (i = 0;i < retresult->len;i++) {
					(retresult->a + i)->coordinates.len = numdim;

					(retresult->a + i)->coordinates.a = (CHAR_T **)calloc((retresult->a + i)->coordinates.len, sizeof(CHAR_T*));
					if (retresult->a == NULL) {
						retresult->len = (i == 0) ? 0 : i - 1;
						PALO_SUFFIX(free_arg_palo_dataset_array_contents)(retresult);
						errmsg = MSG_PALO_ERR_NO_MEM;
						result = PALO_ERR_NO_MEM;
						break;
					}
				}

				if (result == PALO_SUCCESS) {
					for (i = 0;i < retresult->len;i++) {

						if (cvp[i].path.size() != numdim) {
							errmsg = MSG_ERROR_NUMBER_DIM_IN_CUBE;
							result = PALO_ERR_SYSTEM;
						} else {
							for (j = 0;j < numdim;j++) {
								result = UTF8_TO_STR(convs, (retresult->a + i)->coordinates.a + j, (*(so->myServer))[utf8_db].dimension[dimids[j]][cvp[i].path[j]].getCacheData().nelement.c_str());
								if (result != PALO_SUCCESS) {
									errmsg = MSG_ENCODING_FAILED;
									break;
								}
							}
						}
						if (result != PALO_SUCCESS) {
							PALO_SUFFIX(free_arg_palo_dataset_array_contents)(retresult);
							break;
						}

						result = number2valtypes(cvp[i].type, &((retresult->a + i)->value.type));
						if (result == PALO_SUCCESS) {
							switch ((retresult->a + i)->value.type) {
								case argPaloValueTypeDouble:
									(retresult->a + i)->value.val.d = cvp[i].val.d;
									break;
								case argPaloValueTypeStr:
									result = UTF8_TO_STR(convs, &((retresult->a + i)->value.val.s), cvp[i].val.s.c_str());
									if (result != PALO_SUCCESS) {
										errmsg = MSG_ENCODING_FAILED;
									}
									break;
								default:
									result = PALO_ERR_TYPE;
									break;
							}
						}
						if (result != PALO_SUCCESS) {
							PALO_SUFFIX(free_arg_palo_dataset_array_contents)(retresult);
							break;
						}
					}
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			result = PALO_ERR_SYSTEM;
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
		}
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(getdata_multi)(struct errstack *errs, struct arg_palo_value_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_2d *coordinates) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL;
	struct Cell_Values_Coordinates coord;
	size_t vsize;

	size_t i, j, k, rows, cols;

	Cell_Values_C cvc;

	cvc.len = 0;
	cvc.a = NULL;

	coord.a = NULL;

#ifdef ENCODING_M
	utf8_db = const_cast<char *>(database);
	utf8_cube = const_cast<char *>(cube);
#else
	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}
#endif

	rows = coordinates->rows;
	cols = coordinates->cols;

	coord.rows = rows;
	coord.cols = cols;

	coord.a = (char **)calloc(rows*cols, sizeof(char*));

	result = PALO_SUCCESS;

	if (coord.a != NULL) {
		for (i = 0; i < rows; i++) {
			for (j = 0; j < cols; j++) {
				k = i * cols + j;
#ifdef ENCODING_M
				*(coord.a + k) = *(coordinates->a + k) ;
#else
				result = STR_TO_UTF8(convs, coord.a + k, *(coordinates->a + k));
				if (result != PALO_SUCCESS) {
					errmsg = MSG_ENCODING_FAILED;
					break;
				}
#endif
			}
			if (result != PALO_SUCCESS) {
				break;
			}
		}
	} else {
		result = PALO_ERR_NO_MEM;
		errmsg = MSG_PALO_ERR_NO_MEM;
	}

	if (result == PALO_SUCCESS) {
		try {

			(*(so->myServer))[utf8_db].cube[utf8_cube].CellValues(coord, &cvc);

			if (cvc.a != NULL) {

				vsize = cvc.len;

				retresult->a = (struct arg_palo_value*)calloc(vsize, sizeof(struct arg_palo_value));
				if (retresult->a != NULL) {
					retresult->len = vsize;
					for (i = 0; i < vsize; i++) {
						result = number2valtypes((cvc.a + i)->type, &((retresult->a + i)->type));
						if (result == PALO_SUCCESS) {
							switch ((retresult->a + i)->type) {
								case argPaloValueTypeDouble:
									(retresult->a + i)->val.d = (cvc.a + i)->val.d;
									break;
								case argPaloValueTypeStr:
									result = UTF8_TO_STR(convs, &((retresult->a + i)->val.s), (cvc.a + i)->val.s);
									if (result != PALO_SUCCESS) {
										errmsg = MSG_ENCODING_FAILED;
									}
									break;
								case argPaloValueTypeError:
									(retresult->a + i)->val.err.code = PALO_ERR_NEWCODES((cvc.a + i)->val.errorcode);
									break;

								default:
									result = PALO_ERR_TYPE;
									break;
							}
						}
						if (result != PALO_SUCCESS) {
							retresult->len = i;
							PALO_SUFFIX(free_arg_palo_value_array_contents)(retresult);
							break;
						}
					}
				} else {
					result = PALO_ERR_NO_MEM;
					errmsg = MSG_PALO_ERR_NO_MEM;
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			result = PALO_ERR_SYSTEM;
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
		}
	}

	FreeCell_Values_C_Content(cvc);

#ifndef ENCODING_M
	free(utf8_db);
	free(utf8_cube);

	if (coord.a != NULL) {
		for (i = 0; i < rows; i++) {
			for (j = 0; j < cols; j++) {
				k = i * cols + j;
				free(*(coord.a + k));
			}
		}
	}
#endif

	free(coord.a);			 // contents have been freed above

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(setdata_multi_extended)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset_array *dsa, splash_mode mode, unsigned short add, unsigned short eventprocessor, const struct arg_str_array_2d *locked_coordinates) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;
	size_t i, j, k, vsize, vsize2;

	SPLASH_MODE splashmode;
	vector<string> elems, cell;
	vector<vector<string> > coord, cells;

	CELL_VALUE cv;
	std::vector<CELL_VALUE> cvc;

	result = splasmode2number(mode, &splashmode);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, NULL);
	}

#ifdef ENCODING_M
	name = const_cast<char *>(database);
#else
	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}
#endif
	std::string utf8_db(name);
#ifndef ENCODING_M
	free(name);
#endif
	name = NULL;

#ifdef ENCODING_M
	name = const_cast<char *>(cube);
#else
	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}
#endif
	std::string utf8_cube(name);
#ifndef ENCODING_M
	free(name);
#endif
	name = NULL;

	vsize = dsa->len;

	coord.reserve(vsize);

	for (i = 0; i < vsize; i++) {
		vsize2 = (dsa->a + i)->coordinates.len;
		elems.reserve(vsize2);
		for (j = 0; j < vsize2; j++) {
#ifdef ENCODING_M
			name = *((dsa->a + i)->coordinates.a + j) ;
#else
			result = STR_TO_UTF8(convs, &name, *((dsa->a + i)->coordinates.a + j));
			if (result != PALO_SUCCESS) {
				errmsg = MSG_ENCODING_FAILED;
				break;
			}
#endif
			elems.push_back(name);
#ifndef ENCODING_M
			free(name);
#endif
		}
		if (result != PALO_SUCCESS) {
			break;
		}
		coord.push_back(elems);
		elems.clear();

		switch ((dsa->a + i)->value.type) {
			case argPaloValueTypeDouble:
				cv.type = CELL_VALUE::NUMERIC;
				break;

			case argPaloValueTypeStr:
				cv.type = CELL_VALUE::STRING;
				break;

			default:
				result = PALO_ERR_INV_ARG;
				break;
		}

		if (result == PALO_SUCCESS) {
			switch (cv.type) {
				case CELL_VALUE::NUMERIC:
					cv.val.d = (dsa->a + i)->value.val.d;
					break;
				case CELL_VALUE::STRING:
#ifdef ENCODING_M
					name = (dsa->a + i)->value.val.s;
#else
					result = STR_TO_UTF8(convs, &name, (dsa->a + i)->value.val.s);
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
					}
#endif
					cv.val.s = name;
#ifndef ENCODING_M
					free(name);
#endif
					break;

				default:
					result = PALO_ERR_TYPE;
					break;
			}
		}
		if (result != PALO_SUCCESS) {
			break;
		}
		cvc.push_back(cv);
		cv.val.d = 0;
		cv.val.s.clear();
	}

	if ((result == PALO_SUCCESS) && (locked_coordinates != NULL)) {
		vsize = locked_coordinates->rows;
		vsize2 = locked_coordinates->cols;
		cells.resize(vsize);
		cell.resize(vsize2);

		for (i = 0; i < vsize; i++) {
			for (j = 0; j < vsize2; j++) {

				k = i * vsize2 + j;

				if (*(locked_coordinates->a + k) != NULL) {
#ifdef ENCODING_M
					name = *(locked_coordinates->a + k) ;
#else
					result = STR_TO_UTF8(convs, &name, *(locked_coordinates->a + k));
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
#endif
				} else  {
					name = (char*)calloc(1, sizeof(char));
					if (name == NULL) {
						result = PALO_ERR_NO_MEM;
						errmsg = MSG_PALO_ERR_NO_MEM;
						break;
					}
				}

				cell[j] = name;
#ifndef ENCODING_M
				free(name);
#endif
			}
			if (result != PALO_SUCCESS) {
				break;
			}
			cells[i] = cell;
		}
	}

	if (result == PALO_SUCCESS) {
		try {
			(*(so->myServer))[utf8_db].cube[utf8_cube].CellReplaceBulkWithLock(coord, cvc, splashmode, add != 0, eventprocessor != 0, cells);
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			result = PALO_ERR_SYSTEM;
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
		}
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(setdata_multi)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset_array *dsa, splash_mode mode, unsigned short add, unsigned short eventprocessor) {
	return PALO_SUFFIX(setdata_multi_extended)(errs, so, convs, database, cube, dsa, mode, add, eventprocessor, NULL);
}

palo_err PALO_SUFFIX(setdata_extended)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset *ds, splash_mode mode, unsigned short add, unsigned short eventprocessor, const struct arg_str_array_2d *locked_coordinates) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;
	size_t i, j, k, vsize = ds->coordinates.len, vsize2;
	std::vector<std::string> elements(vsize), cell;
	std::vector<std::vector<std::string> > cells;

	CELL_VALUE cv;
	SPLASH_MODE splashmode;

	result = splasmode2number(mode, &splashmode);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, NULL);
	}

	switch (ds->value.type) {
		case argPaloValueTypeDouble:
			cv.type = CELL_VALUE::NUMERIC;
			break;
		case argPaloValueTypeStr:
			cv.type = CELL_VALUE::STRING;
			break;
		default:
			ERRSTACK_RETURN(errs, PALO_ERR_INV_ARG, __FUNCTION__, __LINE__, __FILE__, NULL);
			break;
	}

	if (cv.type == CELL_VALUE::STRING) {
		result = STR_TO_UTF8(convs, &name, ds->value.val.s);
		if (result != PALO_SUCCESS) {
			ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
		}
		cv.val.s = name;
		free(name);
	} else {
		cv.val.d = ds->value.val.d;
	}

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;

	vsize = ds->coordinates.len;
	for (i = 0; i < vsize; i++) {
		result = STR_TO_UTF8(convs, &name, *(ds->coordinates.a + i));
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
			break;
		}
		elements[i] = name;
		free(name);
	}

	if ((result == PALO_SUCCESS) && (locked_coordinates != NULL)) {
		vsize = locked_coordinates->rows;
		vsize2 = locked_coordinates->cols;
		cells.resize(vsize);
		cell.resize(vsize2);
	
		for (i = 0; i < vsize; i++) {
			for (j = 0; j < vsize2; j++) {
				k = i * vsize2 + j;

				if (*(locked_coordinates->a + k) != NULL) {
					result = STR_TO_UTF8(convs, &name, *(locked_coordinates->a + k));
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				} else  {
					name = (char*)calloc(1, sizeof(char));
					if (name == NULL) {
						result = PALO_ERR_NO_MEM;
						errmsg = MSG_PALO_ERR_NO_MEM;
						break;
					}
				}

				cell[j] = name;
				free(name);
			}
			if (result != PALO_SUCCESS) {
				break;
			}
			cells[i] = cell;
		}
	}

	if (result == PALO_SUCCESS) {
		try {
			(*(so->myServer))[utf8_db].cube[utf8_cube].CellReplaceWithLock(elements, cv, splashmode, add != 0, eventprocessor != 0, cells);
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			result = PALO_ERR_SYSTEM;
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
		}
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(setdata)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset *ds, splash_mode mode) {
	return PALO_SUFFIX(setdata_extended)(errs, so, convs, database, cube, ds, mode, false, true, NULL); 
}

palo_err PALO_SUFFIX(getdata_export_rule)(struct errstack *errs, struct arg_getdata_export_result *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array *export_area, struct arg_getdata_export_options *options, unsigned short use_rules) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;
	size_t vsize = export_area->len, vsize2, i, j , numdim;

	vector<string> elems;
	vector<vector<string> > area(vsize);

	struct arg_getdata_export_options_filter_m Filter;
	std::string condition;

	long double counter, denominator ;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;

	Filter.cmp1 = options->filter.cmp1;
	Filter.cmp2 = options->filter.cmp2;
	Filter.andor12 = options->filter.andor12;
	Filter.val1.type = options->filter.val1.type;
	Filter.val2.type = options->filter.val2.type;

	if (options->filter.val1.type == argPaloValueTypeStr) {
		if (options->filter.val1.val.s != NULL) {
			result = STR_TO_UTF8(convs, &name, options->filter.val1.val.s);
			if (result != PALO_SUCCESS) {
				ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
			}
		} else {
			name = (char*)calloc(1, sizeof(char));
			if (name == NULL) {
				ERRSTACK_RETURN(errs, PALO_ERR_NO_MEM, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_NO_MEM);
			}
		}

		Filter.val1.val.s = name;
	} else {
		Filter.val1.val.s = NULL;
		Filter.val1.val.d = options->filter.val1.val.d;
	}

	if (options->filter.val2.type == argPaloValueTypeStr) {
		if (options->filter.val2.val.s != NULL) {
			result = STR_TO_UTF8(convs, &name, options->filter.val2.val.s);
			if (result != PALO_SUCCESS) {
				free(Filter.val1.val.s);
				ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
			}
		} else {
			name = (char*)calloc(1, sizeof(char));
			if (name == NULL) {
				free(Filter.val1.val.s);
				ERRSTACK_RETURN(errs, PALO_ERR_NO_MEM, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_NO_MEM);
			}
		}
		Filter.val2.val.s = name;
	} else {
		Filter.val2.val.s = NULL;
		Filter.val2.val.d = options->filter.val2.val.d;
	}

	getConditionString(&Filter, condition);

	for (i = 0; i < vsize; i++) {
		vsize2 = (export_area->a + i)->len;
		elems.resize(vsize2);
		for (j = 0; j < vsize2; j++) {
			if (*((export_area->a + i)->a + j) != NULL)
			{
				result = STR_TO_UTF8(convs, &name, *((export_area->a + i)->a + j));
				if (result != PALO_SUCCESS) {
					errmsg = MSG_ENCODING_FAILED;
					break;
				}
			} else  {
				name = (char*)calloc(1, sizeof(char));
				if (name == NULL) {
					result = PALO_ERR_NO_MEM;
					errmsg = MSG_PALO_ERR_NO_MEM;
					break;
				}
			}

			elems[j] = name;
			free(name);
		}
		if (result != PALO_SUCCESS) {
			break;
		}
		area[i] = elems;
	}

	vsize = options->last_coordinates.len;
	elems.resize(vsize);
	for (i = 0; i < vsize; i++) {
		if (((options->last_coordinates.a + i) != NULL))
		{
			result = STR_TO_UTF8(convs, &name, *(options->last_coordinates.a + i));
			if (result != PALO_SUCCESS) {
				errmsg = MSG_ENCODING_FAILED;
				break;
			}
		} else  {
			name = (char*)calloc(1, sizeof(char));
			if (name == NULL) {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
				break;
			}
		}
		elems[i] = name;
		free(name);
	}

	if (result == PALO_SUCCESS) {
		try {

			vector<CELL_VALUE_EXPORTED> cve;
			(*(so->myServer))[utf8_db].cube[utf8_cube].CellExport(cve, area, options->num_datasets, elems, condition, palo_bool2number(options->base_only), palo_bool2number(options->ignore_empty), use_rules);
			DIMENSION_LIST dimids = (*(so->myServer))[utf8_db].cube[utf8_cube].getCacheData().dimensions;

			vsize = cve.size();
			numdim = dimids.size();

			if (vsize > 0) {
				denominator = (cve[vsize-1].exportinfo.allcells == 0) ? 1 : (long double)cve[vsize-1].exportinfo.allcells;
				counter = (long double)cve[vsize-1].exportinfo.usedcells;
				retresult->progress =  counter / denominator;
				retresult->progress = (retresult->progress > 1) ? 1 : retresult->progress;
				retresult->results.len = vsize - 1;
				retresult->results.a = NULL;
			}

			if (vsize > 1) {
				retresult->results.a = (struct arg_palo_dataset*)calloc(retresult->results.len, sizeof(struct arg_palo_dataset));
				if (retresult->results.a != NULL) {

					for (i = 0;i < retresult->results.len;i++) {
						(retresult->results.a + i)->coordinates.len = numdim;

						(retresult->results.a + i)->coordinates.a = (CHAR_T **)calloc((retresult->results.a + i)->coordinates.len, sizeof(CHAR_T*));
						if (retresult->results.a == NULL) {
							retresult->results.len = (i == 0) ? 0 : i - 1;
							PALO_SUFFIX(free_arg_getdata_export_result_contents)(retresult);
							errmsg = MSG_PALO_ERR_NO_MEM;
							result = PALO_ERR_NO_MEM;
							break;
						}
					}

					if (result == PALO_SUCCESS) {
						for (i = 0;i < retresult->results.len;i++) {

							if (cve[i].cvp.path.size() != numdim) {
								errmsg = MSG_ERROR_NUMBER_DIM_IN_CUBE;
								result = PALO_ERR_SYSTEM;
							} else {
								for (j = 0;j < numdim;j++) {
									result = UTF8_TO_STR(convs, (retresult->results.a + i)->coordinates.a + j, (*(so->myServer))[utf8_db].dimension[dimids[j]][cve[i].cvp.path[j]].getCacheData().nelement.c_str());
									if (result != PALO_SUCCESS) {
										errmsg = MSG_ENCODING_FAILED;
										break;
									}
								}
							}
							if (result != PALO_SUCCESS) {
								PALO_SUFFIX(free_arg_getdata_export_result_contents)(retresult);
								break;
							}

							result = number2valtypes(cve[i].cvp.type, &((retresult->results.a + i)->value.type));
							if (result == PALO_SUCCESS) {
								switch ((retresult->results.a + i)->value.type) {
									case argPaloValueTypeDouble:
										(retresult->results.a + i)->value.val.d = cve[i].cvp.val.d;
										break;
									case argPaloValueTypeStr:
										(retresult->results.a + i)->value.val.d = 0;
										result = UTF8_TO_STR(convs, &((retresult->results.a + i)->value.val.s), cve[i].cvp.val.s.c_str());
										if (result != PALO_SUCCESS) {
											errmsg = MSG_ENCODING_FAILED;
										}
										break;
									case argPaloValueTypeError:
										(retresult->results.a + i)->value.val.d = cve[i].cvp.val.errorcode;
										(retresult->results.a + i)->value.val.err.code = PALO_ERR_NEWCODES(cve[i].cvp.val.errorcode);
										result = UTF8_TO_STR(convs, &((retresult->results.a + i)->value.val.s), cve[i].cvp.val.s.c_str());
										if (result != PALO_SUCCESS) {
											errmsg = MSG_ENCODING_FAILED;
										}
										break;
									default:
										result = PALO_ERR_TYPE;
										break;
								}
							}
							if (result != PALO_SUCCESS) {
								PALO_SUFFIX(free_arg_getdata_export_result_contents)(retresult);
								break;
							}
						}
					}
				} else {
					errmsg = MSG_PALO_ERR_NO_MEM;
					result = PALO_ERR_NO_MEM;
				}
			} else {
				if (vsize == 0) {
					result = PALO_ERR_SYSTEM;
				}
			}
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			result = PALO_ERR_SYSTEM;
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
		}
	}

	free(Filter.val1.val.s);
	free(Filter.val2.val.s);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(getdata_export)(struct errstack *errs, struct arg_getdata_export_result *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array *export_area, struct arg_getdata_export_options *options) {
	return PALO_SUFFIX(getdata_export_rule)(errs, retresult, so, convs, database, cube, export_area, options, false);
}


palo_err PALO_SUFFIX(cube_clear)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned short complete, const struct arg_str_array_array *elements) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL, *name = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		if (complete != 0) {
			(*(so->myServer))[utf8_db].cube[utf8_cube].clear();
		} else {

			size_t vsize = elements->len, vsize2, i, j;

			vector<string> elems;
			vector<vector<string> > area(vsize);

			vsize = elements->len;
			for (i = 0; i < vsize; i++) {
				vsize2 = (elements->a + i)->len;
				elems.resize(vsize2);
				for (j = 0; j < vsize2; j++) {
					result = STR_TO_UTF8(convs, &name, *((elements->a + i)->a + j));
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
					elems[j] = name;
					free(name);
				}
				if (result != PALO_SUCCESS) {
					break;
				}
				area[i] = elems;
			}
			(*(so->myServer))[utf8_db].cube[utf8_cube].clear(area);
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(cell_copy_extended)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, cell_copy_mode mode, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *from, const struct arg_str_array *to, const struct arg_str_array_array *predict_area, const struct arg_str_array_2d *locked_coordinates, double* val, unsigned short use_rules) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	bool UseRules = (use_rules != 0);
	size_t i, j, k, vsize, vsize2;
	std::vector<std::string> cell, elems, fromelements;
	std::vector<std::vector<std::string> > cells;
	std::vector<std::vector<std::string> > area;

	char *name = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;


	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;


	if ((mode == cell_copy_default) && (from != NULL))
	{
		vsize = from->len;
		fromelements.resize(vsize);

		for (i = 0; i < vsize; i++) {
			result = STR_TO_UTF8(convs, &name, *(from->a + i));
			if (result != PALO_SUCCESS) {
				errmsg = MSG_ENCODING_FAILED;
				break;
			}
			fromelements[i] = name;
			free(name);
		}
	}

	vsize = to->len;
	vector<string> toelements(vsize);

	if (result == PALO_SUCCESS) {
		for (i = 0; i < vsize; i++) {
			result = STR_TO_UTF8(convs, &name, *(to->a + i));
			if (result != PALO_SUCCESS) {
				errmsg = MSG_ENCODING_FAILED;
				break;
			}
			toelements[i] = name;
			free(name);
		}
	}

	if ((result == PALO_SUCCESS) && (locked_coordinates != NULL)) {
		vsize = locked_coordinates->rows;
		vsize2 = locked_coordinates->cols;
		cells.resize(vsize);
		cell.resize(vsize2);
	
		for (i = 0; i < vsize; i++) {
			for (j = 0; j < vsize2; j++) {
				k = i * vsize2 + j;

				if (*(locked_coordinates->a + k) != NULL) {
					result = STR_TO_UTF8(convs, &name, *(locked_coordinates->a + k));
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				} else  {
					name = (char*)calloc(1, sizeof(char));
					if (name == NULL) {
						result = PALO_ERR_NO_MEM;
						errmsg = MSG_PALO_ERR_NO_MEM;
						break;
					}
				}

				cell[j] = name;
				free(name);
			}
			if (result != PALO_SUCCESS) {
				break;
			}
			cells[i] = cell;
		}
	}

	if ((result == PALO_SUCCESS) && (predict_area != NULL)) {
		vsize = predict_area->len;
		area.resize(vsize);
	
		for (i = 0; i < vsize; i++) {
			vsize2 = (predict_area->a + i)->len;
			elems.resize(vsize2);
			for (j = 0; j < vsize2; j++) {
				if (*((predict_area->a + i)->a + j) != NULL) {
					result = STR_TO_UTF8(convs, &name, *((predict_area->a + i)->a + j));
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				} else  {
					name = (char*)calloc(1, sizeof(char));
					if (name == NULL) {
						result = PALO_ERR_NO_MEM;
						errmsg = MSG_PALO_ERR_NO_MEM;
						break;
					}
				}

				elems[j] = name;
				free(name);
			}
			if (result != PALO_SUCCESS) {
				break;
			}
			area[i] = elems;
		}
	}

	if (result == PALO_SUCCESS) {
		try {
			if (mode == cell_copy_predict_linear_regression) {
				(*(so->myServer))[utf8_db].cube[utf8_cube].CellPredictLinearRegression(area, toelements, UseRules, cells);
			} else {
				if (val != NULL) {
					(*(so->myServer))[utf8_db].cube[utf8_cube].CellCopy(fromelements, toelements, *val, UseRules, cells);
				} else {
					(*(so->myServer))[utf8_db].cube[utf8_cube].CellCopy(fromelements, toelements, UseRules, cells);
				}
			}
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			result = PALO_ERR_SYSTEM;
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
		}

	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(cell_copy)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *from, const struct arg_str_array *to, unsigned short withValue, double val) {
	return PALO_SUFFIX(cell_copy_extended)(errs, so, convs, cell_copy_default, database, cube, from, to, NULL, NULL, (withValue) ? &val : NULL, false);
}

palo_err PALO_SUFFIX(event_lock_begin)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, CHAR_T *source, CHAR_T *AreaID) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *utf8_source = NULL, *utf8_area = NULL;

	result = STR_TO_UTF8(convs, &utf8_source, source);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_area, AreaID);
	if (result != PALO_SUCCESS) {
		free(utf8_source);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer)).EventLockBegin(utf8_source, utf8_area);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_source);
	free(utf8_area);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(event_lock_end)(struct errstack *errs, struct sock_obj *so, struct conversions *convs) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	try {
		(*(so->myServer)).EventLockEnd();
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(server_info)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, struct server_info *retresult) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	SERVER_INFO srvinfo;

	try {
		srvinfo = (*(so->myServer)).getCacheDataCopy();

		retresult->major_version = srvinfo.major_version;
		retresult->minor_version = srvinfo.minor_version;
		retresult->bugfix_version = srvinfo.bugfix_version;
		retresult->build_number = srvinfo.build_number;
		retresult->encryption = srvinfo.encryption;
		retresult->https_port = srvinfo.httpsPort;
		retresult->data_sequence_number = srvinfo.data_sequence_number;
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

void PALO_SUFFIX(init_rule_info_strings)(struct arg_rule_info *retresult) {

	retresult->definition = NULL;
	retresult->extern_id = NULL;
	retresult->comment = NULL;

	return;
}

void PALO_SUFFIX(init_rule_info)(struct arg_rule_info *retresult) {

	retresult->identifier = -1;
	retresult->timestamp = 0;
	retresult->activated = 0;
	retresult->position = 0;
	PALO_SUFFIX(init_rule_info_strings)(retresult);

	return;
}

palo_err PALO_SUFFIX(set_rule_info)(wchar_t **errmsg, struct arg_rule_info *retresult, struct conversions *convs, RULE_INFO & ri) {
	palo_err result = PALO_SUCCESS;

	PALO_SUFFIX(init_rule_info_strings)(retresult);

	retresult->identifier = ri.identifier;
	retresult->timestamp = ri.timestamp;
	retresult->activated = ri.activated;
	retresult->position = ri.position;

	result = UTF8_TO_STR(convs, &(retresult->definition), ri.definition.c_str());
	if (result != PALO_SUCCESS) {
		*errmsg = MSG_ENCODING_FAILED;
		return result;
	}
	result = UTF8_TO_STR(convs, &(retresult->extern_id), ri.extern_id.c_str());
	if (result != PALO_SUCCESS) {
		*errmsg = MSG_ENCODING_FAILED;
		return result;
	}
	result = UTF8_TO_STR(convs, &(retresult->comment), ri.comment.c_str());
	if (result != PALO_SUCCESS) {
		*errmsg = MSG_ENCODING_FAILED;
		return result;
	}

	return PALO_SUCCESS;

}


palo_err PALO_SUFFIX(rule_add)(struct errstack *errs, struct arg_rule_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const CHAR_T *definition, unsigned short use_identifier, const CHAR_T *extern_id, const CHAR_T *comment, unsigned short activate, double position) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, definition);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_definition(name);
	free(name);
	name = NULL;

	if (extern_id != NULL) {
		result = STR_TO_UTF8(convs, &name, extern_id);
		if (result != PALO_SUCCESS) {
			ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
		}
	} else {
		name = (char*)calloc(1, sizeof(char));
		ERRSTACK_RETURN(errs, PALO_ERR_NO_MEM, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_NO_MEM);
	}

	std::string utf8_extern_id(name);
	free(name);
	name = NULL;

	if (comment != NULL) {
		result = STR_TO_UTF8(convs, &name, comment);
		if (result != PALO_SUCCESS) {
			ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
		}
	} else {
		name = (char*)calloc(1, sizeof(char));
		ERRSTACK_RETURN(errs, PALO_ERR_NO_MEM, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_NO_MEM);
	}

	std::string utf8_comment(name);
	free(name);
	name = NULL;

	PALO_SUFFIX(init_rule_info)(retresult);

	try {
		RULE_INFO ri = (*(so->myServer))[utf8_db].cube[utf8_cube].RuleCreate(utf8_definition, use_identifier, utf8_extern_id, utf8_comment, activate, position);
		result = PALO_SUFFIX(set_rule_info)(&errmsg, retresult, convs, ri);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		PALO_SUFFIX(free_arg_rule_info_contents)(retresult);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(rule_modify)(struct errstack *errs, struct arg_rule_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, long id, const CHAR_T *definition, unsigned short use_identifier, const CHAR_T *extern_id, const CHAR_T *comment, unsigned short activate, double position) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, definition);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_definition(name);
	free(name);
	name = NULL;

	if (extern_id != NULL) {
		result = STR_TO_UTF8(convs, &name, extern_id);
		if (result != PALO_SUCCESS) {
			ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
		}
	} else {
		name = (char*)calloc(1, sizeof(char));
		ERRSTACK_RETURN(errs, PALO_ERR_NO_MEM, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_NO_MEM);
	}

	std::string utf8_extern_id(name);
	free(name);
	name = NULL;

	if (comment != NULL) {
		result = STR_TO_UTF8(convs, &name, comment);
		if (result != PALO_SUCCESS) {
			ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
		}
	} else {
		name = (char*)calloc(1, sizeof(char));
		ERRSTACK_RETURN(errs, PALO_ERR_NO_MEM, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_NO_MEM);
	}

	std::string utf8_comment(name);
	free(name);
	name = NULL;

	PALO_SUFFIX(init_rule_info)(retresult);

	try {
		RULE_INFO ri = (*(so->myServer))[utf8_db].cube[utf8_cube].RuleModify(id, utf8_definition, use_identifier, utf8_extern_id, utf8_comment, activate, position);
		result = PALO_SUFFIX(set_rule_info)(&errmsg, retresult, convs, ri);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		PALO_SUFFIX(free_arg_rule_info_contents)(retresult);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(rule_info)(struct errstack *errs, struct arg_rule_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, long id, unsigned short use_identifier) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;

	PALO_SUFFIX(init_rule_info_strings)(retresult);

	try {
		RULE_INFO ri = (*(so->myServer))[utf8_db].cube[utf8_cube].RuleInfo(id, use_identifier);
		result = PALO_SUFFIX(set_rule_info)(&errmsg, retresult, convs, ri);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_SYSTEM;
#ifndef _DEBUG
	} catch (...) {
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
		result = PALO_ERR_SYSTEM;
#endif
	}

	if (result != PALO_SUCCESS) {
		PALO_SUFFIX(free_arg_rule_info_contents)(retresult);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(cell_rule_info)(struct errstack *errs, struct arg_rule_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array *coordinates) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	size_t i, vsize = coordinates->len;
	vector<string> elements(vsize);

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;

	vsize = coordinates->len;
	for (i = 0; i < vsize; i++) {
		result = STR_TO_UTF8(convs, &name, *(coordinates->a + i));
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
			break;
		}
		elements[i] = name ;
		free(name);
		name = NULL;
	}

	PALO_SUFFIX(init_rule_info)(retresult);

	if (result == PALO_SUCCESS) {
		try {
			CELL_VALUE cv;
			(*(so->myServer))[utf8_db].cube[utf8_cube].CellValue(cv, elements, 1);

			if (cv.ruleID != -1) {
				RULE_INFO ri = (*(so->myServer))[utf8_db].cube[utf8_cube].RuleInfo(cv.ruleID, 0);
				result = PALO_SUFFIX(set_rule_info)(&errmsg, retresult, convs, ri);
			}
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_SYSTEM;
#ifndef _DEBUG
		} catch (...) {
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
			result = PALO_ERR_SYSTEM;
#endif
		}
	}

	if (result != PALO_SUCCESS) {
		PALO_SUFFIX(free_arg_rule_info_contents)(retresult);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(rule_delete)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, long identifier) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;

	try {
		(*(so->myServer))[utf8_db].cube[utf8_cube].RuleDestroy(identifier);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(list_rules)(struct errstack *errs, struct arg_rule_info_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned short use_identifier) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	size_t vsize, i;

	retresult->len = 0;
	retresult->a = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;


	try {
		vector<RULE_INFO> riv = (*(so->myServer))[utf8_db].cube[utf8_cube].Rules(use_identifier);

		vsize = riv.size();

		if (vsize > 0) {

			retresult->a = (struct arg_rule_info*)calloc(vsize, sizeof(struct arg_rule_info));
			retresult->len = vsize;

			if (retresult->a != NULL) {
				for (i = 0; i < vsize; i++) {
					result = PALO_SUFFIX(set_rule_info)(&errmsg, retresult->a + i, convs, riv[i]);
					if (result != PALO_SUCCESS) {
						retresult->len = i;
						PALO_SUFFIX(free_arg_rule_info_array_contents)(retresult);
						break;
					}
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(list_rulefunctions)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	try {
		string functions = (*(so->myServer)).RuleFunctions();

		result = UTF8_TO_STR(convs, retresult, functions.c_str());
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(rule_parse)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const CHAR_T *definition) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, definition);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_definition(name);
	free(name);
	name = NULL;


	try {
		string xmldef = (*(so->myServer))[utf8_db].cube[utf8_cube].RuleParse(utf8_definition);

		result = UTF8_TO_STR(convs, retresult, xmldef.c_str());
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

void PALO_SUFFIX(init_lock_info)(struct arg_lock_info *retresult) {

	retresult->area.len = 0;
	retresult->area.a = NULL;
	retresult->user = NULL;

	retresult->identifier = -1;
	retresult->steps = 0;

	return;
}

palo_err PALO_SUFFIX(set_lock_info)(struct sock_obj *so, wchar_t **errmsg, struct arg_lock_info *retresult, struct conversions *convs, LOCK_INFO &li, char *utf8_db, char *utf8_cube) {
	palo_err result = PALO_SUCCESS;
	size_t i, j, vsize = li.area.size(), vsize2;

	retresult->area.len = 0;
	retresult->area.a = NULL;
	retresult->user = NULL;

	retresult->identifier = li.lockid;
	retresult->steps = li.steps;

	result = UTF8_TO_STR(convs, &(retresult->user), li.user.c_str());
	if (result != PALO_SUCCESS) {
		*errmsg = MSG_ENCODING_FAILED;
		return result;
	}

	retresult->area.a = (struct arg_str_array*)calloc(vsize, sizeof(struct arg_str_array));

	if (retresult->area.a == NULL) {
		*errmsg = MSG_PALO_ERR_NO_MEM;
		return PALO_ERR_NO_MEM;
	}

	retresult->area.len = vsize;

	DIMENSION_LIST dimensions = (*(so->myServer))[utf8_db].cube[utf8_cube].getCacheData().dimensions;

	for (i = 0; i < vsize; i++) {
		vsize2 = li.area[i].size();
		(retresult->area.a + i)->a = (CHAR_T **)calloc(vsize2, sizeof(CHAR_T*));
		if ((retresult->area.a + i)->a == NULL) {
			*errmsg = MSG_PALO_ERR_NO_MEM;
			return PALO_ERR_NO_MEM;
		}
		(retresult->area.a + i)->len = vsize2;
		for (j = 0; j < vsize2; j++) {
			result = UTF8_TO_STR(convs, (retresult->area.a + i)->a + j, (*(so->myServer))[utf8_db].dimension[dimensions[i]][li.area[i][j]].getCacheData().nelement.c_str());
			if (result != PALO_SUCCESS) {
				*errmsg = MSG_ENCODING_FAILED;
				return result;
			}
		}
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(cube_lock)(struct errstack *errs, struct arg_lock_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array_array *locked_area) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL, *name;

	size_t i, j, vsize = locked_area->len, vsize2;
	vector<string> elems;
	vector<vector<string> > area(vsize);

	PALO_SUFFIX(init_lock_info)(retresult);

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	for (i = 0; i < vsize; i++) {
		vsize2 = (locked_area->a + i)->len;
		elems.resize(vsize2);
		for (j = 0; j < vsize2; j++) {
			result = STR_TO_UTF8(convs, &name, *((locked_area->a + i)->a + j));
			if (result != PALO_SUCCESS) {
				errmsg = MSG_ENCODING_FAILED;
				break;
			}
			elems[j] = name;
			free(name);
		}
		if (result != PALO_SUCCESS) {
			break;
		}
		area[i] = elems;
	}

	if (result == PALO_SUCCESS) {
		try {
			LOCK_INFO li = (*(so->myServer))[utf8_db].cube[utf8_cube].Lock(area);
			result = PALO_SUFFIX(set_lock_info)(so, &errmsg, retresult, convs, li, utf8_db, utf8_cube);
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			result = PALO_ERR_SYSTEM;
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
		}
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		PALO_SUFFIX(free_arg_lock_info_contents)(retresult);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(cube_locks)(struct errstack *errs, struct arg_lock_info_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL;

	size_t i, vsize;

	retresult->a = NULL;
	retresult->len = 0;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		vector<LOCK_INFO> lia = (*(so->myServer))[utf8_db].cube[utf8_cube].Locks();

		vsize = lia.size();
		retresult->a = (struct arg_lock_info*)calloc(vsize , sizeof(struct arg_lock_info));

		if (retresult->a != NULL) {
			retresult->len = lia.size();
			for (i = 0; i < vsize; i++) {
				result = PALO_SUFFIX(set_lock_info)(so, &errmsg, retresult->a + i, convs, lia[i], utf8_db, utf8_cube);
				if (result != PALO_SUCCESS) {
					break;
				}
			}
		} else {
			errmsg = MSG_PALO_ERR_NO_MEM;
			result = PALO_ERR_NO_MEM;
		}

	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		PALO_SUFFIX(free_arg_lock_info_array_contents)(retresult);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(cube_rollback)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned long lockid, long steps) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].cube[utf8_cube].Rollback(lockid, steps);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(cube_commit)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned long lockid) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *utf8_db = NULL, *utf8_cube = NULL;

	result = STR_TO_UTF8(convs, &utf8_db, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	result = STR_TO_UTF8(convs, &utf8_cube, cube);
	if (result != PALO_SUCCESS) {
		free(utf8_db);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	try {
		(*(so->myServer))[utf8_db].cube[utf8_cube].Commit(lockid);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	free(utf8_db);
	free(utf8_cube);

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(dimension_subset)(struct errstack *errs, struct arg_subset_result_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, int indent,
												struct arg_alias_filter_settings *alias,
												struct arg_field_filter_settings *field,
												struct arg_basic_filter_settings *basic,
												struct arg_data_filter_settings *data,
												struct arg_sorting_filter_settings *sorting,
												struct arg_structural_filter_settings *structural,
												struct arg_text_filter_settings *text) {

	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *name = NULL;
	size_t i, j, size1, size2;

	AliasFilterSettings m_alias;
	FieldFilterSettings m_field;
	BasicFilterSettings m_basic;
	DataFilterSettings m_data;
	SortingFilterSettings m_sorting;
	StructuralFilterSettings m_structural;
	TextFilterSettings m_text;

	std::vector<std::string> elems;

	retresult->len = 0;
	retresult->a = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, dimension);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_dimension(name);
	free(name);
	name = NULL;

	m_alias.active = (alias->active != 0);

	if (m_alias.active) {
		if (alias->attribute1 != NULL) {
			result = STR_TO_UTF8(convs, &name, alias->attribute1);
			if (result != PALO_SUCCESS) {
				ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
			} 
		}
		m_alias.attribute1 = (name == NULL) ? "" : name;
		free(name);
		name = NULL;

		if (alias->attribute2 != NULL) {
			result = STR_TO_UTF8(convs, &name, alias->attribute2);
			if (result != PALO_SUCCESS) {
				ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
			} 
		}
		m_alias.attribute2 = (name == NULL) ? "" : name;
		free(name);
		name = NULL;

		m_alias.flags = alias->flags;
		m_alias.indent = indent;
	}

	m_field.active = (field->active != 0);

	if (m_field.active) {
		size1 = field->advanced.len;
		m_field.advanced.resize(size1);

		for (i=0; i < size1; i++) {
			size2 = (field->advanced.a[i]).len;
			elems.resize(size2);

			for(j=0; j < size2; j++) {
				if ((field->advanced.a[i]).a[j] != NULL) {
					result = STR_TO_UTF8(convs, &name, (field->advanced.a[i]).a[j]);
					if (result != PALO_SUCCESS) {
						ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
					}
				}
				elems[j] = (name == NULL) ? "" : name;
				free(name);
				name = NULL;
			}
			m_field.advanced[i] = elems;
		}

		m_field.flags = field->flags;
		m_field.indent = indent;
	}
	
	m_basic.active = (basic->active != 0);

	if (m_basic.active) {
		size1 = basic->manual_subset.len;
		m_basic.manual_subset.resize(size1);

		for (i=0; i < size1; i++) {
			if (basic->manual_subset.a[i] != NULL) {
				result = STR_TO_UTF8(convs, &name, basic->manual_subset.a[i]);
				if (result != PALO_SUCCESS) {
					ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
				}
			}
			m_basic.manual_subset[i] = (name == NULL) ? "" : name;
			free(name);
			name = NULL;
		}

		m_basic.flags = basic->flags;
		m_basic.indent = indent;
		m_basic.manual_subset_set = (basic->manual_subset_set != 0);
	}

	m_data.active = (data->active != 0);

	if (m_data.active) {
		if (data->cube != NULL) {
			result = STR_TO_UTF8(convs, &name, data->cube);
			if (result != PALO_SUCCESS) {
				ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
			}
		}	
		m_data.cube = (name == NULL) ? "" : name;
		free(name);
		name = NULL;

		m_data.cmp.use_strings = (data->cmp.use_strings != 0);
		m_data.cmp.par1d = data->cmp.par1d;
		m_data.cmp.par2d = data->cmp.par2d;

		if (data->cmp.par1s != NULL) {
			result = STR_TO_UTF8(convs, &name, data->cmp.par1s);
			if (result != PALO_SUCCESS) {
				ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
			}
		}	
		m_data.cmp.par1s = (name == NULL) ? "" : name;
		free(name);
		name = NULL;

		if (data->cmp.par2s != NULL) {
			result = STR_TO_UTF8(convs, &name, data->cmp.par2s);
			if (result != PALO_SUCCESS) {
				ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
			}
		}	
		m_data.cmp.par2s = (name == NULL) ? "" : name;
		free(name);
		name = NULL;

		if (data->cmp.op1 != NULL) {
			result = STR_TO_UTF8(convs, &name, data->cmp.op1);
			if (result != PALO_SUCCESS) {
				ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
			}
		}	
		m_data.cmp.op1 = (name == NULL) ? "" : name;
		free(name);
		name = NULL;

		if (data->cmp.op2 != NULL) {
			result = STR_TO_UTF8(convs, &name, data->cmp.op2);
			if (result != PALO_SUCCESS) {
				ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
			}
		}	
		m_data.cmp.op2 = (name == NULL) ? "" : name;
		free(name);
		name = NULL;

		m_data.coords_set = (data->coords_set != 0);

		size1 = data->coords.len;
		m_data.coords.resize(size1);

		for (i=0; i < size1; i++) {
			size2 = (data->coords.a[i]).str.len;
			elems.resize(size2);

			for(j=0; j < size2; j++) {
				if ((data->coords.a[i]).str.a[j] != NULL) {
					result = STR_TO_UTF8(convs, &name, (data->coords.a[i]).str.a[j]);
					if (result != PALO_SUCCESS) {
						ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
					}
				}
				elems[j] = (name == NULL) ? "" : name;
				free(name);
				name = NULL;
			}
			m_data.coords[i] = std::pair<bool, std::vector<std::string> >(((data->coords.a[i]).boolval !=0), elems);
		}

		m_data.upper_percentage_set = (data->upper_percentage_set != 0);
		m_data.lower_percentage_set = (data->lower_percentage_set != 0);
		m_data.upper_percentage = data->upper_percentage;
		m_data.lower_percentage = data->lower_percentage;
		m_data.top = data->top;

		m_data.flags = data->flags;
		m_data.indent = indent;
	}

	m_sorting.active = (sorting->active != 0);

	if (m_sorting.active) {
		if (sorting->attribute != NULL) {
			result = STR_TO_UTF8(convs, &name, sorting->attribute);
			if (result != PALO_SUCCESS) {
				ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
			} 
		}
		m_sorting.attribute = (name == NULL) ? "" : name;
		free(name);
		name = NULL;

		m_sorting.level = sorting->level;
		m_sorting.flags = sorting->flags;
		m_sorting.limit_count = sorting->limit_count;
		m_sorting.limit_start = sorting->limit_start;
		m_sorting.indent = indent;

	}

	m_structural.active = (structural->active != 0);

	if (m_structural.active) {

		if (structural->bound != NULL) {
			result = STR_TO_UTF8(convs, &name, structural->bound);
			if (result != PALO_SUCCESS) {
				ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
			} 
		}
		m_structural.bound = (name == NULL) ? "" : name;
		free(name);
		name = NULL;

		if (structural->revolve_element != NULL) {
			result = STR_TO_UTF8(convs, &name, structural->revolve_element);
			if (result != PALO_SUCCESS) {
				ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
			} 
		}
		m_structural.revolve_elem = (name == NULL) ? "" : name;
		free(name);
		name = NULL;

		m_structural.level = (structural->level != 0);
		m_structural.revolve = (structural->revolve != 0);
		m_structural.level_start = structural->level_start;
		m_structural.level_end = structural->level_end;
		m_structural.revolve_count = structural->revolve_count;

		m_structural.flags = structural->flags;
		m_structural.indent = indent;
	}


	m_text.active = (text->active != 0);

	if (m_text.active) {

		size1 = text->reg_exps.len;
		m_text.regexps.resize(size1);

		for (i=0; i < size1; i++) {
			if (text->reg_exps.a[i] != NULL) {
				result = STR_TO_UTF8(convs, &name, text->reg_exps.a[i]);
				if (result != PALO_SUCCESS) {
					ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
				}
			}
			m_text.regexps[i] = (name == NULL) ? "" : name;
			free(name);
			name = NULL;
		}

		m_text.flags = text->flags;
		m_text.indent = indent;
	}

	try {
		ElementExList el;
		(*(so->myServer))[utf8_db].dimension[utf8_dimension].subset(el, so->myServer, m_alias, m_field, m_basic, m_data, m_sorting, m_structural, m_text);

		size_t len = el.size();

		if (len > 0) {
			retresult->a = (struct arg_subset_result*)calloc(len, sizeof(struct arg_subset_result));
			retresult->len = (unsigned int)len;

			if (retresult->a != NULL) {
				ElementExList::iterator it, elb = el.begin(), ele = el.end();
				for (it = elb, i = 0; it != ele; ++it, ++i) {

					(retresult->a+i)->identifier = it->m_einf.element;
					(retresult->a+i)->depth = it->m_einf.depth;
					(retresult->a+i)->index = it->get_idx(indent);

					(retresult->a+i)->has_children = (it->m_einf.children.size() > 0);

					result = UTF8_TO_STR(convs, &((retresult->a+i)->name), it->get_name().c_str());
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}

					result = UTF8_TO_STR(convs, &((retresult->a+i)->alias), it->get_alias().c_str());
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}

					result = UTF8_TO_STR(convs, &((retresult->a+i)->path), it->path.c_str());
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		}

	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		PALO_SUFFIX(free_arg_subset_result_array_contents)(retresult);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(cell_drill_through)(struct errstack *errs, struct arg_str_array_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *coordinates, drillthrough_type mode) {

	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	size_t i, j, vsize = coordinates->len, vsize2;
	vector<string> elements(vsize);

	retresult->len = 0;
	retresult->a = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;

	for (i = 0; i < vsize; i++) {
		result = STR_TO_UTF8(convs, &name, *(coordinates->a + i));
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
			break;
		}
		elements[i] = name ;
		free(name);
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	try {

		std::vector<DRILLTHROUGH_INFO> cdt;

		(*(so->myServer))[utf8_db].cube[utf8_cube].CellDrillThrough(cdt, elements, Cube::DRILLTHROUGH_TYPE(mode)); 

		vsize = cdt.size();

		retresult->a = (struct arg_str_array*)calloc(vsize, sizeof(struct arg_str_array));

		if (retresult->a == NULL) {
			ERRSTACK_RETURN(errs, PALO_ERR_NO_MEM, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_NO_MEM);
		}

		retresult->len = vsize;
		std::string line;
		std::string sep(";");

		for (i = 0; i < vsize; i++) {
			(retresult->a + i)->a = (CHAR_T **)calloc(1, sizeof(CHAR_T*));
			if ((retresult->a + i)->a == NULL) {
				errmsg = MSG_PALO_ERR_NO_MEM;
				result = PALO_ERR_NO_MEM;
				break;
			}

			vsize2 = cdt[i].line.size();
			line.clear();
			for (j = 0; j < vsize2; j++) {
				line.append(cdt[i].line[j]);
				line.append(sep);
			}

			(retresult->a + i)->len = 1;
			result = UTF8_TO_STR(convs, (retresult->a + i)->a, line.c_str());
			if (result != PALO_SUCCESS) {
				errmsg = MSG_ENCODING_FAILED;
				result = result;
				break;
			}
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		PALO_SUFFIX(free_arg_str_array_array_contents)(retresult);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(edelete_bulk)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const arg_str_array *dimension_elements) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	size_t i, vsize = dimension_elements->len;
	vector<string> elements(vsize);

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, dimension);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_dim(name);
	free(name);
	name = NULL;

	for (i = 0; i < vsize; i++) {
		result = STR_TO_UTF8(convs, &name, *(dimension_elements->a + i));
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
			break;
		}
		elements[i] = name ;
		free(name);
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	try {
		(*(so->myServer))[utf8_db].dimension[utf8_dim].bulkDeleteElements(elements);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(get_svs_info)(struct errstack *errs, struct arg_svs_info *retresult, struct sock_obj *so, struct conversions *convs) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	try {
		SUPERVISION_SERVER_INFO svs_info = (*(so->myServer)).getSVSInfo();
		retresult->svs_active = svs_info.svs_active;
		retresult->login_mode = svs_login_mode(svs_info.login_mode);
		retresult->cube_worker_active = svs_info.cube_worker_active;
		retresult->drill_through_enabled = svs_info.drill_through_enabled;
		retresult->windows_sso_enabled = svs_info.windows_sso_enabled;

	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(cell_goalseek_extended)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *path, double val, goalseek_mode mode, const struct arg_str_array_array *goalseek_area) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	size_t vsize, vsize2, i, j;

	char *name = NULL;
	std::vector<std::string> elems;
	std::vector<std::vector<std::string> > area;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;

	vsize = path->len;
	vector<string> elements(vsize);

	for (i = 0; i < vsize; i++) {
		result = STR_TO_UTF8(convs, &name, *(path->a + i));
		if (result != PALO_SUCCESS) {
			errmsg = MSG_ENCODING_FAILED;
			break;
		}
		elements[i] = name;
		free(name);
	}

	if (goalseek_area != NULL) {
		vsize = goalseek_area->len;
		area.resize(vsize);
	
		for (i = 0; i < vsize; i++) {
			vsize2 = (goalseek_area->a + i)->len;
			elems.resize(vsize2);
			for (j = 0; j < vsize2; j++) {
				if (*((goalseek_area->a + i)->a + j) != NULL) {
					result = STR_TO_UTF8(convs, &name, *((goalseek_area->a + i)->a + j));
					if (result != PALO_SUCCESS) {
						errmsg = MSG_ENCODING_FAILED;
						break;
					}
				} else  {
					name = (char*)calloc(1, sizeof(char));
					if (name == NULL) {
						result = PALO_ERR_NO_MEM;
						errmsg = MSG_PALO_ERR_NO_MEM;
						break;
					}
				}

				elems[j] = name;
				free(name);
			}
			if (result != PALO_SUCCESS) {
				break;
			}
			area[i] = elems;
		}
	}

	if (result == PALO_SUCCESS) {
		try {
			(*(so->myServer))[utf8_db].cube[utf8_cube].CellGoalSeek(elements, val, (jedox::palo::Cube::GOALSEEK_TYPE)mode, area);
		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			result = PALO_ERR_SYSTEM;
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
		}
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(cell_goalseek)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *path, double val) {
	return PALO_SUFFIX(cell_goalseek_extended)(errs, so, convs, database, cube, path, val, goalseek_complete, NULL); 
}

palo_err PALO_SUFFIX(dimension_list_elements2_raw)(struct errstack *errs, struct arg_dim_element_info2_raw_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;
	size_t elemcount = 0, pos = 0;

	retresult->a = NULL;
	retresult->len = 0;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, dimension);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_dim(name);
	free(name);
	name = NULL;

	try {
		const Dimension& dim = (*(so->myServer))[utf8_db].dimension[utf8_dim];
		std::unique_ptr<DimensionCache::CacheIterator> it = const_cast<Dimension&>(dim).getIterator();

		elemcount = dim.getCacheData().number_elements;

		if (elemcount > 0) {
			retresult->a = (struct arg_dim_element_info2_raw*)calloc(elemcount, sizeof(struct arg_dim_element_info2_raw));

			if (retresult->a != NULL) {
				retresult->len = elemcount;

				while (!(*it).end()) {
					pos = (**it).position;

					result = PALO_SUFFIX(set_element_info2_raw)(&errmsg, retresult->a + pos, convs, **it);
					if (result != PALO_SUCCESS) {
						break;
					}

					++*it;
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		}

	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		PALO_SUFFIX(free_arg_dim_element_info2_raw_array_contents)(retresult);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(ecreate_bulk_simple)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, de_type type, const struct arg_str_array *dim_elements, const struct arg_consolidation_element_info_array_array *consolidation_elements) {

	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;
	ELEMENT_INFO::TYPE ltype;
	size_t i, j, vsize=dim_elements->len, vsize2=consolidation_elements->len, vsize3=0;
	vector<string> elems(vsize);
	std::vector<std::vector<std::string> > children(vsize2);
	std::vector<double> factors;
	std::vector<std::vector<double> > weights(vsize2);

	result = types2number(type, &ltype);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, NULL);
	}

#ifdef ENCODING_M
	name = const_cast<char *>(database);
#else
	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}
#endif

	std::string utf8_db(name);

#ifndef ENCODING_M
	free(name);
#endif

	name = NULL;

#ifdef ENCODING_M
	name = const_cast<char *>(dimension);
#else
	result = STR_TO_UTF8(convs, &name, dimension);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}
#endif

	std::string utf8_dim(name);

#ifndef ENCODING_M
	free(name);
#endif

	name = NULL;

	for (i = 0; i < vsize2; i++) {
		vsize3 = (consolidation_elements->a + i)->len;
		elems.reserve(vsize3);
		factors.reserve(vsize3);
		for (j = 0; j < vsize3; j++) {
#ifdef ENCODING_M
			name = ((consolidation_elements->a + i)->a + j)->name;
#else
			result = STR_TO_UTF8(convs, &name, ((consolidation_elements->a + i)->a + j)->name);
			if (result != PALO_SUCCESS) {
				ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
			}
#endif
			elems.push_back(name);
#ifndef ENCODING_M
			free(name);
#endif
			factors.push_back(((consolidation_elements->a + i)->a + j)->factor);
		}
		children.push_back(elems);
		elems.clear();
		weights.push_back(factors);
		factors.clear();
	}

	for (i = 0; i < vsize; i++) {

#ifdef ENCODING_M
		name = *(dim_elements->a + j) ;
#else
		result = STR_TO_UTF8(convs, &name, *(dim_elements->a + j));
		if (result != PALO_SUCCESS) {
			ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
		}
#endif
		elems.push_back(name);

#ifndef ENCODING_M
			free(name);
#endif
	}

	try {
		(*(so->myServer))[utf8_db].dimension[utf8_dim].bulkCreateElements(elems, ltype, children, weights);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(cube_convert)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, cube_type newtype) {

	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

#ifdef ENCODING_M
	name = const_cast<char *>(database);
#else
	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}
#endif

	std::string utf8_db(name);

#ifndef ENCODING_M
	free(name);
#endif

	name = NULL;

#ifdef ENCODING_M
	name = const_cast<char *>(cube);
#else
	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}
#endif

	std::string utf8_cube(name);

#ifndef ENCODING_M
	free(name);
#endif

	name = NULL;

	try {
		(*(so->myServer))[utf8_db].cube[utf8_cube].convert(CUBE_INFO::TYPE(newtype));
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(dimension_restricted_list_elements)(struct errstack *errs, struct arg_dim_element_info2_raw_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, long id, long start, long limit) {
	if (*(so->version) < 302) {
		if (id == -2) {
			return PALO_SUFFIX(dimension_list_elements2_raw)(errs, retresult, so, convs, database, dimension);
		} else {
			ERRSTACK_RETURN(errs, PALO_ERR_NOT_IMPLEMENTED, __FUNCTION__, __LINE__, __FILE__, wcsdup(L"This server doesn't support this call since it is too old."));
		}
	}

	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;
	size_t elemcount = 0, pos = 0;

	retresult->a = NULL;
	retresult->len = 0;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, dimension);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_dim(name);
	free(name);
	name = NULL;

	try {
		const Dimension& dim = (*(so->myServer))[utf8_db].dimension[utf8_dim];
		std::list<ELEMENT_INFO> elems = dim.getElements(id, start, limit);
		size_t elemcount = elems.size();

		if (elemcount > 0) {
			retresult->a = (struct arg_dim_element_info2_raw*)calloc(elemcount, sizeof(struct arg_dim_element_info2_raw));

			if (retresult->a != NULL) {
				retresult->len = elemcount;
			
				std::list<ELEMENT_INFO>::iterator it = elems.begin();
				std::list<ELEMENT_INFO>::iterator endlist = elems.end();

				size_t i = 0;
				while ((i < elemcount) && (it != endlist)) {
					result = PALO_SUFFIX(set_element_info2_raw)(&errmsg, retresult->a + i, convs, *it);
					if (result != PALO_SUCCESS) {
						break;
					}
					i++;
					++it;
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		}

	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		PALO_SUFFIX(free_arg_dim_element_info2_raw_array_contents)(retresult);
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(dimension_restricted_flat_list_elements)(struct errstack *errs, struct arg_dim_element_info2_raw_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, long start, long limit) {
	return PALO_SUFFIX(dimension_restricted_list_elements)(errs, retresult, so, convs, database, dimension, -2, start, limit);
}

palo_err PALO_SUFFIX(dimension_restricted_top_list_elements)(struct errstack *errs, struct arg_dim_element_info2_raw_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, long start, long limit) {
	return PALO_SUFFIX(dimension_restricted_list_elements)(errs, retresult, so, convs, database, dimension, -1, start, limit);
}

palo_err PALO_SUFFIX(dimension_restricted_children_list_elements)(struct errstack *errs, struct arg_dim_element_info2_raw_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, long element_identifier, long start, long limit) {
	return PALO_SUFFIX(dimension_restricted_list_elements)(errs, retresult, so, convs, database, dimension, element_identifier, start, limit);
}

palo_err PALO_SUFFIX(change_password)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *oldpassword, const CHAR_T *newpassword) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	result = STR_TO_UTF8(convs, &name, oldpassword);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_oldpassword(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, newpassword);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_newpassword(name);
	free(name);
	name = NULL;

	try {
		(*(so->myServer)).changePassword(utf8_oldpassword, utf8_newpassword);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(api_version)(struct errstack *errs, struct api_info *retresult) {
	palo_err result = PALO_SUCCESS;
	VERSION_INFO apiinfo;
	api_version(apiinfo);

	retresult->major_version = apiinfo.major_version;
	retresult->minor_version = apiinfo.minor_version;
	retresult->bugfix_version = apiinfo.bugfix_version;
	retresult->build_number = apiinfo.build_number;

	return PALO_SUCCESS;

}

palo_err PALO_SUFFIX(dimension_flat_count)(struct errstack *errs, size_t *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;
	size_t elemcount = 0, pos = 0;
	*retresult = 0;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, dimension);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_dim(name);
	free(name);
	name = NULL;

	try {
		if (*(so->version) < 302) {
			*retresult = (*(so->myServer))[utf8_db].dimension[utf8_dim].getCacheData().number_elements;
		} else {
			*retresult =(*(so->myServer))[utf8_db].dimension[utf8_dim].getFlatCount();
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;

}

palo_err PALO_SUFFIX(set_client_version)(struct errstack *errs, struct conversions *convs, const CHAR_T *client_version) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	result = STR_TO_UTF8(convs, &name, client_version);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_client_version(name);
	free(name);
	name = NULL;

	try {
		JedoxXLHelper::getInstance().SetVersionInfo(utf8_client_version);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;

}


palo_err PALO_SUFFIX(set_user_password)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *user, const CHAR_T *password) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	result = STR_TO_UTF8(convs, &name, user);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_user(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, password);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_password(name);
	free(name);
	name = NULL;

	try {
		(*(so->myServer)).changeUserPassword(utf8_user, utf8_password);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;

}

palo_err PALO_SUFFIX(is_admin_user)(struct errstack *errs, unsigned short int *retresult, struct sock_obj *so, struct conversions *convs) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	*retresult = 0;

	try {
		GROUP_NAME_LIST groups;
		//make sure, sid is up to date
		try {
			groups = (*(so->myServer)).getUserInfo((*(so->myServer)).getSID()).ngroups;
		} catch (...) {
			groups = (*(so->myServer)).getUserInfo((*(so->myServer)).getSID()).ngroups;
		}

		*retresult = (find(groups.begin(), groups.end(), "admin") != groups.end()) ? 1 : 0;
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(is_gpu_server)(struct errstack *errs, unsigned short int *retresult, struct sock_obj *so, struct conversions *convs) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	*retresult = 0;

	try {
		*retresult = ((*(so->myServer)).getCacheData().bugfix_version == 5);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(rules_activate)(struct errstack *errs, struct arg_rule_info_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids, rule_activation_mode mode) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	size_t vsize, i;

	retresult->len = 0;
	retresult->a = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;

	ELEMENT_LIST rids(len);
	for (i = 0; i < len; i++) {
		rids[i] = *(ids+i);
	}

	try {
		vector<RULE_INFO> riv = (*(so->myServer))[utf8_db].cube[utf8_cube].RulesActivate(rids, mode);

		vsize = riv.size();

		if (vsize > 0) {

			retresult->a = (struct arg_rule_info*)calloc(vsize, sizeof(struct arg_rule_info));
			retresult->len = vsize;

			if (retresult->a != NULL) {
				for (i = 0; i < vsize; i++) {
					result = PALO_SUFFIX(set_rule_info)(&errmsg, retresult->a + i, convs, riv[i]);
					if (result != PALO_SUCCESS) {
						retresult->len = i;
						PALO_SUFFIX(free_arg_rule_info_array_contents)(retresult);
						break;
					}
				}
			} else {
				result = PALO_ERR_NO_MEM;
				errmsg = MSG_PALO_ERR_NO_MEM;
			}
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}

palo_err PALO_SUFFIX(rules_delete)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	result = STR_TO_UTF8(convs, &name, database);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_db(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, cube);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_cube(name);
	free(name);
	name = NULL;

	ELEMENT_LIST rids(len);
	for (size_t i = 0; i < len; i++) {
		rids[i] = *(ids+i);
	}

	try {
		(*(so->myServer))[utf8_db].cube[utf8_cube].RulesDestroy(rids);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NEWCODES(exp.code());
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		result = PALO_ERR_SYSTEM;
		errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(rules_move)(struct errstack *errs, struct arg_rule_info_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids, double start_position, double below_position) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;
	char *name = NULL;

	size_t vsize, i;

	retresult->len = 0;
	retresult->a = NULL;

	if (len > 0) {

		result = STR_TO_UTF8(convs, &name, database);
		if (result != PALO_SUCCESS) {
			ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
		}

		std::string utf8_db(name);
		free(name);
		name = NULL;

		result = STR_TO_UTF8(convs, &name, cube);
		if (result != PALO_SUCCESS) {
			ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
		}

		std::string utf8_cube(name);
		free(name);
		name = NULL;

		ELEMENT_LIST rids(len);
		for (i = 0; i < len; i++) {
			rids[i] = *(ids+i);
		}

		try {
			vector<RULE_INFO> riv = (*(so->myServer))[utf8_db].cube[utf8_cube].RulesMove(rids, start_position, below_position);
			vsize = riv.size();

			if (vsize > 0) {

				retresult->a = (struct arg_rule_info*)calloc(vsize, sizeof(struct arg_rule_info));
				retresult->len = vsize;

				if (retresult->a != NULL) {
					for (i = 0; i < vsize; i++) {
						result = PALO_SUFFIX(set_rule_info)(&errmsg, retresult->a + i, convs, riv[i]);
						if (result != PALO_SUCCESS) {
							retresult->len = i;
							PALO_SUFFIX(free_arg_rule_info_array_contents)(retresult);
							break;
						}
					}
				} else {
					result = PALO_ERR_NO_MEM;
					errmsg = MSG_PALO_ERR_NO_MEM;
				}
			}

		} catch (const SocketException& exp) {
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NETWORK;
		} catch (const PaloServerException& exp) {
			if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
			result = PALO_ERR_NEWCODES(exp.code());
		} catch (const std::exception& exp) {
			result = PALO_ERR_SYSTEM;
			if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
				errmsg = NULL;
			}
#ifndef _DEBUG
		} catch (...) {
			result = PALO_ERR_SYSTEM;
			errmsg = MSG_NO_STANDARD_EXCEPTION_OCCURED;
#endif
		}

		if (result != PALO_SUCCESS) {
			ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
		}
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(test_connection)(struct errstack *errs, struct conversions *convs, const CHAR_T *hostname, unsigned int service, const CHAR_T *username, const CHAR_T *pw_hash, const CHAR_T *trustfile) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	char *name = NULL;

	result = PALO_SUFFIX(init_ssl)(errs, convs, trustfile);

	result = STR_TO_UTF8(convs, &name, hostname);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_host(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, username);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_user(name);
	free(name);
	name = NULL;

	result = STR_TO_UTF8(convs, &name, pw_hash);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	std::string utf8_pass(name);
	free(name);
	name = NULL;

	try {
		JedoxXLHelper::getInstance().TestConnection(utf8_host, service, utf8_user, utf8_pass);
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		result = PALO_ERR_NEWCODES(exp.code());
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		errmsg = MSG_CONNECT_FAILED;
		result = PALO_ERR_SYSTEM;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}


palo_err PALO_SUFFIX(get_rights)(struct errstack *errs, permission_type *retresult, struct sock_obj *so, struct conversions *convs, permission_art pa) {
	palo_err result = PALO_SUCCESS;
	wchar_t *errmsg = NULL;

	*retresult = permission_type_unknown;

	try {
		USER_INFO ui = (*(so->myServer)).getUserInfo();

		std::map<std::string, char>::iterator it = ui.permissions.find(getPermissionArtString(pa));

		if (it != ui.permissions.end()) {

			switch (toupper(it->second)) {
				case 'N':
					*retresult = permission_type_none;
					break;

				case 'R':
					*retresult = permission_type_read;
					break;

				case 'W':
					*retresult = permission_type_write;
					break;

				case 'D':
					*retresult = permission_type_delete;
					break;

				case 'S':
					*retresult = permission_type_splash;
					break;

				default:
					*retresult = permission_type_unknown;
					break;
			}
		} else {
			*retresult = permission_type_splash;
		}
	} catch (const SocketException& exp) {
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
		result = PALO_ERR_NETWORK;
	} catch (const PaloServerException& exp) {
		result = PALO_ERR_NEWCODES(exp.code());
		if (utf82wcs(convs, &errmsg, exp.Description().c_str()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
	} catch (const std::exception& exp) {
		result = PALO_ERR_SYSTEM;
		if (utf82wcs(convs, &errmsg, exp.what()) != PALO_SUCCESS) {
			errmsg = NULL;
		}
#ifndef _DEBUG
	} catch (...) {
		errmsg = MSG_CONNECT_FAILED;
		result = PALO_ERR_SYSTEM;
#endif
	}

	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, errmsg);
	}

	return PALO_SUCCESS;
}
