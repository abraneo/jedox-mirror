/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *
 * \author Florian Schaper <florian.schaper@jedox.com>
 * \author Frieder Hofmann <frieder.hofmann@jedox.com>
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * \author Jiri Junek <jiri.junek@qbicon.cz>
 * 
 *
 */

#ifndef LIBPALO_NG_TYPES_H
#define LIBPALO_NG_TYPES_H

#define ERROR_OUT_OF_MEMORY_MSG "memory allocation failed"

#include <iosfwd>
#include <list>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <map>

#include <boost/functional/hash.hpp>
#include <stdlib.h>

#include <libpalo_ng/config_ng.h>

#undef ERROR

namespace jedox {
namespace util {
class UTF8Comparer;
}
namespace palo {

typedef UINT IdentifierType;

/*!
 * \brief
 *  All Strings are UTF8.
 */

enum SPLASH_MODE {
	MODE_SPLASH_UNKNOWN = -1, MODE_SPLASH_NONE = 0, MODE_SPLASH_DEFAULT, MODE_SPLASH_ADD, MODE_SPLASH_SET
};

/*!
 * \brief
 *  Login modes used by the Supervision Server.
 *
 * \author Frieder Hofmann <frieder.hofmann@jedox.com>
 */

enum SVS_LOGIN_MODE {
	NONE, INFORMATION, AUTHENTIFICATION, AUTHORIZATION
};

enum PERMISSION {
	NONE_PERM, READ_PERM, WRITE_PERM, DELETE_PERM, SPLASH_PERM
};

PERMISSION stringToPermission(std::string &s);

/*!
 * \brief
 * Provides informations about a Supervision Server.
 *
 * \author Frieder Hofmann <frieder.hofmann@jedox.com>
 */
struct SUPERVISION_SERVER_INFO {
	bool svs_active;
	SVS_LOGIN_MODE login_mode;
	bool cube_worker_active;
	bool drill_through_enabled;
	bool dimension_worker_enabled;
	bool windows_sso_enabled;

	friend std::istream& operator>>(std::istream& in, SUPERVISION_SERVER_INFO& serverInfo);
	friend std::ostream& operator<<(std::ostream& out, const SUPERVISION_SERVER_INFO& svsInfo);
};


struct LICENSE_INFO {
	std::string key;
	std::string customer;
	int version;
	int license_count;
	int named_count;
	unsigned long long start;
	unsigned long long expiration;
	int sharing_limit;
	int gpu_count;
	std::string features;

	friend std::istream& operator>>(std::istream& in, LICENSE_INFO &licenseInfo);
};

struct LICENSE_LIST {
	std::string hw_key;
	std::vector<LICENSE_INFO> licenses;

	friend std::istream& operator>>(std::istream& in, LICENSE_LIST &licenseList);
};

/*!
 * \brief
 * List of dimension ID's.
 */
typedef std::vector<IdentifierType> DIMENSION_LIST;

/*!
 * \brief
 * List of element ID's.
 */
typedef std::vector<IdentifierType> ELEMENT_LIST;

typedef std::vector<IdentifierType> GROUP_LIST;
typedef std::vector<std::string> GROUP_NAME_LIST;

// std::ostream& xml( std::ostream& os ); // Not yet implemented, maybe never
std::ostream& csv(std::ostream& os);

// std::istream& xml( std::istream& is ); // Not yet implemented, maybe never
std::istream& csv(std::istream& is);

/*!
 * \brief
 * List of element weights.
 */
typedef std::vector<double> ELEMENT_WEIGHT_LIST;

static const UINT invalid_server_info_data_sequence_number = static_cast<UINT>(-1);

struct SERVER_INFO {
	UINT major_version;
	UINT minor_version;
	UINT bugfix_version;
	UINT build_number;
	UINT encryption;
	UINT httpsPort;
    /*!
     * \brief
     * If the server supports the data_sequence_number, this will increment on every write on the server.
     * If the server doesn't support it, it will be set to invalid_server_info_data_sequence_number. 
     */
    UINT data_sequence_number;

