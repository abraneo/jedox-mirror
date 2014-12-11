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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Lists/SubSet.h"
#include "ViewCalculateJob.h"
#include "Lists/PickList.h"
#include "Lists/TextFilter.h"
#include "Lists/AliasFilter.h"
#include "Lists/StructuralFilter.h"
#include "Lists/DataFilter.h"
#include "Lists/SortingFilter.h"
#include "Parser/RuleParserDriver.h"
#include "Olap/Rule.h"

namespace palo {

class CreateAxis {
public:
	CreateAxis(StringBuffer &body, ViewAxis &axis, map<string, pair<PDimension, PSubSet> > &subsets, PCubeArea area, map<IdentifierType, IdentifierType> &id2ord, ViewCalculateJob *job);
	void createAxis(size_t curr, bool check_limit);
private:
	StringBuffer &body;
	vector<pair<PDimension, SubSet::Iterator> > prevElems;
	ViewAxis &axis;
	map<string, pair<PDimension, PSubSet> > &subsets;
	PCubeArea area;
	map<IdentifierType, IdentifierType> &id2ord;
	ViewCalculateJob *job;
	vector<pair<bool, set<IdentifierType> > > valid;
	vector<PCubeArea> currAreas;
	vector<size_t> dependencies;
	vector<bool> dependent;
	size_t axisSize;
	static const size_t MAX_SIZE = 100000;
};

CreateAxis::CreateAxis(StringBuffer &body, ViewAxis &axis, map<string, pair<PDimension, PSubSet> > &subsets, PCubeArea area, map<IdentifierType, IdentifierType> &id2ord, ViewCalculateJob *job) :
	body(body), axis(axis), subsets(subsets), area(area), id2ord(id2ord), job(job), axisSize(0)
{
	prevElems.resize(axis.as.size());
	valid.resize(axis.as.size(), make_pair(false, set<IdentifierType>()));
	currAreas.resize(axis.as.size());
	dependencies.resize(axis.as.size(), (size_t)-1);
	dependent.resize(axis.as.size(), false);
	map<string, size_t> name_pos;
	for (size_t i = 0; i < axis.as.size(); ++i) {
		if (!axis.as[i].parent.empty()) {
			map<string, size_t>::iterator it = name_pos.find(axis.as[i].parent);
			if (it == name_pos.end()) {
				throw ParameterException(ErrorException::ERROR_ID_NOT_FOUND, "Subset dependencies invalid. Subset not found on axis or bad order.", PaloRequestHandler::VIEW_AXES, (unsigned int)i);
			}
			dependencies[i] = it->second;
			dependent[it->second] = true;
		}
		name_pos[axis.as[i].subsetHandle] = i;
	}
}

void CreateAxis::createAxis(size_t curr, bool check_limit)
{
	if (curr == axis.as.size()) {
		if (check_limit && ++axisSize > MAX_SIZE) {
			throw ParameterException(ErrorException::ERROR_MAX_ELEM_REACHED, "Axis too big.", PaloRequestHandler::VIEW_AXES, 0);
		}
		for (vector<pair<PDimension, SubSet::Iterator> >::iterator it = prevElems.begin(); it != prevElems.end(); ++it) {
			vector<User::RoleDbCubeRight> vRights;
			if (it->first->getDimensionType() == Dimension::VIRTUAL) {
				job->appendElement(&body, it->first, 0, it->second.getId(), false, vRights, it->second.getDepth(), it->second.getIndent());
			} else {
				job->appendElement(&body, it->first, it->second.getElement(), 0, false, vRights, it->second.getDepth(), it->second.getIndent());
			}
			body.appendCsvString(it->second.getSearchAlias(false).empty() ? "" : StringUtils::escapeString(it->second.getSearchAlias(false)));
			body.appendCsvString(it->second.getPath());
			body.appendEol();
		}
	} else {
		map<string, pair<PDimension, PSubSet> >::iterator it_sub = subsets.find(axis.as[curr].subsetHandle);
		set<IdentifierType> valid_curr;
		set<IdentifierType> *valid_tmp = &valid[curr].second;
		PCubeArea curr_area = area;
		if (it_sub == subsets.end()) {
			throw ParameterException(ErrorException::ERROR_ID_NOT_FOUND, "no such subset", PaloRequestHandler::VIEW_AXES, axis.as[curr].subsetHandle);
		}
		if (axis.as[curr].zeroSup && area && area->getSize()) {
			bool calc = true;
			if (axis.as[curr].parent.empty()) {
				calc = !valid[curr].first;
				valid[curr].first = true;
			} else {
				valid_tmp = &valid_curr;
				curr_area = currAreas[dependencies[curr]];
			}
			if (calc) {
				PPlanNode plan = curr_area->getCube()->createPlan(curr_area, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_SORTED_PLAN);
				vector<PPlanNode> children;
				children.push_back(plan);
				int dimIndex = id2ord[it_sub->second.first->getId()];
				plan.reset(new QuantificationPlanNode(curr_area, children, dimIndex, it_sub->second.first->getDimensionType() == Dimension::VIRTUAL, true, 0));
				PCellStream cs = curr_area->getCube()->evaluatePlan(plan, EngineBase::ANY, true);
				while (cs->next()) {
					IdentifierType id = cs->getKey()[dimIndex];
					valid_tmp->insert(id);
				}
			}
		}

		for (SubSet::Iterator eit = it_sub->second.second->begin(true); !eit.end(); ++eit) {
			if (!(area && axis.as[curr].zeroSup) || valid_tmp->find(eit.getId()) != valid_tmp->end()) {
				prevElems[curr] = make_pair(it_sub->second.first, eit);
				if (dependent[curr] && area) {
					currAreas[curr].reset(new CubeArea(*curr_area));
					PSet s(new Set);
					s->insert(eit.getId());
					currAreas[curr]->insert(id2ord[it_sub->second.first->getId()], s, true);
				}
				createAxis(curr + 1, check_limit);
			}
		}
	}
}

void ViewCalculateJob::compute()
{
	if (!jobRequest->viewSubsets || !jobRequest->viewAxes) {
		throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "subset or axis not defined", PaloRequestHandler::VIEW_AXES, "");
	}
	bool bHasRules = hasRules();
	server = bHasRules ? Context::getContext()->getServerCopy() : Context::getContext()->getServer();
	checkToken(server);
	findDatabase(true, bHasRules);
	if (jobRequest->cube != NO_IDENTIFIER) {
		findCube(true, bHasRules);
	}

