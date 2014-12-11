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
 * \author Martin Schoenert, triagens GmbH, Cologne, Germany
 * \author Radu Ialovoi started this file with code migrated from  Engine.cpp
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "BytecodeGenerator.h"

#include "Parser/Node.h"
#include "Parser/SourceNode.h"

namespace palo {

/****************************************************************************
 **
 *F  NewEntryRule(rule,cube) . . . . . . . . .  allocate a new rule upon entry
 **
 */
bool bytecode_generator::EmitCode(paloLegacy::Bytecode bc)
{
	m_rule.bytecode[m_rule.gc_bc_nr] = bc;
	m_rule.gc_bc_nr++;
	if (m_rule.gc_bc_max <= m_rule.gc_bc_nr) {
		m_rule.gc_bc_max = m_rule.gc_bc_max * 5 / 4 + 32;
		void * new_mem = realloc(m_rule.bytecode, m_rule.gc_bc_max * sizeof(paloLegacy::Bytecode));
		if (NULL == new_mem) {
			return false;
		}

		m_rule.bytecode = (paloLegacy::Bytecode*)new_mem;
	}
	return true;
}

bool bytecode_generator::EmitId(IdentifierType id)
{
	bool result = true;
	result &= EmitCode((paloLegacy::Bytecode)id);
	result &= EmitCode((paloLegacy::Bytecode)(id >> 16));
	return result;
}

bool bytecode_generator::EmitForceTypeCode(uint8_t have, uint8_t want)
{
	bool ret_code = true;

	if (want == Node::NODE_NUMERIC && have == Node::NODE_STRING) {
		DEBUG_PRINTF(("STR_2_DBL\n"));
		ret_code = EmitCode(STR_2_DBL);
	} else if (want == Node::NODE_STRING && have == Node::NODE_NUMERIC) {
		DEBUG_PRINTF(("DBL_2_STR\n"));
		ret_code = EmitCode(DBL_2_STR);
	}
	return ret_code;
}

bool bytecode_generator::EmitNopCode()

{
	DEBUG_PRINTF(("NOP\n"));
	return EmitCode(NOP);
}

bool bytecode_generator::EmitHaltCode()
{
	DEBUG_PRINTF(("HALT\n"));
	return EmitCode(HALT);
}

bool bytecode_generator::EmitLdConstDblCode(double dbl)
{
	double * dbl_consts_tmp;
	uint32_t j;

	DEBUG_PRINTF(("LD_CONST_DBL %f\n", dbl));
	while (m_rule.gc_dbl_const_nr >= m_rule.gc_dbl_const_max) {
		m_rule.gc_dbl_const_max *= 2;
		dbl_consts_tmp = new double[m_rule.gc_dbl_const_max];

		if (NULL == dbl_consts_tmp) {
			return false;
		}
		memset(dbl_consts_tmp, 0, sizeof(double) * m_rule.gc_dbl_const_max);

		for (j = 0; j < m_rule.gc_dbl_const_max / 2; j++) {
			dbl_consts_tmp[j] = m_rule.dbl_consts[j];
		}
		delete [] m_rule.dbl_consts;
		m_rule.dbl_consts = dbl_consts_tmp;
	}
	m_rule.dbl_consts[m_rule.gc_dbl_const_nr] = dbl;
	if (!EmitCode(LD_CONST_DBL))
		return false;
	if (!EmitCode(m_rule.gc_dbl_const_nr))
		return false;
	m_rule.gc_dbl_const_nr++;
	return true;
}

bool bytecode_generator::EmitLdConstStrCode(string str)
{
	string * str_consts_tmp;
	uint32_t j;

	DEBUG_PRINTF(("LD_CONST_STR %s\n", str.c_str()));
	while (m_rule.gc_str_const_nr >= m_rule.gc_str_const_max) {
		m_rule.gc_str_const_max *= 2;
		str_consts_tmp = new string[m_rule.gc_str_const_max]; //TODO: this might get out of mem
		for (j = 0; j < m_rule.gc_str_const_max / 2; j++) {
			str_consts_tmp[j] = m_rule.str_consts[j];
		}
		delete[] m_rule.str_consts;
		m_rule.str_consts = str_consts_tmp;
	}
	m_rule.str_consts[m_rule.gc_str_const_nr] = str;
	if (!EmitCode(LD_CONST_STR))
		return false;
	if (!EmitCode(m_rule.gc_str_const_nr))
		return false;
	m_rule.gc_str_const_nr++;
	return true;
}

bool bytecode_generator::EmitSourceCode(const IdentifiersType &elementIDs, const vector<uint8_t> &restriction, bool marker, uint8_t want)
{
	uint8_t hit = m_rule.ubm_flag;
	IdentifiersType::const_iterator source = elementIDs.begin();
	vector<uint8_t>::const_iterator mask = restriction.begin();
	int sourceId = -1;
	bool relativeCell = false;
	bool subDimensionalCell = false;

	for (uint32_t j = 0; j < m_rule.cube->nrDimensions; j++) {
		if (restriction[j] == AreaNode::OFFSET_RESTRICTION) {
			relativeCell = true;
			break;
		}
	}
	for (uint32_t j = 0; j < m_rule.cube->nrDimensions && m_rule.dest_area; j++) {
		if (restriction[j] == AreaNode::ABS_RESTRICTION && m_rule.dest_area->borders[j]->size > 1) {
			// case not supported - error 0011779
			subDimensionalCell = true;
			break;
		}
	}
	for (size_t otherSourceId = 0; otherSourceId < m_rule.gc_copy_nr; otherSourceId++) {
		uint8_t j;
		for (j = 0; j < m_rule.cube->nrDimensions; j++) {
			if (restriction[j] != m_rule.copy_mask[otherSourceId][j] || m_rule.copy_source[otherSourceId][j] != elementIDs[j]) {
				// different source
				break;
			}
		}
		if (j == m_rule.cube->nrDimensions) {
			// identical source
			sourceId = (int)otherSourceId;
			if (!subDimensionalCell && !relativeCell && (getIfLevel() == 0 || m_rule.ubm_flag)) {
				m_rule.source_precalc[sourceId] = 1;
			}
			break;
		}
	}
	if (sourceId == -1) {
		sourceId = m_rule.gc_copy_nr++;
		m_rule.source_precalc[sourceId] = !subDimensionalCell && !relativeCell && (getIfLevel() == 0 || m_rule.ubm_flag) ? 1 : 0;
	/* XXX may need to realloc m_rule.copy_mask, ... */

	//	disable precalculation: m_rule.source_precalc[sourceId] = 0;
		m_rule.copy_mask[sourceId] = NULL;
		m_rule.copy_source[sourceId] = NULL;

		m_rule.copy_mask[sourceId] = new paloLegacy::EPath;
		if (NULL == m_rule.copy_mask[sourceId]) {
			goto EmitSourceCode_CLEANUP;
		}
		memset(m_rule.copy_mask[sourceId], 0, sizeof(paloLegacy::EPath));

		m_rule.copy_source[sourceId] = new paloLegacy::EPath;
		if (NULL == m_rule.copy_source[sourceId]) {
			goto EmitSourceCode_CLEANUP;
		}
		memset(m_rule.copy_source[sourceId], 0, sizeof(paloLegacy::EPath));

		for (uint8_t j = 0; j < m_rule.cube->nrDimensions; j++) {
			m_rule.copy_mask[sourceId][j] = *mask;
			m_rule.copy_source[sourceId][j] = *source;
			if ((m_rule.copy_mask[sourceId][j] != SourceNode::NO_RESTRICTION) && (m_rule.ubm_mask[j] == 0)) {
				hit = 0;
			}
			if (m_rule.copy_mask[sourceId][j] != SourceNode::NO_RESTRICTION && m_rule.copy_source[sourceId][j] != m_rule.ubm_source[j]) {
				hit = 0;
			}
			++source;
			++mask;
		}
		if (m_rule.ubm_flag && marker && !hit) {
			printf("the marker did not generate a hit\n");
		}
	}
	if (want == Node::NODE_NUMERIC) {
		if (hit) {
			DEBUG_PRINTF(("LD_SRC_HIT_DBL\n"));
			if (!EmitCode(LD_SRC_HIT_DBL)) {
				goto EmitSourceCode_CLEANUP;
			}
			if (!EmitCode(sourceId)) {
				goto EmitSourceCode_CLEANUP;
			}
		} else {
			DEBUG_PRINTF(("LD_SRC_DBL %d\n", i));
			if (!EmitCode(LD_SRC_DBL)) {
				goto EmitSourceCode_CLEANUP;
			}
			if (!EmitCode(sourceId)) {
				goto EmitSourceCode_CLEANUP;
			}
		}
	} else {
		DEBUG_PRINTF(("LD_SRC_STR %d\n", i));
		if (!EmitCode(LD_SRC_STR)) {
			goto EmitSourceCode_CLEANUP;
		}
		if (!EmitCode(sourceId)) {
			goto EmitSourceCode_CLEANUP;
		}
	}
	return true;

EmitSourceCode_CLEANUP: if (NULL != m_rule.copy_mask[sourceId]) {
		delete [] m_rule.copy_mask[sourceId];
		m_rule.copy_mask[sourceId] = NULL;

	}
	if (NULL != m_rule.copy_source[sourceId]) {
		delete [] m_rule.copy_mask[sourceId];
		m_rule.copy_mask[sourceId] = NULL;

	}
	return false;
}

bool bytecode_generator::EmitVariableStrCode(uint8_t dim)
{
	DEBUG_PRINTF(("LD_VAR_STR %d\n", dim));
	if (!EmitCode(LD_VAR_STR))
		return false;
	if (!EmitCode(dim))
		return false;
	return true;
}

bool bytecode_generator::EmitOp2DblCode(string name)
{
	DEBUG_PRINTF(("OP2_DBL %s\n", name.c_str()));

	if (name == "+") {
		if (!EmitCode(OP2_SUM_DBL))
			return false;
	} else if (name == "add") {
		if (!EmitCode(OP2_SUM_DBL))
			return false;
	} else if (name == "-") {
		if (!EmitCode(OP2_DIFF_DBL))
			return false;
	} else if (name == "del") {
		if (!EmitCode(OP2_DIFF_DBL))
			return false;
	} else if (name == "*") {
		if (!EmitCode(OP2_PROD_DBL))
			return false;
	} else if (name == "mul") {
		if (!EmitCode(OP2_PROD_DBL))
			return false;
	} else if (name == "/") {
		if (!EmitCode(OP2_QUO_DBL))
			return false;
	} else if (name == "div") {
		if (!EmitCode(OP2_QUO_DBL))
			return false;
	}

	else if (name == "log") {
		if (!EmitCode(OP2_LOG_DBL))
			return false;
	} else if (name == "mod") {
		if (!EmitCode(OP2_MOD_DBL))
			return false;
	} else if (name == "power") {
		if (!EmitCode(OP2_POWER_DBL))
			return false;
	} else if (name == "quotient") {
		if (!EmitCode(OP2_QUOTIENT_DBL))
			return false;
	} else if (name == "randbetween") {
		if (!EmitCode(OP2_RANDBETWEEN_DBL))
			return false;
	} else if (name == "round") {
		if (!EmitCode(OP2_ROUND_DBL))
			return false;
	} else {
		Logger::error << "unknown operator D <- D,D " << name << endl;
		/* XXX actually we would need a double drop instruction */
		if (!EmitLdConstDblCode(0.0))
			return false;
	}
	return true;
}

bool bytecode_generator::EmitOp1DblCode(string name)
{
	DEBUG_PRINTF(("OP1_DBL %s\n", name.c_str()));

	if (name == "abs") {
		if (!EmitCode(OP1_ABS_DBL))
			return false;
	} else if (name == "acos") {
		if (!EmitCode(OP1_ACOS_DBL))
			return false;
	} else if (name == "asin") {
		if (!EmitCode(OP1_ASIN_DBL))
			return false;
	} else if (name == "atan") {
		if (!EmitCode(OP1_ATAN_DBL))
			return false;
	} else if (name == "ceiling") {
		if (!EmitCode(OP1_CEILING_DBL))
			return false;
	} else if (name == "cos") {
		if (!EmitCode(OP1_COS_DBL))
			return false;
	} else if (name == "even") {
		if (!EmitCode(OP1_EVEN_DBL))
			return false;
	} else if (name == "exp") {
		if (!EmitCode(OP1_EXP_DBL))
			return false;
	} else if (name == "fact") {
		if (!EmitCode(OP1_FACT_DBL))
			return false;
	} else if (name == "floor") {
		if (!EmitCode(OP1_FLOOR_DBL))
			return false;
	} else if (name == "int") {
		if (!EmitCode(OP1_INT_DBL))
			return false;
	} else if (name == "ln") {
		if (!EmitCode(OP1_LN_DBL))
			return false;
	} else if (name == "log10") {
		if (!EmitCode(OP1_LOG10_DBL))
			return false;
	} else if (name == "odd") {
		if (!EmitCode(OP1_ODD_DBL))
			return false;
	} else if (name == "sign") {
		if (!EmitCode(OP1_SIGN_DBL))
			return false;
	} else if (name == "sin") {
		if (!EmitCode(OP1_SIN_DBL))
			return false;
	} else if (name == "sqrt") {
		if (!EmitCode(OP1_SQRT_DBL))
			return false;
	} else if (name == "tan") {
		if (!EmitCode(OP1_TAN_DBL))
			return false;
	} else if (name == "trunc") {
		if (!EmitCode(OP1_TRUNC_DBL))
			return false;
	} else if (name == "weekday") {
		if (!EmitCode(OP1_WEEKDAY_DBL))
			return false;
	} else if (name == "not") {
		if (!EmitCode(OP1_NOT_DBL))
			return false;
	} else {
		Logger::error << "unknown function D <- D" << name << endl;
		if (!EmitLdConstDblCode(0.0))
			return false;
	}

	return true;
}

bool bytecode_generator::EmitFuncCode(string name)
{
	DEBUG_PRINTF(("EmitFuncCode(%s)\n", name.c_str()));
	if (name == "char") {
		if (!EmitCode(FUNC_CHAR))
			return false;
		/* S <- D */
	} else if (name == "clean") {
		EmitCode(FUNC_CLEAN);
		/* S <- S */
	} else if (name == "code") {
		if (!EmitCode(FUNC_CODE))
			return false;
		/* D <- S */
	} else if (name == "concatenate") {
		if (!EmitCode(FUNC_CONCATENATE))
			return false;
		/* S <- S,S */
	} else if (name == "date") {
		if (!EmitCode(FUNC_DATE))
			return false;
		/* D <- D,D,D */
	} else if (name == "dateformat") {
		if (!EmitCode(FUNC_DATEFORMAT))
			return false;
		/* S <- D,S */
	} else if (name == "datevalue") {
		if (!EmitCode(FUNC_DATEVALUE))
			return false;
		/* D <- S */
	} else if (name == "exact") {
		if (!EmitCode(FUNC_EXACT))
			return false;
		/* D <- S,S */
	} else if (name == "left") {
		if (!EmitCode(FUNC_LEFT))
			return false;
		/* S <- S,D */
	} else if (name == "len") {
		if (!EmitCode(FUNC_LEN))
			return false;
		/* D <- S */
	} else if (name == "lower") {
		if (!EmitCode(FUNC_LOWER))
			return false;
		/* S <- S */
	} else if (name == "mid") {
		if (!EmitCode(FUNC_MID))
			return false;
		/* S <- S,D,D */
	} else if (name == "now") {
		if (!EmitCode(FUNC_NOW))
			return false;
		/* D <- */
	} else if (name == "pi") {
		if (!EmitCode(FUNC_PI))
			return false;
		/* D <- */
	} else if (name == "proper") {
		if (!EmitCode(FUNC_PROPER))
			return false;
		/* S <- S */
	} else if (name == "rand") {
		if (!EmitCode(FUNC_RAND))
			return false;
		/* D <- */
	} else if (name == "replace") {
		if (!EmitCode(FUNC_REPLACE))
			return false;
		/* S <- S,D,D,S */
	} else if (name == "rept") {
		if (!EmitCode(FUNC_REPT))
			return false;
		/* S <- S,D */
	} else if (name == "right") {
		if (!EmitCode(FUNC_RIGHT))
			return false;
		/* S <- S,D */
	} else if (name == "search") {
		if (!EmitCode(FUNC_SEARCH))
			return false;
		/* D <- S,S */
	} else if (name == "str") {
		if (!EmitCode(FUNC_STR))
			return false;
		/* S <- D,D,D */
	} else if (name == "substitute") {
		if (!EmitCode(FUNC_SUBSTITUTE))
			return false;
		/* S <- S,S,S */
	} else if (name == "trim") {
		if (!EmitCode(FUNC_TRIM))
			return false;
		/* S <- S */
	} else if (name == "upper") {
		if (!EmitCode(FUNC_UPPER))
			return false;
		/* S <- S */
	} else if (name == "value") {
		if (!EmitCode(FUNC_VALUE))
			return false;
		/* D <- S */
	} else if (name == "palo.cubedimension") {
		if (!EmitCode(PALO_CUBEDIMENSION))
			return false;
		/* S <- S,S,D */
	} else if (name == "palo.echild") {
		if (!EmitCode(PALO_ECHILD))
			return false;
		/* S <- S,S,S,D */
	} else if (name == "palo.echildcount") {
		if (!EmitCode(PALO_ECHILDCOUNT))
			return false;
		/* D <- S,S,S */
	} else if (name == "palo.ecount") {
		if (!EmitCode(PALO_ECOUNT))
			return false;
		/* D <- S,S */
	} else if (name == "palo.efirst") {
		if (!EmitCode(PALO_EFIRST))
			return false;
		/* S <- S,S */
	} else if (name == "palo.eindent") {
		if (!EmitCode(PALO_EINDENT))
			return false;
		/* D <- S,S,S */
	} else if (name == "palo.eindex") {
		if (!EmitCode(PALO_EINDEX))
			return false;
		/* D <- S,S,S */
	} else if (name == "palo.eischild") {
		if (!EmitCode(PALO_EISCHILD))
			return false;
		/* D <- S,S,S,S */
	} else if (name == "palo.elevel") {
		if (!EmitCode(PALO_ELEVEL))
			return false;
		/* D <- S,S,S */
	} else if (name == "palo.ename") {
		if (!EmitCode(PALO_ENAME))
			return false;
		/* S <- S,S,D */
	} else if (name == "palo.enext") {
		if (!EmitCode(PALO_ENEXT))
			return false;
		/* S <- S,S,S */
	} else if (name == "palo.eparent") {
		if (!EmitCode(PALO_EPARENT))
			return false;
		/* S <- S,S,S,D */
	} else if (name == "palo.eparentcount") {
		if (!EmitCode(PALO_EPARENTCOUNT))
			return false;
		/* D <- S,S,S */
	} else if (name == "palo.eprev") {
		if (!EmitCode(PALO_EPREV))
			return false;
		/* S <- S,S,S */
	} else if (name == "palo.esibling") {
		if (!EmitCode(PALO_ESIBLING))
			return false;
		/* S <- S,S,S,D */
	} else if (name == "palo.eoffset") {
		if (!EmitCode(PALO_EOFFSET))
			return false;
		/* S <- S,S,S,D */
	} else if (name == "palo.etoplevel") {
		if (!EmitCode(PALO_ETOPLEVEL))
			return false;
		/* D <- S,S */
	} else if (name == "palo.etype") {
		if (!EmitCode(PALO_ETYPE))
			return false;
		/* S <- S,S,S */
	} else if (name == "palo.eweight") {
		if (!EmitCode(PALO_EWEIGHT))
			return false;
		/* D <- S,S,S,S */
	}

	else {
		Logger::error << "unknown function " << name << endl;
	}

	return true;
}

bool bytecode_generator::EmitOp2ComparisonDblCode(string name)
{
	DEBUG_PRINTF(("OP2_COMPARISON_DBL %s\n", name.c_str()));
	if (name == "==" || name == "eq") {
		if (!EmitCode(OP2_EQ_DBL))
			return false;
	} else if (name == "!=" || name == "ne") {
		if (!EmitCode(OP2_NE_DBL))
			return false;
	} else if (name == "<=" || name == "le") {
		if (!EmitCode(OP2_LE_DBL))
			return false;
	} else if (name == "<" || name == "lt") {
		if (!EmitCode(OP2_LT_DBL))
			return false;
	} else if (name == ">=" || name == "ge") {
		if (!EmitCode(SWAP_DBL))
			return false;
		if (!EmitCode(OP2_LE_DBL))
			return false;
	} else if (name == ">" || name == "gt") {
		if (!EmitCode(SWAP_DBL))
			return false;
		if (!EmitCode(OP2_LT_DBL))
			return false;
	}
	return true;
}

bool bytecode_generator::EmitOp2ComparisonStrCode(string name)
{
	DEBUG_PRINTF(("OP2_COMPARISON_STR %s\n", name.c_str()));
	if (name == "==" || name == "eq") {
		if (!EmitCode(OP2_EQ_STR))
			return false;
	} else if (name == "!=" || name == "ne") {
		if (!EmitCode(OP2_NE_STR))
			return false;
	} else if (name == "<=" || name == "le") {
		if (!EmitCode(OP2_LE_STR))
			return false;
	} else if (name == "<" || name == "lt") {
		if (!EmitCode(OP2_LT_STR))
			return false;
	} else if (name == ">=" || name == "ge") {
		if (!EmitCode(SWAP_STR))
			return false;
		if (!EmitCode(OP2_LE_STR))
			return false;
	} else if (name == ">" || name == "gt") {
		if (!EmitCode(SWAP_STR))
			return false;
		if (!EmitCode(OP2_LT_STR))
			return false;
	}
	return true;
}

bool bytecode_generator::EmitCallDataDblCode(uint8_t params, uint8_t dims, int16_t contextCubeIndex, const int16_t* dimensionOrder, uint32_t *dimensionElements)
{
	uint8_t i;
	DEBUG_PRINTF(("CALL_DATA_DBL %d\n", num));
	if (!EmitCode(CALL_DATA_DBL)) {
		return false;
	}
	if (!EmitCode(params)) {
		return false;
	}
	if (!EmitCode(contextCubeIndex)) {
		return false;
	}
	if (contextCubeIndex != -1) {
		if (!EmitCode(dims)) {
			return false;
		}
	} else {
	}
	for (i = 0; i < dims; i++) {
		if (contextCubeIndex != -1) {
			if (!EmitCode(dimensionOrder[i])) {
				return false;
			}
		}
		if ((dimensionOrder && dimensionOrder[i] != -1) || (dimensionElements && dimensionElements[i] != NO_IDENTIFIER)) {
			continue;
		}
	}
	if (dimensionElements) {
		for (i = 0; i < dims; i++) {
			if (!EmitId(dimensionElements[i])) {
				return false;
			}
		}
	}
	return true;
}

bool bytecode_generator::EmitCallDataStrCode(uint8_t params, uint8_t dims, int16_t contextCubeIndex, const int16_t* dimensionOrder, uint32_t *dimensionElements)
{
	uint8_t i;
	DEBUG_PRINTF(("CALL_DATA_STR %d\n", num));
	if (!EmitCode(CALL_DATA_STR)) {
		return false;
	}
	if (!EmitCode(params)) {
		return false;
	}
	if (!EmitCode(contextCubeIndex)) {
		return false;
	}
	if (contextCubeIndex != -1) {
		if (!EmitCode(dims)) {
			return false;
		}
	}
	for (i = 0; i < dims; i++) {
		if (contextCubeIndex != -1) {
			if (!EmitCode(dimensionOrder[i])) {
				return false;
			}
		}
		if ((dimensionOrder && dimensionOrder[i] != -1) || (dimensionElements && dimensionElements[i] != NO_IDENTIFIER)) {
			continue;
		}
	}
	if (dimensionElements) {
		for (i = 0; i < dims; i++) {
			if (!EmitId(dimensionElements[i])) {
				return false;
			}
		}
	}
	return true;
}

bool bytecode_generator::EmitIfCondCode(uint32_t *end_of_cond, uint32_t *end_of_if)
{

	DEBUG_PRINTF(("JUMP_IF_NOT\n"));
	if (!EmitCode(JUMP_IF_NOT))
		return false;
	if (!EmitCode(0))
		return false;

	*end_of_cond = m_rule.gc_bc_nr - 1;

	*end_of_if = m_rule.gc_bc_nr - 1;

	return true;
}

bool bytecode_generator::EmitIfIfCode(uint32_t *end_of_cond, uint32_t *end_of_if)
{

	DEBUG_PRINTF(("JUMP\n"));
	if (!EmitCode(JUMP))
		return false;
	if (!EmitCode(0))
		return false;

	*end_of_if = m_rule.gc_bc_nr - 1;

	DEBUG_PRINTF(("backpatch %d\n", (m_rule.gc_bc_nr - *end_of_cond) - 1));
	m_rule.bytecode[*end_of_cond] = (m_rule.gc_bc_nr - *end_of_cond) - 1;

	return true;
}

bool bytecode_generator::EmitIfElseCode(uint32_t *end_of_cond, uint32_t *end_of_if)
{

	DEBUG_PRINTF(("backpatch %d\n", (m_rule.gc_bc_nr - *end_of_if) - 1));
	m_rule.bytecode[*end_of_if] = (m_rule.gc_bc_nr - *end_of_if) - 1;

	return true;
}

bool bytecode_generator::EmitContinue()
{
	DEBUG_PRINTF(("CONTINUE\n"));
	return EmitCode(CONTINUE);
}

bool bytecode_generator::EmitStet()
{

	DEBUG_PRINTF(("STET\n"));
	if (!m_rule.is_cons) {
		m_rule.precalcStet = 1;
	}
	return EmitCode(STET);
}

bool bytecode_generator::EmitAggrCode(string name, uint8_t num)
{
	DEBUG_PRINTF(("AGGR_ %s\n", name.c_str()));

	if (name == "sum") {
		if (!EmitCode(AGGR_SUM))
			return false;
	} else if (name == "product") {
		if (!EmitCode(AGGR_PROD))
			return false;
	} else if (name == "min") {
		if (!EmitCode(AGGR_MIN))
			return false;
	} else if (name == "max") {
		if (!EmitCode(AGGR_MAX))
			return false;
	} else if (name == "count") {
		if (!EmitCode(AGGR_COUNT))
			return false;
	} else if (name == "first") {
		if (!EmitCode(AGGR_FIRST))
			return false;
	} else if (name == "last") {
		if (!EmitCode(AGGR_LAST))
			return false;
	} else if (name == "average") {
		if (!EmitCode(AGGR_AVG))
			return false;
	} else if (name == "and") {
		if (!EmitCode(AGGR_AND))
			return false;
	} else if (name == "or") {
		if (!EmitCode(AGGR_OR))
			return false;
	}
	return EmitCode(num);
}

bool bytecode_generator::EmitIsError()
{
	if (!EmitCode(FUNC_ISERROR))
		return false;
	return true;
}
}
