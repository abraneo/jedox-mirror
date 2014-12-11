/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as published
 * by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * If you are developing and distributing open source applications under the
 * GPL License, then you are free to use Palo under the GPL License.  For OEMs,
 * ISVs, and VARs who distribute Palo with their products, and do not license
 * and distribute their source code under the GPL, Jedox provides a flexible
 * OEM Commercial License.
 *
 * \author Frank Celler, triagens GmbH, Cologne, Germany
 * \author Achim Brandt, triagens GmbH, Cologne, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef PALO_DISPATCHER_PALO_JOB_REQUEST_H
#define PALO_DISPATCHER_PALO_JOB_REQUEST_H 1

#include "palo.h"

#include "HttpServer/HttpJobRequest.h"
#include "Olap/PaloSession.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief view definitions
////////////////////////////////////////////////////////////////////////////////

//this should be updated accordingly
#define ALIAS_FILTER_NUMFLAGS 5

struct AliasFilterBase {
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

struct DataFilterBase {
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
#define PICKLIST_NUM_FLAGS 5

struct PickListBase {
	enum PickListFlag {
		// Put manually picked elements after the others. Default: before
		INSERT_BACK = 0x1,
		// Put manually picked elements before the others
		MERGE = 0x2,
		SUB = 0x4, // Use the picklist as a filter, not as a union.
		// Default value, put manually picked elements before the others
		INSERT_FRONT = 0x8,
		DFILTER = 0x10
	};
};

//this should be updated accordingly
#define SORTING_FILTER_NUM_FLAGS 21

struct SortingFilterBase {
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
		PATH = 0x80000,
		NOSORT = 0x100000,
	};
};

//this should be updated accordingly
#define STRUCTURAL_FILTER_NUMFLAGS 12

struct StructuralFilterBase {
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

struct TextFilterBase {

	enum TextFilterFlag {
		// use additional columns for searching uses the attribute-columns of this dimension per default
		ADDITIONAL_FIELDS = 0x1,
		// do not use POSIX-Basic expressions but use Perl-Extended regular expressions
		EXTENDED = 0x2
	};

};

struct FilterSettings {
	FilterSettings() : active(false), flags(0), indent(0) {}
	enum FilterType {
		PICKLIST = 0, STRUCTURAL, ALIAS, TEXT, DATA, SORTING, TOTAL_NUMBER
	};
	bool active;
	unsigned int flags;
	unsigned int indent;
};

struct AliasFilterSettings : public FilterSettings {
	string attribute1;
	string attribute2;
};

struct FieldFilterSettings : public FilterSettings {
	vector<vector<string> > advanced;
};

struct BasicFilterSettings : public FilterSettings {
	vector<vector<string> > manual_subset;
};

struct DataFilterSettings : public FilterSettings {
	DataFilterSettings() : FilterSettings(), upper_percentage_set(false), lower_percentage_set(false), upper_percentage(0), lower_percentage(0), top(0) {}
	string cube;

	struct Comparison {
		Comparison() : use_strings(false), force(false), par1d(0), par2d(0) {}
		bool use_strings;
		bool force;

		string op1;
		double par1d;
		string par1s;

		string op2;
		double par2d;
		string par2s;
	} cmp;

	// pair: (is_all, array)
	typedef vector<vector<string> > CoordsType;
	CoordsType coords;

