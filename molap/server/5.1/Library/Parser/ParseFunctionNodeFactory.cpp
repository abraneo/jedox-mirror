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

#include "Parser/ParseFunctionNodeFactory.h"

#include "Parser/FunctionNodeAdd.h"
#include "Parser/FunctionNodeDel.h"
#include "Parser/FunctionNodeDiv.h"
#include "Parser/FunctionNodeError.h"
#include "Parser/FunctionNodeMul.h"
#include "Parser/FunctionNodeParse.h"

#include <sstream>

namespace palo {

ParseFunctionNodeFactory::ParseFunctionNodeFactory(set<string>* functionList)
{
	for (set<string>::iterator i = functionList->begin(); i != functionList->end(); ++i) {
		if (!i->empty()) {
			paloFunctions[*i] = &FunctionNodeParse::createNode;
		}
	}

	// register default functions
	paloFunctions["add"] = &FunctionNodeAdd::createNode;
	paloFunctions["del"] = &FunctionNodeDel::createNode;
	paloFunctions["div"] = &FunctionNodeDiv::createNode;
	paloFunctions["mul"] = &FunctionNodeMul::createNode;
}

ParseFunctionNodeFactory::~ParseFunctionNodeFactory()
{
}

FunctionNode* ParseFunctionNodeFactory::createFunction(const string& name, vector<Node*> *params)
{

	map<string, FunctionNode::CreateFunc_ptr>::iterator func = paloFunctions.find(name);

	if (func == paloFunctions.end()) {
		// not found
		return FunctionNodeError::createNode(name, params);
	}

	FunctionNode::CreateFunc_ptr p = func->second;
	return p(name, params);
}
}
