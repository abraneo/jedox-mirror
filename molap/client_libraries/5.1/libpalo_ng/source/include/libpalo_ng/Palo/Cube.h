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
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * \author Jiri Junek <jiri.junek@qbicon.cz>
 * 
 *
 */

#ifndef CUBE_H
#define CUBE_H

#include <string>

#include <libpalo_ng/Palo/types.h>

namespace jedox {
namespace palo {

class ServerImpl;
class PaloClient;
class SIElements;
class SIDimensions;
class SICubes;

#define WRONGDIMENSIONCOUNT "not the correct amount of dimensions"

/*!
 * \brief
 * little helper functions that return the id or ids of elements
 *
 * \author
 * Frieder Hofmann <frieder.hofmann@jedox.com>
 */
struct CubeHelper {
	typedef std::vector<std::string> ELEMENT_NAME_LIST;

	static IdentifierType getElementID(boost::shared_ptr<const SIElements> cache, const std::string& element_name);
	static ELEMENT_LIST getElementIDs(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db, const DIMENSION_LIST& dimension_ids, const ELEMENT_NAME_LIST& element_names);
	static ELEMENT_LIST getElementIDs(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dimension_id, const ELEMENT_NAME_LIST& element_names);
	static std::vector<ELEMENT_LIST> getElementIDs(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db, const DIMENSION_LIST& dimension_ids, const std::vector<ELEMENT_NAME_LIST>& element_names);
	static std::vector<ELEMENT_LIST> getCoordIDs(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db, const DIMENSION_LIST& dimension_ids, const std::vector<std::vector<std::string> > &coords);
	static void coordinates2query(const std::vector<std::vector<std::string> > & coordinates, std::stringstream &query, boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db, const DIMENSION_LIST& dimlist);
};

/** @brief
 *  Class for operating on cubes.
 *  All Strings are UTF8.
 */
class LIBPALO_NG_CLASS_EXPORT Cube {
public:

	// The online cache needs access to the cubes fields and methods for optimal efficiency.
	friend class OnlineCubeCache;
	friend class Database;
	friend class DatabaseCache;

	enum DRILLTHROUGH_TYPE {
		History = 1, Details = 2
	};

	enum AGGREGATION_TYPE {
		AGGREGATION_SUM = 0, AGGREGATION_AVG = 1, AGGREGATION_COUNT = 2, AGGREGATION_MAX = 3, AGGREGATION_MIN = 4
	};

	enum EXPANDAGGR_TYPE {
		SELF = 1, CHILDREN = 2, LEAVES = 4
	};

	enum GOALSEEK_TYPE {
		GOALSEEK_COMPLETE = 0, GOALSEEK_EQUAL = 1, GOALSEEK_RELATIVE = 2
	};

	enum COPY_FUNCTION {
		COPY = 0, PREDICT_LINEAR_REGRESSION = 1
	};

	/** @brief
	 *  Do NOT use explicitly.
	 */
	Cube(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, IdentifierType db, IdentifierType cube);

	void ActualInfo(CUBE_INFO &ci);
	void ActualInfoPerm(CUBE_INFO_PERMISSIONS &ci, bool showPermission);

	/** @brief
	 *  Get information about this cube in a structure of type CUBE_INFO.
	 *
	 *  @return CUBE_INFO for this cube.
	 */
	const CUBE_INFO& getCacheData() const;

	/** @brief
	 *  Get information about this cube in a structure of type CUBE_INFO.
	 *
	 *  @return CUBE_INFO for this cube.
	 */
	const CUBE_INFO getCacheDataCopy() const;

	/** @brief
	 *  rename this cube
	 *
	 *  @param newName : new name
	 */
	void rename(const std::string& newName);

	/** @brief
	 *  clear the contents of this cube
	 */
	void clear();

	/* @brief
	 * clear the contents of the sub cube identified by the supplied coordinates
	 *
	 * @param coordinates The coordinates of the sub cube
	 */
	void clear(const std::vector<std::vector<std::string> > & coordinates);

	/** @brief
	 *  destroy this cube
	 */
	bool destroy();

	/** @brief
	 *  save this cube to the disk
	 */
	bool save();

	/** @brief
	 *  load cube data from disk
	 *
	 *  @return success/failure
	 */
	bool load();

	/** @brief
	 *  completely unload this cube
	 *
	 *  @return : True if success, false otherwise
	 */
	bool unload();

