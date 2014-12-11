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

#pragma once

using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;


namespace Jedox {
	namespace Palo {
		namespace Comm {

			// Begin lib related 
							
			/**@brief gives informations about Api version */
			public value struct ApiInfo {
				unsigned long MajorVersion;
				unsigned long MinorVersion;
				unsigned long BugfixVersion;
				unsigned long BuildNumber;
			};

			// Begin lib related 

			// Begin server related 

			/**@brief gives informations about the server */
			public value struct ServerInfo {
				unsigned long MajorVersion;
				unsigned long MinorVersion;
				unsigned long BugfixVersion;
				unsigned long BuildNumber;
				unsigned long Encryption;
				unsigned int HttpsPort;
				unsigned long DataSequenceNumber;
			};

			/**@enum SVSLoginMode gives information about the SuperVisionServer login mode */
			public enum class SVSLoginMode {
				None = none,
				Information = information,
				Authentification = authentification,
				Authorization = authorization
			};

			/**@brief gives informations about the SuperVisionServer */
			public value struct SuperVisionServerInfo {
				bool SVSActive;
				SVSLoginMode LoginMode;
				bool CubeWorkerActive;
				bool DrillThroughEnabled;
				bool WindowsSsoEnabled;
			};

			/**@enum art of permission you want to ask about */
			public enum struct PermissionArt {
				PermissionArtUser = permission_art_user,
				PermissionArtPassword = permission_art_password,
				PermissionArtGroup = permission_art_group,
				PermissionArtDatabase = permission_art_database,
				PermissionArtCube = permission_art_cube,
				PermissionArtDimension = permission_art_dimension,
				PermissionArtDimensionElement = permission_art_dimension_element,
				PermissionArtCellData = permission_art_cell_data,
				PermissionArtRights = permission_art_rights,
				PermissionArtSystemOperations = permission_art_system_operations,
				PermissionArtEventProcessor = permission_art_event_processor,
				PermissionArtSubsetView = permission_art_sub_set_view,
				PermissionArtUserInfo = permission_art_user_info,
				PermissionArtRule = permission_art_rule,
				PermissionArtDrillthrough = permission_art_drillthrough,
				PermissionArtSteReports = permission_art_ste_reports,
				PermissionArtSteFiles = permission_art_ste_files,
				PermissionArtStePalo = permission_art_ste_palo,
				PermissionArtSteUsers = permission_art_ste_users,
				PermissionArtSteEtl = permission_art_ste_etl,
				PermissionArtSteConns = permission_art_ste_conns,
				PermissionArtSteScheduler = permission_art_ste_scheduler,
				PermissionArtSteLogs = permission_art_ste_logs,
				PermissionArtSteLicenses = permission_art_ste_licenses,
				PermissionArtSteMobile = permission_art_ste_mobile,
				PermissionArtSteAnalyzer = permission_art_ste_analyzer,
				PermissionArtSteSessions = permission_art_ste_sessions,
				PermissionArtSteSettings = permission_art_ste_settings
			};

			/**@enum type of permission */
			public enum struct PermissionType {
				PermissionTypeUnknown = permission_type_unknown,
				PermissionTypeNone = permission_type_none,
				PermissionTypeRead = permission_type_read,
				PermissionTypeWrite = permission_type_write,
				PermissionTypeDelete = permission_type_delete,
				PermissionTypeSplash = permission_type_splash
			};

			// End server related 

			// Begin Database related 

			/**@enum DatabaseStatus gives information about the status of the DB */
			public enum class DatabaseStatus {
				UnloadedDatabase = unloaded_db,
				LoadedDatabase = loaded_db,
				ChangedDatabase = changed_db,
				LoadingDatabase = loading_db,
				UnknownDatabaseStatus = unknown_db_status
			};
			
			/**@enum DatabaseType contains informations about the type of the specific database */
			public enum class DatabaseType {
				NormalDatabase = normal_db,
				SystemDatabase = system_db,
				UserinfoDatabase = user_info_db,
				UnknownDatabaseType = unknown_db_type
			};

			/**@brief Contains detailed information about the database 
			   e.g. the number of dimensions */			 
			public value struct DatabaseInfo {
				long id;
				String^ Name;
				size_t NumberDimensions;
				size_t NumberCubes;
				DatabaseStatus Status;
				DatabaseType Type;
			};

			// End Database related 

			// Begin Dimension related 

			/**@enum DimensionType enumerates the types of dimensions */
			public enum class DimensionType {
				NormalDimension = normal_dim,
				SystemDimension = system_dim,
				AttributeDimension = attribut_dim,
				UserInfoDimension = user_info_dim,
				SystemIdDimension = system_id_dim,
				UnknownDimensionType = unknown_dim_type
			};
			
			/**@brief gives informations about the specific dimension */
			public value struct DimensionInfoSimple {
				long id;
				String^ Name;
				long AssocDimensionId;
				long AttributeCubeId;
				long RightsCubeId;
				size_t NumberElements;
				size_t MaximumLevel;
				size_t MaximumIndent;
				size_t MaximumDepth;
				DimensionType Type;
			};

			// End Dimension related 

			// Begin Element related 

			/**@enum DimElementType gives information about the type of elements */
			public enum class DimElementType {
				DimElementTypeNumeric = de_n,
				DimElementTypeString = de_s,
				DimElementTypeConsolidated = de_c,
				DimElementTypeRule = de_r
			};

			/**@brief contains the consolidation info of an element */
			public value struct ConsolidationInfo {
				ConsolidationInfo(String^ name, double factor) : Name(name), Factor(factor) {}

				String^ Name;
				DimElementType Type;
				double Factor;
			};

			/**@brief contains the extended consolidation info of an element */
			public value struct ConsolidationExtendedInfo {
				long Identifier;
				String^ Name;
				DimElementType Type;
				double Factor;
			};

			/**@brief contains informations about the parent element */
			public value struct ParentInfoRaw {
				long Identifier;
			};

			/**@brief contains information about the child element */
			public value struct ChildInfoRaw {
				long Identifier;
				double Factor;
			};

			/**@brief contains Info about a specific element  */
			public ref class ElementInfo {
			public:
				ElementInfo() : Identifier(-1) {}
				ElementInfo(String^ name) : Name(name), NumberParents(0), NumberChildren(0) {}

				long Identifier;
				String^ Name;
				size_t Position;
				size_t Level;
				size_t Indent;
				size_t Depth;
				DimElementType Type;
				size_t NumberParents;
				array<ParentInfoRaw>^ Parents;
				size_t NumberChildren;
				array<ChildInfoRaw>^ Children;
			};

			/**@enum AddOrUpdateElementMode enumerates several Add or Update types */
			public enum class AddOrUpdateElementMode {
				AddOrUpdateElementModeAdd = dimension_add_or_update_element_mode_add,
				AddOrUpdateElementModeForceAdd = dimension_add_or_update_element_mode_force_add,
				AddOrUpdateElementModeUpdate = dimension_add_or_update_element_mode_update,
				AddOrUpdateElementModeAddOrUpdate = dimension_add_or_update_element_mode_add_or_update
			};

			// End Element related 

			// Begin Subset related

			/**@enum AliasFilterFlags enumerates several alias filter flags */
			public enum class AliasFilterFlags {
				SearchOne = search_one, // search one attribute for an alias -- DEFAULT show this field in excel
				SearchTwo = search_two, // search two attributes for an alias -- show this field in excel
				HideDouble = hide_double, // hide double aliases, aliases don't have to be unique !! (Makes not really sense)
				DisplayAlias = display_alias,//Choose an attribute-dimension that serves as alias pool and show the aliases instead
				UseFilterExp = use_filter_exp // use advanced filter expressions for attribute-values
			};

			/**@enum DataFilterFlags enumerates several data filter flags */
			public enum class DataFilterFlags {
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

			/**@enum PickListFlags enumerates several picklist filter flags */
			public enum class PickListFlags {
				InsertBack = insert_back, // Put manually picked elements after the others. Default: before
				MergeElements = merge_elements, // Put manually picked elements before the others
				Sub = sub,	 // Use the picklist as a filter, not as a union.
				InsertFront = insert_front // Default value, put manually picked elements before the others
			};

			/**@enum SortingFilterFlags enumerates several sorting filter flags */
			public enum class SortingFilterFlags {
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
				ConsolidatedOrder = consolidated_order, //Sort by consolidation order
				ElementPath = element_path //Return also path
			};

			/**@enum StructuralFilterFlags enumerates several structural filter flags */
			public enum class StructuralFilterFlags {
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

			/**@enum TextFilterFlags enumerates several text filter flags */
			public enum class TextFilterFlags {
				AdditionalFields = additional_fields, // use additional columns for searching uses the attribute-columns of this dimension per default
				Extended = extended // do not use POSIX-Basic expressions but use Perl-Extended regular expressions
			};