	bool upper_percentage_set, lower_percentage_set;
	double upper_percentage, lower_percentage;
	int top;
};

struct SortingFilterSettings : public FilterSettings {
	SortingFilterSettings() : FilterSettings(), level(0), limit_count(0), limit_start(0) {}
	string attribute;
	int level;
	unsigned int limit_count;
	unsigned int limit_start;
};

struct StructuralFilterSettings : public FilterSettings {
	StructuralFilterSettings() : FilterSettings(), level(false), level_start(0), level_end(0), revolve(false), revolve_count(0) {}
	string bound;
	bool level;
	int level_start, level_end;
	bool revolve;
	string revolve_elem;
	int revolve_count;
};

struct TextFilterSettings : public FilterSettings {
	vector<string> regexps;
};

struct ViewSubset {
	string dimension;
	vector<BasicFilterSettings> basic;
	TextFilterSettings text;
	SortingFilterSettings sorting;
	AliasFilterSettings alias;
	FieldFilterSettings field;
	vector<StructuralFilterSettings> structural;
	vector<DataFilterSettings> data;
};

struct AxisSubset {
	string subsetHandle;
	string parent;
	string calculation;
	bool zeroSup;
};

typedef vector<AxisSubset> AxisSubsets;

struct ViewAxis {
	AxisSubsets as;
};

struct ViewArea {
	vector<string> properties;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief palo job request
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS PaloJobRequest : public HttpJobRequest {

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	PaloJobRequest(const string& name);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief destructor
	////////////////////////////////////////////////////////////////////////////////

	~PaloJobRequest();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets response
	////////////////////////////////////////////////////////////////////////////////

	HttpResponse* getResponse() {
		return response;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets response
	////////////////////////////////////////////////////////////////////////////////

	void setResponse(HttpResponse* response) {
		this->response = response;
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	void handleDone(Job*);

private:
	void initialize();

public:

	// session
	string sid;
	bool hasSession;
	string negotiationId;

	// https port
	int httpsPort;

	// directory paths
	string browserPath;

	// tokens
	uint32_t * serverToken;
	uint32_t * databaseToken;
	uint32_t * dimensionToken;
	uint32_t * cubeToken;
	uint32_t * clientCacheToken;

	// identifiers
	IdentifierType cube; // default NO_IDENTIFIER
	IdentifierType database; // default NO_IDENTIFIER
	IdentifierType dimension; // default NO_IDENTIFIER
	IdentifierType element; // default NO_IDENTIFIER
	IdentifierType lock; // default NO_IDENTIFIER
	IdentifierType rule; // default NO_IDENTIFIER
	IdentifierType parent; // default ALL_IDENTIFIERS
	IdentifierType limitStart; // default 0
	IdentifierType limitCount; // default NO_IDENTIFIER

	// booleans
	bool add; // default FALSE
	bool baseOnly; // default FALSE
	bool complete; // default FALSE
	bool eventProcess; // default TRUE
	bool showAttribute; // default FALSE
	bool showInfo; // default FALSE
	bool showGputype; // default TRUE
	bool showLockInfo; // default FALSE
	bool showNormal; // default TRUE
	bool showRule; // default FALSE
	bool showSystem; // default FALSE
	bool skipEmpty; // default TRUE
	bool useIdentifier; // default FALSE
	bool useRules; // default FALSE
	bool showUserInfo; // default FALSE
	bool showPermission; // default FALSE

	// strings
	string * actcode;
	string * action;
	string * comment;
	string * condition;
	string * cubeName;
	string * databaseName;
	string * definition;
	string * dimensionName;
	string * elementName;
	string * event;
	string * externPassword;
	string * externalIdentifier;
	string * functions;
	string * lickey;
	string * machineString;
	string * newName;
	string * optionalFeatures;
	string * password;
	string * requiredFeatures;
	string * source;
	string * user;
	string * value;

	// unsigned integers
	uint32_t blockSize; // default 1000
	uint32_t mode; // default 0
	uint32_t position; // default 0
	uint32_t splash; // default 1
	uint32_t steps; // default 0
	uint32_t type; // default 0
	uint32_t function; // default 0
	uint32_t activate; // default 1=ACTIVE

	// list of identifiers
	IdentifiersType *dimensions;
	IdentifiersType *path;
	IdentifiersType *pathTo;
	IdentifiersType *elements;
	IdentifiersType *properties;
	IdentifiersType *rules;
	IdentifiersType *positions;

	// list of unsigned integers
	vector<uint32_t> * types;
	vector<uint32_t> * expand;

	// list of doubles
	vector<vector<double> > *weights;
	vector<double> *dPositions;

	// list of strings
	vector<string> * dimensionsName;
	vector<string> * elementsName;
	vector<string> * pathName;
	vector<string> * pathToName;
	vector<string> * values;

	// list of list of identifiers
	vector<IdentifiersType> *area;
	vector<IdentifiersType> *children;
	vector<IdentifiersType> *paths;
	vector<IdentifiersType> *lockedPaths;

	// list of strings
	vector<vector<string> > * areaName;
	vector<vector<string> > * childrenName;
	vector<vector<string> > * pathsName;

	string httpRequest;

	map<string, ViewSubset> * viewSubsets;
	vector<ViewAxis> * viewAxes;
	ViewArea * viewArea;

private:
	HttpResponse* response;
};
}

#endif