	friend std::istream& operator>>(std::istream& in, SERVER_INFO& serverInfo);
	friend std::ostream& operator<<(std::ostream& out, const SERVER_INFO& serverInfo);
};

/*!
 * \brief
 * Provides informations about an database.
 *
 * Provides the id of an database, it's name and it's number
 * of dimensions and cubes.
 *
 * \see
 * UNSERIALIZEABLE
 * \author Florian Schaper <florian.schaper@jedox.com>
 */
struct DATABASE_INFO {
	IdentifierType database;
	std::string ndatabase;
	UINT number_dimensions;
	UINT number_cubes;
	enum STATUS {
		UNLOADED = 0, LOADED, CHANGED, LOADING
	} status;
	enum TYPE {
		NORMAL = 0, SYSTEM, USERINFO = 3
	} type;

	friend std::istream& operator>>(std::istream& in, DATABASE_INFO& databaseInfo);
	friend std::ostream& operator<<(std::ostream& out, const DATABASE_INFO& databaseInfo);
};

struct DATABASE_INFO_PERMISSIONS : DATABASE_INFO {
	PERMISSION permission;

	friend std::istream& operator>>(std::istream& in, DATABASE_INFO_PERMISSIONS& databaseInfo);
};

struct DATABASE_INFO_PERMISSIONS_LIST {
	std::vector<DATABASE_INFO_PERMISSIONS> databases;

	friend std::istream& operator>>(std::istream& in, DATABASE_INFO_PERMISSIONS_LIST &dbList);
};

/*!
 * \brief
 * Provides information about an dimension.
 *
 * Provides information about an dimension:
 * - the id of the dimension
 * - the name of the dimension
 * - the number of elements the dimension contains
 * - the maximum level of the dimension
 * - the maximum indent of the dimension
 * - the maximum depth of the dimension
 *
 * \see
 * UNSERIALIZEABLE | ELEMENT_INFO | CUBE_INFO
 * \author Florian Schaper <florian.schaper@jedox.com>
 */
struct DIMENSION_INFO {
	IdentifierType dimension;
	std::string ndimension;
	UINT number_elements;
	UINT maximum_level;
	UINT maximum_indent;
	UINT maximum_depth;
	enum TYPE {
		NORMAL = 0, SYSTEM, ATTRIBUTE, USERINFO, SYSTEM_ID
	} type;
	IdentifierType assoc_dimension;
	IdentifierType attribute_cube;
	IdentifierType rights_cube;

	friend std::istream& operator>>(std::istream& in, DIMENSION_INFO& dimensionInfo);
	friend std::ostream& operator<<(std::ostream& out, const DIMENSION_INFO& dimensionInfo);
};

struct DIMENSION_INFO_PERMISSIONS : DIMENSION_INFO {
	PERMISSION permission;

	friend std::istream& operator>>(std::istream& in, DIMENSION_INFO_PERMISSIONS& dimensionInfo);
};

struct DIMENSION_INFO_PERMISSIONS_LIST {
	std::vector<DIMENSION_INFO_PERMISSIONS> dimensions;

	friend std::istream& operator>>(std::istream& in, DIMENSION_INFO_PERMISSIONS_LIST &dimList);
};

/*!
 * \brief
 * Provides informations about an element.
 *
 * Provides informations about:
 * - the id of the element
 * - the name of the element
 * - the position of the element
 * - the level of the element
 * - the indent of the element
 * - the depth of the element
 * - the elements type
 * - the number of parent elements
 * - a list of the elements parents
 * - the number of child elements
 * - a list of the elements child elements
 * - a list representing the weight of each child element
 *
 * \see
 * UNSERIALIZEABLE | DIMENSION_INFO
 * \author Florian Schaper <florian.schaper@jedox.com>
 */
struct ELEMENT_INFO {
	IdentifierType element;
	std::string nelement;
	IdentifierType position;
	IdentifierType level;
	UINT indent;
	UINT depth;
	enum TYPE {
		UNKNOWN = 0, NUMERIC = 1, STRING = 2, CONSOLIDATED = 4
	} type;
	UINT number_parents;
	ELEMENT_LIST parents;
	UINT number_children;
	ELEMENT_LIST children;
	ELEMENT_WEIGHT_LIST weights;

	friend std::istream& operator>>(std::istream& in, ELEMENT_INFO& elementInfo);
	friend std::ostream& operator<<(std::ostream& out, const ELEMENT_INFO& elementInfo);
};

struct ELEMENT_INFO_PERM : public ELEMENT_INFO {
	std::string permission;