			/**
			@brief decides whether attributes and wich one should be in return value of method DimensionSubset
			@param Active decides whether filter is used
			@param AliasFilterFlags 
						- SearchOne: search one attribute for an alias -- DEFAULT show this field in excel
						- SearchTwo: search two attributes for an alias -- show this field in excel
						- HideDouble: hide double aliases, aliases don't have to be unique !! (Makes not really sense)
						- DisplayAlias: Choose an attribute-dimension that serves as alias pool and show the aliases instead
						- UseFilterExp: use advanced filter expressions for attribute-values
			@param Advanced Array of string arrays with count 2 the first string of the inner array specify the attribute 
						and the second the condition (e.g. ">5000")
			*/	
			public value struct ArgAliasFilterSettings {
				bool Active;
				AliasFilterFlags Flags;
				String^ Attribute1;
				String^ Attribute2;
			};

			/**
			@brief affects the selection of elements based on conditions for attributes
			@param Active  decides whether filter is used
			@param AliasFilterFlags 
						- SearchOne: search one attribute for an alias -- DEFAULT show this field in excel
						- SearchTwo: search two attributes for an alias -- show this field in excel
						- HideDouble: hide double aliases, aliases don't have to be unique !! (Makes not really sense)
						- DisplayAlias: Choose an attribute-dimension that serves as alias pool and show the aliases instead
						- UseFilterExp: use advanced filter expressions for attribute-values
			@param Advanced Array of string arrays with count 2 the first string of the inner array specify the attribute 
						and the second the condition (e.g. ">5000")
				\n \n sample: \n \n
			   @include Afilter/Program.cs
			*/	
			public value struct ArgFieldFilterSettings {
				bool Active;
				AliasFilterFlags Flags;
				array<array<String^>^>^ Advanced;
			};

			/**
			@brief affects how manually selected elements are added to the selection
			@param Active decides whether filter is used
			@param Flags 
						- InsertFront: Default value, put manually picked elements before the others
						- MergeElements: orders basic elements to the other elements \n
						- InsertBack: put picked elements after the others
						- Sub: Use the picklist as a filter, not as a union
			@param ManualSubsetSet Have to be true, to add picklist to the subset
			@param Manualsubset Array with manual picked element names
			  \n \n sample: \n \n
			@include Picklist/Program.cs
			*/
			public value struct ArgBasicFilterSettings {
				bool Active;
				PickListFlags Flags;
				bool ManualSubsetSet;
				array<String^>^ ManualSubset;
			};

			/**@brief helper type used in ArgDataFilterSettings */
			public value struct DataComparison {
				bool UseStrings;
				String^ Op1;
				double Par1d;
				String^ Par1s;
				String^ Op2;
				double Par2d;
				String^ Par2s;
			};

			/**@brief helper type used in ArgDataFilterSettings */
			public value struct BoolStringArray {
				bool BoolVal;
				array<String^>^ Str;
			};

			/**
			@brief affects the selection of elements based on data of a cube
			@param Active decides whether filter is used
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
			@param Cube name of the cube 
			@param Cmp 
				   - bool UseStrings: If set to false double values (Par1d, Par2d) are used otherwise string values (Par1s , Par2s)
				   - String^ Op1 - Operator for the limitation of the selection of elements. Possible Operators: "=",">=",">","<=","=","<>".
				   - double Par1d - Value that relates to Op1
				   - String^ Par1s - Value that relates to Op1
				   - String^ Op2 - Operator for the limitation of the selection of elements. Possible Operators: "=",">=",">","<=","=","<>".
				   - double Par2d - Value that relates to Op2
				   - String^ Par2s - Value that relates to Op2
			@param Coordset Have to be true, to use the coordinates of the cube
			@param Coords Array with coordinats of the cube
			@param UpperPercentageSet Have to be true to use this UpperPercentage filter
			@param LowerPercentageSet Have to be true to use this LowerPercentage filter
			@param UpperPercentage Only Products in the defined area will be shown
			@param LowerPercentage Only Products in the defined area will be shown
			@param Top Only top elements will be shown
				\n \n sample: \n \n
			@include DFilter/Program.cs
			*/
			public value struct ArgDataFilterSettings {
				bool Active;
				DataFilterFlags Flags;
				String^ Cube;
				DataComparison Cmp;
				bool CoordsSet;
				array<BoolStringArray>^ Coords;
				bool UpperPercentageSet;
				bool LowerPercentageSet;
				double UpperPercentage;
				double LowerPercentage;
				int Top;
			};

			/**
			@brief affects the sorting of the selection
			@param Active decides whether filter is used
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
					 - ConsolidatedOrder : sort by consolidation order
			@param Attribute name of the Attribute
			@param Level Indention level of elements, which should be sort (minimum indention is 1)
			  \n \n sample: \n \n
			@include Sort/Program.cs
			*/
			public value struct ArgSortingFilterSettings {
				bool Active;
				SortingFilterFlags Flags;
				String^ Attribute;
				int Level;
				unsigned int Limit_count;
				unsigned int Limit_start;
			};

			/**
			@brief affects the selection of elements based on type and hierachical level
			@param Active decides whether filter is used
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
			@param RevolveElement name of the Element, where to start revolving 
			@param RevolveCount Length of the resulting elementlist
			*/
			public value struct ArgStructuralFilterSettings {
				bool Active;
				StructuralFilterFlags Flags;
				String^ Bound;
				bool Level;
				int LevelStart;
				int LevelEnd;
				bool Revolve;
				String^ RevolveElement;
				int RevolveCount;
			};

			/**
			@brief affects the selection of elements based on string pattern
			@param Active decides whether filter is used
			@param Flags 
					 - AdditionalFields: use additional columns for searching uses the attribute-columns of this dimension per default
					 - Extended: do not use POSIX-Basic expressions but use Perl-Extended regular expressions
			@param RegularExpressions: String pattern, after the elements will be selected
			  \n \n sample: \n \n
			@include TFilter/Program.cs
			*/
			public value struct ArgTextFilterSettings {
				bool Active;
				TextFilterFlags Flags;
				array<String^>^ RegularExpressions;
			};

			/**@brief information of one returned subset element*/
			public value struct SubsetResult {
				String^ Name;
				String^ Alias;
				String^ Path;
				size_t Index;
				size_t Depth;
				long Identifier;
				bool HasChildren;
			};

			// End Subset related

			// Start Cube related

			/**@enum CubeStatus enumerates the cube status e. g. unloaded cube*/
			public enum class CubeStatus {
				UnloadedCube = unloaded_cube,
				LoadedCube = loaded_cube,
				ChangedCube = changed_cube,
				UnknownCubeStatus = unknown_cube_status
			};
			
			/**@enum CubeType contains the type of a cube e. g. attribute cube */
			public enum class CubeType {
				NormalCube = normal_cube,
				SystemCube = system_cube,
				AttributeCube = attribut_cube,
				UserInfoCube = user_info_cube,
				GpuCube = gpu_cube,
				UnknownCubeType = unknown_cube_type
			};

			/**@brief contains several informations about a cube */
			public value struct CubeInfo {
				long id;
				String^ Name;
				size_t NumberDimensions;
				array<String^>^ Dimensions;
				long double NumberCells;
				long double NumberFilledCells;
				CubeStatus Status;
				CubeType Type;
			};

			/**@enum RuleActivationMode contains the mdoe of rule activation */
			public enum class  RuleActivationMode {
				RuleModeDeactivate = rule_mode_deactivate,
				RuleModeActivate = rule_mode_activate,
				RuleModeToggle = rule_mode_toggle
			};

			// End Cube related

			// Start Cell related

			/**@enum DrillThroughType contains the type of a drill through */
			public enum class DrillThroughType {
				History = history, 
				Details = details
			};

			/**@brief contains informations about one returned drill through line */
			public value struct DrillThroughSet {
				array<String^>^ Items;
			};

			/**@brief contains information about a rule */
			public ref class RuleInfo {
			public:
				long id;
				String^ definition;
				String^ extern_id;
				String^ comment;
				unsigned long long timestamp;
				bool activated;
				double position;
			};

			/**@brief contains information about a lock */
			public ref struct LockInfo {
				long id;
				array<array<String^>^>^ area;
				String^ user;
				unsigned long steps;
			};

			/**@enum SplashMode enumerates several splash modes */
			public enum class SplashMode {
				SplashModeDisable = splash_mode_disable,
				SplashModeDefault = splash_mode_default,
				SplashModeBaseSet = splash_mode_base_set,
				SplashModeBaseAdd = splash_mode_base_add
			};

			/**@enum CopyCellMode enumerates several copyCell modes */
			public enum class CopyCellMode {
				CopyCellDefault = cell_copy_default,
				CopyCellPredictLinearRegression = cell_copy_predict_linear_regression
			};

			/**@enum GoalSeekMode enumerates several goalseek modes */
			public enum class GoalSeekMode {
				GoalSeekComplete = goalseek_complete,
				GoalSeekEqual = goalseek_equal,
				GoalSeekRelative = goalseek_relative
			};

			/**@enum CellValueType enumerates types of values in cells*/
			public enum class CellValueType {
				CellValueTypeString,
				CellValueTypeDouble,
				CellValueTypeError
			};

			/**@brief contains the value of a cell */
			public value struct CellValueValue {
				String^ StrValue;
				double DblValue;
			};

			/**@brief the value and type of a cell  */
			public value struct CellValue {
				CellValueType Type;
				CellValueValue Value;
			};