	/** @brief
	 *  replaces the value of a cell inside this cube
	 *
	 *  @param elements : path of the cell being changed
	 *  @param cellvalue : the value we want to write
	 *  @param splashmode : the mode of splashing to apply -- see
	 *                   SPLASH_MODE.
	 *  @param add : if true a numeric value will be added
	 *  @param eventprocessor : if false, the eventprocessor will not be called
	 */
	bool CellReplace(const std::vector<std::string>& elements, const CELL_VALUE& cellvalue, SPLASH_MODE splashmode, bool add, bool eventprocessor);

	/** @brief
	 *  replaces the value of a cell inside this cube
	 *
	 *  @param elements : path of the cell being changed
	 *  @param cellvalue : the value we want to write
	 *  @param splashmode : the mode of splashing to apply -- see
	 *                   SPLASH_MODE.
	 */
	bool CellReplace(const ELEMENT_LIST& elements, const CELL_VALUE& cellvalue, SPLASH_MODE splashmode);

	/** @brief
	 *  replaces the value of a cell inside this cube
	 *
	 *  @param elements : path of the cell being changed
	 *  @param cellvalue : the value we want to write
	 *  @param splashmode : the mode of splashing to apply -- see
	 *                   SPLASH_MODE.
	 *  @param add : if true a numeric value will be added
	 *  @param eventprocessor : if false, the eventprocessor will not be called
	 */
	bool CellReplace(const ELEMENT_LIST& elements, const CELL_VALUE& cellvalue, SPLASH_MODE splashmode, bool add, bool eventprocessor);

	/** @brief
	 *  replaces the value of a cell inside this cube
	 *
	 *  @param elements : path of the cell being changed
	 *  @param cellvalue : the value we want to write
	 *  @param splashmode : the mode of splashing to apply -- see
	 *                   SPLASH_MODE.
	 */
	bool CellReplace(const std::vector<std::string>& elements, const CELL_VALUE& cellvalue, SPLASH_MODE splashmode);

	/** @brief
	 *  replaces the value of a cell inside this cube without changing value of selected cells
	 *
	 *  @param elements : path of the cell being changed
	 *  @param cellvalue : the value we want to write
	 *  @param splashmode : the mode of splashing to apply -- see
	 *                   SPLASH_MODE.
	 *  @param add : if true a numeric value will be added
	 *  @param eventprocessor : if false, the eventprocessor will not be called
	 *  @param lockedCoordinates : paths of the cells that stays unchanged (including base values when consolidations are specified)
	 */
	bool CellReplaceWithLock(const std::vector<std::string> & elements, const CELL_VALUE& cellvalue, SPLASH_MODE splashmode, bool add, bool eventprocessor, const std::vector<std::vector<std::string> > &lockedCoordinates);

	/** @brief
	 *  replace a whole set of cell values
	 *
	 *  @param elements : paths of the cells that receive the values
	 *  @param cellvalue : the values we want to write
	 *  @param splashmode : the mode of splashing to apply -- see
	 *        SPLASH_MODE
	 *  @param add : if true a numeric value will be added
	 *  @param eventprocessor : if false, the eventprocessor will not be called
	 */
	bool CellReplaceBulk(const std::vector<std::vector<std::string> > & elements, const std::vector<CELL_VALUE>& cellvalue, SPLASH_MODE splashmode, bool add, bool eventprocessor);

	/** @brief
	 *  replace a whole set of cell values
	 *
	 *  @param elements : paths of the cells that receive the values
	 *  @param cellvalue : the values we want to write
	 *  @param splashmode : the mode of splashing to apply -- see
	 *        SPLASH_MODE
	 */
	bool CellReplaceBulk(const std::vector<std::vector<std::string> > & elements, const std::vector<CELL_VALUE>& cellvalue, SPLASH_MODE splashmode);

	/** @brief
	 *  replace a whole set of cell values - locking value of selected cells
	 *
	 *  @param elements : paths of the cells that receive the values
	 *  @param cellvalue : the values we want to write
	 *  @param splashmode : the mode of splashing to apply -- see
	 *        SPLASH_MODE
	 *  @param add : if true a numeric value will be added
	 *  @param eventprocessor : if false, the eventprocessor will not be called
	 *  @param lockedCoordinates : paths of the cells that stays unchanged (including base values when consolidations are specified)
	 */
	bool CellReplaceBulkWithLock(const std::vector<std::vector<std::string> > & coordinates, const std::vector<CELL_VALUE>& cellvalues, SPLASH_MODE splashmode, bool add, bool eventprocessor, const std::vector<std::vector<std::string> > &lockedCoordinates);