	friend std::istream& operator>>(std::istream& in, ELEMENT_INFO_PERM& elementInfo);
	friend std::ostream& operator<<(std::ostream& out, const ELEMENT_INFO_PERM& elementInfo);
};

/*!
 * \brief
 * Provides information about an cube.
 *
 * Provides the cubes ID, it's name, the number of dimensions
 * the cube consists of as well as a list with those ID's.
 *
 * \see
 * UNSERIALIZEABLE
 * \author Florian Schaper <florian.schaper@jedox.com>
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 */
struct CUBE_INFO {
	IdentifierType cube;
	std::string ncube;
	UINT number_dimensions;
	DIMENSION_LIST dimensions;
	long double number_cells;
	long double number_filled_cells;
	enum STATUS {
		UNLOADED = 0, LOADED, CHANGED
	} status;
	enum TYPE {
		NORMAL = 0, SYSTEM, ATTRIBUTE, USERINFO, GPU
	} type;

	friend std::istream& operator>>(std::istream& in, CUBE_INFO& cubeInfo);
	friend std::ostream& operator<<(std::ostream& out, const CUBE_INFO& cubeInfo);
};

struct CUBE_INFO_PERMISSIONS : CUBE_INFO {
	PERMISSION permission;
	friend std::istream& operator>>(std::istream& in, CUBE_INFO_PERMISSIONS& cubeInfo);
};

struct CUBE_INFO_PERMISSIONS_LIST {
	std::vector<CUBE_INFO_PERMISSIONS> cubes;
	friend std::istream& operator>>(std::istream& in, CUBE_INFO_PERMISSIONS_LIST &dbList);
};

enum LOCK_STATUS {
	Unlocked = 0, LockedByMe = 1, LockedByOther = 2
};

/*!
 * \brief
 * Provides information about the value of a cell.
 *
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 */
struct CELL_VALUE {
	enum CELL_VALUE_TYPE {
		NUMERIC = 1, STRING = 2, ERROR = 99
	} type;
	bool exists;
	struct value {
		double d;
		unsigned int errorcode;
		std::string s;
	} val;
	IdentifierType ruleID;
	LOCK_STATUS lock_status;

	CELL_VALUE() :
		ruleID(~1), lock_status(Unlocked)
	{
	}

	friend std::istream& operator>>(std::istream& in, CELL_VALUE& CellValue);
};

/*!
 * \brief
 * Provides information about the value of a cell.
 *
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 */
struct CELL_VALUE_PATH : CELL_VALUE {
	ELEMENT_LIST path;

	friend std::istream& operator>>(std::istream& in, CELL_VALUE_PATH& CellValuePath);
};

struct CELL_VALUE_PATH_EXPORTED {
	enum CELL_VALUE_TYPE {
		NUMERIC = 1, STRING = 2, ERROR = 99
	} type;
	bool exists;
	struct value {
		double d;
		unsigned int errorcode;
		std::string s;
	} val;

	ELEMENT_LIST path;

};

/*!
 * \brief
 * Provides information about the value of a cell.
 *
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 */
struct CELL_VALUE_EXPORTED {
	enum TYPE {
		CELLVALUE, EXPORTINFO
	} type;
	CELL_VALUE_PATH_EXPORTED cvp;
	struct EXPORT_INFO {
		long double usedcells;
		long double allcells;
	} exportinfo;

	friend std::istream& operator>>(std::istream& in, CELL_VALUE_EXPORTED& cellValueExp);
};

struct CELL_VALUE_PATH_PROPS : CELL_VALUE_PATH {
	std::vector<std::string> prop_vals;

	friend std::istream& operator>>(std::istream& in, CELL_VALUE_PATH_PROPS& CellValuePathProps);
};

struct CELL_VALUE_PROPS : CELL_VALUE {
	std::vector<std::string> prop_vals;
	CELL_VALUE_PROPS &operator=(const CELL_VALUE_PATH_PROPS &val) {*(CELL_VALUE *)this = val; prop_vals = val.prop_vals; return *this;}

	friend std::istream& operator>>(std::istream& in, CELL_VALUE_PROPS& CellValueProps);
};

struct CELL_VALUE_EXPORTED_PROPS : CELL_VALUE_EXPORTED {
	std::vector<std::string> prop_vals;