			// End Cell related

			// Begin Export related

			/**@enum CompareOp enumerates Operators to specialize the data export: 
			 (Use "gt" for "greater than" "gte" for "greater or equal", 
			 "lt" for "less than", "lte" for "less or equal", 
			 "eq" for "qual" or "neq" for "not equal" */
			public enum class CompareOp {
				CompareOpGT,
				CompareOpLT,
				CompareOpGTE,
				CompareOpLTE,
				CompareOpEQ,
				CompareOpNEQ,
				CompareOpTRUE
			};

			/**@enum BoolOp enumerates the boolean operators*/
			public enum class BoolOp {
				BoolOpAND,
				BoolOpOR,
				BoolOpXOR
			};

			/**@brief contains opportunities to filter the values*/
			public value struct ValueFilter {
				CompareOp CmpOp1;
				CellValue Value1;
				BoolOp AndOr12;
				CompareOp CmpOp2;
				CellValue Value2;
			};

			/**@brief describes the export options*/
			public value struct GetDataExportOptions {
				bool BaseElementsOnly;
				bool IngoreEmptyCells;
				ValueFilter Filter;
				array<String^>^ LastCoordinates;
				unsigned long NumDatasets;
			};

			/**@brief the data at specified coordinate */
			public value struct Dataset {
				array<String^>^ Coordinates;
				CellValue Value;
			};

			// End Export related

			// Begin SetDataMulti related

			// @@@ Man kann wahrscheinlich dies durch Dataset ersetzen

			/**@brief the value for a cell, d is used exactly when s is null */
			public value struct ValueSet {
				String^ s;
				double d;
			};

			/**@brief the value at a specified coordinate */
			public value struct DataSetMulti {
				array<String^>^ Coordinates;
				ValueSet Value;
			};

			// End SetDataMulti related


