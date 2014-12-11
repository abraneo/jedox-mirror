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
 * 
 *
 */

#ifndef PALO_JOBS_CELL_VALUES_JOB_H
#define PALO_JOBS_CELL_VALUES_JOB_H 1

#include "palo.h"

#include "Olap/SubCubeList.h"
#include "PaloJobs/AreaJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief cells value
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS CellValuesJob : public AreaJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new CellValuesJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	CellValuesJob(PaloJobRequest* jobRequest) :
		AreaJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets job type
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return READ_JOB;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start working
	////////////////////////////////////////////////////////////////////////////////
private:

	struct PathCmp {
		bool operator()(const IdentifiersType &v1, const IdentifiersType &v2) const {
			size_t dimCount = v1.size();
			for (size_t i = 0; i < dimCount; i++) {
				if (v1[i] != v2[i]) {
					return v1[i] < v2[i];
				}
			}
			return false;
		}
	};
	map<IdentifiersType, pair<CellValue, vector<CellValue> >, PathCmp> orderedPaths;

public:

	void compute() {
		findCube(true, false);
		checkProperties();

		set<size_t> invalidPaths;
		findCellPaths(&invalidPaths, 0);

		vector<User::RoleDbCubeRight> vRights;
		if (User::checkUser(user)) {
			user->fillRights(vRights, User::cellDataRight, database, cube);
		}
		bool checkPermissions = cube->getMinimumAccessRight(user) == RIGHT_NONE;

		boost::scoped_ptr<SubCubeList> subCubes(new SubCubeList());
		buildSubCubes(subCubes.get(), invalidPaths);

		for (SubCubeList::iterator it = subCubes->begin(); it != subCubes->end(); ++it) {
			PCubeArea calcArea = checkRights(vRights, checkPermissions, it->second, 0, cube, database, user, true, noPermission, isNoPermission, this->dims);

			PCellStream cs;
			PCellStream props;
			if (calcArea->getSize()) {
				RulesType rulesType = jobRequest->showRule ? ALL_RULES : RulesType(ALL_RULES | NO_RULE_IDS);

				cs = cube->calculateArea(calcArea, CubeArea::ALL, rulesType, false, UNLIMITED_SORTED_PLAN);
				if (jobRequest->properties) {
					props = getCellPropsStream(database, cube, calcArea, *jobRequest->properties);
				}
			}
			loop(it->second, calcArea, cs, NULL, props, vRights, 0, false);
		}

		response = new HttpResponse(HttpResponse::OK);
		setToken(cube);

		size_t i = 0;
		set<size_t>::iterator endip = invalidPaths.end();
		CellValue errValue(ErrorException::ERROR_INVALID_COORDINATES);
		const vector<CellValue> emptyProp;
		for (vector<IdentifiersType>::iterator it = cellPaths->begin(); it != cellPaths->end(); ++it, i++) {
			if (invalidPaths.find(i) == endip) {
				map<IdentifiersType, pair<CellValue, vector<CellValue> >, PathCmp>::iterator pit = orderedPaths.find(*it);
				generateCellValueResponse(pit->first, pit->second.first, pit->second.second);
			} else {
				generateCellValueResponse(*it, errValue, emptyProp);
			}
		}
		
		#ifdef ENABLE_TEST_MODE
			if (Logger::isDebug()){
				Logger::debug << "CellValuesJob: Target area cells = " << i << endl;
				FILE* pFile = NULL;
				stringstream sstrm;
				sstrm << "CellValuesJob: Target area cells = " << i;
				pFile = fopen("queries.txt", "a");
				string h(sstrm.str());
				fprintf(pFile,h.c_str());
				fprintf(pFile,"\n");
				fclose(pFile);
			}
		#endif
	}

private:

	void buildSubCubes(SubCubeList *subCubes, set<size_t> &invalidPaths) {
		//sort cellPaths
		CellValue res;
		vector<CellValue> prop_v;
		size_t i = 0;
		IdentifiersType refKey;
		int32_t diffDim = -1;
		bool oneDiffCoord = true;
		set<size_t>::iterator endip = invalidPaths.end();
		for (vector<IdentifiersType>::iterator it = cellPaths->begin(); it != cellPaths->end(); ++it, i++) {
			IdentifiersType &key = *it;
			if (invalidPaths.find(i) == endip) {
				orderedPaths.insert(make_pair(key, make_pair(res, prop_v)));
			}

			if (oneDiffCoord) {
				if (i == 0) {
					refKey = key;
				} else {
					int32_t size = (int32_t)key.size();
					int32_t diffCount = 0;
					for (int32_t j = 0; j < size; j++) {
						if (key[j] != refKey[j]) {
							diffCount++;
							if (diffCount > 1) {
								oneDiffCoord = false;
								break;
							}
							if (j != diffDim) {
								if (diffDim == -1) {
									diffDim = j;
								} else {
									oneDiffCoord = false;
									break;
								}
							}
						}
					}

				}
			}
		}

		if (oneDiffCoord && orderedPaths.size()) {
			int32_t size = (int32_t)refKey.size();
			PArea area(new Area(size));
			for (int32_t i = 0; i < size; i++) {
				if (i != diffDim) {
					PSet s(new Set);
					s->insert(refKey[i]);
					area->insert(i, s);
				}
			}
			if (diffDim != -1) {
				PSet s(new Set);
				for (map<IdentifiersType, pair<CellValue, vector<CellValue> >, PathCmp>::iterator it = orderedPaths.begin(); it != orderedPaths.end(); ++it) {
					s->insert((it->first)[diffDim]);
				}
				area->insert(diffDim, s);
			}
			subCubes->insertAndMerge(PCubeArea(new CubeArea(database, cube, *area)));
		} else {
			for (map<IdentifiersType, pair<CellValue, vector<CellValue> >, PathCmp>::iterator it = orderedPaths.begin(); it != orderedPaths.end(); ++it) {
				subCubes->insertAndMerge(PCubeArea(new CubeArea(database, cube, it->first)));
			}
		}
	}

	virtual void appendValue(const IdentifiersType &key, const CellValue &value, const vector<CellValue> &prop_vals) {
		map<IdentifiersType, pair<CellValue, vector<CellValue> >, PathCmp>::iterator pit = orderedPaths.find(key);
		if (pit != orderedPaths.end()) {
			pit->second.first = value;
			pit->second.second = prop_vals;
		} else {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "CellValuesJob::appendValue: invalid cell key!");
		}
	}
};

}

#endif