	friend std::istream& operator>>(std::istream& in, CELL_VALUE_EXPORTED_PROPS& cellValueExpProps);
};

/*!
 * \brief
 * Provides information about the needed session id.
 *
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 */
struct LOGIN_DATA {
	std::string sid;
	unsigned int ttl;
	std::string optFeatures;

	friend std::istream& operator>>(std::istream& in, LOGIN_DATA& logindata);
};

struct RULE_INFO {
	IdentifierType identifier;
	std::string definition;
	std::string extern_id;
	std::string comment;
	unsigned long long timestamp;
	bool activated;
	double position;

	friend std::istream& operator>>(std::istream& in, RULE_INFO& rule_info);
};

struct LOCK_INFO {
	IdentifierType lockid;
	std::vector<std::vector<long> > area;
	std::string user;
	unsigned long steps;

	friend std::istream& operator>>(std::istream& in, LOCK_INFO& lock_info);
};

struct DRILLTHROUGH_INFO {
	std::vector<std::string> line;

	friend std::istream& operator>>(std::istream& in, DRILLTHROUGH_INFO& drillthrough_info);
};

// For internal use
// May vanish in the future
class C_ARRAY_LONG {
public:
	size_t m_len;
	long* m_a;

	C_ARRAY_LONG() :
		m_len(0), m_a(NULL)
	{
	}

	void init(size_t len)
	{
		m_delete();
		m_a = (long *)calloc(len, sizeof(long));
		m_len = (m_a == NULL) ? 0 : len;
	}

	~C_ARRAY_LONG()
	{
		m_delete();
	}

	std::string Liste(char separator, bool with_asterix)
	{
		std::stringstream idstr;

		idstr.setf(std::ios_base::fixed, std::ios_base::floatfield);
		idstr.precision(PRECISION);

		if (with_asterix && (m_len == 0)) {
			idstr << '*' << separator;
		} else {
			for (size_t i = 0; i < m_len; i++) {
				idstr << *(m_a + i) << separator;
			}
		}

		std::string str = idstr.str();

		return str.substr(0, str.size() - 1);
	}

private:

	void m_delete()
	{
		if (m_a != NULL) {
			free(m_a);
			m_a = NULL;
		}
		m_len = 0;
	}
};

// For internal use
// May vanish in the future
class C_ARRAY_ARRAY_LONG {
public:
	size_t m_len;
	C_ARRAY_LONG* m_a;

	C_ARRAY_ARRAY_LONG(size_t len)
	{
		m_a = (C_ARRAY_LONG *)calloc(len, sizeof(C_ARRAY_LONG));
		m_len = (m_a == NULL) ? 0 : len;
	}

	~C_ARRAY_ARRAY_LONG()
	{
		if (m_a != NULL) {
			free(m_a);
			m_a = NULL;
		}
		m_len = 0;
	}

	std::string ListeListe(char separator1, char separator2, bool with_asterix)
	{

		std::stringstream idstr;

		for (size_t i = 0; i < m_len; i++) {
			idstr << ((m_a + i)->Liste(separator1, with_asterix)) << separator2;
		}

		std::string str = idstr.str();

		return str.substr(0, str.size() - 1);
	}

};

// For internal use
// May vanish in the future
class C_2DARRAY_LONG {
public:
	size_t m_rows;
	size_t m_cols;
	long* m_a;

	C_2DARRAY_LONG(size_t rows, size_t cols);

	~C_2DARRAY_LONG()
	{
		free(m_a);
	}

