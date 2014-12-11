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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "PaloBrowser/CubeAnalyzerJob.h"

#include "PaloDocumentation/CubeAnalyzerDocumentation.h"
#include "PaloDocumentation/HtmlFormatter.h"
#include "PaloJobs/AreaJob.h"
#include "Engine/EngineBase.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// PaloBrowserJob methods
// /////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

bool CubeAnalyzerJob::initialize()
{
	bool ok = PaloJob::initialize();

	if (!ok) {
		return false;
	}

	if (jobRequest->action) {
		action = *(jobRequest->action);
	}

	if (!action.empty()) {
		if (action == "load" || action == "save") {
			jobType = WRITE_JOB;
		}
	}
	return true;
}

void CubeAnalyzerJob::compute()
{
	findCube(true, false);

	// get the area
	const IdentifiersType* dimensions = cube->getDimensions();
	vector<CPDimension> dims;
	for (IdentifiersType::const_iterator it = dimensions->begin(); it != dimensions->end(); ++it) {
		dims.push_back(database->lookupDimension(*it, false));
	}
//	pdims = &dims;

	uint32_t numResult = 0;
	PCubeArea cubeArea;
	string pathStringMessage = "";
	string message1;

	if (jobRequest->area != 0 || jobRequest->areaName != 0) {
		try {
			if (jobRequest->area) {
				cubeArea = area(database, cube, jobRequest->area, &dims, numResult, false);
			} else if (jobRequest->areaName) {
				cubeArea = area(database, cube, jobRequest->areaName, &dims, numResult, false);
			}

			for (size_t i = 0; i < cube->getDimensions()->size(); i++) {
				if (i > 0) {
					pathStringMessage += ",";
				}

				if (cubeArea->elemCount(i) == dims[i]->sizeElements() || dims[i]->getDimensionType() == Dimension::VIRTUAL) {
					pathStringMessage += "*";
				} else {
					for (Area::ConstElemIter it = cubeArea->elemBegin(i); it != cubeArea->elemEnd(i);) {
						pathStringMessage += StringUtils::convertToString(*it);
						++it;
						if (it != cubeArea->elemEnd(i)) {
							pathStringMessage += ":";
						}
					}
				}
			}
		} catch (ParameterException &e) {
			numResult = 0;
			message1 = createHtmlMessage("Error", e.getMessage());
		}
	} else {
		pathStringMessage = "*";

		for (size_t i = 1; i < cube->getDimensions()->size(); i++) {
			pathStringMessage += ",*";
		}
		vector<IdentifiersType> fullArea(cube->getDimensions()->size());
		cubeArea = area(database, cube, &fullArea, &dims, numResult, false);
	}

	PPlanNode plan;
	if (numResult) {
		vector<User::RoleDbCubeRight> vRights;
		PCubeArea calcArea = checkRights(vRights, false, cubeArea, 0, cube, database, user, true, noPermission, isNoPermission, this->dims);
		plan = cube->createPlan(calcArea, CubeArea::ALL, RulesType(DIRECT_RULES || NOCACHE), true, UNLIMITED_SORTED_PLAN);
	}

	CubeAnalyzerDocumentation sbd(database, cube, plan, message1);
	HtmlFormatter hf(templatePath + "/browser_cube_analyzer.tmpl");

	generateResult(hf.getDocumentation(&sbd));
}

void CubeAnalyzerJob::appendValue(const IdentifiersType &key, const CellValue &val, const vector<CellValue> &prop_vals)
{
	identifiers.push_back(BrowserDocumentation::convertToString(&key));

	if (pdims) {
		StringBuffer sb;

		size_t size = key.size();
		for (size_t i = 0; i < size; i++) {
			const Dimension *dim = (*pdims)[i].get();
			string elementName;
			if (dim->getDimensionType() != Dimension::VIRTUAL) {
				const Element *elem = dim->lookupElement(key[i], false);
				elementName = elem->getName(dim->getElemNamesVector());
			} else {
				elementName = StringUtils::convertToString(key[i]);
			}
			sb.appendText(elementName);
			if (i < size - 1) {
				sb.appendText(", ");
			}
		}
		elemNames.push_back(sb.c_str());
	}

	if (val.isError()) {
		type.push_back("*ERROR*");
		rule.push_back("-");
		value.push_back(StringUtils::convertToString(val.getError()));
	} else {
		if (val.isString()) {
			if (!val.isEmpty()) {
				type.push_back(BrowserDocumentation::convertElementTypeToString(Element::STRING));
				rule.push_back(val.getRuleId() == NO_RULE ? "-" : StringUtils::convertToString(val.getRuleId()));
				value.push_back(StringUtils::escapeHtml(val));
			} else {
				type.push_back(BrowserDocumentation::convertElementTypeToString(Element::STRING));
				rule.push_back(val.getRuleId() == NO_RULE ? "-" : StringUtils::convertToString(val.getRuleId()));
				value.push_back("");
			}
		} else if (val.isNumeric()) {
			if (!val.isEmpty()) {
				type.push_back(BrowserDocumentation::convertElementTypeToString(Element::NUMERIC));
				rule.push_back(val.getRuleId() == NO_RULE ? "-" : StringUtils::convertToString(val.getRuleId()));
				value.push_back(val.toString());
			} else {
				type.push_back(BrowserDocumentation::convertElementTypeToString(Element::NUMERIC));
				rule.push_back(val.getRuleId() == NO_RULE ? "-" : StringUtils::convertToString(val.getRuleId()));
				value.push_back("0");
			}
		}
	}
}

}
