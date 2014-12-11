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

#include "Parser/PaloFunctionNodeFactory.h"

#include "Parser/FunctionNodeAbs.h"
#include "Parser/FunctionNodeAcos.h"
#include "Parser/FunctionNodeAdd.h"
#include "Parser/FunctionNodeAggregate.h"
#include "Parser/FunctionNodeAsin.h"
#include "Parser/FunctionNodeAtan.h"
#include "Parser/FunctionNodeCeiling.h"
#include "Parser/FunctionNodeChar.h"
#include "Parser/FunctionNodeClean.h"
#include "Parser/FunctionNodeCode.h"
#include "Parser/FunctionNodeConcatenate.h"
#include "Parser/FunctionNodeContinue.h"
#include "Parser/FunctionNodeCos.h"
#include "Parser/FunctionNodeDate.h"
#include "Parser/FunctionNodeDatevalue.h"
#include "Parser/FunctionNodeDateformat.h"
#include "Parser/FunctionNodeDel.h"
#include "Parser/FunctionNodeDiv.h"
#include "Parser/FunctionNodeEq.h"
#include "Parser/FunctionNodeError.h"
#include "Parser/FunctionNodeEven.h"
#include "Parser/FunctionNodeExact.h"
#include "Parser/FunctionNodeExp.h"
#include "Parser/FunctionNodeFact.h"
#include "Parser/FunctionNodeFloor.h"
#include "Parser/FunctionNodeGe.h"
#include "Parser/FunctionNodeGt.h"
#include "Parser/FunctionNodeIf.h"
#include "Parser/FunctionNodeInt.h"
#include "Parser/FunctionNodeIsError.h"
#include "Parser/FunctionNodeLe.h"
#include "Parser/FunctionNodeLeft.h"
#include "Parser/FunctionNodeLen.h"
#include "Parser/FunctionNodeLn.h"
#include "Parser/FunctionNodeLog.h"
#include "Parser/FunctionNodeLog10.h"
#include "Parser/FunctionNodeLower.h"
#include "Parser/FunctionNodeLt.h"
#include "Parser/FunctionNodeMid.h"
#include "Parser/FunctionNodeMod.h"
#include "Parser/FunctionNodeMul.h"
#include "Parser/FunctionNodeNe.h"
#include "Parser/FunctionNodeNot.h"
#include "Parser/FunctionNodeNow.h"
#include "Parser/FunctionNodeOdd.h"
#include "Parser/FunctionNodePi.h"
#include "Parser/FunctionNodePower.h"
#include "Parser/FunctionNodeProper.h"
#include "Parser/FunctionNodeQuotient.h"
#include "Parser/FunctionNodeRand.h"
#include "Parser/FunctionNodeRandbetween.h"
#include "Parser/FunctionNodeReplace.h"
#include "Parser/FunctionNodeRept.h"
#include "Parser/FunctionNodeRight.h"
#include "Parser/FunctionNodeRound.h"
#include "Parser/FunctionNodeSearch.h"
#include "Parser/FunctionNodeSign.h"
#include "Parser/FunctionNodeSin.h"
#include "Parser/FunctionNodeSqrt.h"
#include "Parser/FunctionNodeStet.h"
#include "Parser/FunctionNodeStr.h"
#include "Parser/FunctionNodeSubstitute.h"
#include "Parser/FunctionNodeTan.h"
#include "Parser/FunctionNodeTrim.h"
#include "Parser/FunctionNodeTrunc.h"
#include "Parser/FunctionNodeUpper.h"
#include "Parser/FunctionNodeValue.h"
#include "Parser/FunctionNodeValuedate.h"
#include "Parser/FunctionNodeWeekday.h"

#include "Parser/FunctionNodePaloCubedimension.h"
#include "Parser/FunctionNodePaloData.h"
#include "Parser/FunctionNodePaloEchild.h"
#include "Parser/FunctionNodePaloEchildcount.h"
#include "Parser/FunctionNodePaloEcount.h"
#include "Parser/FunctionNodePaloEfirst.h"
#include "Parser/FunctionNodePaloEindent.h"
#include "Parser/FunctionNodePaloEindex.h"
#include "Parser/FunctionNodePaloEischild.h"
#include "Parser/FunctionNodePaloElevel.h"
#include "Parser/FunctionNodePaloEname.h"
#include "Parser/FunctionNodePaloEnext.h"
#include "Parser/FunctionNodePaloEparent.h"
#include "Parser/FunctionNodePaloEparentcount.h"
#include "Parser/FunctionNodePaloEprev.h"
#include "Parser/FunctionNodePaloEsibling.h"
#include "Parser/FunctionNodePaloEtoplevel.h"
#include "Parser/FunctionNodePaloEtype.h"
#include "Parser/FunctionNodePaloEtype.h"
#include "Parser/FunctionNodePaloEweight.h"
#include "Parser/FunctionNodePaloMarker.h"
#include "Parser/FunctionNodePaloEoffset.h"