			/**@brief This class is the main class to use for communication 
			   with the Palo server */			
			[ComVisible(true)]
			public ref class Connection {
			public:
				/**@brief constructor
				   @param hostname hostname or ip of the server 
				   @param service  used port of the server
				   @param username username used for this connection
				   @param password password used for this connection
				 */
				Connection(String^ hostname, String^ service, String^ username, String^ password);

				/**@brief constructor
				   @param hostname hostname or ip of the server 
				   @param service  used port of the server
				   @param username username used for this connection
				   @param password password used for this connection
				   @param trust_file absolute path to a file with list of trusted CAs in pem format
				 */
				Connection(String^ hostname, String^ service, String^ username, String^ password, String^ trust_file) ;

				/**@brief destructor
					implicitly called by dispose.
				 */
				~Connection(void);

				// Begin Lib related

				/**@brief (static) Retrieves Information about the Api
				   @return Returns Information about the Api
				 */
				static ApiInfo ApiInformation();

				/**@brief (static) Set internal client_version
				   @param client_version the client_version which should be shown
				   @return returns nothing	
				 */
				static void SetClientVersion(String^ client_version);

				/**@brief (static) Set the list of truted CAs.
				   @param trust_file absolute path to a file with list of trusted CAs in pem format
				   @return returns nothing
				 */
				static void InitSSL(String^ TrustFile);

				// End Lib related

				// Begin Server related

				/**@brief (static) Test connection parameters.
				   @param hostname hostname or ip of the server 
				   @param service  used port of the server
				   @param username username used for this connection
				   @param password password used for this connection
				   @return returns nothing
				 */
				static void TestConnection(String^ hostname, unsigned int port, String^ username, String^ pw_hash);
				
				/**@brief (static) Test connection parameters.
				   @param hostname hostname or ip of the server 
				   @param service  used port of the server
				   @param username username used for this connection
				   @param password password used for this connection
				   @param trust_file absolute path to a file with list of trusted CAs in pem format
				   @return returns nothing
				 */
				static void TestConnection(String^ hostname, unsigned int port, String^ username, String^ pw_hash, String^ trust_file);

				/**@brief checks validity of internal stored pointers
				   @return returns nothing
				 */
				void CheckValidity();

				/**@ obsolete
				   @return returns nothing	
				 */
				void PrepareLogout();

				/**@brief checks if used user is admin.
				   @return returns result of check
				 */
				bool IsAdminUser();

				/**@brief checks if used user is admin.
				   @return returns result of check
				 */
				bool IsGpuServer();

				/**@brief Retrieves Information about the server
				   @return Returns Information about the server
				 */
				ServerInfo ServerInformation();

				/**@brief Retrieves Information about the SuperVisionServer
				   @return Returns Information about the SuperVisionServer
				 */
				SuperVisionServerInfo SVSInformation();

				/**@brief checks if metadatacache is still valid and invalidate it otherwise.
				   @return returns nothing
				 */
				void Ping();

				/**@brief shuts down a server
				   @return returns nothing	
					\n \n sample: \n \n
				   @include ServerShutdown/Program.cs
				  */			
				void ServerShutdown();

				/**@brief Set password for user
				   @param username username for which password should be set
				   @param password password to be set
				   @return returns nothing
				   */
				void SetUserPassword(String^ user, String^ password);

				/**@brief Change password
				   @param oldpassword old password
				   @param oldpassword new password
				   @return returns nothing
				   */
				void ChangePassword(String^ oldpassword, String^ newpassword);

				/**@brief Retrieves a list of the existing rules functions as XML
				   @return Returns XML representation of the rule function list
					\n \n sample: \n \n
				   @include ListRuleFunctions/Program.cs
				 */
				String^ ListRuleFunctions();

				/**@brief get permission type for specific permission art
				   @param permissionart art of permission 
				   @return type of permsission for selected permission art
				   */
				PermissionType GetRights(PermissionArt permissionart);

				// End Server related

				// Start Database related

				/**@brief load the database
				   @param database name of the database
				   @return return nothing
					\n \n sample: \n \n
				   @include DatabaseLoad/Program.cs
				  */			
				void DatabaseLoad(String^ database);

				/**@brief unload the selected database
				   @param database name of database
				   @return returns nothing
					\n \n sample: \n \n
				   @include UnloadDatabase/Program.cs
				  */				
				void UnloadDatabase(String^ database);

				/**@brief save database data to disk 
				   @param database name of the database
				   @return returns nothing
					\n \n sample: \n \n
				   @include DatabaseSave/Program.cs
				 */
				void DatabaseSave(String^ database);

				/**@brief Adds a databases 
				   @param database name of the database
				   @return returns nothing
					\n \n sample: \n \n
				   @include AddDatabase/Program.cs
				 */				
				void AddDatabase(String^ database);

				/**@brief Deletes the specified database
				   @param database name of the database
				   @return returns nothing 
					\n \n sample: \n \n
				   @include DeleteDatabase/Program.cs
				   */
				void DeleteDatabase(String^ database);

				/**@brief lists all databases 
					\n \n sample: \n \n
				   @include RootListDatabases/Program.cs
				*/		
				array<String^>^ RootListDatabases();

				/**@brief lists all databases of a type	
				 */				
				array<String^>^ RootListDatabases(DatabaseType Type);

				/**@brief retrieves the type of the specified database
				   @param database name of the database	
				   @return returns the type of the database
					\n \n sample: \n \n
				   @include GetDatabaseType/Program.cs
				  */			
				DatabaseType GetDatabaseType(String^ database);

				/**@brief retrieves information about the specified database
				   @param database name of the database
				   @return returns information about the specified database
					\n \n sample: \n \n
				   @include DatabaseInformation/Program.cs
				  */				
				DatabaseInfo DatabaseInformation(String^ database);

				// End Database related

				// Start Dimension related

				/************************************************************************/
				/* lookup helpers                                                       */
				/************************************************************************/

				/**@brief retrieves name of a dimension for a given id
				   @param database name of the database
				   @param id id of the dimension
				   @param 
				   @return name of the dimension
				  */				
				String^ GetDimensionNameFromID(String^ database, long id);

				/**@brief Adds a dimension to to the specified database
				   @param database name of the database 
				   @param dimension name of the dimension
				   @return returns nothing
				   \n \n sample: \n \n
				   @include DatabaseAddDimension/Program.cs
				 */				 			 				 								
				void DatabaseAddDimension(String^ database, String^ dimension);

				/**@brief Adds a dimension to to the specified database
				   @param database name of the database 
				   @param dimension name of the dimension
				   @param type type of the dimension	(NormalDimension or UserInfoDimension)
				   @return returns nothing
				   \n \n sample: \n \n
				   @include DatabaseAddDimension/Program.cs
				 */				 			 				 								
				void DatabaseAddDimension(String^ database, String^ dimension, DimensionType type);

				/**@brief Renames a dimension
				   @param database name of the database 
				   @param dimension_oldname name of the dimension
				   @param dimension_newname New name of the dimension	
				   @return returns nothing
				   \n \n sample: \n \n
				   @include DatabaseRenameDimension/Program.cs
				 */				 				 		 				 				
				void DatabaseRenameDimension(String^ database, String^ dimension_oldname, String^ dimension_newname);

				/**@brief deletes the selected dimension
				   @param database name of the database
				   @param dimension name of the dimension
				   @return returns nothing
				   \n \n sample: \n \n
				   @include DeleteDimension/Program.cs
				 */				 				 				 				
				void DeleteDimension(String^ database, String^ dimension);

				/**@brief deletes all elements of the given dimension
				   @param database name of the database
				   @param dimension name of the dimension
				   @return returns nothing
				   \n \n sample: \n \n
				   @include ClearDimension/Program.cs
				 */				 				 				 				
				void ClearDimension(String^ database, String^ dimension);

				/**@brief retrieves the type of the specified dimension
				   @param database name of the database
				   @param dimension name of the dimension
				   @return returns the type of the dimension
					\n \n sample: \n \n
				   @include GetDimensionType/Program.cs
				  */
				DimensionType GetDimensionType(String^ database, String^ dimension);
				
				/**@brief retrieves inormation about the specified dimension
				   @param database name of the database
				   @param dimension name of the dimension
				   @return returns information about the specified dimension
					\n \n sample: \n \n
				   @include DimensionInformationSimple/Program.cs
				*/
				DimensionInfoSimple DimensionInformationSimple(String^ database, String^ dimension);

				/**@brief retrieves the name of the attribute dimension
				   @param database name of the database
				   @param dimension name of the dimension
				   @return returns the name of the attribute dimension
					\n \n sample: \n \n
				   @include GetAttributeDimension/Program.cs
				  */			
				String^ GetAttributeDimension(String^ database, String^ dimension);

				/**@brief retrieves 1. dimension, that contains all elements according to dimension_elements
				   @param database name of the database
				   @param dimension_elements Names of the elements
				   @param should_be_unique if true, throws an exception, if there's a second dimension,
					that contains all elements according to dimension_elements
				   @return name of the specified dimension
					\n \n sample: \n \n
				   @include ElementDimension/Program.cs
				 */
				String^ ElementDimension(String^ database, array<String^>^ dimension_elements, bool should_be_unique);
				
				/**@brief retrieves first dimension of the cube, that contains all elements according to dimension_elements
				   @param database name of the database
				   @param cube name of the cube
				   @param dimension_elements Names of the elements
				   @param should_be_unique if true, throws an exception, if there's a second dimension of the cube, that contains all elements according to dimension_elements
				   @return name of the specified dimension
					\n \n sample: \n \n
				   @include ElementCubeDimension/Program.cs
				  */				
				String^ ElementCubeDimension(String^ database, String^ cube, array<String^>^ dimension_elements, bool should_be_unique);

				/**@brief lists all dimensions in the specified database
				   @param database name of the database
				   @return List of dimensions
					\n \n sample: \n \n
				   @include DatabaseListDimensions/Program.cs
				 */				 				 				
				array<String^>^ DatabaseListDimensions(String^ database);

				/**@brief Lists all dimensions of a type in the specified database
				   @param database name of the database
				   @param Type of dimensions
				   @return List of dimensions
					\n \n sample: \n \n
				 */	
				array<String^>^ DatabaseListDimensions(String^ database, DimensionType Type);

				// End Dimension related

				// Start Element related

				/************************************************************************/
				/* lookup helpers                                                       */
				/************************************************************************/

				/**@brief retrieves name of an element for a given id
				   @param database name of the database
				   @param dimension name of the dimension
				   @param id id of the element
				   @return name of the element
				  */				
				String^ GetElementNameFromID(String^ database, String^ dimension, long id);

				/**@brief retrieves type of an element for a given id
				   @param database name of the database
				   @param dimension name of the dimension
				   @param id id of the element
				   @return type of the element
				  */				
				DimElementType GetElementTypeFromID(String^ database, String^ dimension, long id);

				/**@brief retrieves position of an element for a given id
				   @param database name of the database
				   @param dimension name of the dimension
				   @param id id of the element
				   @return position of the element
				  */				
				unsigned int GetElementPositionFromID(String^ database, String^ dimension, long id);

				/**@brief retrieves id of an element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param element name of the element
				   @return id
				  */				
				long GetElementIDFromName(String^ database, String^ dimension, String^ element);

				/**@brief creates an element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param element name of the element
				   @return information of element
				  */				
				ElementInfo^ ElementCreate(String^ database, String^ dimension, String^ element);

				/**@brief creates an element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param element name of the element
				   @param type type of the element
				   @param ci children of the element 
				   @return information of element
				  */				
				ElementInfo^ ElementCreate(String^ database, String^ dimension, String^ element, DimElementType type, array<ConsolidationInfo>^ ci);

				/**@brief Creates several elements in the specified dimension
				   @param database name of the database
				   @param dimension name of the dimension
				   @param elements names of the elements
				   @param type type of the elements
				   @param children children of the elements
				   @param weights weights of the children
				   @return returns nothing
				 */				 				 				 				 				 				
				void ElementCreateMulti(String^ database, String^ dimension, array<String^>^ elements, int type, array<array<String^>^>^ children, array<array<double>^>^ weights);

				/**@brief check if an element exists
				   @param database name of the database
				   @param dimension name of the dimension
				   @param element name of the element to check
				   @return information of element
				  */				
				bool ElementExists(String^ database, String^ dimension, String^ element);

				/**@brief add element in dimension or update properties of the element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param element name of the element
				   @param mode mode of operation
				   @param type type of the element
				   @param ci children of the element
				   @param append_c if true the children are added otherwise they are replaced
				   @return returns nothing
					\n \n sample: \n \n
				   @include DimensionAddOrUpdateElement/Program.cs
				  */
				void DimensionAddOrUpdateElement(String^ database, String^ dimension, String^ element, AddOrUpdateElementMode mode, DimElementType type, array<ConsolidationInfo>^ ci, bool append_c);

				/**@brief add element in dimension or update properties of the element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param element name of the element
				   @param type type of the element
				   @param ci new children of the element 
				   @return returns nothing
				*/
				void DimensionAddOrUpdateElement(String^ database, String^ dimension, String^ element, DimElementType type, array<ConsolidationInfo>^ ci);

				/**@brief add element in dimension or update properties of the element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param element name of the element
				   @param mode mode of operation
				   @param type type of the element
				   @return returns nothing
				  */
				void DimensionAddOrUpdateElement(String^ database, String^ dimension, String^ element, AddOrUpdateElementMode mode, DimElementType type);

				/**@brief add numeric non consolidated element in dimension or update properties of the element
				   @param database the selected database
				   @param dimension name of the dimension
				   @param element name of the element
				   @returns returns nothing
				  */
				void DimensionAddOrUpdateElement(String^ database, String^ dimension, String^ element);

				/**@brief Renames an Element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param de_oldname name of the element
				   @param de_newname New name of the element
				   @return returns nothing
				   \n \n sample: \n \n
				   @include DimElementRename/Program.cs
				 */				 				 				 				 			 				
				void DimElementRename(String^ database, String^ dimension, String^ de_oldname, String^ de_newname);

				/**@brief Deletes an element from to the specified dimension
				   @param database name of the database
				   @param dimension name of the dimension
				   @param dimension_element name of the element
				   @return returns nothing
				   \n \n sample: \n \n
				   @include DimElementDelete/Program.cs
				 */				 				 				 				 				 				
				void DimElementDelete(String^ database, String^ dimension, String^ dimension_element);

				/**@brief Deletes several elements from to the specified dimension
				   @param database name of the database
				   @param dimension name of the dimension
				   @param elements Names of the elements
				   @return returns nothing
				 */				 				 				 				 				 				
				void DimElementDeleteMulti(String^ database, String^ dimension, array<String^>^ elements);

				/**@brief Moves an element to the specified position in a dimension
				   @param database name of the database
				   @param dimension name of the dimension
				   @param dimension_element name of the element
				   @param new_position position the element should be moved to
				   @return returns nothing
					\n \n sample: \n \n
				   @include DimElementMove/Program.cs
				 */				
				void DimElementMove(String^ database, String^ dimension, String^ dimension_element, unsigned long new_position);

				/**@brief Returns the type of the specified element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param dimension_element name of the element
				   @return returns the Type of the element
					\n \n sample: \n \n
				   @include ElementType/Program.cs
				 */  				 
				DimElementType ElementType(String^ database, String^ dimension, String^ dimension_element);

				/**@brief retrieves inormation about the specified element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param element name of the element
				   @return returns information about the specified element
					\n \n sample: \n \n
				   @include ElementInformationSimple/Program.cs
				*/
				ElementInfo^ ElementInformationSimple(String^ database, String^ dimension, String^ element);

				/**@brief Returns the name of the n'th element. starting at 1.
				   @param database name of the database
				   @param dimension name of the dimension
				   @param n ppsition of the element in dimension 
				   @return returns the name of the specified element				
					\n \n sample: \n \n
				   @include ElementName/Program.cs
				 */				
				String^ ElementName(String^ database, String^ dimension, unsigned long n);

				/**@brief Checks if the specified element is a child of the other specified element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param de_parent name of the element which should be the parentelement
				   @param de_child name of the element which should be the childelement 
				   @return bool TRUE, if the parent element contains the specified child element
					\n \n sample: \n \n
				   @include ElementIsChild/Program.cs
				*/				
				bool ElementIsChild(String^ database, String^ dimension, String^ de_parent, String^ de_child);

				/**@brief Retrieves the count of parents of the specified element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param dimension_element name of the element
				   @return Count of the parent elements
					\n \n sample: \n \n
				   @include ElementParentCount/Program.cs
				 */				 				 
				unsigned int ElementParentCount(String^ database, String^ dimension, String^ dimension_element);


				/**@brief Returns the count of children of the specified elements
				   @param database name of the database
				   @param dimension name of the dimension
				   @param dimension_element name of the element
				   @return count of the child elements
					\n \n sample: \n \n
				   @include ElementChildCount/Program.cs
				 */				 				 				 				 				
				unsigned int ElementChildCount(String^ database, String^ dimension, String^ dimension_element);

				/**@brief Returns the consolidation factor of the child element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param de_parent name of the element
				   @param de_child name of the child element
				   @return returns the Factor of consolidation
					\n \n sample: \n \n
				   @include ElementWeight/Program.cs
				*/
				double ElementWeight(String^ database, String^ dimension, String^ de_parent, String^ de_child);

				/**@brief Retrieves the n'th parent name of the specified element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param dimension_element name of the element
				   @param n Number of the parent to retrieve
				   @return returns the name of the element
					\n \n sample: \n \n
				   @include ElementParentName/Program.cs
				*/
				String^ ElementParentName(String^ database, String^ dimension, String^ dimension_element, unsigned long n);

				 /**@brief Retrieves the name of the n'th child of an element
					@param database name of the database
					@param dimension name of the dimension
					@param dimension_element name of the element
					@param n Number of the cild element to retrieve
					@return returns the name of the child element
					 \n \n sample: \n \n
				   @include ElementChildName/Program.cs
				  */
				String^ ElementChildName(String^ database, String^ dimension, String^ dimension_element, unsigned long n);

				/**@brief Returns the position in the dimension of the specified elements
				   @param  database name of the database
				   @param  dimension name of the dimension
				   @param  dimension_element name of the element
				   @return Position of the element
					\n \n sample: \n \n
				   @include ElementIndex/Program.cs
				 */				 			
				unsigned int ElementIndex(String^ database, String^ dimension, String^ dimension_element);

				/**@brief Retrieves the level of an element in the consolidation hierarchy. The minimal level is 1.
				   @param database name of the database
				   @param dimension name of the dimension
				   @param dimension_element name of the element
				   @return returns the Level of element
					\n \n sample: \n \n
				   @include ElementLevel/Program.cs
				 */				 				 				 				 				
				unsigned int ElementLevel(String^ database, String^ dimension, String^ dimension_element);

				/**@brief Retrieves the indentation level of an element in the consolidation hierarchy. The minimal indention is 1.
				   @param database name of the database
				   @param dimension name of the dimension
				   @param dimension_element name of the element
				   @return returns the Level of indention
					\n \n sample: \n \n
				   @include ElementIndent/Program.cs
				 */				
				unsigned int ElementIndent(String^ database, String^ dimension, String^ dimension_element);

				/**@brief Returns the n'th sibling of an element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param dimension_element name of the element
				   @param n Offset of the sibling (can be negative)
				   @return returns the name of the n'th sibling
					\n \n sample: \n \n
				   @include ElementSibling/Program.cs
				*/
				String^ ElementSibling(String^ database, String^ dimension, String^ dimension_element, long n);

				/**@brief Retrieves the first element from the specified dimension
				   @param database name of the database
				   @param dimension name of the dimension
				   @return returns the name of the first element in specified dimension
					\n \n sample: \n \n
				   @include ElementFirst/Program.cs
				*/
				String^ ElementFirst(String^ database, String^ dimension);

				/**@brief Retrieves the next element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param dimension_element name of the element
				   @return returns the name of the next element
					\n \n sample: \n \n
				   @include ElementNext/Program.cs
				*/
				String^ ElementNext(String^ database, String^ dimension, String^ dimension_element);

				/**@brief Retrieves the previous element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param dimension_element name of the element
				   @return returns the name of the previous element
					\n \n sample: \n \n
				   @include ElementPrev/Program.cs
				 */
				String^ ElementPrev(String^ database, String^ dimension, String^ dimension_element);

				/**@brief Retrieves the count of all elements in the specified dimension
				   @param database name of the dimension
				   @param dimension name of the dimension
				   @return Count of elements or error on failure
					\n \n sample: \n \n
				   @include ElementCount/Program.cs
				 */				 			 				 				
				unsigned int ElementCount(String^ database, String^ dimension);

				/**@brief Returns the maximal consolidation level in a dimension (consolidation depth)
				   @param database name of the database
				   @param dimension name of the dimension
				   @return returns the Maximal consolidation level
					\n \n sample: \n \n
				   @include ElementTopLevel/Program.cs
				*/
				unsigned int ElementTopLevel(String^ database, String^ dimension);

				/**@brief Lists all elements contained in the specified dimension 
				   @param database name of the database
				   @param dimension name of the dimension
				   @return returns a list of elements
				 */				 			 				 				 
				array<ElementInfo^>^ DimensionListElements(String^ database, String^ dimension);

				/**@brief returns the number of elements contained in the specified dimension
						accessable by connection user
				   @param database name of the database
				   @param dimension name of the dimension
				   @return returns the count of elements
				 */				 			 				 				 
				size_t DimensionFlatCount(String^ database, String^ dimension);

				/**@brief returns a restricted flat lists of elements contained in the specified dimension 
				   @param database name of the database
				   @param dimension name of the dimension
				   @param start startposition
				   @param limit at most limit elements
				   @return returns a list of elements
				 */				 			 				 				 
				array<ElementInfo^>^ DimensionRestrictedFlatListDimElements(String^ database, String^ dimension, long start, long limit);

				/**@brief returns a restricted flat lists of elements contained in the specified dimension which includes a specific element
				   @param database name of the database
				   @param dimension name of the dimension
				   @param element name of the element
				   @param start initial startposition
				   @param limit at most limit elements
				   @return returns a list of elements including
				 */				 			 				 				 
				array<ElementInfo^>^ DimensionRestrictedFlatListFindElement(String^ database, String^ dimension, String^ element, long %start, long limit);
				
				/**@brief returns a restricted lists of top elements contained in the specified dimension 
				   @param database name of the database
				   @param dimension name of the dimension
				   @param start startposition
				   @param limit at most limit elements
				   @return returns a list of elements
				 */				 			 				 				 
				array<ElementInfo^>^ DimensionRestrictedTopListDimElements(String^ database, String^ dimension, long start, long limit);

				/**@brief returns a restricted lists of children elements contained in the specified dimension 
				   @param database name of the database
				   @param dimension name of the dimension
				   @param elementidentifier identifier of parent
				   @param start startposition
				   @param limit at most limit elements
				   @return returns a list of elements
				 */				 			 				 				 
				array<ElementInfo^>^ DimensionRestrictedChildrenListDimElements(String^ database, String^ dimension, long elementidentifier, long start, long limit);

				/**@brief Lists all children of an consolidated element.
				   @param database name of the database
				   @param dimension name of the dimension
				   @param element name of the element
				   @return List of children
					\n \n sample: \n \n
				   @include DimElementListConsolidated/Program.cs
				 */
				array<ConsolidationInfo>^ DimElementListConsolidated(String^ database, String^ dimension, String^ element);

				/**@brief Lists all children with extended info of an consolidated element.
				   @param database name of the database
				   @param dimension name of the dimension
				   @param element name of the element
				   @return List of children with extende info
				 */
				array<ConsolidationExtendedInfo>^ ElementListConsolidated(String^ database, String^ dimension, String^ element);

				/**@brief Retrieves an array with elements which are up to the filter criteria of the subset filter
				   @param database name of the database
				   @param dimension name of the dimension 
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
				array<SubsetResult>^ DimensionSubset(	String^ database, String^ dimension, long indent,
														ArgAliasFilterSettings alias,
														ArgFieldFilterSettings afilter,
														ArgBasicFilterSettings picklist,
														ArgDataFilterSettings dfilter,
														ArgSortingFilterSettings sort,
														ArgStructuralFilterSettings hfilter,
														ArgTextFilterSettings tfilter);

				/**@brief Lists a part of the attribute values for a specific attribute
				   @param database name of the database
				   @param dimension name of the dimension
				   @param elements id of the elements for which attribute values should be retrieve
				   @param attribute name of the attribute for which attribute values should be retrieve
				   @return a dictionary beetween id and value
				 */
				Dictionary<long, String^>^ GetAttributeValues(String^ database, String^ dimension, array<long>^ elements, String^ attribute);

				/**@brief Lists a part of the attribute values for a specific attribute
				   @param database name of the database
				   @param dimension name of the dimension
				   @param elements id of the elements for which attribute values should be retrieve
				   @param attribute name of the attribute for which attribute values should be retrieve
				   @param Lastelement id of the last element for which attribute values has been retrieved
				   @param NumDatasets maximal number of attribute vlaues whcih should be retrieved
						beginning with the element next after Lastelement
				   @return a dictionary beetween id and value
				 */
				Dictionary<long, String^>^ GetAttributeValues(String^ database, String^ dimension, array<long>^ elements, String^ attribute, long Lastelement, unsigned long NumDatasets);

				/**@brief Lists a part of the attribute values for an array of attribute ids
				   @param database name of the database
				   @param dimension name of the dimension
				   @param elements id of the elements for which attribute values should be retrieve
				   @param attribute id of the attributes for which attribute values should be retrieve
				   @return a dictionary beetween id and array of values
				 */
				Dictionary<long, array<String^>^ >^ GetAttributeValues(String^ database, String^ dimension, array<long>^ elements, array<long>^ attributes);

				// End Element related

				// Begin Cube related

				/************************************************************************/
				/* lookup helpers                                                       */
				/************************************************************************/

				/**@brief retrieves name of a cube for a given id
				   @param database name of the database
				   @param id id of the cube
				   @param 
				   @return name of the cube
				  */				
				String^ GetCubeNameFromID(String^ database, long id);

				/**@brief loads the cube
				   @param database name of the database 
				   @param cube name of the cube
				   @return return nothing
				   \n \n sample: \n \n
				   @include CubeLoad/Program.cs
				  */
				void CubeLoad(String^ database, String^ cube);

				/**@brief Unload a cube
				   @param database name of the database
				   @param cube name of the cube
				   @return returns nothing
					\n \n sample: \n \n
				   @include UnloadCube/Program.cs
				 */
				void UnloadCube(String^ database, String^ cube);

				/**@brief save cube data to disk
				   @param database name of the database
				   @param cube name of the cube
				   @return returns nothing
					\n \n sample: \n \n
				   @include CubeCommitLog/Program.cs
				 */
				void CubeCommitLog(String^ database, String^ cube);

				/**@brief adds a cube into the given database
				   @param database name of the database
				   @param cube name of the cube
				   @param dimensions Names of the Dimensions of the cube	
				   @return returns nothing				 			 				 				
				   \n \n sample: \n \n
				   @include DatabaseAddCube/Program.cs
				 */				
				void DatabaseAddCube(String^ database, String^ cube, array<String^>^ dimensions);

				/**@brief adds a cube into the given database
				   @param database name of the database
				   @param cube name of the cube
				   @param dimensions Names of the Dimensions of the cube	
				   @param type type of the cube	(NormalCube or UserInfoCube)
				   @return returns nothing				 			 				 				
				 */				
				void DatabaseAddCube(String^ database, String^ cube, array<String^>^ dimensions, CubeType type);

				/**@brief Renames a cube
				   @param database name of the database 
				   @param cube_oldname name of the cube
				   @param cube_newname New name of the cube
				   @return returns nothing
				   \n \n sample: \n \n
				   @include DatabaseRenameCube/Program.cs
				 */				 				 		 				 				
				void DatabaseRenameCube(String^ database, String^ cube_oldname, String^ cube_newname);

				/**@brief deletes a cube
				   @param database name of the database
				   @param cube name of the cube
				   @return returns nothing
					\n \n sample: \n \n
				   @include DeleteCube/Program.cs
				   */
				void DeleteCube(String^ database, String^ cube);

				/**@brief delete all values of a cube
				   @param database name of the database
				   @param cube name of the cube
				   @return returns nothing
					\n \n sample: \n \n
				   @include CubeClear/Program.cs
				 */			
				void CubeClear(String^ database, String^ cube);

				/**@brief delete all values of a sub cube
				   @param database name of the database
				   @param cube name of the cube
				   @param elements coordinates of the sub cube
				   @return returns nothing
				 */				
				void CubeClear(String^ database, String^ cube, array<array<String^>^>^ elements);

				/**@brief convert the type of a cube
				   @param database name of the database
				   @param cube name of the cube
				   @param newType the desired type of the cube
				   @return returns nothing
				 */				
				void CubeConvert(String^ database, String^ cube, CubeType newType);

				/**@brief retrieves the type of the specified cube
				   @param database name of the database
				   @param cube name of the cube
				   @return returns the type of the specified cube
					\n \n sample: \n \n
				   @include GetCubeType/Program.cs
				  */
				CubeType GetCubeType(String^ database, String^ cube);

				/**@brief retrieves informations about the specified cube
				   @param database name of the database
				   @param cube name of the cube 
				   @return returns information about the queried cube
					\n \n sample: \n \n
				   @include CubeInformation/Program.cs
				  */				
				CubeInfo CubeInformation(String^ database, String^ cube);

				/**@brief retrieves the name of he appropriated attribute cube
				   @param database name of the database
				   @param dimension name of the dimension
				   @return returns the name of the specified cube
					\n \n sample: \n \n
				   @include GetAttributeCube/Program.cs
				  */				
				String^ GetAttributeCube(String^ database, String^ dimension);
				
				/**@brief retrieves the name of the privilege cube
				   @param database name of the database
				   @param dimension name of the dimension
				   @return returns name of the privilege cube
					\n \n sample: \n \n
				   @include GetRightsCube/Program.cs
				  */				
				String^ GetRightsCube(String^ database, String^ dimension);

				/**@brief list all cubes in specified database
				   @param database name of the database 
				   @return array with all cubes in specified database 
				   \n \n sample: \n \n
				 */	
				array<String^>^ DatabaseListCubes(String^ database);

				/**@brief lists all cubes of a type in specified database
				   @param database name of the database
				   @param Type Type of cubes
				   @return array with all cubes in specified database 
				 */	
				array<String^>^ DatabaseListCubes(String^ database, CubeType Type);

				/**@brief lists all cubes of a type in specified database
				   @param database name of the database
				   @param Type Type of cubes
				   @param OnlyCubesWithCells if True only Cube with cells are returned
				   @return array with cubes in specified database 				 
				 */	
				array<String^>^ DatabaseListCubes(String^ database, CubeType Type, bool OnlyCubesWithCells);

				/**@brief Lists all dimensions of a cube in the specified database
				   @param database name of the database
				   @param cube name of the Cube
				   @return List of dimensions
					\n \n sample: \n \n
				   @include CubeListDimensions/Program.cs
				 */				 				 				 				
				array<String^>^ CubeListDimensions(String^ database, String^ cube);

				/**@brief Lists all cubes using the specified dimension
				   @param database name of the database
				   @param dimension name of the dimension
				   @return List of cubes
					\n \n sample: \n \n
				   @include DimensionListCubes/Program.cs
				 */				  				 				 				
				array<String^>^ DimensionListCubes(String^ database, String^ dimension);

				/**@brief Lists all cubes of a type using the specified dimension
				   @param database name of the database
				   @param dimension name of the dimension
				   @param Type of cubes
				   @return List of cubes
					\n \n sample: \n \n
				 */	
				array<String^>^ DimensionListCubes(String^ database, String^ dimension, CubeType Type);

				/**@brief Lists all cubes of a type using the specified dimension
				   @param database name of the database
				   @param dimension name of the dimension
				   @param Type Type of cubes
				   @param OnlyCubesWithCells if True only Cube with cells are returned
				   @return array with cubes in specified database 				 
				 */	
				array<String^>^ DimensionListCubes(String^ database, String^ dimension, CubeType Type, bool OnlyCubesWithCells);

				/* Rules */

				/**@brief Creates a new rule for the specified cube
				   @param database name of the database
				   @param cube name of the cube
				   @param Definiton definition of the rule
				   @param Activate if false, the rule will be deactivated
				   @return information about the rule
					\n \n sample: \n \n
				   @include RuleCreate/Program.cs
				 */
				RuleInfo^ RuleCreate(String^ database, String^ cube, String^ Definiton, bool Activate);

				/**@brief Creates a new rule for the specified cube
				   @param database name of the database
				   @param cube name of the cube 
				   @param Definiton definition of the rule
				   @param ExternId extern identifier
				   @param Comment Comment
				   @param Activate if false, the rule will be deactivated
				   @return information about the rule 
				 */
				RuleInfo^ RuleCreate(String^ database, String^ cube, String^ Definiton, String^ ExternId, String^ Comment, bool Activate);
				
				/**@brief Creates a new rule for the specified cube
				   @param database name of the database
				   @param cube name of the cube 
				   @param Definiton definition of the rule
				   @param ExternId extern identifier
				   @param Comment Comment
				   @param Activate if false, the rule will be deactivated
				   @param position: if 0, the rule will be added to the end of list (max_position+1)
				   @return information about the rule 
				 */
				RuleInfo^ RuleCreate(String^ database, String^ cube, String^ Definiton, String^ ExternId, String^ Comment, bool Activate, double Position);

				/**@brief Modifies a rule for the specified cube
				   @param database name of the database
				   @param cube name of the cube
				   @param id id of the rule
				   @param Definiton definition of the rule
						  if null or empty the definition will not be changed
				   @param Activate the desired activation value
				   @return information about the rule 
					\n \n sample: \n \n
				   @include RuleModify/Program.cs
				 */
				RuleInfo^ RuleModify(String^ database, String^ cube, long id, String^ Definiton, bool Activate);

				/**@brief Modifies a rule for the specified cube
				   @param database name of the database
				   @param cube name of the cube
				   @param id id of the rule
				   @param Definiton definition of the rule
						  if null or empty the definition will not be changed
				   @param ExternId extern identifier
				   @param Comment Comment
				   @param Activate if false, the rule will be deactivated
				   @return information about the rule 
				 */
				RuleInfo^ RuleModify(String^ database, String^ cube, long id, String^ Definiton, String^ ExternId, String^ Comment, bool Activate);
				

				/**@brief Modifies a rule for the specified cube
				   @param database name of the database
				   @param cube name of the cube
				   @param id id of the rule
				   @param Definiton definition of the rule
						  if null or empty the definition will not be changed
				   @param ExternId extern identifier
				   @param Comment Comment
				   @param Activate if false, the rule will be deactivated
				   @param position: if 0, the rule position will not change
				   @return information about the rule 
				 */
				RuleInfo^ RuleModify(String^ database, String^ cube, long id, String^ Definiton, String^ ExternId, String^ Comment, bool Activate, double Position);

				/**@brief moves rules for the specified cube
				   @param database name of the database
				   @param cube name of the cube
				   @param ids ids of the rules
				   @param startPosition: new position of the first rule, if 0 the rule is moved to the end
				   @param belowPosition: position of the next rule moved rules should be positioned before. Used for position distance calculation.
						  If 0 then moved rules are positioned with step 1.0 (default = 0)
				   @return returns nothing
				 */
				array<RuleInfo^>^ RulesMove(String^ database, String^ cube, array<long>^ ids, double startPosition, double belowPosition);

				/**@brief Modifies the activation mode of rules for the specified cube
				   @param database name of the database
				   @param cube name of the cube
				   @param ids ids of the rules
				   @param mode the desired activation mode
				   @return information about the rules 
				 */
				array<RuleInfo^>^ RulesActivate(String^ database, String^ cube, array<long>^ ids, RuleActivationMode mode);

				/**@brief retrieve information about the rule
				   @param database name of the database
				   @param cube name of the cube
				   @param id id of the rule
				   @return information about the rule 
					\n \n sample: \n \n
				   @include RuleInformation/Program.cs
				 */
				RuleInfo^ RuleInformation(String^ database, String^ cube, long id);

				/**@brief retrieve information about the rule applied to a cell
				   @param database name of the database
				   @param cube name of the cube
				   @param coordinates coordinates of the cell
				   @return information about the rule 
					\n \n sample: \n \n
				   @include CellRuleInformation/Program.cs
				 */
				RuleInfo^ CellRuleInformation(String^ database, String^ cube, array<String^>^ coordinates);

				/**@brief parses a rule for the specified cube
				   @param database name of the database
				   @param cube name of the cube
				   @param Definiton definition of the rule
				   @return XML representation of the definition
					\n \n sample: \n \n
				   @include RuleParse/Program.cs
				 */
				String^ RuleParse(String^ database, String^ cube, String^ Definiton);
				
				/**@brief deletes a given rule
				   @param database name of the database
				   @param cube name of the cube to be queried
				   @param id of the rule
				   @return returns nothing
					\n \n sample: \n \n
				   @include RuleDelete/Program.cs
				 */				 				
				void RuleDelete(String^ database, String^ cube, long id);
				
				/**@brief deletes rules for the specified cube
				   @param database name of the database
				   @param cube name of the cube
				   @param ids ids of the rules
				   @return returns nothing
				 */
				void RulesDelete(String^ database, String^ cube, array<long>^ ids);
				
				/**@brief Retrieves a list of the existing rules
				   @param database name of the database
				   @param cube name of the cube
				   @return Array with information of the rules
					\n \n sample: \n \n
				   @include ListRules/Program.cs
				 */
				array<RuleInfo^>^ ListRules(String^ database, String^ cube);
				
				// End Cube related

				// Start Cell related

				/**@brief retrieves value for a cell
				   @param str_result it's valid if not null
				   @param dbl_result it's valid if str_result is null
				   @param database name of the database
				   @param cube name of the cube 
				   @param coordinates coordinates of the cell
				   @return returns nothing
					\n \n sample: \n \n
				   @include GetData/Program.cs
				 */		
				void GetData(String^ %str_result, double %dbl_result, String^ database, String^ cube, array<String^>^ coordinates);

				/**@brief get values for several cells
				   @param database name of the database
				   @param cube name of the cube
				   @param elements list of coordinates
				   @return Array with Values
					\n \n sample: \n \n
				   @include GetDataMulti/Program.cs
				 */
				array<CellValue>^ GetDataMulti(String^ database, String^ cube, array<array<String^>^>^ elements);

				/**@brief get values for an area of cells
				   @param database name of the database
				   @param cube name of the cube
				   @param elements list of elements
				   @return Array with Values
					\n \n sample: \n \n
				   @include GetDataArea/Program.cs
				  */
				array<CellValue>^ GetDataArea(String^ database, String^ cube, array<array<String^>^>^ elements);

				/**@brief Sets the value for a string cell
				   @param database name of the database
				   @param cube name of the cube
				   @param coordinates coordinates of the cell
				   @param str_value	value to be set	
				   @return returns nothing		
					\n \n sample: \n \n
				   @include		/Program.cs
				 */
				void SetData(String^ database, String^ cube, array<String^>^ coordinates, String^ str_value);

				/**@brief Sets the value for a numeric cell
				   @param database name of the database
				   @param cube name of the cube
				   @param coordinates coordinates of the cell
				   @param dbl_value	value to be set	
				   @return nothing
				 */
				void SetData(String^ database, String^ cube, array<String^>^ coordinates, double dbl_value);

				/**@brief Sets the value for a numeric cell
				   @param database name of the database
				   @param cube name of the cube
				   @param coordinates coordinates of the cell
				   @param dbl_value	value to be set
				   @param mode specifies the splash mode
						  (type: "SPLASH_MODE_NONE", "SPLASH_MODE_DEFAULT", "SPLASH_MODE_SET" or "SPLASH_MODE_ADD").	
				   @return returns nothing		
				 */
				void SetData(String^ database, String^ cube, array<String^>^ coordinates, double dbl_value, SplashMode mode);

				/**@brief Sets the value for a cell
				   @param database name of the database
				   @param cube name of the cube
				   @param coordinates coordinates where the value should be set
				   @param dbl_value	double value to be set, is used if str_value is = null
				   @param str_value string value to be set
				   @param mode specifies the splash mode
							   (type: "SPLASH_MODE_NONE", "SPLASH_MODE_DEFAULT", "PLASH_MODE_SET" or "SPLASH_MODE_ADD").
				   @return returns nothing
				 */
				void SetData(String^ database, String^ cube, array<String^>^ coordinates, String^ str_value, double dbl_value, SplashMode mode);

				/**@brief Sets the value for a cell
				   @param database name of the database
				   @param cube name of the cube
				   @param coordinates coordinates where the value should be set
				   @param dbl_value	double value to be set, is used if str_value is = null
				   @param str_value string value to be set
				   @param mode specifies the splash mode
							   (type: "SPLASH_MODE_NONE", "SPLASH_MODE_DEFAULT", "PLASH_MODE_SET" or "SPLASH_MODE_ADD").
				   @param add decides whether value should be added to to current value
							   (only for "SPLASH_MODE_NONE", "SPLASH_MODE_DEFAULT" otherwise ignored).
				   @param eventprocessor value of false disable call to supervision event processor
				   @param locked_elements a list of locked elements
				   @return returns nothing
				 */
				void SetData(String^ database, String^ cube, array<String^>^ coordinates, String^ str_value, double dbl_value, SplashMode mode, bool add, bool eventprocessor, array<array<String^>^>^ locked_elements);

				/**@brief Sets values for multiple cells
				   @param database name of the database
				   @param cube name of the cube
				   @param dsa the coordinates and values of the cells to be set
				   @param mode specifies the splash mode
							   (type: "SPLASH_MODE_NONE", "SPLASH_MODE_DEFAULT", "SPLASH_MODE_SET" or "SPLASH_MODE_ADD").	
				   @param Add decides whether value should be added to to current value
							   (only for "SPLASH_MODE_NONE", "SPLASH_MODE_DEFAULT" otherwise ignored).
				   @param Eventprocessor value of false disable call to supervision event processor
				   @return returns nothing
				 */
				void SetDataMulti(String^ database, String^ cube, array<DataSetMulti>^ dsa, SplashMode mode, bool Add, bool EventProccessor);

				/**@brief Sets values for multiple cells
				   @param database name of the database
				   @param cube name of the cube
				   @param dsa the coordinates and values of the cells to be set
				   @param mode specifies the splash mode
							   (type: "SPLASH_MODE_NONE", "SPLASH_MODE_DEFAULT", "SPLASH_MODE_SET" or "SPLASH_MODE_ADD").	
				   @param Add decides whether value should be added to to current value
							   (only for "SPLASH_MODE_NONE", "SPLASH_MODE_DEFAULT" otherwise ignored).
				   @param Eventprocessor value of false disable call to supervision event processor
				   @param locked_elements a list of locked elements
				   @return returns nothing
				 */
				void SetDataMulti(String^ database, String^ cube, array<DataSetMulti>^ dsa, SplashMode mode, bool Add, bool EventProccessor, array<array<String^>^>^ locked_elements);

				/**@brief copies the values according the structure from 'from' to 'to'
				   @param database database name of the database
				   @param cube name of the cube
				   @param from source coordinates
				   @param to target coordinates
				   @return returns nothing
					\n \n sample: \n \n
				   @include CopyCell/Program.cs
				 */
				void CopyCell(String^ database, String^ cube, array<String^>^ from, array<String^>^ to);

				/**@brief copies the values according the structure from 'from' to 'to' and then dbl_value is splashed to 'to'
				   @param database name of the database
				   @param cube name of the cube
				   @param from source coordinates
				   @param to target coordinates
				   @param dbl_value value to be splashed
				   @return returns nothing
				 */
				void CopyCell(String^ database, String^ cube, array<String^>^ from, array<String^>^ to, double dbl_value);

				/**@brief copies the values according the structure from source to 'to'
				   @param mode CopyCellMode
				   @param database name of the database
				   @param cube name of the cube
				   @param from source coordinates or null if irrelevant
					(only CopyCellDefault otherwise ignored).
				   @param to target coordinates
				   @param predict_area a list of locked elements
					(ignored for CopyCellDefault).
				   @param locked_elements a list of locked elements
				   @param dbl_value value to be splashed afterwards or null if irrelevant
					(only CopyCellDefault otherwise ignored).
				   @param use_rules decides whether rules should be used 
				   @return returns nothing
				 */
				void CopyCell(CopyCellMode mode, String^ database, String^ cube, array<String^>^ from, array<String^>^ to, array<array<String^>^>^ predict_area, array<array<String^>^>^ locked_elements, double^ dbl_value, bool use_rules);

				/**@brief Puts value into cell and calculates values for sister cells in order to parents remain unchanged
							using goal seek mode GoalSeekComplete.
				   @param database name of the database
				   @param cube name of the cube
				   @param path coordinates of the cell
				   @param dbl_value value to be splashed
				   @return returns nothing
				 */
				void GoalSeek(String^ database, String^ cube, array<String^>^ path, double dbl_value);

				/**@brief Puts value into cell and calculates values for sister cells in order to parents remain unchanged.
				   @param database name of the database
				   @param cube name of the cube
				   @param path coordinates of the cell
				   @param dbl_value value to be splashed
				   @param mode goal seek mode to be used
				   @param goalseek_area siblings to reallocate for each dimension
					(only for GoalSeekEqual, GoalSeekRelative, otherwise ignored).
				   @return returns nothing
				 */
				void GoalSeek(String^ database, String^ cube, array<String^>^ path, double dbl_value, GoalSeekMode mode, array<array<String^>^>^ goalseek_area);

				/**@brief locks a cube area.
				   @param database name of the database
				   @param cube name of the cube
				   @param area area to be locked
				   @return returns information about lock
				 */
				LockInfo^ CubeLock(String^ database, String^ cube, array<array<String^>^>^ area);

				/**@brief Retrieves information about locks for a cube
				   @param database name of the database
				   @param cube name of the cube
				   @return returns information about all locks of the cube
				 */
				array<LockInfo^>^ CubeLocks(String^ database, String^ cube);
				
				/**@brief rollback all steps for a locked area and release lock.
				   @param database name of the database
				   @param cube name of the cube
				   @param lockid identifies the locked area
				   @return returns nothing
				 */
				void CubeRollback(String^ database, String^ cube, long lockid);

				/**@brief rollback a number of steps for a locked area.
				   @param database name of the database
				   @param cube name of the cube
				   @param lockid identifies the locked area
				   @param steps number of steps to be rollbacked.
				   @return returns nothing
				 */
				void CubeRollback(String^ database, String^ cube, long lockid, long steps);
				
				/**@brief commit all steps for a locked area and release lock.
				   @param database name of the database
				   @param cube name of the cube
				   @param lockid identifies the locked area
				   @return returns nothing
				 */
				void CubeCommit(String^ database, String^ cube, long lockid);
	
				/**@brief Execute a drillthrough
				   @param database name of the database
				   @param cube name of the cube
				   @param coordinates coordinates of the cells for which a drillthrough should be executed
				   @param mode mode of the drillthrough
				   @return returns drillthrough information
				 */
				array<DrillThroughSet>^ FastCellDrillThrough(String^ database, String^ cube, array<String^>^ coordinates, DrillThroughType mode);
				
				/**@brief exports the data as specified in opt
				   @param database name of the database
				   @param cube name of the cube
				   @param area subcube to be exported
				   @param opts options for the export
				   @return Array with Coordinates and Values
					\n \n sample: \n \n
				   @include GetDataExport/Program.cs
				 */
				array<Dataset>^ GetDataExport(String^ database, String^ cube, array<array<String^>^>^ area, GetDataExportOptions opts);
				
				/**@brief exports the data as specified in opt
				   @param database name of the database
				   @param cube name of the cube
				   @param area subcube to be exported
				   @param opts options for the export
				   @param progress how much of the cube is processed
				   @return Array with Coordinates and Values
				 */
				array<Dataset>^ GetDataExport(String^ database, String^ cube, array<array<String^>^>^ area, GetDataExportOptions opts, double %progress);

				/**@brief exports the data as specified in opt
				   @param database name of the database
				   @param cube name of the cube
				   @param area subcube to be exported
				   @param opts options for the export
				   @param useRules if true apply rules before exporting			
				   @param progress how much of the cube is processed
				   @return Array with Coordinates and Values
				 */
				array<Dataset>^ GetDataExport(String^ database, String^ cube, array<array<String^>^>^ area, GetDataExportOptions opts, bool useRules, double %progress);

				// End Cell related

			protected:
				void static throwException(String^ s);
				void static throwException(libpalo_err err);

			private:
				static String^ TrustFile = nullptr;
				
				struct sock_obj* so;
				bool IsGpu;

				void DoInit(String^ hostname, String^ service, String^ username, String^ password, String^ trust_file);
				DatabaseType GetDatabaseTypeHelper(unsigned long type);
				DimensionType GetDimensionTypeHelper(unsigned long type);
				array<ElementInfo^>^ RestrictedList(String^ database, String^ dimension, String^ element, long parentid, long %start, long limit);

				inline DimElementType getType(de_type Type);
				inline void FillElementInfo(ElementInfo^ ei, arg_dim_element_info2_raw_w &elementinfo); 
				CubeType GetCubeTypeHelper(unsigned long type);
			};