	std::string ListeListe(char separator1, char separator2, bool with_asterix)
	{
		std::stringstream idstr;

		for (size_t i = 0; i < m_rows; i++) {
			idstr << Liste(i, separator1, with_asterix) << separator2;
		}

		std::string str = idstr.str();

		return str.substr(0, str.size() - 1);
	}

private:
	std::string Liste(size_t row, char separator, bool with_asterix)
	{
		std::stringstream idstr;

		idstr.setf(std::ios_base::fixed, std::ios_base::floatfield);
		idstr.precision(PRECISION);

		if (with_asterix && (m_cols == 0)) {
			idstr << '*' << separator;
		} else {
			size_t j = m_cols * row;
			for (size_t i = 0; i < m_cols; i++) {
				idstr << *(m_a + j + i) << separator;
			}
		}

		std::string str = idstr.str();

		return str.substr(0, str.size() - 1);
	}
	;

};

// faster methods for C

struct Cell_Values_Coordinates {
	size_t rows;
	size_t cols;
	char **a;
};

struct Cell_Value_C {
	enum TYPE {
		NUMERIC = 1, STRING = 2, ERROR = 99
	} type;
	union Cell_Value_VALUE {
		char *s;
		double d;
		unsigned int errorcode;
	} val;

};

struct Cell_Values_C {
	size_t len;
	Cell_Value_C *a;
};

std::istream& operator>>(std::istream& in, Cell_Value_C& cellValueC);
void LIBPALO_NG_EXPORT FreeCell_Values_C_Content(Cell_Values_C& cvc);

// Begin Subsets

//this should be updated accordingly
#define ALIAS_FILTER_NUMFLAGS 5

struct LIBPALO_NG_CLASS_EXPORT AliasFilterBase {
	enum AliasFilterFlag {
		// search one attribute for an alias -- DEFAULT show this field in excel
		SEARCH_ONE = 0x1,
		// search two attributes for an alias -- show this field in excel
		SEARCH_TWO = 0x2,
		// hide double aliases(?) aliases don't have to be unique !! (?)  (Makes not really sense)
		HIDE_DOUBLE = 0x4,
		//Choose an attribute-dimension that serves as alias pool and show the aliases instead
		DISPLAY_ALIAS = 0x8,
		//(the client is responsible for showing the field display_alias -- the flag fills the field)
		// use advanced filter expressions for attribute-values (AFILTER in Excel)
		USE_FILTEREXP = 0x10
	};
};

//this should be updated accordingly
#define DATA_FILTER_NUMFLAGS 14

struct LIBPALO_NG_CLASS_EXPORT DataFilterBase {
	enum DataFilterFlag {
		// use min operator on cell values
		DATA_MIN = 0x1,
		// use max operator on cell values
		DATA_MAX = 0x2,
		// use sum operator on cell values
		DATA_SUM = 0x4,
		// use average operator on cell values
		DATA_AVERAGE = 0x8,
		//conditions must be true for at least one value
		DATA_ANY = 0x10,
		//conditions must be true for all values
		DATA_ALL = 0x20,
		//TODO interpret data as strings, only usable if there is no iteration over cell values (fixed coordinates, no *)
		//the elements could then be sorted according to these strings or flags like TOP could be applied
		DATA_STRING = 0x40,
		// compute data only for consolidations (don't filter leaves)
		ONLY_CONSOLIDATED = 0x80,
		// compute data only for leaves (don't filter consolidations)
		ONLY_LEAVES = 0x100,
		// sort elements from highest to lowest and choose those
		// that contribute the first p1% (set percentage method)
		UPPER_PERCENTAGE = 0x200,
		// sort elements from lowest to highest and choose those
		// that contribute the first p1% (set percentage method)
		LOWER_PERCENTAGE = 0x400,
		// sort elements from highest to lowest and choose those
		// that contribute p2% after removing the first elements
		// that make up p1%
		MID_PERCENTAGE = 0x800,
		//pick only the top-x elements. x set by set_top
		TOP = 0x1000,
		//Do not use rules when extracting cell values
		NORULES = 0x2000

	};
};

//this should be updated accordingly
#define PICKLIST_NUM_FLAGS 4

struct LIBPALO_NG_CLASS_EXPORT PickListBase {
	enum PickListFlag {
		// Put manually picked elements after the others. Default: before
		INSERT_BACK = 0x1,
		// Put manually picked elements before the others
		MERGE = 0x2,
		SUB = 0x4, // Use the picklist as a filter, not as a union.
		// Default value, put manually picked elements before the others
		INSERT_FRONT = 0x8
	};
};

//this should be updated accordingly
#define SORTING_FILTER_NUM_FLAGS 19

struct LIBPALO_NG_CLASS_EXPORT SortingFilterBase {
	enum SortingFilterFlags {
		//sort filtered elements according to name (default, not necessary to pass)
		TEXT = 0x1,
		//sort filtered elements according to value computed by data-filter !! MIGHT BE STRING DATA !!
		NUMERIC = 0x2,
		//sort according to an attribute (to be set separately)
		USE_ATTRIBUTE = 0x4,
		//sort according to aliases as determined by alias filter
		USE_ALIAS = 0x8,
		//show parents below their children
		REVERSE_ORDER = 0x10,
		//do not sort consolidated elements
		LEAVES_ONLY = 0x20,
		//sort in order highest to lowest. (default, not necessary to pass)
		STANDARD_ORDER = 0x40,
		//show whole hierarchy
		WHOLE = 0x80,
		//position --default
		POSITION = 0x100,
		//reverse the sorting
		REVERSE_TOTAL = 0x200,
		//sort on the level of level-element
		SORT_ONE_LEVEL = 0x400,
		//do NOT build a tree -- default
		FLAT_HIERARCHY = 0x800,
		// build a tree, but do not follow the children-list of an element that has been filtered out before
		NO_CHILDREN = 0x1000,
		//only sort consolidations
		CONSOLIDATED_ONLY = 0x2000,
		// sort all levels except one
		SORT_NOT_ONE_LEVEL = 0x4000,
		//Show duplicates, default value is 0 (flag inactive - duplicates are hidden)
		SHOW_DUPLICATES = 0x8000,
		//this will completely reverse the ordering
		REVERSE_TOTAL_EX = 0x10000,
		//Limit number of elements returned
		LIMIT = 0x20000,
		//Sort by consolidation order
		CONSOLIDATION_ORDER = 0x40000,
		//Return also path
		PATH = 0x80000
	};
};

//this should be updated accordingly
#define STRUCTURAL_FILTER_NUMFLAGS 12

struct LIBPALO_NG_CLASS_EXPORT StructuralFilterBase {
	enum StructuralFilterFlag {
		// choose elements below including element passed to set_bound
		BELOW_INCLUSIVE = 0x1,
		// choose elements below excluding element passed to set_bound
		BELOW_EXCLUSIVE = 0x2,
		// Remove all consolidated elements from set, show only leaves
		HIDE_CONSOLIDATED = 0x4,
		// Remove all non-consolidated elements, show aggregations only
		HIDE_LEAVES = 0x8,
		// pick elements from top to bottom (levels to be specified separately)
		HIERARCHIAL_LEVEL = 0x10,
		// pick elements from bottom to top (levels to be specified separately)
		AGGREGATED_LEVEL = 0x20,
		// revolve (repeat) the list, choose all elements on the same level
		REVOLVING = 0x40,
		// add all elements with the same or a higher level than elemname and repeat the list
		REVOLVE_ADD_ABOVE = 0x80,
		// add all elements with the same or a lower level than elemname and repeat the list
		REVOLVE_ADD_BELOW = 0x100,
		// choose elements above excluding element passed to set_bound
		ABOVE_EXCLUSIVE = 0x200,
		// choose elements above including element passed to set_bound
		ABOVE_INCLUSIVE = 0x400,
		//simply repeat the list without further filtering
		CYCLIC = 0x800
	};
};

//this should be updated accordingly
#define TEXT_FILTER_NUM_FLAGS 2

struct LIBPALO_NG_CLASS_EXPORT TextFilterBase {