	/** @brief
	 *  Puts value into cell and calculates values for sister cells in order to keep parents unchanged.
	 *
	 *  @param path : path of the cell that will be set to value
	 *  @param value : the value we want to write
	 */
	bool CellGoalSeek(const std::vector<std::string> & path, double value);

	std::vector<CELL_VALUE_EXPORTED> CellExport(const std::vector<std::vector<std::string> > &coordinates, unsigned long blocksize, const std::vector<std::string> &start, const std::string condition, unsigned short BaseOnly = 0, unsigned short SkipEmpty = 1, unsigned short use_rules = 0);
	void CellExport(std::vector<CELL_VALUE_EXPORTED> &result, const std::vector<std::vector<std::string> > &coordinates, unsigned long blocksize, const std::vector<std::string> &start, const std::string condition, unsigned short BaseOnly = 0, unsigned short SkipEmpty = 1, unsigned short use_rules = 0);
	void CellExport(std::vector<CELL_VALUE_EXPORTED> &result, const std::vector<ELEMENT_LIST> &coordinates, unsigned long blocksize, const ELEMENT_LIST &start, const std::string condition, unsigned short BaseOnly = 0, unsigned short SkipEmpty = 1, unsigned short use_rules = 0);
	void DataFeedExport(std::vector<CELL_VALUE_EXPORTED> &result, unsigned long blocksize, const ELEMENT_LIST& start, unsigned short SkipEmpty = 1 );

	/** @brief
	 *  copy the value of one cell to the other cell
	 *
	 *  @param path : path of the cell that holds our source value
	 *  @param path_to : destination path
	 *  @param value : value to be written
	 */
	bool CellCopy(const std::vector<std::string>& path, const std::vector<std::string>& path_to, double value);

	/** @brief
	 *  copy the value of one cell to the other cell
	 *
	 *  @param path : path of the cell that holds our source-value
	 *  @param path_to : destination path
	 */
	bool CellCopy(const std::vector<std::string>& path, const std::vector<std::string>& path_to);

	/** @brief
	 *  Get values from a set of cells
	 *
	 *  @param coordinates : a list of cell coordinates
	 *  @param showRule : returns also the id of the rule, which is applied
	 *  @param showLockState : returns also the lockstate
	 */
	std::vector<CELL_VALUE> CellValues(const std::vector<std::vector<std::string> > & coordinates, unsigned short showRule = 0, unsigned short showLockState = 0);

	/** @brief
	 *  Get values from a set of cells
	 *
	 *	@param res : output collection of values. Resulting values are appended to the end, collection is not cleared.
	 *  @param coordinates : a list of cell coordinates
	 *  @param showRule : returns also the id of the rule, which is applied
	 *  @param showLockState : returns also the lockstate
	 */
	void CellValues(std::vector<CELL_VALUE> & res, const std::vector<std::vector<std::string> > & coordinates, unsigned short showRule = 0, unsigned short showLockState = 0);

	/** @brief
	 *  retrieve the value of a cell
	 *
	 *  @param elements : path of the cell
	 *  @param showRule : returns also the id of the rule, which is applied
	 *  @param showLockState : returns also the lockstate
	 */
	CELL_VALUE CellValue(const std::vector<std::string>& elements, unsigned short showRule = 0, unsigned short showLockState = 0);
	void CellValue(CELL_VALUE &result, const std::vector<std::string>& elements, unsigned short showRule = 0, unsigned short showLockState = 0, bool ForceServerCall = false);

	/** @brief
	 *  get information about an area of cells inside this cube
	 *
	 *  @param showRule : returns also the id of the rule, which is applied
	 *  @param coordinates : For each dimension, a list of elements. The cartesian product defines the area.
	 *  @param showLockState : returns also the lockstate
	 */
	std::vector<CELL_VALUE_PATH> CellArea(const std::vector<std::vector<std::string> > & coordinates, unsigned short showRule = 0, unsigned short showLockState = 0);

	void CellDrillThrough(std::vector<DRILLTHROUGH_INFO> &result, const std::vector<std::string>& elements, DRILLTHROUGH_TYPE mode);
	void MDXCellDrillThrough(std::vector<DRILLTHROUGH_INFO> &result, ELEMENT_LIST elemids, DRILLTHROUGH_TYPE mode);

	/** @brief
	 *  set a lock on an area of cells inside this cube
	 *
	 *  @param coordinates : For each dimension, a list of elements. The cartesian product defines the area.
	 */
	LOCK_INFO Lock(const std::vector<std::vector<std::string> > & coordinates);