			private ref class Helper {
			public:
				static String^ CharPtrToStringFree(wchar_t *s);
				static array<String^>^ ArgStringArrayToManaged(struct arg_str_array_w a);
				static array<String^>^ ArgStringArrayToManagedFree(struct arg_str_array_w a);
				static void ManagedToArgStringArray(array<String^>^ src, struct arg_str_array_w &dest);
				static void FreeConvertedArgStringArray(struct arg_str_array_w &dest);
				static array<String^>^ ManagedJaggedArrayMemberToStringArray(array<String^>^ a);
				static CellValue ArgPaloValueToCellValue(struct arg_palo_value_w pv);
				static String^ GetErrorMessage(libpalo_err err);
				static void ConvertStringToUTF8(std::string& To, String^ From);
				static void ConvertStringArrayToUTF8(std::vector<std::string>& To, array<String^>^ From);
				static void ConvertStringArrayArrayToUTF8(std::vector<std::vector<std::string> >& To, array<array<String^>^>^ From);
				static void ConvertDArrayArrayToDVector(std::vector< std::vector<double> >& To, array<array<double>^>^ From);
				static String^ ConvertUTF8ToString(const std::string& From);
				static array<String^>^ ConvertUTF8ToStringArray(const std::vector<std::string>& From);

			private:
				static void HandleConversionError(bool ToUtf8);
				static void wcs2utf8(char **utf8_str, const wchar_t *s, size_t len);
				static void utf82wcs(wchar_t **wcs, const char *utf8_str);
			};


			private ref class ErrorInformation {
			public:
				ErrorInformation(): ErrorCode(0), ErrorMessage(nullptr) {}
				ErrorInformation(int ArgErrorCode, String^ ArgErrorMessage): ErrorCode(ArgErrorCode), ErrorMessage(ArgErrorMessage) {}
				int GetCode();
				int GetOriginalCode();
				String^ GetMessage();
				void Check();
				void ProcessError(int Originalerror, const std::string& Message);
				void ProcessSocketError(const std::string& Message);
				void ProcessStandardError(const std::string& Message);
				void ProcessUnknownError();

			private:
				int ErrorCode;
				int OriginalErrorCode;

				String^ ErrorMessage;

				void ProcessErrorCode();
			};


			public ref class PaloCommException : public ApplicationException {
			public:
				PaloCommException(String^ message);
			};


			public ref class PaloException : public ApplicationException {
			public:
				int ErrorCode;
				int OriginalErrorCode;
				String^ PaloMessage;

				PaloException(String^ Message, int Code);
				PaloException(String^ Message, int Code, int Orignalcode);
				PaloException(libpalo_err err);
			
			private:
				void Set(int Code, int Originalcode);
			};

		}
	}
}
