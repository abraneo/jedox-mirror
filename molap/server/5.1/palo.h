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
 * \author Zurab Khadikov, Jedox AG, Freiburg, Germany
 * \author Christoffer Anselm, Jedox AG, Freiburg, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef PALO_H
#define PALO_H 1

#if defined(_MSC_VER)
#include "config_win.h"
#include "System/system-vs-net-2003.h"
#else
#include "config.h"
#include "System/system-unix.h"
#include <pthread.h>
#endif

#ifdef ENABLE_GOOGLE_CPU_PROFILER
#include <google/profiler.h>
#endif

#include <stdlib.h>
#include <stddef.h>

#include <algorithm>
#include <deque>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <list>
#include <limits>
#include <iterator>

#ifndef __CUDACC__
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#endif // __CUDACC__
namespace palo {
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Engine feature activation defines
////////////////////////////////////////////////////////////////////////////////
#define GPU_COMPLEX_RULES
#define ENABLE_PLAN_FOR_PALO_DATA

class Element;

////////////////////////////////////////////////////////////////////////////////
// defines
////////////////////////////////////////////////////////////////////////////////

// /////////////////////////////////////////////////////////////////////////////
// @brief license type of GPU version
// /////////////////////////////////////////////////////////////////////////////

#define GPU_LICENSE_TYPE "20"

////////////////////////////////////////////////////////////////////////////////
/// @brief no-identifier marker
////////////////////////////////////////////////////////////////////////////////

#define NO_IDENTIFIER ((IdentifierType) ~(uint32_t)0)

////////////////////////////////////////////////////////////////////////////////
/// @brief all-identifiers marker
////////////////////////////////////////////////////////////////////////////////

#define ALL_IDENTIFIERS ((IdentifierType)(NO_IDENTIFIER-1))

////////////////////////////////////////////////////////////////////////////////
/// @brief plan of unlimited size - sorted by keys
////////////////////////////////////////////////////////////////////////////////

const uint64_t UNLIMITED_SORTED_PLAN = (uint64_t) - 1;

////////////////////////////////////////////////////////////////////////////////
/// @brief plan of unlimited size - NOT sorted by keys
////////////////////////////////////////////////////////////////////////////////

const uint64_t UNLIMITED_UNSORTED_PLAN = (uint64_t)0;

// /////////////////////////////////////////////////////////////////////////////
// types
// /////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief identifier
////////////////////////////////////////////////////////////////////////////////

typedef uint32_t IdentifierType;


#ifndef __CUDACC__

////////////////////////////////////////////////////////////////////////////////
/// @brief depth
////////////////////////////////////////////////////////////////////////////////

typedef uint32_t DepthType;

////////////////////////////////////////////////////////////////////////////////
/// @brief indent
////////////////////////////////////////////////////////////////////////////////

typedef uint32_t IndentType;

////////////////////////////////////////////////////////////////////////////////
/// @brief level
////////////////////////////////////////////////////////////////////////////////

typedef uint32_t LevelType;

////////////////////////////////////////////////////////////////////////////////
/// @brief position
////////////////////////////////////////////////////////////////////////////////

typedef uint32_t PositionType;

////////////////////////////////////////////////////////////////////////////////
/// @brief element list
////////////////////////////////////////////////////////////////////////////////

typedef vector<Element*> ElementsType;

////////////////////////////////////////////////////////////////////////////////
/// @brief identifier list
////////////////////////////////////////////////////////////////////////////////

//typedef vector<IdentifierType> IdentifiersType;
class IdentifiersType : public vector<IdentifierType> {
public:
	IdentifiersType() :
			vector<IdentifierType>()
	{
	}
	IdentifiersType(size_t n, const IdentifierType value = 0) :
			vector<IdentifierType>(n, value)
	{
	}
	IdentifiersType(const IdentifierType *first, const IdentifierType *last) :
			vector<IdentifierType>(first, last)
	{
	}
	IdentifiersType(const IdentifiersType &idt) :
			vector<IdentifierType>(idt)
	{
	}

	bool operator==(const IdentifiersType &idt) const
	{
		size_t n = size();
		if (n == idt.size()) {
			return n ? !memcmp(&(*this)[0], &idt[0], n * sizeof(IdentifierType)) : true;
		} else {
			return false;
		}
	}
	bool operator!=(const IdentifiersType &idt) const {return !(*this == idt);}