	enum TextFilterFlag {
		// use additional columns for searching uses the attribute-columns of this dimension per default
		ADDITIONAL_FIELDS = 0x1,
		// do not use POSIX-Basic expressions but use Perl-Extended regular expressions
		EXTENDED = 0x2
	};

};

struct FilterSettings {
	FilterSettings() :
		active(false), flags(0), indent(0)
	{
	}

	bool active;
	unsigned int flags;
	unsigned int indent;
};

struct AliasFilterSettings : public FilterSettings {
	std::string attribute1;
	std::string attribute2;
};

struct FieldFilterSettings : public FilterSettings {
	std::vector<std::vector<std::string> > advanced;
};

struct BasicFilterSettings : public FilterSettings {
	bool manual_subset_set;
	std::vector<std::string> manual_subset;
	std::vector<std::string> manual_paths;
};

struct DataFilterSettings : public FilterSettings {
	std::string cube;

	struct Comparison {
		Comparison() : use_strings(false), par1d(0), par2d(0) {}
		bool use_strings;

		std::string op1;
		double par1d;
		std::string par1s;

		std::string op2;
		double par2d;
		std::string par2s;
	} cmp;

	bool coords_set;

	// pair: (is_all, array)
	typedef std::vector<std::pair<bool, std::vector<std::string> > > CoordsType;
	CoordsType coords;