#include <sstream>

namespace palo {

map<string, FunctionNode::CreateFunc_ptr> PaloFunctionNodeFactory::paloFunctions;
bool PaloFunctionNodeFactory::isRegistered;

PaloFunctionNodeFactory::PaloFunctionNodeFactory()
{
	if (!isRegistered) {
		registerFunctions();
	}
}

PaloFunctionNodeFactory::~PaloFunctionNodeFactory()
{
}

FunctionNode* PaloFunctionNodeFactory::createFunction(const string& name, vector<Node*> *params)
{

	map<string, FunctionNode::CreateFunc_ptr>::iterator func = paloFunctions.find(name);

	if (func == paloFunctions.end()) {
		// not found
		return FunctionNodeError::createNode(name, params);
	}

	FunctionNode::CreateFunc_ptr p = func->second;
	return p(name, params);
}

void PaloFunctionNodeFactory::registerFunctions()
{
	registerFunc("+", &FunctionNodeAdd::createNodeInfix);
	registerFunc("-", &FunctionNodeDel::createNodeInfix);
	registerFunc("*", &FunctionNodeMul::createNodeInfix);
	registerFunc("/", &FunctionNodeDiv::createNodeInfix);

	registerFunc(">=", &FunctionNodeGe::createNodeInfix);
	registerFunc(">", &FunctionNodeGt::createNodeInfix);
	registerFunc("==", &FunctionNodeEq::createNodeInfix);
	registerFunc("!=", &FunctionNodeNe::createNodeInfix);
	registerFunc("<=", &FunctionNodeLe::createNodeInfix);
	registerFunc("<", &FunctionNodeLt::createNodeInfix);

	registerFunc("abs", &FunctionNodeAbs::createNode);
	registerFunc("acos", &FunctionNodeAcos::createNode);
	registerFunc("average", &FunctionNodeAggregate::createNode);
	registerFunc("add", &FunctionNodeAdd::createNode);
	registerFunc("and", &FunctionNodeAggregate::createNode);
	registerFunc("asin", &FunctionNodeAsin::createNode);
	registerFunc("atan", &FunctionNodeAtan::createNode);
	registerFunc("ceiling", &FunctionNodeCeiling::createNode);
	registerFunc("char", &FunctionNodeChar::createNode);
	registerFunc("clean", &FunctionNodeClean::createNode);
	registerFunc("code", &FunctionNodeCode::createNode);
	registerFunc("concatenate", &FunctionNodeConcatenate::createNode);
	registerFunc("continue", &FunctionNodeContinue::createNode);
	registerFunc("count", &FunctionNodeAggregate::createNode);
	registerFunc("cos", &FunctionNodeCos::createNode);
	registerFunc("date", &FunctionNodeDate::createNode);
	registerFunc("datevalue", &FunctionNodeDatevalue::createNode);
	registerFunc("dateformat", &FunctionNodeDateformat::createNode);
	registerFunc("del", &FunctionNodeDel::createNode);
	registerFunc("div", &FunctionNodeDiv::createNode);
	registerFunc("eq", &FunctionNodeEq::createNode);
	registerFunc("even", &FunctionNodeEven::createNode);
	registerFunc("exact", &FunctionNodeExact::createNode);
	registerFunc("exp", &FunctionNodeExp::createNode);
	registerFunc("fact", &FunctionNodeFact::createNode);
	registerFunc("first", &FunctionNodeAggregate::createNode);
	registerFunc("floor", &FunctionNodeFloor::createNode);
	registerFunc("ge", &FunctionNodeGe::createNode);
	registerFunc("gt", &FunctionNodeGt::createNode);
	registerFunc("if", &FunctionNodeIf::createNode);
	registerFunc("int", &FunctionNodeInt::createNode);
	registerFunc("iserror", &FunctionNodeIsError::createNode);
	registerFunc("last", &FunctionNodeAggregate::createNode);
	registerFunc("le", &FunctionNodeLe::createNode);
	registerFunc("left", &FunctionNodeLeft::createNode);
	registerFunc("len", &FunctionNodeLen::createNode);
	registerFunc("ln", &FunctionNodeLn::createNode);
	registerFunc("log", &FunctionNodeLog::createNode);
	registerFunc("log10", &FunctionNodeLog10::createNode);
	registerFunc("lower", &FunctionNodeLower::createNode);
	registerFunc("lt", &FunctionNodeLt::createNode);
	registerFunc("max", &FunctionNodeAggregate::createNode);
	registerFunc("mid", &FunctionNodeMid::createNode);
	registerFunc("min", &FunctionNodeAggregate::createNode);
	registerFunc("mod", &FunctionNodeMod::createNode);
	registerFunc("mul", &FunctionNodeMul::createNode);
	registerFunc("ne", &FunctionNodeNe::createNode);
	registerFunc("not", &FunctionNodeNot::createNode);
	registerFunc("now", &FunctionNodeNow::createNode);
	registerFunc("odd", &FunctionNodeOdd::createNode);
	registerFunc("or", &FunctionNodeAggregate::createNode);
	registerFunc("pi", &FunctionNodePi::createNode);
	registerFunc("power", &FunctionNodePower::createNode);
	registerFunc("product", &FunctionNodeAggregate::createNode);
	registerFunc("proper", &FunctionNodeProper::createNode);
	registerFunc("quotient", &FunctionNodeQuotient::createNode);
	registerFunc("rand", &FunctionNodeRand::createNode);
	registerFunc("randbetween", &FunctionNodeRandbetween::createNode);
	registerFunc("replace", &FunctionNodeReplace::createNode);
	registerFunc("rept", &FunctionNodeRept::createNode);
	registerFunc("right", &FunctionNodeRight::createNode);
	registerFunc("round", &FunctionNodeRound::createNode);
	registerFunc("search", &FunctionNodeSearch::createNode);
	registerFunc("sign", &FunctionNodeSign::createNode);
	registerFunc("sin", &FunctionNodeSin::createNode);
	registerFunc("sqrt", &FunctionNodeSqrt::createNode);
	registerFunc("stet", &FunctionNodeStet::createNode);
	registerFunc("str", &FunctionNodeStr::createNode);
	registerFunc("substitute", &FunctionNodeSubstitute::createNode);
	registerFunc("sum", &FunctionNodeAggregate::createNode);
	registerFunc("tan", &FunctionNodeTan::createNode);
	registerFunc("trim", &FunctionNodeTrim::createNode);
	registerFunc("trunc", &FunctionNodeTrunc::createNode);
	registerFunc("upper", &FunctionNodeUpper::createNode);
	registerFunc("value", &FunctionNodeValue::createNode);
	registerFunc("valuedate", &FunctionNodeValuedate::createNode);
	registerFunc("weekday", &FunctionNodeWeekday::createNode);

	registerFunc("palo.cubedimension", &FunctionNodePaloCubedimension::createNode);
	registerFunc("palo.data", &FunctionNodePaloData::createNode);
	registerFunc("palo.echild", &FunctionNodePaloEchild::createNode);
	registerFunc("palo.echildcount", &FunctionNodePaloEchildcount::createNode);
	registerFunc("palo.ecount", &FunctionNodePaloEcount::createNode);
	registerFunc("palo.efirst", &FunctionNodePaloEfirst::createNode);
	registerFunc("palo.eindent", &FunctionNodePaloEindent::createNode);
	registerFunc("palo.eindex", &FunctionNodePaloEindex::createNode);
	registerFunc("palo.eischild", &FunctionNodePaloEischild::createNode);
	registerFunc("palo.elevel", &FunctionNodePaloElevel::createNode);
	registerFunc("palo.ename", &FunctionNodePaloEname::createNode);
	registerFunc("palo.enext", &FunctionNodePaloEnext::createNode);
	registerFunc("palo.eparent", &FunctionNodePaloEparent::createNode);
	registerFunc("palo.eparentcount", &FunctionNodePaloEparentcount::createNode);
	registerFunc("palo.eprev", &FunctionNodePaloEprev::createNode);
	registerFunc("palo.esibling", &FunctionNodePaloEsibling::createNode);
	registerFunc("palo.etoplevel", &FunctionNodePaloEtoplevel::createNode);
	registerFunc("palo.etype", &FunctionNodePaloEtype::createNode);
	registerFunc("palo.eweight", &FunctionNodePaloEweight::createNode);
	registerFunc("palo.marker", &FunctionNodePaloMarker::createNode);
	registerFunc("palo.eoffset", &FunctionNodePaloEoffset::createNode);

	isRegistered = true;
}
}