	int compare(const IdentifiersType &o) const;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief type for one element and weight
////////////////////////////////////////////////////////////////////////////////

typedef pair<Element*, double> ElementWeightType;

////////////////////////////////////////////////////////////////////////////////
/// @brief type for elements and weights
////////////////////////////////////////////////////////////////////////////////

typedef vector<ElementWeightType> ElementsWeightType;

////////////////////////////////////////////////////////////////////////////////
/// @brief list of parents
////////////////////////////////////////////////////////////////////////////////

typedef ElementsType ParentsType;

////////////////////////////////////////////////////////////////////////////////
/// @brief type for identifiers and weights
////////////////////////////////////////////////////////////////////////////////

typedef map<IdentifierType, double> IdentifiersWeightMap;

////////////////////////////////////////////////////////////////////////////////
/// @brief type for identifier and weight
////////////////////////////////////////////////////////////////////////////////

typedef pair<IdentifierType, double> IdentifierWeightType;

////////////////////////////////////////////////////////////////////////////////
/// @brief type for identifiers and weights
////////////////////////////////////////////////////////////////////////////////

typedef vector<IdentifierWeightType> IdentifiersWeightType;

////////////////////////////////////////////////////////////////////////////////
/// @brief type for optimized vector of bits
////////////////////////////////////////////////////////////////////////////////
typedef vector<bool> BitVector;

////////////////////////////////////////////////////////////////////////////////
/// @brief type for filenames
////////////////////////////////////////////////////////////////////////////////

struct FileName {
	FileName() :
			path("."), name("ERROR"), extension("")
	{
	}

	FileName(const string& path, const string& name, const string& extension) :
			path(path.empty() ? "." : path), name(name), extension(extension)
	{
	}

	FileName(const FileName& old) :
			path(old.path), name(old.name), extension(old.extension)
	{
	}

	FileName(const FileName& old, const string& extension) :
			path(old.path), name(old.name), extension(extension)
	{
	}

	string fullPath() const
	{
		if (path.empty()) {
			if (extension.empty()) {
				return name;
			} else {
				return name + "." + extension;
			}
		} else if (extension.empty()) {
			return path + "/" + name;
		} else {
			return path + "/" + name + "." + extension;
		}
	}

	string path;
	string name;
	string extension;
};

#endif

// /////////////////////////////////////////////////////////////////////////////
// GPU types
// /////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief GPU numeric cell type (float or double)
////////////////////////////////////////////////////////////////////////////////

#ifdef GPU_FRAMETYPE_32BIT
typedef float gpuCellType;
#else
typedef double gpuCellType;
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief GPU bin type for packed coordinates
////////////////////////////////////////////////////////////////////////////////

#ifdef GPU_FRAMETYPE_32BIT
typedef uint32_t gpuBinType;

#	define GPU_INVALID_BIN_VALUE (0xFFFFFFFF)
#else
typedef uint64_t gpuBinType;

#	define GPU_INVALID_BIN_VALUE (0xFFFFFFFFFFFFFFFFULL)
#endif

// Single cell
typedef vector<gpuBinType> GpuBinPath;

#ifndef __CUDACC__

////////////////////////////////////////////////////////////////////////////////
/// @brief type for filter base range
////////////////////////////////////////////////////////////////////////////////

struct BaseRangeType {
	IdentifierType low;
	IdentifierType high;

	struct BaseRangeFlagsType {
		BaseRangeFlagsType() :
				mergeNext(false), mergeNextWeight(false)
		{
		}
		;

		uint8_t mergeNext :1; // merges with next range if ignoring weight
		uint8_t mergeNextWeight :1; // merges with next range even if weight matters
	} flags;

	BaseRangeType(IdentifierType low, IdentifierType high) :
			low(low), high(high), flags()
	{
	}
	;

	BaseRangeType() :
			low(0), high(0), flags()
	{
	}
	;

	// Attention! if this.low == y.low then the bigger area (greater .high) is returned as "less"
	bool operator<(const BaseRangeType& _yVal) const
	{
		return (this->low < _yVal.low || (this->low == _yVal.low && this->high > _yVal.high));
	}
	;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief type for filter base ranges
////////////////////////////////////////////////////////////////////////////////

typedef vector<BaseRangeType> BaseRangesType;

////////////////////////////////////////////////////////////////////////////////
/// @brief type for base range
////////////////////////////////////////////////////////////////////////////////

struct BaseRangeWeightType : public BaseRangeType {
	double weight;