	/** @brief
	 *  set a lock on an area of cells inside this cube
	 *
	 *  @param coordinates : For each dimension, a list of elements. The cartesian product defines the area.
	 */
	LOCK_INFO Lock();

	/** @brief
	 *  get information about all locks on this cube
	 */
	std::vector<LOCK_INFO> Locks();

	/** @brief
	 *  commits changes of a locked cube area
	 *
	 *  @param LockID : Identifier of the locked area
	 */
	bool Commit(IdentifierType LockID);

	/** @brief
	 *  rollback all changes of a locked area and remove the lock
	 *
	 *  @param LockID : Identifier of the locked area
	 */
	bool Rollback(IdentifierType LockID);

	/** @brief
	 *  rollback changes of a locked cube area
	 *
	 *  @param LockID : Identifier of the locked area
	 *  @param steps : number of steps to rollback
	 */
	bool Rollback(IdentifierType LockID, long steps);

	// even faster method for C

	void CellValues(const Cell_Values_Coordinates& coordinates, Cell_Values_C* cvc);

	// for RADU and only for him
	void CellValues(C_2DARRAY_LONG& coord, Cell_Values_C* cvrc);
	inline void MDXCellValue(CELL_VALUE &result, const ELEMENT_LIST & elemids)
	{
		CellValue(result, elemids, 0, 0, false);
	}
	;
	inline void MDXCellValues(std::vector<CELL_VALUE> & res, const std::vector<ELEMENT_LIST> & coord)
	{
		CellValues(res, coord, 0, 0);
	}
	;

	/** beginning of rule methods */

	/** @brief
	 *  Create a new Rule and get its id
	 *
	 *  @param definition: String defining the rule. Must be already CSV encoded.
	 *  @param use_identifier: if 1 , ids will be used in the representation of the rule (default = 0).
	 *  @param extern_id: external identifier
	 *  @param comment:  a string comment
	 *  @param activate: if 0, the rule will be deactivated
	 *  @param position: if 0, the rule will be added to the end of list
	 *
	 *  @return id , representation, extern_id, comment and timestamp of the rule
	 */
	RULE_INFO RuleCreate(const std::string& definition, unsigned short use_identifier = 0, std::string extern_id = "", std::string comment = "", unsigned short activate = 1, double position = 0);

	/** @brief
	 *  modify a Rule
	 *
	 *  @param id: the id of the rule
	 *  @param definition: String defining the rule. Must be already CSV encoded.
	 If empty, the definition will not be changed.
	 *  @param use_identifier: if 1 , ids will be used in the representation of the rule (default = 0).
	 *  @param extern_id: external identifier
	 *  @param comment:  a string comment
	 *  @param activate: if 0, the rule will be deactivated
	 *
	 *  @return id , representation, extern_id, comment and timestamp of the rule
	 */
	RULE_INFO RuleModify(long id, const std::string& definition = "", unsigned short use_identifier = 0, std::string extern_id = "", std::string comment = "", unsigned short activate = 1, double position = 0);

	/** @brief
	 *  modify a Rule
	 *
	 *  @param ruleIds: list of rule Ids to be moved
	 *  @param startPosition: new position of the first rule, if 0 the rule is moved to the end
	 *  @param belowPosition: position of the next rule moved rules should be positioned before. Used for position distance calculation.
	 *                        If 0 then moved rules are positioned with step 1.0
	 *
	 *  @return: List of rule definitions and their IDs
	 */
	std::vector<RULE_INFO> RulesMove(const ELEMENT_LIST &ruleIds, double startPosition, double belowPosition = 0);

	/** @brief
	 *  (de)activate Rules
	 *
	 *  @param ruleIds: list of rule Ids
	 *  @param activate: 0 - deactivate, 1 - activate, 2 - toggle state
	 *
	 *  @return: List of rule definitions and their IDs
	 */
	std::vector<RULE_INFO> RulesActivate(const ELEMENT_LIST &ruleIds, unsigned short activate = 1);

	/** @brief
	 *  Get information about a Rule
	 *
	 *  @param id: the id of the rule
	 *  @param use_identifier: if 1 , ids will be used in the representation of the rule (default = 0).
	 *
	 *  @return id , representation, extern_id, comment, timestamp and activatation status of the rule
	 */
	RULE_INFO RuleInfo(IdentifierType id, unsigned short use_identifier = 0);

	/** @brief
	 *  Get the XML representation of a rule
	 *
	 *  @param definition String defining the rule. Must be already CSV encoded.
	 *
	 *  @return XML representation of a rule
	 */
	std::string RuleParse(const std::string& definition);

