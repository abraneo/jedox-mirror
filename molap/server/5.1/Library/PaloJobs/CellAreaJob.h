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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef PALO_JOBS_CELL_AREA_JOB_H
#define PALO_JOBS_CELL_AREA_JOB_H 1

#include "palo.h"

#include "Exceptions/ErrorException.h"
#include "Olap/Cube.h"
#include "PaloJobs/AreaJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief cell area
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS CellAreaJob : public AreaJob {

	struct StreamToAreaMapper {
		typedef multimap<IdentifierType, size_t> Map;
		typedef vector<pair<IdentifiersType, pair<CellValue, vector<CellValue> > > > Results;

		vector<Map> mapping;
		vector<size_t> offsets;
		Results results;
	};

	StreamToAreaMapper mapper;


public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new CellAreaJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	CellAreaJob(PaloJobRequest* jobRequest) : AreaJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets job type
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return READ_JOB;
	}

public:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief start working
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		findCube(true, false);
		checkProperties();
		const IdentifiersType *dimensions = cube->getDimensions();

		if (!jobRequest->area && !jobRequest->areaName) {
			throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "area is empty", PaloRequestHandler::ID_AREA, "");
		}

		if (!jobRequest->area) {
			if (dimensions->size() != jobRequest->areaName->size()) {
				throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of area elements", PaloRequestHandler::NAME_AREA, "");
			}
			jobRequest->area = new vector<IdentifiersType>(jobRequest->areaName->size());
			for (uint32_t i = 0; i < jobRequest->areaName->size(); i++) {
				CPDimension dim = database->lookupDimension((*dimensions)[i], false);
				for (uint32_t j = 0; j < jobRequest->areaName->at(i).size(); j++) {
					IdentifierType id = dim->findElementByName(jobRequest->areaName->at(i).at(j), 0, false)->getIdentifier();
					jobRequest->area->at(i).push_back(id);
				}
			}
		} else {
			if (dimensions->size() != jobRequest->area->size()) {
				throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of area elements", PaloRequestHandler::ID_AREA, "");
			}
			for (uint32_t i = 0; i < jobRequest->area->size(); ++i) {
				CPDimension dim = database->lookupDimension((*dimensions)[i], false);
				if (dim->getDimensionType() == Dimension::VIRTUAL) {
					continue;
				}
				for (uint32_t j = 0; j < jobRequest->area->at(i).size(); ++j) {
					dim->findElement(jobRequest->area->at(i).at(j), 0, false);
				}
			}
		}

		AggregationPlanNode::AggregationType aggrType = aggregationType();
		bool isSum = aggrType == AggregationPlanNode::SUM;

		vector<User::RoleDbCubeRight> vRights;
		if (User::checkUser(user)) {
			user->fillRights(vRights, User::cellDataRight, database, cube);
		}
		bool checkPermissions = cube->getMinimumAccessRight(user) == RIGHT_NONE;

		double cellCount = fillEmptyDim(vRights, checkPermissions, *jobRequest->area, cube, database, user);
		if (cellCount > max_cell_count) {
			throw ErrorException(ErrorException::ERROR_MAX_CELL_REACHED, "area too big");
		}

		response = new HttpResponse(HttpResponse::OK);
		setToken(cube);

		bool strElem;
		initResponse(*jobRequest->area);
		PArea area(new Area(*jobRequest->area));
		if (area->getSize()) {
			PCubeArea calcArea = checkRights(vRights, checkPermissions, area, isSum ? 0 : &strElem, cube, database, user, true, noPermission, isNoPermission, this->dims);
			if (!isSum && strElem) {
				throw ParameterException(ErrorException::ERROR_INVALID_ELEMENT_TYPE, "cannot calculate aggregation on string area", "element type", Element::STRING);
			}

			PCellStream cs;
			PCellStream props;
			if (calcArea->getSize()) {
				if (isSum) {
					cs = cube->calculateArea(calcArea, CubeArea::ALL, ALL_RULES, false, UNLIMITED_SORTED_PLAN);
				} else {
					PAggregationMaps aggregationMaps(new AggregationMaps());
					PCubeArea aggrArea = calcArea->expandAggregation(*aggregationMaps.get(), *jobRequest->expand);

					PPlanNode plan = cube->createPlan(aggrArea, CubeArea::ALL, ALL_RULES, true, UNLIMITED_SORTED_PLAN);
					vector<PPlanNode> children;
					if (plan->getType() == UNION) {
						children = plan->getChildren();
					} else {
						children.push_back(plan);
					}
					PPlanNode aggregationNode = PPlanNode(new AggregationPlanNode(calcArea, children, aggregationMaps, 0, CPCube(), aggrType));
					cs = cube->evaluatePlan(aggregationNode, EngineBase::ANY, true);
				}
				if (jobRequest->properties) {
					props = getCellPropsStream(database, cube, calcArea, *jobRequest->properties);
				}
			}
			loop(area, calcArea, cs, NULL, props, vRights, 0, true); //calls appendValue

			generateResponse();
		}
	}