	bool upper_percentage_set, lower_percentage_set;
	double upper_percentage, lower_percentage;
	int top;

	DataFilterSettings() : FilterSettings(), coords_set(true), upper_percentage_set(false), lower_percentage_set(false), upper_percentage(0), lower_percentage(0), top(0) {}
};

struct SortingFilterSettings : public FilterSettings {
	SortingFilterSettings() : FilterSettings(), level(0), limit_count(0), limit_start(0) {}
	std::string attribute;
	IdentifierType level;
	UINT limit_count;
	UINT limit_start;
};

struct StructuralFilterSettings : public FilterSettings {
	StructuralFilterSettings() : FilterSettings(), level(false), level_start(0), level_end(0), revolve(false), revolve_count(0) {}
	std::string bound;
	bool level;
	IdentifierType level_start, level_end;
	bool revolve;
	std::string revolve_elem;
	int revolve_count;
};

struct TextFilterSettings : public FilterSettings {
	std::vector<std::string> regexps;
};

struct ELEMENT_INFO_EXT {
	std::string search_alias;
	ELEMENT_INFO m_einf;
	std::string path;
	std::vector<size_t> parentsInSub;
	std::vector<size_t> childrenInSub;

	ELEMENT_INFO_EXT(const ELEMENT_INFO& einf) :
		m_einf(einf)
	{
	}

	ELEMENT_INFO_EXT()
	{
	}

	inline const std::string& get_name() const
	{
		return m_einf.nelement;
	}

	inline const std::string& get_alias() const
	{
		return search_alias;
	}

	inline IdentifierType get_idx(const unsigned int indent) const
	{
		return ((indent == 1) ? m_einf.indent : ((indent == 2) ? m_einf.level : m_einf.depth));
	}
};

typedef std::vector<ELEMENT_INFO_EXT> ElementExList;
// End Subsets

struct USER_INFO {
	IdentifierType user;
	std::string nuser;
	GROUP_LIST groups;
	GROUP_NAME_LIST ngroups;
	unsigned int ttl;
	std::map<std::string, char> permissions;
	std::string license_key;

	friend std::istream& operator>>(std::istream& in, USER_INFO& user_info);
};

struct AXIS_SUBSET_DEF {
	AXIS_SUBSET_DEF(const std::string &sh, const std::string &par, const std::string &calc, bool zs) : subsetHandle(sh), parent(par), calculation(calc), zeroSup(zs) {}
	std::string subsetHandle;
	std::string parent;
	std::string calculation;
	bool zeroSup;
};

typedef std::vector<AXIS_SUBSET_DEF> AxisSubsets;

struct AxisElement {
	static const unsigned int MEMBERS_COUNT = 7;
	enum MEMBERS {
		NAME = 0x01,
		TYPE = 0x02,
		ALIAS = 0x04,
		HASCHILDREN = 0x08,
		HASPARENTS = 0x10,
		AXIS = 0x20,
		INDENT = 0x40,
		LINE = 0x80
	};
	AxisElement(IdentifierType axisId) : type(ELEMENT_INFO::UNKNOWN), hasChildrenInSubset(false), hasParentsInSubset(false), axisId(axisId), idx(0) {}
	AxisElement(const std::string &name, ELEMENT_INFO::TYPE type, const std::string &alias, bool hasChildrenInSubset, bool hasParentsInSubset, IdentifierType axisId, IdentifierType idx, size_t line) : name(name), type(type), alias(alias), hasChildrenInSubset(hasChildrenInSubset), hasParentsInSubset(hasParentsInSubset), axisId(axisId), idx(idx), line(line) {}
	std::string name;
	ELEMENT_INFO::TYPE type;
	std::string alias;
	bool hasChildrenInSubset;
	bool hasParentsInSubset;
	IdentifierType axisId;
	IdentifierType idx;
	size_t line;
};

} /* namespace palo */
} /* namespace jedox */

#endif							 // LIBPALO_NG_TYPES_H