	if (cube && bHasRules) {
		createRules();
	}

	map<string, pair<PDimension, PSubSet> > subsets;
	for (map<string, ViewSubset>::iterator it = jobRequest->viewSubsets->begin(); it != jobRequest->viewSubsets->end(); ++it) {
		pair<PDimension, PSubSet> &elems = subsets[it->first];
		elems.first = database->findDimensionByName(it->second.dimension, user, false);
		elems.second = subset(database, user, elems.first, it->second);
	}

	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();
	setToken(server);

	PCubeArea area;
	map<IdentifierType, IdentifierType> id2ord;
	if (cube) {
		const IdentifiersType *dims = cube->getDimensions();
		if (dims->size() != subsets.size()) {
			throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of subsets on axes", PaloRequestHandler::VIEW_AXES, "");
		}
		area.reset(new CubeArea(database, cube, dims->size()));
		for (size_t i = 0; i < dims->size(); ++i) {
			id2ord[dims->at(i)] = i;
		}
		for (map<string, pair<PDimension, PSubSet> >::iterator it = subsets.begin(); it != subsets.end(); ++it) {
			map<IdentifierType, IdentifierType>::iterator id_it = id2ord.find(it->second.first->getId());
			if (id_it == id2ord.end()) {
				throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "dimension not in cube", PaloRequestHandler::VIEW_SUBSETS, it->second.first->getName());
			}
			area->insert(id_it->second, it->second.second->getSet(false), true);
		}
	}

	for (size_t i = 0; i < 3; ++i) {
		if (!jobRequest->viewAxes->at(i).as.empty()) {
			body.appendText("[Axis ");
			body.appendInteger(i);
			body.appendText("]");
			body.appendEol();
			CreateAxis ca(body, jobRequest->viewAxes->at(i), subsets, area, id2ord, this);
			ca.createAxis(0, cube || subsets.size() > 1);
		}
	}

	if (cube) {
		PCellStream cs;
		PCellStream props;
		vector<User::RoleDbCubeRight> vRights;
		if (User::checkUser(user)) {
			user->fillRights(vRights, User::cellDataRight, database, cube);
		}
		bool checkPermissions = cube->getMinimumAccessRight(user) == RIGHT_NONE;
		PCubeArea calcArea = checkRights(vRights, checkPermissions, area, 0, cube, database, user, true, noPermission, isNoPermission, this->dims);

		if (jobRequest->viewArea && !jobRequest->viewArea->properties.empty()) {
			if (jobRequest->properties) {
				delete jobRequest->properties;
			}
			jobRequest->properties = new IdentifiersType;
			jobRequest->properties->reserve(jobRequest->viewArea->properties.size());
			PDimension cellPropDim = database->findDimensionByName(SystemDatabase::NAME_CELL_PROPERTIES_DIMENSION, user, false);
			for (vector<string>::iterator it = jobRequest->viewArea->properties.begin(); it != jobRequest->viewArea->properties.end(); ++it) {
				Element *el = cellPropDim->findElementByName(*it, user.get(), false);
				jobRequest->properties->push_back(el->getIdentifier());
			}
		}

		if (calcArea->getSize()) {
			cs = cube->calculateArea(calcArea, CubeArea::ALL, ALL_RULES, true, UNLIMITED_SORTED_PLAN);
			if (jobRequest->viewArea && !jobRequest->viewArea->properties.empty()) {
				props = getCellPropsStream(database, cube, calcArea, *jobRequest->properties);
			}
		}
		body.appendText("[Area]");
		body.appendEol();
		jobRequest->showRule = true;
		jobRequest->showLockInfo = true;
		loop(area, calcArea, cs, NULL, props, vRights, 0, false);
	}
}

