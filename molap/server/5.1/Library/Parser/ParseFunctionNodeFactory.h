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
 * 
 *
 */

#ifndef PARSER_PARSE_FUNCTION_NODE_FACTORY_H
#define PARSER_PARSE_FUNCTION_NODE_FACTORY_H 1

#include <string>
#include <vector>
#include <map>
#include "Parser/FunctionNode.h"
#include "Parser/FunctionNodeFactory.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parse function node factory
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ParseFunctionNodeFactory : public FunctionNodeFactory {

public:
	ParseFunctionNodeFactory(set<string>* functionList);
	~ParseFunctionNodeFactory();

public:
	FunctionNode* createFunction(const string& name, vector<Node*> *params);

private:
	map<string, FunctionNode::CreateFunc_ptr> paloFunctions;

};
}
#endif
