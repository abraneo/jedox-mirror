////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// Copyright (C) 2006-2013 Jedox AG
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

#pragma once

using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;

namespace Jedox {
	namespace Palo {
		namespace Comm {
			/**@enum DimElementType gives information about the type of elements */
			__value public enum DimElementType
			{
			    DimElementTypeNumeric = de_n,
			    DimElementTypeString = de_s,
			    DimElementTypeConsolidated = de_c,
			    DimElementTypeRule = de_r
			};

			/**@enum SplashMode enumerates one of four splash modes */
			__value public enum SplashMode
			{
			    SplashModeDisable = splash_mode_disable,
			    SplashModeDefault = splash_mode_default,
			    SplashModeBaseSet = splash_mode_base_set,
			    SplashModeBaseAdd = splash_mode_base_add
			};

			/**@enum DimensionAddOrUpdateElementMode enumerates several Add or Update types */
			__value public enum DimensionAddOrUpdateElementMode
			{
			    DimensionAddOrUpdateElementModeAdd = dimension_add_or_update_element_mode_add,
			    DimensionAddOrUpdateElementModeForceAdd = dimension_add_or_update_element_mode_force_add,
			    DimensionAddOrUpdateElementModeUpdate = dimension_add_or_update_element_mode_update,
			    DimensionAddOrUpdateElementModeAddOrUpdate = dimension_add_or_update_element_mode_add_or_update
			};

			/**@enum AddOrUpdateElementMode is an Alias for DimensionAddOrUpdateElementMode*/
			__value public enum AddOrUpdateElementMode
			{
			    AddOrUpdateElementModeAdd = DimensionAddOrUpdateElementModeAdd,
			    AddOrUpdateElementModeForceAdd = DimensionAddOrUpdateElementModeForceAdd,
			    AddOrUpdateElementModeUpdate = DimensionAddOrUpdateElementModeUpdate,
			    AddOrUpdateElementModeAddOrUpdate = DimensionAddOrUpdateElementModeAddOrUpdate
			};

			__value public struct ServerInfo {
				unsigned long MajorVersion;
				unsigned long MinorVersion;
				unsigned long BugfixVersion;
				unsigned long BuildNumber;
				unsigned long Encryption;
				unsigned long HttpsPort;
				unsigned long DataSequenceNumber;
			};

			__value public struct ApiInfo {
				unsigned long MajorVersion;
				unsigned long MinorVersion;
				unsigned long BugfixVersion;
				unsigned long BuildNumber;
			};

            /**@enum DatabaseStatus gives information about the status of the DB */
			__value public enum DatabaseStatus
			{
			    UnloadedDatabase = unloaded_db,
			    LoadedDatabase = loaded_db,
			    ChangedDatabase = changed_db,
			    LoadingDatabase = loading_db,
			    UnknownDatabaseStatus = unknown_db_status
			};
			
			/**@enum DatabaseType contains informations about the type of the specific database */
			__value public enum DatabaseType
			{
			    NormalDatabase = normal_db,
			    SystemDatabase = system_db,
			    UserinfoDatabase = user_info_db,
			    UnknownDatabaseType = unknown_db_type
			};
            
			/**@brief Contains detailed information about the database 
               e.g. the number of dimensions */			 
			__value public struct DatabaseInfo {
				long id;
				String *Name;
				unsigned int NumberDimensions;
				unsigned int NumberCubes;
				DatabaseStatus Status;
				DatabaseType Type;
			};
            
			/**@enum DimensionType Enumerates the types of dimensions */
			__value public enum DimensionType
			{
			    NormalDimension = normal_dim,
			    SystemDimension = system_dim,
			    AttributeDimension = attribut_dim,
			    UserInfoDimension = user_info_dim,
				SystemIdDimension = system_id_dim,
			    UnknownDimensionType = unknown_dim_type
			};
			
			/**@brief gives informations about the specific dimension */
			__value public struct DimensionInfo {
				long id;
				String *Name;
				String *AssocDimension;
				String *AttributeCube;
				String *RightsCube;
				size_t NumberElements;
				size_t MaximumLevel;
				size_t MaximumIndent;
				size_t MaximumDepth;
				DimensionType Type;
			};

			__value public struct DimensionInfoSimple {
				long id;
				String *Name;
				long AssocDimensionId;
				long AttributeCubeId;
				long RightsCubeId;
				size_t NumberElements;
				size_t MaximumLevel;
				size_t MaximumIndent;
				size_t MaximumDepth;
				DimensionType Type;
			};


			/**@enum CubeStatus enumerates the cube status e. g. unloaded cube*/
			__value public enum CubeStatus
			{
			    UnloadedCube = unloaded_cube,
			    LoadedCube = loaded_cube,
			    ChangedCube = changed_cube,
			    UnknownCubeStatus = unknown_cube_status
			};
			
			/**@enum CubeType contains the type of a cube e. g. attribute cube */
			__value public enum CubeType
			{
			    NormalCube = normal_cube,
			    SystemCube = system_cube,
			    AttributeCube = attribut_cube,
			    UserInfoCube = user_info_cube,
				GpuCube = gpu_cube,
			    UnknownCubeType = unknown_cube_type
			};

			/**@brief contains several informations about a cube */
			__value public struct CubeInfo {
				long id;
				String *Name;
				unsigned int NumberDimensions;
				String *Dimensions[];
				long double NumberCells;
				long double NumberFilledCells;
				CubeStatus Status;
				CubeType Type;
			};

			/**@brief contains the consolidation info of an element */
			__value public struct ConsolidationInfo {
				ConsolidationInfo( String *name, double factor );
				String *Name;
				DimElementType Type;
				double Factor;
			};

			__value public struct ConsolidationExtendedInfo {
				long Identifier;
				String *Name;
				DimElementType Type;
				double Factor;
			};

			/**@brief contains information about a rule */
			__value public struct RuleInfo {
				long id;
				String *definition;
				String *extern_id;
				String *comment;
				unsigned long long timestamp;
				bool activated;
			};

			__value public enum RuleActivationMode
			{
				RuleModeDeactivate = rule_mode_deactivate,
				RuleModeActivate = rule_mode_activate,
				RuleModeToggle = rule_mode_toggle
			};

			
			/**@brief contains information about a lock */
			__value public struct LockInfo {
				long id;
				Collections::ArrayList *area;
				String *user;
				unsigned long steps;
			};

			__value public struct ValueSet {
				String *s;
				double d;
			};

			/**@brief the value at a specified coordinate */
			__value public struct DataSetMulti {
				String* Coordinates[];
				ValueSet Value;
			};

			// deprecated
			__value public struct ParentInfo {
				ParentInfo( String *name );
				long Identifier;
				String *Name;
				DimElementType Type;
			};

			// deprecated
			__value public struct ChildInfo {
				ChildInfo( String *name );
				long Identifier;
				String *Name;
				DimElementType Type;
				double Factor;
			};

			/**@brief contains informations about the parent element */
			__value public struct ParentInfoRaw {
				long Identifier;
			};

			/**@brief contains information about the child element */
			__value public struct ChildInfoRaw {
				long Identifier;
				double Factor;
			};

			// deprecated
			__value public struct DimElementInfo2 {
				DimElementInfo2( String *name );
				long Identifier;
				String *Name;
				unsigned long Position;
				unsigned long Level;
				unsigned long Indent;
				unsigned long Depth;
				DimElementType Type;
				size_t NumberParents;
				ParentInfo Parents[];
				size_t NumberChildren;
				ChildInfo Children[];
			};

			/**@brief Contains Info about a specific element  */
			public __gc class ElementInfo {
			public:
				ElementInfo() : Identifier(-1) {}
				ElementInfo( String *name ) : Name(name), NumberParents(0), NumberChildren(0){}
				long Identifier;
				String *Name;
				unsigned long Position;
				unsigned long Level;
				unsigned long Indent;
				unsigned long Depth;
				DimElementType Type;
				size_t NumberParents;
				ParentInfoRaw Parents[];
				size_t NumberChildren;
				ChildInfoRaw Children[];
			};