	BaseRangeWeightType() :
			BaseRangeType(), weight(0)
	{
	}
	;

	BaseRangeWeightType(IdentifierType low, IdentifierType high, double weight) :
			BaseRangeType(low, high), weight(weight)
	{
	}
	;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief type for base ranges
/// element = range
////////////////////////////////////////////////////////////////////////////////

typedef vector<BaseRangeWeightType> BaseRangesWeightType;

struct TranslationParameters {
	vector<uint32_t> binDimensionsCount; // the number of dimensions per bin
	vector<uint32_t> dimensionsPos; // bit positions inside bins
	vector<uint32_t> dimensionsMap; // dimension index mapping
	vector<gpuBinType> dimensionsBitmask; // stored bit masks for dimensions
};
#endif

// /////////////////////////////////////////////////////////////////////////////
// enumerations
// /////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief type of database, dimension or cubes
////////////////////////////////////////////////////////////////////////////////

enum ItemType {
	NORMALTYPE, SYSTEMTYPE, ATTRIBUTETYPE, USER_INFOTYPE, GPUTYPE
};

////////////////////////////////////////////////////////////////////////////////
/// @brief type for rights
////////////////////////////////////////////////////////////////////////////////

enum RightsType {
	RIGHT_NONE, RIGHT_READ, RIGHT_WRITE, RIGHT_DELETE, RIGHT_SPLASH, RIGHT_EMPTY
};

////////////////////////////////////////////////////////////////////////////////
/// @brief type of job
////////////////////////////////////////////////////////////////////////////////

enum JobType {
	WRITE_JOB, READ_JOB, SPECIAL_JOB, UNKNOWN_JOB
};

////////////////////////////////////////////////////////////////////////////////
/// @brief type for worker login mode
////////////////////////////////////////////////////////////////////////////////

enum WorkerLoginType {
	WORKER_NONE, WORKER_INFORMATION, WORKER_AUTHENTICATION, WORKER_AUTHORIZATION
};

////////////////////////////////////////////////////////////////////////////////
/// @brief result status of worker
////////////////////////////////////////////////////////////////////////////////

enum ResultStatus {
	RESULT_OK, RESULT_FAILED
};

////////////////////////////////////////////////////////////////////////////////
/// @brief status of worker
////////////////////////////////////////////////////////////////////////////////

enum WorkerStatus {
	WORKER_NOT_RUNNING, WORKER_RUNNING,
};

////////////////////////////////////////////////////////////////////////////////
/// @brief possible encryption types
////////////////////////////////////////////////////////////////////////////////

enum Encryption_e {
	ENC_NONE = 0, ENC_OPTIONAL = 1, ENC_REQUIRED = 2
};

////////////////////////////////////////////////////////////////////////////////
/// @brief possible activation options
////////////////////////////////////////////////////////////////////////////////

enum ActivationType {
	INACTIVE = 0, ACTIVE = 1, TOGGLE = 2
};

enum svsStatusChange {
	SVS_NONE, SVS_RESTART, SVS_STOP, SVS_START
};

////////////////////////////////////////////////////////////////////////////////
/// @brief type of rules to evaluate
////////////////////////////////////////////////////////////////////////////////

enum RulesType {
	NO_RULES = 0, DIRECT_RULES = 1, INDIRECT_RULES = 2, ALL_RULES = 3, NOCACHE = 4, NO_RULE_IDS = 8
};

////////////////////////////////////////////////////////////////////////////////
/// @brief plan node types
////////////////////////////////////////////////////////////////////////////////

enum PlanNodeType {
	UNION, SOURCE, AGGREGATION, ADDITION, SUBTRACTION, MULTIPLICATION, DIVISION, LEGACY_RULE, TRANSFORMATION, CACHE, QUERY_CACHE, CONSTANT, CELLMAP, CELL_RIGHTS, COMPLETE, ROUNDCORRECT, QUANTIFICATION
};

const IdentifierType NO_RULE = ~0;


class Commitable;
class CommitableList;
class Server;
class DatabaseList;
class Database;
class SystemDatabase;
class NormalDatabase;
class UserInfoDatabase;
class ConfigDatabase;
class CubeList;
class Cube;
class AttributesCube;
class SystemCube;
class SessionInfoCube;
class JobInfoCube;
class RightsCube;
class ConfigurationCube;
class SubsetViewCube;
class DimensionList;
class Dimension;
class AliasDimension;
class BasicDimension;
class AttributesDimension;
class NormalDimension;
class SystemDimension;
class CubeDimension;
class DimensionDimension;
class RightsDimension;
class ConfigurationDimension;
class SubsetViewDimension;
class UserInfoDimension;
class RuleList;
class Rule;
class UserList;
class User;
class Relations;
class WeightedSet;
class ElementPage;
class Lock;
class LockList;
class PaloSharedMutex;
class JournalFile;
class JournalMem;
struct CellValueContext;
struct ElementsContext;
struct FileName;
class CubeWorker;
class LoginWorker;
class DimensionWorker;
class RollbackStorage;
class RuleMarker;
class RuleNode;
class EngineBase;
class EngineCpu;
class EngineGpu;
class CellValueStream;
class Area;
class CubeArea;
class SubCubeList;
class Set;
class PlanNode;
class CombinationProcessor;
class StorageCpu;
class StringStorageCpu;
class GpuDevice;
class GpuWorker;
class GpuStorage;
class GpuPage;
class PageCollection;
class GpuPageCollectionFast;
class GpuPageCollectionSlow;
class GlobalMemPool;
class LockedCells;
class PathTranslator;
class AggregationMap;
typedef vector<AggregationMap> AggregationMaps;
class Condition;

#ifndef __CUDACC__

typedef pair<IdentifierType, IdentifierType> dbID_cubeID;
typedef set<dbID_cubeID> CubesWithDBs; //dbID, cubeID

template<typename ValueType> class ICellMap;

typedef boost::shared_ptr<Commitable> PCommitable;
typedef boost::shared_ptr<const Commitable> CPCommitable;

typedef boost::shared_ptr<CommitableList> PCommitableList;
typedef boost::shared_ptr<const CommitableList> CPCommitableList;

typedef boost::shared_ptr<Server> PServer;
typedef boost::shared_ptr<const Server> CPServer;

typedef boost::shared_ptr<DatabaseList> PDatabaseList;
typedef boost::shared_ptr<const DatabaseList> CPDatabaseList;

typedef boost::shared_ptr<Database> PDatabase;
typedef boost::shared_ptr<const Database> CPDatabase;

typedef boost::shared_ptr<SystemDatabase> PSystemDatabase;
typedef boost::shared_ptr<const SystemDatabase> CPSystemDatabase;

typedef boost::shared_ptr<NormalDatabase> PNormalDatabase;
typedef boost::shared_ptr<const NormalDatabase> CPNormalDatabase;

typedef boost::shared_ptr<UserInfoDatabase> PUserInfoDatabase;
typedef boost::shared_ptr<const UserInfoDatabase> CPUserInfoDatabase;

typedef boost::shared_ptr<ConfigDatabase> PConfigDatabase;
typedef boost::shared_ptr<const ConfigDatabase> CPConfigDatabase;

typedef boost::shared_ptr<CubeList> PCubeList;
typedef boost::shared_ptr<const CubeList> CPCubeList;

typedef boost::shared_ptr<Cube> PCube;
typedef boost::shared_ptr<const Cube> CPCube;

typedef boost::shared_ptr<AttributesCube> PAttributesCube;
typedef boost::shared_ptr<const AttributesCube> CPAttributesCube;

typedef boost::shared_ptr<SystemCube> PSystemCube;
typedef boost::shared_ptr<const SystemCube> CPSystemCube;

typedef boost::shared_ptr<SessionInfoCube> PSessionInfoCube;
typedef boost::shared_ptr<const SessionInfoCube> CPSessionInfoCube;

typedef boost::shared_ptr<JobInfoCube> PJobInfoCube;
typedef boost::shared_ptr<const JobInfoCube> CPJobInfoCube;

typedef boost::shared_ptr<RightsCube> PRightsCube;
typedef boost::shared_ptr<const RightsCube> CPRightsCube;

typedef boost::shared_ptr<ConfigurationCube> PConfigurationCube;
typedef boost::shared_ptr<const ConfigurationCube> CPConfigurationCube;

typedef boost::shared_ptr<SubsetViewCube> PSubsetViewCube;
typedef boost::shared_ptr<const SubsetViewCube> CPSubsetViewCube;

typedef boost::shared_ptr<DimensionList> PDimensionList;
typedef boost::shared_ptr<const DimensionList> CPDimensionList;

typedef boost::shared_ptr<Dimension> PDimension;
typedef boost::shared_ptr<const Dimension> CPDimension;

typedef boost::shared_ptr<AliasDimension> PAliasDimension;
typedef boost::shared_ptr<const AliasDimension> CPAliasDimension;

typedef boost::shared_ptr<BasicDimension> PBasicDimension;
typedef boost::shared_ptr<const BasicDimension> CPBasicDimension;

typedef boost::shared_ptr<AttributesDimension> PAttributesDimension;
typedef boost::shared_ptr<const AttributesDimension> CPAttributesDimension;

typedef boost::shared_ptr<NormalDimension> PNormalDimension;
typedef boost::shared_ptr<const NormalDimension> CPNormalDimension;

typedef boost::shared_ptr<SystemDimension> PSystemDimension;
typedef boost::shared_ptr<const SystemDimension> CPSystemDimension;

typedef boost::shared_ptr<CubeDimension> PCubeDimension;
typedef boost::shared_ptr<const CubeDimension> CPCubeDimension;

typedef boost::shared_ptr<DimensionDimension> PDimensionDimension;
typedef boost::shared_ptr<const DimensionDimension> CPDimensionDimension;

typedef boost::shared_ptr<RightsDimension> PRightsDimension;
typedef boost::shared_ptr<const RightsDimension> CPRightsDimension;

typedef boost::shared_ptr<ConfigurationDimension> PConfigurationDimension;
typedef boost::shared_ptr<const ConfigurationDimension> CPConfigurationDimension;

typedef boost::shared_ptr<SubsetViewDimension> PSubsetViewDimension;
typedef boost::shared_ptr<const SubsetViewDimension> CPSubsetViewDimension;

typedef boost::shared_ptr<UserInfoDimension> PUserInfoDimension;
typedef boost::shared_ptr<const UserInfoDimension> CPUserInfoDimension;

typedef boost::shared_ptr<RuleList> PRuleList;
typedef boost::shared_ptr<const RuleList> CPRuleList;

typedef boost::shared_ptr<Rule> PRule;
typedef boost::shared_ptr<const Rule> CPRule;

typedef boost::shared_ptr<UserList> PUserList;
typedef boost::shared_ptr<const UserList> CPUserList;

typedef boost::shared_ptr<User> PUser;
typedef boost::shared_ptr<const User> CPUser;

typedef boost::shared_ptr<Relations> PRelations;
typedef boost::shared_ptr<const Relations> CPRelations;

typedef boost::shared_ptr<WeightedSet> PWeightedSet;
typedef boost::shared_ptr<const WeightedSet> CPWeightedSet;

typedef boost::shared_ptr<ElementPage> PElementPage;
typedef boost::shared_ptr<const ElementPage> CPElementPage;

typedef boost::shared_ptr<Lock> PLock;
typedef boost::shared_ptr<const Lock> CPLock;

typedef boost::shared_ptr<LockList> PLockList;
typedef boost::shared_ptr<const LockList> CPLockList;

typedef boost::shared_ptr<PaloSharedMutex> PSharedMutex;
typedef boost::shared_ptr<const PaloSharedMutex> CPSharedMutex;

typedef boost::shared_ptr<JournalFile> PJournalFile;
typedef boost::shared_ptr<const JournalFile> CPJournalFile;

typedef boost::shared_ptr<JournalMem> PJournalMem;
typedef boost::shared_ptr<const JournalMem> CPJournalMem;

typedef boost::shared_ptr<CellValueContext> PCellValueContext;
typedef boost::shared_ptr<const CellValueContext> CPCellValueContext;

typedef boost::shared_ptr<ElementsContext> PElementsContext;
typedef boost::shared_ptr<const ElementsContext> CPElementsContext;

typedef boost::shared_ptr<FileName> PFileName;
typedef boost::shared_ptr<const FileName> CPFileName;

typedef boost::shared_ptr<CubeWorker> PCubeWorker;
typedef boost::shared_ptr<const CubeWorker> CPCubeWorker;

typedef boost::shared_ptr<LoginWorker> PLoginWorker;
typedef boost::shared_ptr<const LoginWorker> CPLoginWorker;

typedef boost::shared_ptr<DimensionWorker> PDimensionWorker;
typedef boost::shared_ptr<const DimensionWorker> CPDimensionWorker;

typedef boost::shared_ptr<RollbackStorage> PRollbackStorage;
typedef boost::shared_ptr<const RollbackStorage> CPRollbackStorage;

typedef boost::shared_ptr<RuleMarker> PRuleMarker;
typedef boost::shared_ptr<const RuleMarker> CPRuleMarker;

typedef boost::shared_ptr<RuleNode> PRuleNode;
typedef boost::shared_ptr<const RuleNode> CPRuleNode;

typedef boost::shared_ptr<EngineBase> PEngineBase;
typedef boost::shared_ptr<const EngineBase> CPEngineBase;

typedef boost::shared_ptr<EngineCpu> PEngineCpu;
typedef boost::shared_ptr<const EngineCpu> CPEngineCpu;

typedef boost::shared_ptr<EngineGpu> PEngineGpu;
typedef boost::shared_ptr<const EngineGpu> CPEngineGpu;

typedef boost::shared_ptr<CellValueStream> PCellStream;
typedef boost::shared_ptr<const CellValueStream> CPCellStream;

typedef boost::shared_ptr<Area> PArea;
typedef boost::shared_ptr<const Area> CPArea;

typedef boost::shared_ptr<vector<IdentifiersType> > PPaths;
typedef boost::shared_ptr<const vector<IdentifiersType> > CPPaths;

typedef boost::shared_ptr<LockedCells> PLockedCells;

typedef boost::shared_ptr<CubeArea> PCubeArea;
typedef boost::shared_ptr<const CubeArea> CPCubeArea;

typedef boost::shared_ptr<SubCubeList> PSubCubeList;
typedef boost::shared_ptr<const SubCubeList> CPSubCubeList;

typedef boost::shared_ptr<Set> PSet;
typedef boost::shared_ptr<const Set> CPSet;

typedef boost::shared_ptr<PlanNode> PPlanNode;
typedef boost::shared_ptr<const PlanNode> CPPlanNode;

typedef boost::shared_ptr<CombinationProcessor> PCombinationProcessor;
typedef boost::shared_ptr<const CombinationProcessor> CPCombinationProcessor;

typedef boost::shared_ptr<StorageCpu> PStorageCpu;
typedef boost::shared_ptr<const StorageCpu> CPStorageCpu;

typedef boost::shared_ptr<StringStorageCpu> PStringStorageCpu;
typedef boost::shared_ptr<const StringStorageCpu> CPStringStorageCpu;

typedef boost::shared_ptr<GpuStorage> PGpuStorage;
typedef boost::shared_ptr<const GpuStorage> CPGpuStorage;

typedef boost::shared_ptr<GpuPage> PGpuPage;
typedef boost::shared_ptr<const GpuPage> CPGpuPage;

typedef boost::shared_ptr<PageCollection> PPageCollection;
typedef boost::shared_ptr<const PageCollection> CPPageCollection;

typedef boost::shared_ptr<GpuPageCollectionFast> PGpuPageCollectionFast;
typedef boost::shared_ptr<const GpuPageCollectionFast> CPGpuPageCollectionFast;

typedef boost::shared_ptr<GpuPageCollectionSlow> PGpuPageCollectionSlow;
typedef boost::shared_ptr<const GpuPageCollectionSlow> CPGpuPageCollectionSlow;

typedef boost::shared_ptr<GpuDevice> PGpuDevice;
typedef boost::shared_ptr<const GpuDevice> CPGpuDevice;

typedef boost::shared_ptr<GlobalMemPool> PGlobalMemPool;
typedef boost::shared_ptr<const GlobalMemPool> CPGlobalMemPool;

typedef boost::shared_ptr<GpuWorker> PGpuWorker;
typedef boost::shared_ptr<const GpuWorker> CPGpuWorker;

typedef boost::shared_ptr<ICellMap<double> > PDoubleCellMap;
typedef boost::shared_ptr<const ICellMap<double> > CPDoubleCellMap;

typedef boost::shared_ptr<PathTranslator> PPathTranslator;
typedef boost::shared_ptr<const PathTranslator> CPPathTranslator;

typedef boost::shared_ptr<AggregationMaps> PAggregationMaps;
typedef boost::shared_ptr<const AggregationMaps> CPAggregationMaps;

typedef boost::shared_ptr<Condition> PCondition;
typedef boost::shared_ptr<const Condition> CPCondition;

#endif // __CUDACC
}

#endif
