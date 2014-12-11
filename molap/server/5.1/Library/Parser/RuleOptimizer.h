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

#ifndef PARSER_RULE_OPTIMIZER_H
#define PARSER_RULE_OPTIMIZER_H 1

#include "palo.h"

#include <set>

#include "Parser/AreaNode.h"

namespace palo {
class Cube;
class ExprNode;
class RuleNode;

////////////////////////////////////////////////////////////////////////////////
/// @brief rule optimizer
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS RuleOptimizer {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a new optimizer for a given cube
	////////////////////////////////////////////////////////////////////////////////

	RuleOptimizer(CPDatabase db, CPCube cube) : db(db), cube(cube), restrictedRule(0), linearRule(false) {};

	////////////////////////////////////////////////////////////////////////////////
	/// @brief destroys a optimizer
	////////////////////////////////////////////////////////////////////////////////

	~RuleOptimizer();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief optimize a rule
	////////////////////////////////////////////////////////////////////////////////

	bool optimize(RuleNode*);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns optimized rule
	////////////////////////////////////////////////////////////////////////////////

	ExprNode * getRestrictedRule() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns restricted identifiers
	////////////////////////////////////////////////////////////////////////////////

	const set<IdentifierType>& getRestrictedIdentifiers() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns restricted dimension
	////////////////////////////////////////////////////////////////////////////////

	IdentifierType getRestrictedDimension() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if rule is linear
	////////////////////////////////////////////////////////////////////////////////

	bool isLinearRule();

private:
	void checkLinearRule(ExprNode *node, const Area *area);
	void checkStetRule(ExprNode *node, const Area *area);

private:
	CPDatabase db;
	CPCube cube;
	ExprNode* restrictedRule;
	set<IdentifierType> restrictedIdentifiers;
	IdentifierType restrictedDimension;
	bool linearRule;
};
}

#endif