			/**@enum CellValueType enumerates types of values in cells*/
			__value public enum CellValueType
			{
			    CellValueTypeString,
			    CellValueTypeDouble,
			    CellValueTypeError
			};

			/**@brief contains the value of a cell */
			__value public struct CellValueValue {
				String *StrValue;
				double DblValue;
			};

			/**@brief the value and type of a cell  */
			__value public struct CellValue {
				CellValueType Type;
				CellValueValue Value;
			};
			
			/**@brief the data at specified coordinate */
			__value public struct Dataset {
				String *Coordinates[];
				CellValue Value;
			};

			/**@enum CompareOp enumerates Operators to specialize the data export: 
			 (Use "gt" for "greater than" "gte" for "greater or equal", 
			 "lt" for "less than", "lte" for "less or equal", 
			 "eq" for "qual" or "neq" for "not equal" */
			__value public enum CompareOp
			{
			    CompareOpGT,
			    CompareOpLT,
			    CompareOpGTE,
			    CompareOpLTE,
			    CompareOpEQ,
			    CompareOpNEQ,
			    CompareOpTRUE
			};

			/**@enum BoolOp enumerates the boolean operators*/
			__value public enum BoolOp
			{
			    BoolOpAND,
			    BoolOpOR,
			    BoolOpXOR
			};

			/**@brief contains opportunities to filter the values*/
			__value public struct ValueFilter {
				CompareOp CmpOp1;
				CellValue Value1;
				BoolOp AndOr12;
				CompareOp CmpOp2;
				CellValue Value2;
			};

			/**@brief describes the export options*/
			__value public struct GetDataExportOptions {
				bool BaseElementsOnly;
				bool IngoreEmptyCells;
				ValueFilter Filter;
				String *LastCoordinates[];
				unsigned int NumDatasets;
			};

			__value public enum DrillThroughType
			{
				History = history, 
				Details = details
			};

			__value public struct DrillThroughSet {
				String *Items[];
			};

			__value public enum SVSLoginMode
			{
				None = none,
				Information = information,
				Authentification = authentification,
				Authorization = authorization
			};

			__value public struct SuperVisionServerInfo {
				bool SVSActive;
				SVSLoginMode LoginMode;
				bool CubeWorkerActive;
				bool DrillThroughEnabled;
				bool WindowsSsoEnabled;
			};

			// Begin Subset
	
			__value public enum AliasFilterFlags {
				SearchOne = search_one, // search one attribute for an alias -- DEFAULT show this field in excel
				SearchTwo = search_two, // search two attributes for an alias -- show this field in excel
				HideDouble = hide_double, // hide double aliases, aliases don't have to be unique !! (Makes not really sense)
				DisplayAlias = display_alias,//Choose an attribute-dimension that serves as alias pool and show the aliases instead
				UseFilterExp = use_filter_exp // use advanced filter expressions for attribute-values
			};

			__value public enum DataFilterFlags {
				DataMin = data_min, // use min operator on cell values
				DataMax = data_max, // use max operator on cell values
				DataSum = data_sum, // use sum operator on cell values
				DataAverage = data_average, // use average operator on cell values
				DataAny = data_any, //conditions must be true for at least one value
				DataAll = data_all, //conditions must be true for all values
				DataString = data_string, //the elements could then be sorted according to these strings or flags like TOP could be applied
				OnlyConsildated = only_consildated, // compute data only for consolidations (don't filter leaves)
				OnlyLeaves = only_leaves, // compute data only for leaves (don't filter consolidations)
				UpperPercentage = upper_percentage, // sort elements from highest to lowest and choose those that contribute the first p1% (set percentage method)
				LowerPercentage = lower_percentage, // sort elements from lowest to highest and choose those that contribute the first p1% (set percentage method)
				MidPercentage = mid_percentage, // sort elements from highest to lowest and choose those that contribute p2% after removing the first elements that make up p1%
				Top = top, //pick only the top-x elements. x set by set_top
				NoRules = no_rules //do not use rules when extracting cell values
			};

			__value public enum PickListFlags {
				InsertBack = insert_back, // Put manually picked elements after the others. Default: before
				MergeElements = merge_elements, // Put manually picked elements before the others
				Sub = sub,	 // Use the picklist as a filter, not as a union.
				InsertFront = insert_front // Default value, put manually picked elements before the others
			};

			__value public enum SortingFilterFlags {
				Text = text, // sort filtered elements according to name (default, not necessary to pass)
				Numeric = numeric, // sort filtered elements according to value computed by data-filter !! MIGHT BE STRING DATA !!
				UseAttribute = use_attribute, // sort according to an attribute (to be set separately)
				UseAlias = use_alias, // sort according to aliases as determined by alias filter
				ReverseOrder = reverse_order, // show parents below their children
				LeavesOnly = leaves_only, // do not sort consolidated elements
				StandardOrder = standard_order, // sort in order highest to lowest. (default, not necessary to pass)
				Whole = whole, //show whole hierarchy
				Position = position, //position --default
				ReverseTotal = reverse_total, //reverse the sorting
				SortOneLevel = sort_one_level, //sort on the level of level-element
				FlatHierarchy = flat_hierarchy, //do NOT build a tree -- default
				NoChildren = no_children, // build a tree, but do not follow the children-list of an element that has been filtered out before
				ConsolidatedOnly = consolidated_only, //only sort consolidations
				SortNotOneLevel = sort_not_one_level, // sort all levels except one
				ShowDuplicates = show_duplicates, //Show duplicates, default value is 0 (flag inactive - duplicates are hidden)
				ReverseTotalEx = reverse_total_ex, //this will completely reverse the ordering
				Limit = limit, //Limit number of elements returned
				ConsolidatedOrder = consolidated_order //Sort by consolidation order
			};

			__value public enum StructuralFilterFlags {
				BelowInclusive = below_inclusive, // choose elements below including element passed to set_bound
				BelowExclusive = below_exclusive, // choose elements below excluding element passed to set_bound
				HideConsolidated = hide_consolidated, // Remove all consolidated elements from set, show only leaves
				HideLeaves = hide_leaves, // Remove all non-consolidated elements, show aggregations only
				HierarchicalLevel = hierarchical_level, // pick elements from top to bottom (levels to be specified separately)
				AggregatedLevel = aggregated_level, // pick elements from bottom to top (levels to be specified separately)
				Revolving = revolving, // revolve (repeat) the list, choose all elements on the same level
				RevolveAddAbove = revolve_add_above, // add all elements with the same or a higher level than elemname and repeat the list
				RevolveAddBelow = revolve_add_below, // add all elements with the same or a lower level than elemname and repeat the list
				AboveExclusive = above_exclusive, // choose elements above excluding element passed to set_bound
				AboveInclusive = above_inclusive, // choose elements above including element passed to set_bound
				Cyclic = cyclic  //simply repeat the list without further filtering
			};

			__value public enum TextFilterFlags {
				AdditionalFields = additional_fields, // use additional columns for searching uses the attribute-columns of this dimension per default
				Extended = extended // do not use POSIX-Basic expressions but use Perl-Extended regular expressions
			};

			__value public struct ArgAliasFilterSettings {
				ArgAliasFilterSettings();
				bool Active;
				AliasFilterFlags Flags;
				String *Attribute1;
				String *Attribute2;
			};

			/**
			@brief takes a subset from the attributes of a cube 
			@param Acitve  Activates the field filter if true in function DimensionSubset
			@param AliasFilterFlags 
						- SearchOne: search one attribute for an alias -- DEFAULT show this field in excel
						- SearchTwo: search two attributes for an alias -- show this field in excel
						- HideDouble: hide double aliases, aliases don't have to be unique !! (Makes not really sense)
						- DisplayAlias: Choose an attribute-dimension that serves as alias pool and show the aliases instead
						- UseFilterExp: use advanced filter expressions for attribute-values
			@param Advanved[] Array with specifications of the attribute filter (e.g. ">5000")
				\n \n sample: \n \n
			   @include Afilter/Program.cs
			*/	
			__value public struct ArgFieldFilterSettings {
				ArgFieldFilterSettings();
				bool Active;
				AliasFilterFlags Flags;
				Array* Advanced[];
			};

