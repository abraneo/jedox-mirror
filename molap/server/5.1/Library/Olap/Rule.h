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
 * 
 *
 */

#ifndef OLAP_RULE_H
#define OLAP_RULE_H 1

#include "palo.h"

#include "Parser/RuleNode.h"

#define SORT_RULES_BY_POSITION

namespace palo {
class ExprNode;
typedef boost::shared_ptr<ExprNode> PExprNode;

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP enterprise rule
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Rule : public Commitable {
public:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates new rule from rule node
	////////////////////////////////////////////////////////////////////////////////

	Rule(PRuleNode ruleNode, PDatabase db, PCube cube, const string& definition, const string& external, const string& comment, time_t timestamp, bool activeRule) :
		Commitable(""), rule(ruleNode), comment(comment), external(external), timestamp(timestamp), activeRule(activeRule), isOptimized(false),
		restrictedRule(PExprNode()), restrictedDimension(0), evalCounter(0), evalNullCounter(0), position(0), custom(false)

	{
		this->rule = ruleNode; // rule can be compilable but not active, ruleNode used for definition generation
		if (this->rule) {
			setDefinitionFromRuleNode(db, cube);
		} else {
			this->definition = definition;
		}
		if (this->rule && !activeRule) {
			this->rule.reset();
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Destructor
	////////////////////////////////////////////////////////////////////////////////

	virtual ~Rule();

	bool isSimpleRule(CPCubeArea area, Variability &varDimensions) const;

public:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets external identifier of rule
	////////////////////////////////////////////////////////////////////////////////

	const string& getExternal() const {
		return external;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets external identifier of rule
	////////////////////////////////////////////////////////////////////////////////

	void setExternal(const string& external) {
		this->external = external;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets comment for a rule
	////////////////////////////////////////////////////////////////////////////////

	const string& getDefinition() const {
		return definition;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets comment for a rule
	////////////////////////////////////////////////////////////////////////////////

	const string& getComment() const {
		return comment;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets comment for a rule
	////////////////////////////////////////////////////////////////////////////////

	void setComment(const string& comment) {
		this->comment = comment;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets timestamp for a rule
	////////////////////////////////////////////////////////////////////////////////

	time_t getTimeStamp() const {
		return timestamp;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets a new definition
	////////////////////////////////////////////////////////////////////////////////

	void setDefinition(PRuleNode node, const string& definition, PDatabase db, PCube cube) {
		rule = node;

		resetCounter();
		if (rule) {
			setDefinitionFromRuleNode(db, cube);
		} else {
			this->definition = definition;
		}

		::time(&timestamp);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets a definition string used by (later) disabled rules
	////////////////////////////////////////////////////////////////////////////////

	void setDefinitionFromRuleNode(PDatabase db, PCube cube) {
		if (rule) {
			resetCounter();
			definition = getTextRepresentation(db, cube); // use parsed & compiled version of definition string for output, if disabled later
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief return true if the rule is active
	////////////////////////////////////////////////////////////////////////////////

	bool isActive() const {
		return activeRule;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief activate or deactivate rule
	////////////////////////////////////////////////////////////////////////////////

	bool setActive(bool isActive, PServer server, PDatabase db, PCube cube, string* errMsg);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the rule option
	////////////////////////////////////////////////////////////////////////////////

	RuleNode::RuleOption getRuleOption() const {
		if (activeRule && rule) {
			return rule->getRuleOption();
		} else {
			return RuleNode::NONE;
		}
	}

	void resetCounter() const {const_cast<Rule*>(this)->evalCounter = 0; const_cast<Rule*>(this)->evalNullCounter = 0;}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief linear rule
	////////////////////////////////////////////////////////////////////////////////

	bool isLinearRule() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief restricted rule
	////////////////////////////////////////////////////////////////////////////////

	bool isRestrictedRule() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief return destination area
	////////////////////////////////////////////////////////////////////////////////

	const Area *getDestinationArea() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if cell path is within destination area
	////////////////////////////////////////////////////////////////////////////////

	bool withinArea(const Area *cellPath) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if cell path is within restricted area of optimized rule
	////////////////////////////////////////////////////////////////////////////////

	bool withinRestricted(const Area *cellPath) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if cell path is within destination area and of correct type
	////////////////////////////////////////////////////////////////////////////////

	bool within(const CubeArea *cellPath) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if cell path is touched by destination area
	///
	/// Returns true if the destination area is within a the sub-cube identified
	/// by a consolidated path.
	////////////////////////////////////////////////////////////////////////////////

	bool contains(const Area *cellPath) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if cell path is touched by restricted area of optimized rule
	////////////////////////////////////////////////////////////////////////////////

	bool containsRestricted(const Area *cellPath) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends a representation to the string buffer
	////////////////////////////////////////////////////////////////////////////////

	void appendRepresentation(StringBuffer* sb, CPDatabase db, CPCube cube, bool names = false) const {
		if (rule) { // activeRule || will be activated now
			rule->appendRepresentation(sb, db, names ? cube : CPCube());
		} else {
			sb->appendText(definition);
		}
	}

	string getTextRepresentation(CPDatabase db, CPCube cube) const {
		StringBuffer sb;
		appendRepresentation(&sb, db, cube, true);
		return sb.c_str();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if elements is contained in rule
	////////////////////////////////////////////////////////////////////////////////

	bool hasElement(CPDimension dimension, IdentifierType element) const {
		if (activeRule && rule) {
			return rule->hasElement(dimension, element);
		} else {
			return false;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if rule has markers
	////////////////////////////////////////////////////////////////////////////////

	bool hasMarkers() const {
		if (activeRule && rule) {
			return !(rule->getExternalMarkers().empty() && rule->getInternalMarkers().empty());
		} else {
			return !(markers.empty());
		}
	}

	void computeMarkers();

	void removeMarkers();

	const vector<PRuleMarker>& getMarkers() const {
		return markers;
	}

	uint32_t guessType(uint32_t level) {
		Logger::trace << "guessType " << "level " << level << "node " << "Rule " << " type " << "none" << endl;
		return rule->guessType(level + 1);
	}

	bool genCode(bytecode_generator& generator) const {
		return rule->genCode(generator, Node::NODE_NUMERIC);
	}

	void increaseEvalCounter(bool nullValue) const {
		Rule& ncthis = const_cast<Rule&>(*this);
		ncthis.evalCounter++;
		if (nullValue) {
			ncthis.evalNullCounter++;
		}
	}
	void getEvalCounters(uint64_t *pevalCounter, uint64_t *pevalNullCounter) const {
		if (pevalCounter) {
			*pevalCounter = evalCounter;
		}
		if (pevalNullCounter) {
			*pevalNullCounter = evalNullCounter;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets a rule priority
	////////////////////////////////////////////////////////////////////////////////
	double getPosition() const {return position;}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets rule priority
	////////////////////////////////////////////////////////////////////////////////
	void setPosition(double position) {checkCheckedOut(); this->position = position;}

	bool isCustom() const {return custom;}
	void setCustom() {checkCheckedOut(); custom = true;}

	void onCubeChange(CPDatabase db, CPCube cube);

	virtual bool merge(const CPCommitable &o, const PCommitable &p);

	virtual PCommitable copy() const;

private:

	void computeContains(CPDatabase db, CPCube cube);

	void computeContainsRestricted(CPDatabase db, CPCube cube);

	void optimizeRule(CPDatabase db, CPCube cube);

	void constructMarkers(const vector<Node*>& markerList, const Area *destinationArea);

	Rule(const Rule &other);

	PRuleMarker convertMarker(CPCube cube, const Area* destination, Node* node);

private:
	PRuleNode rule;	// valid only if activeRule==true
	string definition;
	string comment;
	string external;
	time_t timestamp;
	bool activeRule;

	bool isOptimized;
	PExprNode restrictedRule;
	IdentifierType restrictedDimension;
	set<IdentifierType> restrictedIdentifiers;
	bool linearRule;

	PArea restrictedArea;
	PArea containsArea;
	PArea containsRestrictedArea;

	vector<PRuleMarker> markers;

	uint64_t evalCounter;
	uint64_t evalNullCounter;

	double position;
	bool custom;

	friend class Planner;
};

bool rulePositionCompare(PRule i,PRule j);

}

#endif