private:

	void initResponse(vector<IdentifiersType> &area) {
		mapper.mapping.resize(area.size());
		mapper.offsets.resize(area.size() + 1);
		mapper.offsets[0] = 1;

		int dimIndex = 0;
		for (vector<IdentifiersType>::iterator dimIt = area.begin(); dimIt != area.end(); ++dimIt, dimIndex++) {
			mapper.offsets[dimIndex + 1] = mapper.offsets[dimIndex] * dimIt->size();

			size_t i = 0;
			for (IdentifiersType::iterator it = dimIt->begin(); it != dimIt->end(); ++it, i++) {
				mapper.mapping[dimIndex].insert(StreamToAreaMapper::Map::value_type(*it, i));
			}
		}

		mapper.results.resize(mapper.offsets[dimIndex]);
	}

	void generateResponse() {
		size_t i = 0;
		for (StreamToAreaMapper::Results::iterator it = mapper.results.begin(); it != mapper.results.end(); ++it, i++) {
			AreaJob::appendValue(it->first, it->second.first, it->second.second);
		}
		#ifdef ENABLE_TEST_MODE
		if (Logger::isDebug()){
			Logger::debug << "CellAreaJob: Target area cells = " << i << endl;
			FILE* pFile = NULL;
			stringstream sstrm;
			sstrm << "CellAreaJob: Target area cells = " << i;
			pFile = fopen("queries.txt", "a");
			string h(sstrm.str());
			fprintf(pFile,h.c_str());
			fprintf(pFile,"\n");
			fclose(pFile);
		}
		#endif
	}

	void saveResult(const IdentifiersType &key, size_t dimIndex, const vector<vector<size_t> > &foundOffsets, size_t index, const CellValue &value, const vector<CellValue> &prop_vals) {
		if (dimIndex == key.size()) {
			mapper.results[index].first = key;
			mapper.results[index].second.first = value;
			mapper.results[index].second.second = prop_vals;
		} else {
			for (vector<size_t>::const_iterator it = foundOffsets[dimIndex].begin(); it != foundOffsets[dimIndex].end(); ++it) {
				saveResult(key, dimIndex + 1, foundOffsets, index + *it, value, prop_vals);
			}
		}
	}

	virtual void appendValue(const IdentifiersType &key, const CellValue &value, const vector<CellValue> &prop_vals) {
		vector<vector<size_t> > foundOffsets(key.size());
		for (size_t i = 0; i != key.size(); i++) {
			StreamToAreaMapper::Map::iterator it = mapper.mapping[i].find(key[i]);
			if (it == mapper.mapping[i].end()) {
				// throw error? Unknown path received
			} else {
				do {
					foundOffsets[i].push_back(mapper.offsets[i] * it->second);
				} while (++it != mapper.mapping[i].end() && it->first == key[i]);
			}
		}

		saveResult(key, 0, foundOffsets, 0, value, prop_vals);
	}

	AggregationPlanNode::AggregationType aggregationType() {
		AggregationPlanNode::AggregationType aggrType = AggregationPlanNode::SUM;
		if (jobRequest->function || jobRequest->expand) {
			if (!jobRequest->function) {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "function is empty", PaloRequestHandler::FUNCTION, "");
			}
			if (!jobRequest->expand) {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "expand is empty", PaloRequestHandler::EXPAND, "");
			}
			if (jobRequest->function < AggregationPlanNode::AVG || jobRequest->function > AggregationPlanNode::MIN) {
				throw ParameterException(ErrorException::ERROR_INVALID_AGGR_FUNCTION, "invalid aggregation function", PaloRequestHandler::FUNCTION, "");
			}
			bool err = false;
			size_t dimCount = cube->getDimensions()->size();
			if (dimCount != jobRequest->expand->size()) {
				err = true;
			} else {
				for (size_t i = 0; i < dimCount; i++) {
					uint32_t exp = jobRequest->expand->at(i);
					if (exp != CubeArea::SELF && exp != CubeArea::CHILDREN && exp != CubeArea::LEAVES) {
						err = true;
						break;
					}
				}
			}
			if (err) {
				throw ParameterException(ErrorException::ERROR_INVALID_EXPAND, "invalid aggregation expand type", PaloRequestHandler::EXPAND, "");
			}
			aggrType = (AggregationPlanNode::AggregationType)jobRequest->function;
		}
		return aggrType;
	}
};

}

#endif