			/**
			@brief Adds manually selected elements to the defined subset
			@param Active Activates the basic filter if true in function DimensionSubset
			@param Flags 
						- InsertFront: Default value, put manually picked elements before the others
						- MergeElements: orders basic elements to the other elements \n
						- InsertBack: put picked elements after the others
						- Sub: Use the picklist as a filter, not as a union
			@param ManualSubsetSet Have to be true, to add picklist to the subset
			@param Manualsubset[] Array with manual picked element names strings
			  \n \n sample: \n \n
			@include Picklist/Program.cs
			*/
			__value public struct ArgBasicFilterSettings {
				ArgBasicFilterSettings();
				bool Active;
				PickListFlags Flags;
				bool ManualSubsetSet;
				String *ManualSubset[];
			};

			__value public struct DataComparison {
				bool UseStrings;
				String *Op1;
				double Par1d;
				String *Par1s;
				String *Op2;
				double Par2d;
				String *Par2s;
			};

			__value public struct BoolStringArray {
				bool BoolVal;
				String *Str[];
			};

			/**
			@brief Create a Slice of a cube by data of a cube
			@param Active Activates the structural filter if true in function DimensionSubset 
			@param Flags 
				   - DataMin: use min operator on cell values
				   - DataMax: use max operator on cell values
				   - DataSum: use sum operator on cell values
				   - DataAverage: use average operator on cell values
				   - DataAny: conditions must be true for at least one value
				   - DataAll: conditions must be true for all values
				   - DataString: the elements could then be sorted according to these strings or flags like TOP could be applied
				   - OnlyConsildated: compute data only for consolidations (don't filter leaves)
				   - OnlyLeaves: compute data only for leaves (don't filter consolidations)
				   - UpperPercentage: sort elements from highest to lowest and choose those that contribute the first p1% (set percentage method)
				   - LowerPercentage: sort elements from lowest to highest and choose those that contribute the first p1% (set percentage method)
				   - MidPercentage: sort elements from highest to lowest and choose those that contribute p2% after removing the first elements that make up p1%
				   - Top: pick only the top-x elements. x set by set_top
				   - NoRules: do not use rules when extracting cell values
			@param Cube Name of the cube 
			@param Cmp 
				   - bool UseStrings: If set to false double values (Par1d, Par2d) are used otherwise string values (Par1s , Par2s)
				   - String *Op1 - Operator for the limitation of the selection of elements. Possible Operators: "=",">=",">","<=","=","<>".
				   - double Par1d - Value that relates to Op1
				   - String *Par1s - Value that relates to Op1
				   - String *Op2 - Operator for the limitation of the selection of elements. Possible Operators: "=",">=",">","<=","=","<>".
				   - double Par2d - Value that relates to Op2
				   - String *Par2s - Value that relates to Op2
			@param Coordset Have to be true, to use the coordinates of the cube
			@param Coords[] Array with coordinats of the cube
			@param UpperPercentageSet Have to be true to use this UpperPercentage filter
			@param LowerPercentageSet Have to be true to use this LowerPercentage filter
			@param UpperPercentage Only Products in the defined area will be shown
			@param LowerPercentage Only Products in the defined area will be shown
			@param Top Only top elements will be shown
				\n \n sample: \n \n
			@include DFilter/Program.cs
			*/
			__value public struct ArgDataFilterSettings {
				ArgDataFilterSettings();
				bool Active;
				DataFilterFlags Flags;
				String *Cube;
				DataComparison Cmp;
				bool CoordsSet;
				BoolStringArray Coords[];
				bool UpperPercentageSet;
				bool LowerPercentageSet;
				double UpperPercentage;
				double LowerPercentage;
				int Top;
			};

			
			/**
			@brief Sorting the elements of the created subset
			@param Active Activates the sorting functionality if true in function DimensionSubset 
			@param Flags 
					 - Text: sort filtered elements according to name (default, not necessary to pass)
					 - Numeric: sort filtered elements according to value computed by data-filter - MIGHT BE STRING DATA 
					 - UseAttribute: sort according to an attribute (to be set separately)
					 - UseAlias: sort according to aliases as determined by alias filter
					 - ReverseOrder: show parents below their children
					 - LeavesOnly: do not sort consolidated elements
					 - StandardOrder: sort in order highest to lowest. (default, not necessary to pass)
					 - Whole: show whole hierarchy
					 - Position: position - default
					 - ReverseTotal: reverse the sorting
					 - SortOneLevel: sort on the level of level-element
				 	 - FlatHierarchy: do NOT build a tree
					 - NoChildren: build a tree, but do not follow the children-list of an element that has been filtered out before
					 - ConsolidatedOnly: only sort consolidations
					 - SortNotOneLevel: sort all levels except one
					 - ShowDuplicates: Show duplicates, default value is 0 (flag inactive - duplicates are hidden)
					 - ReverseTotalEx: this will completely reverse the ordering
					 - Limt : this will limit the number of returned elements

			@param Attribute Name of the Attribute
			@param Level Indention level of elements, which should be sort (minimum indention is 1)
			*/
			__value public struct ArgSortingFilterSettings {
				ArgSortingFilterSettings();
				bool Active;
				SortingFilterFlags Flags;
				String *Attribute;
				int Level;
				unsigned int Limit_count;
				unsigned int Limit_start;
			};

			/**
			@brief Filters elements by type and hierachical level
			@param Active Activates the structural filter if true in function DimensionSubset
			@param Flags 
					 - BelowInclusive: choose elements below including element passed to set_bound
					 - BelowExclusive: choose elements below excluding element passed to set_bound
					 - HideConsolidated: Remove all consolidated elements from set, show only leaves
					 - HideLeaves: Remove all non-consolidated elements, show aggregations only
					 - HierarchicalLevel: pick elements from top to bottom (levels to be specified separately)
					 - AggregatedLevel: pick elements from bottom to top (levels to be specified separately)
					 - Revolving revolve: (repeat) the list, choose all elements on the same level
					 - RevolveAddAbove: add all elements with the same or a higher level than elemname and repeat the list
					 - RevolveAddBelow: add all elements with the same or a lower level than elemname and repeat the list
					 - AboveExclusive: choose elements above excluding element passed to set_bound
					 - AboveInclusive: choose elements above including element passed to set_bound
					 - Cyclic: simply repeat the list without further filtering
			@param Bound Selected elements
			@param Level If set to true level filter will be activated
			@param LevelStart Hierachical filter starts at the given level
			@param LevelEnd Hierachical filter end at the given level
			@param Revolve Activates revolving if true
			@param RevolveElement Name of the Element, where to start revolving 
			@param RevelveCount Length of the resulting elementlist
			  \n \n sample: \n \n
			@include Sort/Program.cs
			*/
			__value public struct ArgStructuralFilterSettings {
				ArgStructuralFilterSettings();
				bool Active;
				StructuralFilterFlags Flags;
				String *Bound;
				bool Level;
				int LevelStart;
				int LevelEnd;
				bool Revolve;
				String *RevolveElement;
				int RevolveCount;
			};

							
			/**
			@brief Select Elements by string pattern
			@param Active Activates the text filter if true in function DimensionSubset
			@param Flags 
					 - AdditionalFields: use additional columns for searching uses the attribute-columns of this dimension per default
					 - Extended: do not use POSIX-Basic expressions but use Perl-Extended regular expressions
			@param RegularExpressions: String pattern, after the elements will be selected
			  \n \n sample: \n \n
			@include TFilter/Program.cs
			*/
			__value public struct ArgTextFilterSettings {
				ArgTextFilterSettings();
				bool Active;
				TextFilterFlags Flags;
				String *RegularExpressions[];
			};

			__value public struct SubsetResult {
				String *Name;
				String *Alias;
				String *Path;
				size_t Index;
				size_t Depth;
				long Identifier;
				bool HasChildren;
			};

			// End Subset

			[ComVisible( true )]

