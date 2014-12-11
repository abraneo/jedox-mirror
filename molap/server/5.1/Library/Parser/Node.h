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

#ifndef PARSER_NODE_H
#define PARSER_NODE_H

#include "palo.h"

#include <string>

#include "Collections/StringBuffer.h"

#include "Olap/Server.h"
#include "Olap/Database.h"
#include "Olap/Cube.h"

#include "VirtualMachine/BytecodeGenerator.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief represents rule variability
//  max 256 dimensions = 4x64bits
////////////////////////////////////////////////////////////////////////////////

class Variability {
public:
	Variability() {data[0] = data[1] = data[2] = data[3] = 0;}
	Variability(const Variability &b) {data[0] = b.data[0]; data[1] = b.data[1]; data[2] = b.data[2]; data[3] = b.data[3];}
	bool empty() const {return !data[0] && !data[1] && !data[2] && !data[3];}
	void set(size_t dim) {data[dim >> 6] |= 1LL << (dim & 0x3f);}
	void fill() {data[0] = data[1] = data[2] = data[3] = ~0;}
	Variability& operator |=(const Variability &b) {data[0] |= b.data[0]; data[1] |= b.data[1]; data[2] |= b.data[2]; data[3] |= b.data[3]; return *this;}
	bool operator!=(const Variability &o) const {return data[0]!=o.data[0] || data[1]!=o.data[1] || data[2]!=o.data[2] || data[3]!=o.data[3];}
private:
	uint64_t data[4];
};

////////////////////////////////////////////////////////////////////////////////
/// @brief parser node
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Node {
public:
	enum ValueType {
		NODE_UNKNOWN_VALUE, NODE_NUMERIC, NODE_STRING
	};

	enum NodeType {
		NODE_FUNCTION_PALO_MARKER, NODE_FUNCTION_PALO_DATA, NODE_DOUBLE_NODE, NODE_FUNCTION_DIV, NODE_FUNCTION_IF, NODE_FUNCTION_MULT, NODE_FUNCTION_STET, NODE_SOURCE_NODE, NODE_STRING_NODE, NODE_UNKNOWN, NODE_VARIABLE_NODE, NODE_FUNCTION_ADD, NODE_FUNCTION_SUB
	};

	struct RuleValueType {
		ValueType type;
		double doubleValue;
		string stringValue;
	};

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a new node
	////////////////////////////////////////////////////////////////////////////////

	Node() :
		valid(false) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief destructs a node
	////////////////////////////////////////////////////////////////////////////////

	virtual ~Node() {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief clones an expression node
	////////////////////////////////////////////////////////////////////////////////

	virtual Node * clone() = 0;

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if rule is valid
	////////////////////////////////////////////////////////////////////////////////

	virtual bool isValid() {
		return valid;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if rule is simple expression (constant or area)
	////////////////////////////////////////////////////////////////////////////////

	virtual bool isSimple() const {
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if rule can be converted to a plan
	////////////////////////////////////////////////////////////////////////////////

	virtual bool isPlanCompatible(CPCubeArea area, Variability &varDimensions) const
	{
		varDimensions.fill();
		return false;
	}

	virtual PPlanNode createPlan(CPCubeArea area, CPRule rule, double &constResult, bool &valid, class Planner *planner) const
	{
		constResult = 0;
		valid = true;
		return PPlanNode();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the node type
	////////////////////////////////////////////////////////////////////////////////

	virtual NodeType getNodeType() const {
		return NODE_UNKNOWN;
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief validates the expression
	////////////////////////////////////////////////////////////////////////////////

	virtual bool validate(PServer, PDatabase, PCube, Node*, string&) = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the value type of the underlying expression
	////////////////////////////////////////////////////////////////////////////////

	virtual ValueType getValueType() = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the value of the underlying expression
	////////////////////////////////////////////////////////////////////////////////

	virtual RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if element is involved in rule
	////////////////////////////////////////////////////////////////////////////////

	virtual bool hasElement(CPDimension dimension, IdentifierType element) const {
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends XML representation to string buffer
	////////////////////////////////////////////////////////////////////////////////

	virtual void appendXmlRepresentation(StringBuffer* sb, int indent, bool) = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends representation to string buffer
	////////////////////////////////////////////////////////////////////////////////

	virtual void appendRepresentation(StringBuffer*, CPDatabase, CPCube) const {
	} // = 0;

public:
	virtual bool isDimensionRestriction(CPCube cube, IdentifierType *dimension) {
		return false;
	}

	virtual bool isDimensionTransformation(CPCube cube, IdentifierType *dimension) {
		return false;
	}

	virtual set<Element*> computeDimensionRestriction(CPDatabase db, CPCube cube) {
		set<Element*> elements;

		return elements;
	}

	virtual map<Element*, string> computeDimensionTransformations(CPDatabase db, CPCube cube) {
		map<Element*, string> elements;

		return elements;
	}

	virtual bool isConstant() {
		return false;
	}

	virtual void collectMarkers(vector<Node*>& markers) {
	}

	virtual uint32_t guessType(uint32_t level) {
		Logger::trace << "guessType " << "level " << level << "node " << "unknown" << " type " << "unknown" << endl;
		return Node::NODE_UNKNOWN_VALUE;
	}

	virtual bool genCode(bytecode_generator& generator, uint8_t want) const = 0;
	//    {
	//        return generator.EmitNopCode();
	//    }

protected:
	void identXML(StringBuffer* sb, int ident) {
		for (int i = 0; i < ident * 2; i++) {
			sb->appendChar(' ');
		}
	}

protected:
	bool valid;
};
}
#endif