	/** @brief
	 *  Query existing rules for this cube
	 *
	 *  @param use_identifier: if 1 , ids will be used in the representation of the rule (default = 0).
	 *
	 *  @return: List of rule definitions and their IDs
	 */
	std::vector<RULE_INFO> Rules(unsigned short use_identifier = 0);

	/**@brief
	 * Destroy a rule belonging to this cube using its ID
	 *
	 * @param identifier: id of the rule.
	 *
	 * @return success
	 */
	bool RuleDestroy(IdentifierType identifier);

	/**@brief
	 * Destroy rules belonging to this cube using their IDs
	 *
	 * @param ruleIds: ids of rules.
	 *
	 * @return success
	 */
	bool RulesDestroy(const ELEMENT_LIST &ruleIds);

	/** end of rule methods */

	void convert(const CUBE_INFO::TYPE cubetype);

	unsigned int getSequenceNumberFromCache() const;

	void CellExportProps(std::vector<CELL_VALUE_EXPORTED_PROPS> &result, const std::vector<std::vector<std::string> > &coordinates, const std::vector<std::string> &properties, unsigned long blocksize, const std::vector<std::string>& start, const std::string condition, unsigned short BaseOnly = 0, unsigned short SkipEmpty = 1, unsigned short use_rules = 0);
	void CellValuesProps(std::vector<CELL_VALUE_PROPS> &res, const std::vector<std::vector<std::string> > &coordinates, const std::vector<std::string> &properties);
	void CellValueProps(CELL_VALUE_PROPS &result, const std::vector<std::string> &elements, const std::vector<std::string> &properties);
	void CellAreaProps(std::vector<CELL_VALUE_PATH_PROPS> &result, const std::vector<std::vector<std::string> > &coordinates, const std::vector<std::string> &properties);
	void CellAreaPropsAggr(std::vector<CELL_VALUE_PATH_PROPS> &result, const std::vector<std::vector<std::string> > &coordinates, const std::vector<std::string> &properties, AGGREGATION_TYPE aggr, const std::vector<EXPANDAGGR_TYPE> &expand);
	bool CellCopy(const std::vector<std::string>& path, const std::vector<std::string>& path_to, double value, bool userule);
	bool CellCopy(const std::vector<std::string>& path, const std::vector<std::string>& path_to, bool userule);
	bool CellCopy(const std::vector<std::string>& path, const std::vector<std::string>& path_to, double value, bool userule, const std::vector<std::vector<std::string> > &lockedCoordinates);
	bool CellCopy(const std::vector<std::string>& path, const std::vector<std::string>& path_to, bool userule, const std::vector<std::vector<std::string> > &lockedCoordinates);
	bool CellCopy(const std::vector<std::string>& path, const std::vector<std::string>& path_to, double value, const std::vector<std::vector<std::string> > &lockedCoordinates);
	bool CellCopy(const std::vector<std::string>& path, const std::vector<std::string>& path_to, const std::vector<std::vector<std::string> > &lockedCoordinates);
	bool CellPredictLinearRegression(const std::vector<std::vector<std::string> > &coordinates, const std::vector<std::string>& path_to, bool userule, const std::vector<std::vector<std::string> > &lockedCoordinates);
	bool CellGoalSeek(const std::vector<std::string> & path, double value, GOALSEEK_TYPE type, const std::vector<std::vector<std::string> >& area);

private:
	boost::shared_ptr<ServerImpl> m_ServerImpl;
	boost::shared_ptr<PaloClient> m_PaloClient;
	unsigned int m_Dat;
	unsigned int m_Cube;
	boost::shared_ptr<const SICubes> m_Cache;
	static const std::string CELL_PROPERTIES_DIM_NAME;

	void CellValue(CELL_VALUE &result, const ELEMENT_LIST & elemids, unsigned short showRule, unsigned short showLockState, bool ForceServerCall);
	void CellValues(std::vector<CELL_VALUE> & res, const std::vector<ELEMENT_LIST> & coord, unsigned short showRule, unsigned short showLockState);
	void CellArea(std::vector<CELL_VALUE_PATH>& res, const std::vector<ELEMENT_LIST> & coord, unsigned short showRule = 0, unsigned short showLockState = 0);
	bool CellCopy(COPY_FUNCTION func, const std::vector<std::string> *path, const std::vector<std::vector<std::string> > *area, const std::vector<std::string>& path_to, double *value, bool userule, const std::vector<std::vector<std::string> > &lockedCoordinates);
};

} /* palo */
} /* jedox */

#endif							 // CUBE_H