			/**@brief This class is the main class to use for communication 
			   with the Palo server */			
			public __gc class Connection {
			public:
				Connection( String *hostname, String *service, String *username, String *pw_hash );
				Connection( String *hostname, String *service, String *username, String *pw_hash, String *trust_file  );
				~Connection( void );

				[ComVisible( true )]
				struct sock_obj so;

				void Dispose( void );
				
				void Ping();

				static void TestConnection(String *hostname, unsigned int port, String *username, String *pw_hash);
				static void TestConnection(String *hostname, unsigned int port, String *username, String *pw_hash, String *trust_file);
				void CheckValidity();

				/* list */

				/**@brief lists all databases 
				    \n \n sample: \n \n
				   @include RootListDatabases/Program.cs
				*/		
				String *RootListDatabases()[];

				/**@brief lists all databases of a type	
				 */				
				String *RootListDatabases( DatabaseType Type )[];

				/**@brief list all cubes in specified database
				   @param database Name of the database 
				   @return array with all cubes in specified database 
				   \n \n sample: \n \n
				 */	
				
				String *DatabaseListCubes( String *database )[];
				
				/**@brief lists all cubes of a type in specified database
				   @param database Name of the database
				   @param Type Type of cubes
				   @return array with all cubes in specified database 
				 */	
				String *DatabaseListCubes( String *database, CubeType Type )[];
				
				/**@brief lists all cubes of a type in specified database
				   @param database Name of the database
				   @param Type Type of cubes
				   @param OnlyCubesWithCells if True only Cube with cells are returned
				   @return array with cubes in specified database 				 
				 */	
				String *DatabaseListCubes( String *database, CubeType Type, bool OnlyCubesWithCells )[];

				/**@brief lists all dimensions in the specified database
				   @param database Name of the database
				   @return List of dimensions
				    \n \n sample: \n \n
				   @include DatabaseListDimensions/Program.cs
				 */				 				 				
				String *DatabaseListDimensions( String *database )[];

				/**@brief Lists all dimensions of a type in the specified database
				   @param database Name of the database
				   @param Type of dimensions
				   @return List of dimensions
				    \n \n sample: \n \n
				 */	
				String *DatabaseListDimensions( String *database, DimensionType Type )[];

				/**@brief Lists all dimensions of a cube in the specified database
				   @param database Name of the database
				   @param cube Name of the Cube
				   @return List of dimensions
				    \n \n sample: \n \n
				   @include CubeListDimensions/Program.cs
				 */				 				 				 				
				String *CubeListDimensions( String *database, String *cube )[];

				/**@brief Lists all cubes using the specified dimension
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @return List of cubes
				    \n \n sample: \n \n
				   @include DimensionListCubes/Program.cs
				 */				  				 				 				
				String *DimensionListCubes( String *database, String *dimension )[];

				/**@brief Lists all cubes of a type using the specified dimension
				   @param database Name of the database
				   @param dimension Name of the dimension
   				   @param Type of cubes
				   @return List of cubes
				    \n \n sample: \n \n
				 */	
				String *DimensionListCubes( String *database, String *dimension, CubeType Type )[];

				/**@brief Lists all cubes of a type using the specified dimension
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param Type Type of cubes
				   @param OnlyCubesWithCells if True only Cube with cells are returned
				   @return array with cubes in specified database 				 
				 */	
				String *DimensionListCubes( String *database, String *dimension, CubeType Type, bool OnlyCubesWithCells )[];

				/**@brief retrieves the type of the specified database
				   @param database Name of the database	
				   @return returns the type of the database
				    \n \n sample: \n \n
				   @include GetDatabaseType/Program.cs
				  */			
				DatabaseType GetDatabaseType( String *database );

				/**@brief retrieves the type of the specified dimension
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @return returns the type of the dimension
				    \n \n sample: \n \n
				   @include GetDimensionType/Program.cs
				  */
				DimensionType GetDimensionType( String *database, String *dimension );
				
				/**@brief retrieves the type of the specified cube
				   @param database Name of the database
				   @param cube Name of the cube
				   @return returns the type of the specified cube
				    \n \n sample: \n \n
				   @include GetCubeType/Program.cs
				  */
				CubeType GetCubeType( String *database, String *cube );
				
				/**@brief retrieves the name of the attribute dimension
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @return returns the name of the attribute dimension
				    \n \n sample: \n \n
				   @include GetAttributeDimension/Program.cs
				  */			
				String *GetAttributeDimension( String *database, String *dimension );

				/**@brief retrieves the name of he appropriated attribute cube
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @return returns the name of the specified cube
				    \n \n sample: \n \n
				   @include GetAttributeCube/Program.cs
				  */				
				String *GetAttributeCube( String *database, String *dimension );
				
				/**@brief retrieves the name of the privilege cube
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @return returns name of the privilege cube
				    \n \n sample: \n \n
				   @include GetRightsCube/Program.cs
				  */				
				String *GetRightsCube( String *database, String *dimension );

				// Deprecated 			 				 				 
				DimElementInfo2 DimensionListDimElements2( String *database, String *dimension )[];

				/**@brief Lists all elements contained in the specified dimension 
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @return returns a list of elements
				 */				 			 				 				 
				ElementInfo* DimensionListElements( String *database, String *dimension )[];

				/**@brief Lists all children of an consolidated element.
				   @param database Name of the database				 
				   @param dimension Name of the dimension
				   @param element Name of the element
				   @return List of children
				    \n \n sample: \n \n
				   @include DimElementListConsolidated/Program.cs
				 */				 			 				 				 				
				ConsolidationInfo DimElementListConsolidated( String *database, String *dimension, String *element )[];

				ConsolidationExtendedInfo ElementListConsolidated( String *database, String *dimension, String *element )[];

				bool ElementExists( String *database, String *dimension, String *element );

				/**@brief Returns the count of children of the specified elements
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param dimension_element Name of the element
				   @return count of the child elements
				    \n \n sample: \n \n
				   @include ElementChildCount/Program.cs
				 */				 				 				 				 				
				unsigned int ElementChildCount( String *database, String *dimension, String *dimension_element );

				/**@brief Retrieves the count of parents of the specified element
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param dimension_element Name of the element
				   @return Count of the parent elements
				    \n \n sample: \n \n
				   @include ElementParentCount/Program.cs
				 */				 				 
				unsigned int ElementParentCount( String *database, String *dimension, String *dimension_element );

				/**@brief Retrieves the count of all elements in the specified dimension
				   @param database Name of the dimension
				   @param dimension Name of the dimension
				   @return Count of elements or error on failure
				    \n \n sample: \n \n
				   @include ElementCount/Program.cs
				 */				 			 				 				
				unsigned int ElementCount( String *database, String *dimension );

				/**@brief Returns the position in the dimension of the specified elements
				   @param  database Name of the database
				   @param  dimension Name of the dimension
				   @param  dimension_element Name of the element
				   @return Position of the element
				    \n \n sample: \n \n
				   @include ElementIndex/Program.cs
				 */				 			
				unsigned int ElementIndex( String *database, String *dimension, String *dimension_element );

				/**@brief Checks if the specified element is a child of the other specified element
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param de_parent Name of the element which should be the parentelement
				   @param de_child Name of the element which should be the childelement 
				   @return bool TRUE, if the parent element contains the specified child element
				    \n \n sample: \n \n
				   @include ElementIsChild/Program.cs
				*/				
				bool ElementIsChild( String *database, String *dimension, String *de_parent, String *de_child );

				/**@brief Retrieves the level of an element in the consolidation hierarchy. The minimal level is 1.
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param dimension_element Name of the element
				   @return returns the Level of element
				    \n \n sample: \n \n
				   @include ElementLevel/Program.cs
				 */				 				 				 				 				
				unsigned int ElementLevel( String *database, String *dimension, String *dimension_element );

				/**@brief Retrieves the indentation level of an element in the consolidation hierarchy. The minimal indention is 1.
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param dimension_element Name of the element
				   @return returns the Level of indention
				    \n \n sample: \n \n
				   @include ElementIndent/Program.cs
				 */				
				unsigned int ElementIndent( String *database, String *dimension, String *dimension_element );

				/**@brief Returns the maximal consolidation level in a dimension (consolidation depth)
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @return returns the Maximal consolidation level
				    \n \n sample: \n \n
				   @include ElementTopLevel/Program.cs
				*/
				unsigned int ElementTopLevel( String *database, String *dimension );

				/**@brief Returns the name of the n'th element. starting at 1.
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param n ppsition of the element in dimension 
				   @return returns the name of the specified element				
				    \n \n sample: \n \n
				   @include ElementName/Program.cs
				 */				
				String *ElementName( String *database, String *dimension, unsigned int n );

				/**@brief Returns the type of the specified element
				   @param database Name of the database
  				   @param dimension Name of the dimension
  				   @param dimension_element Name of the element
  				   @return returns the Type of the element
				    \n \n sample: \n \n
				   @include ElementType/Program.cs
  				 */  				 
				DimElementType ElementType( String *database, String *dimension, String *dimension_element );

				/**@brief Returns the n'th sibling of an element
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param dimension_element Name of the element
				   @param n Offset of the sibling (can be negative)
				   @return returns the Name of the n'th sibling
				    \n \n sample: \n \n
				   @include ElementSibling/Program.cs
				*/
				String *ElementSibling( String *database, String *dimension, String *dimension_element, int n );

				/**@brief Returns the consolidation factor of the child element
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param de_parent Name of the element
				   @param de_child Name of the child element
				   @return returns the Factor of consolidation
				    \n \n sample: \n \n
				   @include ElementWeight/Program.cs
				*/
				double ElementWeight( String *database, String *dimension, String *de_parent, String *de_child );

				/**@brief Retrieves the next element
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param dimension_element Name of the element
				   @return returns the Name of the next element
				    \n \n sample: \n \n
				   @include ElementNext/Program.cs
				*/
				String *ElementNext( String *database, String *dimension, String *dimension_element );

				/**@brief Retrieves the previous element
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param dimension_element Name of the element
				   @return returns the Name of the previous element
				    \n \n sample: \n \n
				   @include ElementPrev/Program.cs
				 */
				String *ElementPrev( String *database, String *dimension, String *dimension_element );

				/**@brief Retrieves the n'th parent name of the specified element
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param dimension_element Name of the element
				   @param n Number of the parent to retrieve
				   @return returns the Name of the element
				    \n \n sample: \n \n
				   @include ElementParentName/Program.cs
				*/
				String *ElementParentName( String *database, String *dimension, String *dimension_element, unsigned int n );

				/**@brief Retrieves the first element from the specified dimension
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @return returns the Name of the first element in specified dimension
				    \n \n sample: \n \n
				   @include ElementFirst/Program.cs
				*/
				String *ElementFirst( String *database, String *dimension );
	
				 /**@brief Retrieves the name of the n'th child of an element
				    @param database Name of the database
				    @param dimension Name of the dimension
				    @param dimension_element Name of the element
				    @param n Number of the cild element to retrieve
				    @return returns the Name of the child element
					 \n \n sample: \n \n
				   @include ElementChildName/Program.cs
				  */
				String *ElementChildName( String *database, String *dimension, String *dimension_element, unsigned int n );

				/**@brief retrieves 1. dimension, that contains all elements according to dimension_elements
				   @param database Name of the database
				   @param dimension_elements Names of the elements
				   @param should_be_unique if true, throws an exception, if there's a second dimension,
				    that contains all elements according to dimension_elements
				   @return Name of the specified dimension
				    \n \n sample: \n \n
				   @include ElementDimension/Program.cs
				 */
				String *ElementDimension( String *database, String *dimension_elements[], bool should_be_unique );
				
				/**@brief retrieves first dimension of the cube, that contains all elements according to dimension_elements
				   @param database Name of the database
				   @param cube Name of the cube
				   @param dimension_elements Names of the elements
				   @param should_be_unique if true, throws an exception, if there's a second dimension of the cube, that contains all elements according to dimension_elements
				   @return Name of the specified dimension
				    \n \n sample: \n \n
				   @include ElementCubeDimension/Program.cs
				  */				
				String *ElementCubeDimension( String *database, String *cube, String *dimension_elements[], bool should_be_unique );

				/************************************************************************/
				/* lookup helpers                                                       */
				/************************************************************************/

				/**@brief retrieves name of an element for a given id
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param id id of the element
				   @param 
				   @return Name of the element
				  */				
				String *GetElementNameFromID( String *database, String *dimension, long id );

				/**@brief retrieves position of an element for a given id
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param id id of the element
				   @param 
				   @return type of the element
				  */				
				long GetElementPositionFromID( String *database, String *dimension, long id );

				/**@brief retrieves type of an element for a given id
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param id id of the element
				   @param 
				   @return type of the element
				  */				
				DimElementType GetElementTypeFromID( String *database, String *dimension, long id );

				/**@brief retrieves id of an element
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param element Name of the element
				   @param 
				   @return id
				  */				
				long GetElementIDFromName( String *database, String *dimension, String *element );

				/**@brief retrieves name of a dimension for a given id
				   @param database Name of the database
				   @param id id of the dimension
				   @param 
				   @return Name of the dimension
				  */				
				String* GetDimensionNameFromID(String* database, long id);

				/**@brief retrieves name of a cube for a given id
				   @param database Name of the database
				   @param id id of the cube
				   @param 
				   @return Name of the cube
				  */				
				String* GetCubeNameFromID(String* database, long id);

				ServerInfo ServerInformation();
				unsigned long long LicenseExpires();
				static ApiInfo ApiInformation();

				/**@brief retrieves information about the specified database
				   @param database Name of the database
				   @return returns information about the specified database
				    \n \n sample: \n \n
				   @include DatabaseInformation/Program.cs
				  */				
				DatabaseInfo DatabaseInformation( String *database );

				/**@brief Deprecated use DimensionInformationSimple instead
				*/
				DimensionInfo DimensionInformation( String *database, String *dimension );

				/**@brief retrieves inormation about the specified dimension
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @return returns information about the specified dimension
				    \n \n sample: \n \n
				   @include DimensionInformationSimple/Program.cs
				*/
				DimensionInfoSimple DimensionInformationSimple( String *database, String *dimension );

				/**@brief Deprecated use ElementInformationSimple instead
				*/
				DimElementInfo2 ElementInformation( String *database, String *dimension, String *element );

				/**@brief retrieves inormation about the specified element
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param element Name of the element
				   @return returns information about the specified element
				    \n \n sample: \n \n
				   @include ElementInformationSimple/Program.cs
				*/
				ElementInfo* ElementInformationSimple( String *database, String *dimension, String *element );

				/**@brief retrieves informations about the specified cube
				   @param database Name of the database
				   @param cube Name of the cube 
				   @return returns information about the queried cube
				    \n \n sample: \n \n
				   @include CubeInformation/Program.cs
				  */				
				CubeInfo CubeInformation( String *database, String *cube );

				/* data */
				
				/**@brief retrieves value for a cell
				   @param str_result it's valid if not null
   				   @param dbl_result it's valid if str_result is null
				   @param database Name of the database
				   @param cube Name of the cube 
				   @param coordinates coordinates of the cell
				   @return returns nothing
				    \n \n sample: \n \n
				   @include GetData/Program.cs
				 */		
				void GetData( String *&str_result, double __gc &dbl_result, String *database, String *cube, String *coordinates[] );

				/**@brief Sets the value for a string cell
				   @param database Name of the database
				   @param cube Name of the cube
				   @param coordinates coordinates of the cell
				   @param str_value	value to be set	
				   @return returns nothing		
				    \n \n sample: \n \n
				   @include SetData/Program.cs
				*/
				void SetData( String *database, String *cube, String *coordinates[], String *str_value );

				/**@brief Sets the value for a numeric cell
				   @param database Name of the database
				   @param cube Name of the cube
				   @param coordinates coordinates of the cell
				   @param dbl_value	value to be set	
				   @return nothing
				*/
				void SetData( String *database, String *cube, String *coordinates[], double dbl_value );

				/**@brief Sets the value for a numeric cell
				   @param database Name of the database
				   @param cube Name of the cube
				   @param coordinates coordinates of the cell
				   @param dbl_value	value to be set
				   @param mode specifies the splash mode
				          (type: "SPLASH_MODE_NONE", "SPLASH_MODE_DEFAULT", "SPLASH_MODE_SET" or "SPLASH_MODE_ADD").	
				   @return returns nothing		
				*/
				void SetData( String *database, String *cube, String *coordinates[], double dbl_value, SplashMode mode );

				/**@brief Sets the value for a cell
				   @param database Name of the database
				   @param cube Name of the cube
				   @param coordinates coordinates where the value should be set
				   @param dbl_value	double value to be set, is used if str_value is = null
				   @param str_value string value to be set
				   @param mode specifies the splash mode
						       (type: "SPLASH_MODE_NONE", "SPLASH_MODE_DEFAULT", "PLASH_MODE_SET" or "SPLASH_MODE_ADD").
				   @return returns nothing
				   */
				void SetData( String *database, String *cube, String *coordinates[], String *str_value, double dbl_value, SplashMode mode );

				/**@brief Sets values for multiple cells
				   @param database Name of the database
				   @param cube Name of the cube
				   @param dsa the coordinates and values of the cells to be set
				   @param mode specifies the splash mode
							   (type: "SPLASH_MODE_NONE", "SPLASH_MODE_DEFAULT", "SPLASH_MODE_SET" or "SPLASH_MODE_ADD").	
				   @param Add Values are added to the current value
				   @param EventProccessor if false the vision server is not called
				   @return returns nothing
				   */
				void SetDataMulti( String *database, String *cube, DataSetMulti dsa[], SplashMode mode, bool Add, bool EventProccessor );

				/**@brief copies the values according the structure from 'from' to 'to'
				   @param database database Name of the database
				   @param cube Name of the cube
				   @param from source coordinates
				   @param to target coordinates
				   @return returns nothing
				    \n \n sample: \n \n
				   @include CopyCell/Program.cs
				*/
				void CopyCell( String *database, String *cube, String *from[], String *to[] );

				/**@brief copies the values according the structure from 'from' to 'to' and then dbl_value is splashed to 'to'
				   @param database Name of the database
				   @param cube Name of the cube
				   @param from source coordinates
				   @param to target coordinates
				   @param dbl_value value to be splashed
				   @return returns nothing
				*/
				void CopyCell( String *database, String *cube, String *from[], String *to[], double dbl_value );

				void GoalSeek( String *database, String *cube, String *path[], double dbl_value );

				/**@brief exports the data as specified in opt
				   @param database Name of the database
				   @param cube Name of the cube
				   @param elements list of elements
				   @param opts options for the export
				   @return Array with Coordinates and Values
				    \n \n sample: \n \n
				   @include GetDataExport/Program.cs
				   */
				Dataset GetDataExport( String *database, String *cube, Array *elements[], GetDataExportOptions opts )[];
				
				/**@brief exports the data as specified in opt
				   @param database Name of the database
				   @param cube Name of the cube
				   @param elements list of elements
				   @param opts options for the export
				   @param progress how much of the cube is processed
				   @return Array with Coordinates and Values
				  */
				Dataset GetDataExport( String *database, String *cube, Array *elements[], GetDataExportOptions opts, double __gc &progress )[];

				/**@brief exports the data as specified in opt
				   @param database Name of the database
				   @param cube Name of the cube
				   @param elements list of elements
				   @param opts options for the export
				   @param useRules if true apply rules before exporting			
				   @param progress how much of the cube is processed
				   @return Array with Coordinates and Values
				  */
				Dataset GetDataExport( String *database, String *cube, Array *elements[], GetDataExportOptions opts, bool useRules, double __gc &progress )[];

				Dictionary<long, String*> __gc * GetAttributeValues( String *database, String *dimension, long elements __gc [], String *attribute);
				Dictionary<long, String*> __gc * GetAttributeValues( String *database, String *dimension, long elements __gc [], String *attribute, long Lastelement, unsigned long NumDatasets);

				/**@brief get values for several cells
				   @param database Name of the database
				   @param cube Name of the cube
				   @param elements list of coordinates
				   @return Array with Values
				    \n \n sample: \n \n
				   @include GetDataMulti/Program.cs
				  */
				CellValue GetDataMulti( String *database, String *cube, Array *elements[])[];

				/**@brief get values for an area of cells
				   @param database Name of the database
				   @param cube Name of the cube
				   @param elements list of elements
				   @return Array with Values
				    \n \n sample: \n \n
				   @include GetDataArea/Program.cs
				  */
				CellValue GetDataArea( String *database, String *cube, Array *elements[])[];

				/* admin */
				
				/**@brief Adds a databases 
				   @param database Name of the database
				   @return returns nothing
				    \n \n sample: \n \n
				   @include AddDatabase/Program.cs
				 */				
				void AddDatabase( String *database );

				/**@brief unload the selected database
				   @param database Name of database
				   @return returns nothing
				    \n \n sample: \n \n
				   @include UnloadDatabase/Program.cs
				  */				
				void UnloadDatabase( String *database );

				/**@brief Deletes the specified database
                   @param database Name of the database
                   @return returns nothing 
				    \n \n sample: \n \n
				   @include DeleteDatabase/Program.cs
				   */
				void DeleteDatabase( String *database );

				/**@brief deletes the selected dimension
				  @param database Name of the database
				  @param dimension Name of the dimension
				  @return returns nothing
				   \n \n sample: \n \n
				   @include DeleteDimension/Program.cs
				 */				 				 				 				
				void DeleteDimension( String *database, String *dimension );

				/**@brief deletes all elements of the given dimension
				  @param database Name of the database
				  @param dimension Name of the dimension
				  @return returns nothing
				   \n \n sample: \n \n
				   @include ClearDimension/Program.cs
				 */				 				 				 				
				void ClearDimension( String *database, String *dimension );

				/**@brief adds a cube into the given database
				  @param database Name of the database
				  @param cube Name of the cube
				  @param dimensions Names of the Dimensions of the cube	
				  @return returns nothing				 			 				 				
				   \n \n sample: \n \n
				   @include DatabaseAddCube/Program.cs
				 */				
				void DatabaseAddCube( String *database, String *cube, String *dimensions[] );

				/**@brief adds a cube into the given database
				  @param database Name of the database
				  @param cube Name of the cube
				  @param dimensions Names of the Dimensions of the cube	
				  @param type type of the cube	(NormalCube or UserInfoCube)
				  @return returns nothing				 			 				 				
				 */				
				void DatabaseAddCube( String *database, String *cube, String *dimensions[], CubeType type );

				/**@brief Adds a dimension to to the specified database
				  @param database Name of the database 
				  @param dimension Name of the dimension
				  @return returns nothing
				   \n \n sample: \n \n
				   @include DatabaseAddDimension/Program.cs
				 */				 			 				 								
				void DatabaseAddDimension( String *database, String *dimension );

				/**@brief Adds a dimension to to the specified database
				  @param database Name of the database 
				  @param dimension Name of the dimension
				  @param type type of the dimension	(NormalDimension or UserInfoDimension)
				  @return returns nothing
				   \n \n sample: \n \n
				   @include DatabaseAddDimension/Program.cs
				 */				 			 				 								
				void DatabaseAddDimension( String *database, String *dimension, DimensionType type );

				/**@brief Renames a cube
				  @param database Name of the database 
				  @param cube_oldname Name of the cube
				  @param cube_newname New name of the cube
				  @return returns nothing
				   \n \n sample: \n \n
				   @include DatabaseRenameCube/Program.cs
				 */				 				 		 				 				
				void DatabaseRenameCube( String *database, String *cube_oldname, String *cube_newname );

				/**@brief Renames a dimension
				  @param database Name of the database 
				  @param dimension_oldname Name of the dimension
				  @param dimension_newname New name of the dimension	
				  @return returns nothing
				   \n \n sample: \n \n
				   @include DatabaseRenameDimension/Program.cs
				 */				 				 		 				 				
				void DatabaseRenameDimension( String *database, String *dimension_oldname, String *dimension_newname );

				/**@brief add element in dimension or update properties of the element
				   @param database Name of the database
				   @param dimension Name of the dimension
			       @param element Name of the element
				   @param mode mode of operation
				   @param type type of the element
				   @param ci children of the element
				   @param append_c if true the children are added otherwise they are replaced
				   @return returns nothing
				    \n \n sample: \n \n
				   @include DimensionAddOrUpdateDimElement/Program.cs
				  */
				void DimensionAddOrUpdateDimElement( String *database, String *dimension, String *element, DimensionAddOrUpdateElementMode mode, DimElementType type, ConsolidationInfo ci[], bool append_c );

				ElementInfo* ElementCreate( String *database, String *dimension, String *element, DimElementType type, ConsolidationInfo ci[]);
				ElementInfo* ElementCreate( String *database, String *dimension, String *element);

				/**@brief add element in dimension or update properties of the element
				   @param database Name of the database
				   @param dimension Name of the dimension
			       @param element Name of the element
				   @param type type of the element
				   @param ci new children of the element 
				   @return returns nothing
				*/
				void DimensionAddOrUpdateDimElement( String *database, String *dimension, String *element, DimElementType type, ConsolidationInfo ci[] );

				/**@brief add element in dimension or update properties of the element
				   @param database Name of the database
				   @param dimension Name of the dimension
			       @param element Name of the element
				   @param mode mode of operation
				   @param type type of the element
				   @return returns nothing
				  */
				void DimensionAddOrUpdateDimElement( String *database, String *dimension, String *element, DimensionAddOrUpdateElementMode mode, DimElementType type );

				/**@brief add numeric non consolidated element in dimension or update properties of the element
				   @param database the selected database
				   @param dimension Name of the dimension
			       @param element Name of the element
				   @returns returns nothing
				  */
				void DimensionAddOrUpdateDimElement( String *database, String *dimension, String *element );

				/**@brief see DimensionAddOrUpdateDimElement */
				void DimensionAddOrUpdateElement( String *database, String *dimension, String *element, AddOrUpdateElementMode mode, DimElementType type, ConsolidationInfo ci[], bool append_c );

				/**@brief see DimensionAddOrUpdateDimElement */
				void DimensionAddOrUpdateElement( String *database, String *dimension, String *element, DimElementType type, ConsolidationInfo ci[] );

				/**@brief see DimensionAddOrUpdateDimElement */
				void DimensionAddOrUpdateElement( String *database, String *dimension, String *element, AddOrUpdateElementMode mode, DimElementType type );

				/**@brief see DimensionAddOrUpdateDimElement */
				void DimensionAddOrUpdateElement( String *database, String *dimension, String *element );

				/**@brief Renames an Element
				  @param database Name of the database
				  @param dimension Name of the dimension
				  @param de_oldname Name of the element
				  @param de_newname New name of the element
				  @return returns nothing
				   \n \n sample: \n \n
				   @include DimElementRename/Program.cs
				 */				 				 				 				 			 				
				void DimElementRename( String *database, String *dimension, String *de_oldname, String *de_newname );

				/**@brief Deletes an element from to the specified dimension
				  @param database Name of the database
				  @param dimension Name of the dimension
				  @param dimension_element Name of the element
				  @return returns nothing
				   \n \n sample: \n \n
				   @include DimElementDelete/Program.cs
				 */				 				 				 				 				 				
				void DimElementDelete( String *database, String *dimension, String *dimension_element );

				void DimElementDeleteMulti( String *database, String *dimension, String *dimension_elements[] );

				/**@brief Moves an element to the specified position in a dimension
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param dimension_element Name of the element
				   @param new_position position the element should be moved to
				   @return returns nothing
				    \n \n sample: \n \n
				   @include DimElementMove/Program.cs
				 */				
				void DimElementMove( String *database, String *dimension, String *dimension_element, unsigned int new_position );

				/**@brief loads the cube
				  @param database Name of the database 
				  @param cube Name of the cube
				  @return return nothing
				   \n \n sample: \n \n
				   @include CubeLoad/Program.cs
				  */
				void CubeLoad( String *database, String *cube );

				/**@brief load the database
				   @param database Name of the database
				   @return return nothing
				    \n \n sample: \n \n
				   @include DatabaseLoad/Program.cs
				  */			
				void DatabaseLoad( String *database );

				/**@brief save cube data to disk
				   @param database Name of the database
				   @param cube Name of the cube
				   @return returns nothing
				    \n \n sample: \n \n
				   @include CubeCommitLog/Program.cs
				 */
				void CubeCommitLog( String *database, String *cube );

				/**@brief save database data to disk 
				   @param database Name of the database
				   @return returns nothing
				    \n \n sample: \n \n
				   @include DatabaseSave/Program.cs
				 */
				void DatabaseSave( String *database );

				/**@brief Unload a cube
				   @param database Name of the database
				   @param cube Name of the cube
				   @return returns nothing
				    \n \n sample: \n \n
				   @include UnloadCube/Program.cs
				 */
				void UnloadCube( String *database, String *cube );

				/**@brief deletes a cube
				   @param database Name of the database
				   @param cube Name of the cube
				   @return returns nothing
				    \n \n sample: \n \n
				   @include DeleteCube/Program.cs
				   */
				void DeleteCube( String *database, String *cube );

				/**@brief shuts down a server
				   @return returns nothing	
				    \n \n sample: \n \n
				   @include ServerShutdown/Program.cs
				  */			
				void ServerShutdown();

				void SetUserPassword( String *user, String *password );

				/**@brief Change password
				   @param oldpassword old password
				   @param oldpassword new password
				   @return returns nothing
				   */
				void ChangePassword( String *oldpassword, String *newpassword );

				/**@brief delete all values of a cube
				   @param database Name of the database
				   @param cube Name of the cube
				   @return returns nothing
				    \n \n sample: \n \n
				   @include CubeClear/Program.cs
				 */			
				void CubeClear( String *database, String *cube );

				/**@brief delete all values of a sub cube
				   @param database Name of the database
				   @param cube Name of the cube
				   @param elements coordinates of the sub cube
				   @return returns nothing
				 */				
				void CubeClear( String *database, String *cube, Array *elements[] );

				/* Rules */

				/**@brief Creates a new rule for the specified cube
				   @param database Name of the database
				   @param cube Name of the cube
				   @param Definiton definition of the rule
				   @param Activate if false, the rule will be dactivated
				   @return information about the rule
				    \n \n sample: \n \n
				   @include RuleCreate/Program.cs
				 */
				RuleInfo RuleCreate( String *database, String *cube, String *Definiton, bool Activate );

				/**@brief Creates a new rule for the specified cube
				   @param database Name of the database
				   @param cube Name of the cube 
				   @param Definiton definition of the rule
				   @param ExternId extern identifier
				   @param Comment Comment
				   @param Activate if false, the rule will be dactivated
				   @return information about the rule 
				 */
				RuleInfo RuleCreate( String *database, String *cube, String *Definiton, String *ExternId, String *Comment, bool Activate  );
				
				/**@brief Modifies a rule for the specified cube
				   @param database Name of the database
				   @param cube Name of the cube
				   @param id id of the rule
				   @param Definiton definition of the rule
				          if null or empty the definition will not be changed
				   @param Activate if false, the rule will be dactivated
				   @return information about the rule 
				    \n \n sample: \n \n
				   @include RuleModify/Program.cs
				 */
				RuleInfo RuleModify( String *database, String *cube, long id, String *Definiton, bool Activate  );
				
				/**@brief Modifies a rule for the specified cube
				   @param database Name of the database
				   @param cube Name of the cube
				   @param id id of the rule
				   @param Definiton definition of the rule
				          if null or empty the definition will not be changed
				   @param ExternId extern identifier
				   @param Comment Comment
				   @param Activate if false, the rule will be dactivated
				   @return information about the rule 
				 */
				RuleInfo RuleModify( String *database, String *cube, long id, String *Definiton, String *ExternId, String *Comment, bool Activate );
				
				RuleInfo RulesActivate( String *database, String *cube, long ids __gc [] , RuleActivationMode mode )[];

				/**@brief retrieve information about the rule
				   @param database Name of the database
				   @param cube Name of the cube
				   @param id id of the rule
				   @return information about the rule 
				    \n \n sample: \n \n
				   @include RuleInformation/Program.cs
				 */
				RuleInfo RuleInformation( String *database, String *cube, long id );

				/**@brief retrieve information about the rule applied to a cell
				   @param database Name of the database
				   @param cube Name of the cube
				   @param coordinates coordinates of the cell
				   @return information about the rule 
				    \n \n sample: \n \n
				   @include CellRuleInformation/Program.cs
				 */
				RuleInfo CellRuleInformation( String *database, String *cube, String *coordinates[]);

				/**@brief parses a rule for the specified cube
				   @param database Name of the database
				   @param cube Name of the cube
				   @param Definiton definition of the rule
				   @return XML representation of the definition
				    \n \n sample: \n \n
				   @include RuleParse/Program.cs
				 */
				String *RuleParse( String *database, String *cube, String *Definiton );
				
				/**@brief deletes a given rule
				   @param database Name of the database
				   @param cube Name of the cube to be queried
				   @param id of the rule
				   @return returns nothing
				    \n \n sample: \n \n
				   @include RuleDelete/Program.cs
				 */				 				
				void RuleDelete( String *database, String *cube, long id );
				
				void RulesDelete( String *database, String *cube, long ids __gc [] );
				
				/**@brief Retrieves a list of the existing rules
			   	   @param database Name of the database
				   @param cube Name of the cube
				   @return Array with information of the rules
				    \n \n sample: \n \n
				   @include ListRules/Program.cs
				 */
				RuleInfo ListRules( String *database, String *cube )[];
				
				/**@brief Retrieves a list of the existing rules functions as XML
				   @return Returns XML representation of the rule function list
				    \n \n sample: \n \n
				   @include ListRuleFunctions/Program.cs
				 */
				String *ListRuleFunctions( );

				/**@brief Retrieves an array with elements which are up to the filter criteria of the subset filter
				   @param database Name of the database
				   @param dimension Name of the dimension 
				   @param indent Indentation level of an element in the consolidation hierarchy. The minimal indention is 1.
				   @param alias Defines Aliases, used in attribute filter
				   @param afilter Struct which takes a subset from the attributes of a cube
				   @param picklist Adds defined elements to a subset
				   @param dfilter Defines a subset, which returns elements from the specified cube and criteria
				   @param sort Sort elements of a subset
				   @param hfilter Filters elements by hierachical level
				   @param tfilter Filters elements by text concetps
				   @return Returns an array with elements, which represents a subset
				*/
				SubsetResult DimensionSubset(	String *database, String *dimension, int indent,
												ArgAliasFilterSettings alias,
												ArgFieldFilterSettings afilter,
												ArgBasicFilterSettings picklist,
												ArgDataFilterSettings dfilter,
												ArgSortingFilterSettings sort,
												ArgStructuralFilterSettings hfilter,
												ArgTextFilterSettings tfilter)[];
				
				/**@brief returns a restricted flat lists of elements contained in the specified dimension 
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param start startposition
				   @param limit at most limit elements
				   @return returns a list of elements
				 */				 			 				 				 
				ElementInfo* DimensionRestrictedFlatListDimElements( String *database, String *dimension, long start, long limit )[];

				/**@brief returns a restricted lists of top elements contained in the specified dimension 
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param start startposition
				   @param limit at most limit elements
				   @return returns a list of elements
				 */				 			 				 				 
				ElementInfo* DimensionRestrictedTopListDimElements( String *database, String *dimension, long start, long limit )[];

				/**@brief returns a restricted lists of children elements contained in the specified dimension 
				   @param database Name of the database
				   @param dimension Name of the dimension
				   @param elementidentifier identifier of parent
				   @param start startposition
				   @param limit at most limit elements
				   @return returns a list of elements
				 */				 			 				 				 
				ElementInfo* DimensionRestrictedChildrenListDimElements( String *database, String *dimension, long elementidentifier, long start, long limit )[];

				size_t DimensionFlatCount( String *database, String *dimension);

				LockInfo CubeLock( String *database, String *cube, Array *elements[]);
				LockInfo CubeLocks( String *database, String *cube)[];
				void CubeRollback( String *database, String *cube, long lockid);
				void CubeRollback( String *database, String *cube, long lockid, long steps);
				void CubeCommit( String *database, String *cube, long lockid);
	
				DrillThroughSet CellDrillThrough( String *database, String *cube, String *coordinates[] , DrillThroughType mode)[];
				DrillThroughSet FastCellDrillThrough( String *database, String *cube, String *coordinates[] , DrillThroughType mode)[];

				SuperVisionServerInfo SVSInformation();

				void PrepareLogout();
				void CubeConvert( String *database, String *cube, CubeType newType);

				static void InitSSL(String *TrustFile);
				static void SetClientVersion(String *client_version);

				bool IsAdminUser();
				bool IsGpuServer();


			protected:
				void static throwException( String *s );
				void static throwException( libpalo_err err );

			private:
				struct conversions convs;
				static String *TrustFile = NULL;

				bool IsGpu;

				void CopyCell( String *database, String *cube, String *from[], String *to[], bool withVal, double dbl_value );
				void CubeClear( String *database, String *cube, unsigned short complete, Array *elements[] );
				inline DimElementType getType( de_type Type );
				RuleInfo getRuleInfo(arg_rule_info_w &ruleinfo);
				LockInfo getLockInfo(arg_lock_info_w &lockinfo);
				inline DimElementInfo2 getElementInfo2(arg_dim_element_info2_w &elementinfo);
				inline void FillElementInfo(ElementInfo &ei, arg_dim_element_info2_raw_w &elementinfo ); 

				ElementInfo* RestrictedList( String *database, String *dimension, long id, long start, long limit )[];

				DatabaseType GetDatabaseTypeHelper( unsigned int type );
				DimensionType GetDimensionTypeHelper( unsigned int type );
				CubeType GetCubeTypeHelper( unsigned int type );
				void DoInit( String *hostname, String *service, String *username, String *pw_hash, String *trust_file );
			};