PSubSet ViewCalculateJob::subset(PDatabase database, PUser user, PDimension dim, ViewSubset &subdef)
{
	PSubSet sub(new SubSet(database, dim, user, subdef.basic, subdef.text, subdef.sorting, subdef.alias, subdef.field, subdef.structural, subdef.data));
	sub->apply();
	return sub;
}

void ViewCalculateJob::createRules()
{
	map<string, string> dims;
	vector<string> rules2add;
	for (map<string, ViewSubset>::iterator it = jobRequest->viewSubsets->begin(); it != jobRequest->viewSubsets->end(); ++it) {
		dims[it->first] = it->second.dimension;
	}
	PDimensionList dimList = database->getDimensionList(true);
	database->setDimensionList(dimList);
	for (vector<ViewAxis>::iterator axisit = jobRequest->viewAxes->begin(); axisit != jobRequest->viewAxes->end(); ++axisit) {
		for (AxisSubsets::iterator asit = axisit->as.begin(); asit != axisit->as.end(); ++asit) {
			if (!asit->calculation.empty()) {
				map<string, string>::iterator dimit = dims.find(asit->subsetHandle);
				if (dimit == dims.end()) {
					throw ParameterException(ErrorException::ERROR_ID_NOT_FOUND, "no such subset", PaloRequestHandler::VIEW_AXES, asit->subsetHandle);
				}
				PDimension dim = database->findDimensionByName(dimit->second, user, true);
				dimList->set(dim);
				vector<string> rules;
				StringUtils::splitString3(asit->calculation, rules, ';', false);
				bool simple = false;
				int pos = -100000;
				string userpos;
				for (vector<string>::iterator rit = rules.begin(); rit != rules.end(); ++rit) {
					if (*rit == RuleParserDriver::SIMPLE_RULE) {
						simple = true;
						continue;
					}
					if (!rit->find(RuleParserDriver::POSITION_RULE)) {
						userpos = *rit;
						continue;
					}
					vector<string> parts;
					StringUtils::splitString3(*rit, parts, '=', false);
					if (parts.size() < 2) {
						throw ParameterException(ErrorException::ERROR_PARSING_RULE, "invalid rule", PaloRequestHandler::VIEW_AXES, *rit);
					}
					string name = stripEName(parts[0], '\'');
					dim->addElement(server, database, NO_IDENTIFIER, name, Element::NUMERIC, PUser(), false);
					stringstream rule;
					if (simple) {
						rule << RuleParserDriver::SIMPLE_RULE << ';';
						simple = false;
					}
					if (userpos.empty()) {
						rule << RuleParserDriver::POSITION_RULE << '=' << pos++ << ';';
					} else {
						rule << userpos << ';';
					}
					rule << "['" << dim->getName() << "':'";
					for (string::iterator it = name.begin(); it != name.end(); ++it) {
						if (*it == '\'') {
							rule << '\'';
						}
						rule << *it;
					}
					rule << "']";
					for (vector<string>::iterator it = parts.begin() + 1; it != parts.end(); ++it) {
						rule << "=" << *it;
					}
					rules2add.push_back(rule.str());
				}
				dim->mergeUpdateVector();
			}
		}
	}
	PRuleList ruleList = cube->getRuleList(true);
	cube->setRuleList(ruleList);
	for (vector<string>::iterator it = rules2add.begin(); it != rules2add.end(); ++it) {
		RuleParserDriver driver;
		driver.parse(*it);
		PRuleNode r = PRuleNode(driver.getResult());
		if (r) {
			string errorMsg;
			bool ok = r->validate(server, database, cube, errorMsg);
			if (!ok) {
				throw ParameterException(ErrorException::ERROR_PARSING_RULE, errorMsg, PaloRequestHandler::DEFINITION, *it);
			}
		} else {
			throw ParameterException(ErrorException::ERROR_PARSING_RULE, driver.getErrorMessage(), PaloRequestHandler::DEFINITION, *it);
		}
		PRule rule = PRule(new Rule(r, database, cube, *it, "", "", ::time(NULL), true));
		rule->setPosition(driver.getPosition());
		rule->setCustom();
		ruleList->add(rule, true);
	}
	context->eraseEngineCube(make_pair(database->getId(), cube->getId()));
}

bool ViewCalculateJob::hasRules()
{
	for (vector<ViewAxis>::iterator axisit = jobRequest->viewAxes->begin(); axisit != jobRequest->viewAxes->end(); ++axisit) {
		for (AxisSubsets::iterator asit = axisit->as.begin(); asit != axisit->as.end(); ++asit) {
			if (!asit->calculation.empty()) {
				return true;
			}
		}
	}
	return false;
}

string ViewCalculateJob::stripEName(const string &name, char quote)
{
	stringstream res;
	bool inquote = false;
	for (string::const_iterator it = name.begin(); it != name.end(); ++it) {
		if (inquote) {
			if (*it == quote) {
				string::const_iterator itn(it);
				++itn;
				if (itn != name.end() && *itn == quote) {
					res << *it++;
				} else {
					inquote = false;
				}
			} else {
				res << *it;
			}
		} else {
			if (*it == quote) {
				inquote = true;
			}
		}
	}
	return res.str();
}

}