			private __gc class Helper {
			public:
				static String *CharPtrToStringFree( wchar_t *s );
				static String *ArgStringArrayToManagedFree( struct arg_str_array_w a )[];
				static String *ArgStringArrayToManaged( struct arg_str_array_w a )[];
				static void ManagedToArgStringArray( String *src[], struct arg_str_array_w &dest );
				static void FreeConvertedArgStringArray( struct arg_str_array_w &dest );
				static String *ManagedJaggedArrayMemberToStringArray( Array *a )[];
				static CellValue ArgPaloValueToCellValue( struct arg_palo_value_w pv );
				static String *GetErrorMessage( libpalo_err err );
				static void ConvertStringToUTF8(std::string& To, String * From);
				static String *ConvertUTF8ToString(const std::string& From);
			private:
				static void HandleConversionError(bool ToUtf8);
				static void wcs2utf8( char **utf8_str, const wchar_t *s, size_t len );
				static void utf82wcs( wchar_t **wcs, const char *utf8_str );
			};

			public __gc class ErrorInformation {
			public:
				ErrorInformation(): ErrorMessage(NULL), ErrorCode(0){};
				ErrorInformation(int ArgErrorCode, String* ArgErrorMessage): ErrorMessage(ArgErrorMessage), ErrorCode(ArgErrorCode){};
				String* GetMessage();
				int GetCode();
				void ProcessError(int Originalerror, const std::string& Message);
				void ProcessSocketError(const std::string& Message);
				void ProcessStandardError(const std::string& Message);
				void ProcessUnknownError();
				void Check();
			private:
				String* ErrorMessage;
				int ErrorCode;

				void ProcessErrorCode(int OriginalError);

			};

			public __gc class PaloCommException : public ApplicationException {
			public:
				PaloCommException( String *message );
			};

			public __gc class PaloException : public ApplicationException {
			public:
				PaloException( libpalo_err err );
				PaloException( ErrorInformation* ei );

				int ErrorCode;
				String *PaloMessage;
			};
		}
	}
}
