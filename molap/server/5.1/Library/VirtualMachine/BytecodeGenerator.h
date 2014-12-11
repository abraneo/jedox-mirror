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
 * \author Radu Ialovoi started this file with code migrated from  Engine.h/cpp
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef BYTECODE_GENERATOR_H
#define BYTECODE_GENERATOR_H

#include "palo.h"
#include "Engine/Legacy/Engine.h"

/* #define DEBUG_ENGINE */
#ifdef DEBUG_ENGINE
#define DEBUG_PRINTF(x) printf x
#else
#define DEBUG_PRINTF(x)
#endif

namespace palo {
class bytecode_generator {
public:
	paloLegacy::ERule& m_rule;
	enum WM_OPCODES {
		NOP = 0, HALT = 1,

		PUSH_DBL = 10, PULL_DBL = 11, SWAP_DBL = 12, PUSH_STR = 13, PULL_STR = 14, SWAP_STR = 15,

		DBL_2_STR = 20, STR_2_DBL = 21,

		LD_CONST_DBL = 30, LD_CONST_STR = 31,

		LD_SRC_HIT_DBL = 40,
		/* LD_SRC_HIT_STR */
		/* this does not exist, because you cannot marker on strings */
		LD_SRC_DBL = 41, LD_SRC_STR = 42,

		LD_VAR_STR = 50,

		OP2_SUM_DBL = 60, OP2_DIFF_DBL = 61, OP2_PROD_DBL = 62, OP2_QUO_DBL = 63,

		OP2_EQ_DBL = 70, OP2_NE_DBL = 71, OP2_LT_DBL = 72, OP2_LE_DBL = 73,

		OP2_EQ_STR = 80, OP2_NE_STR = 81, OP2_LT_STR = 82, OP2_LE_STR = 83,

		CALL_GEN_DBL = 90, CALL_GEN_STR = 91,

		CALL_DATA_DBL = 100, CALL_DATA_STR = 101,

		JUMP = 110, JUMP_IF_NOT = 111,

		CONTINUE = 120, STET = 121,

		OP2_LOG_DBL = 130, OP2_MOD_DBL = 131, OP2_POWER_DBL = 132, OP2_QUOTIENT_DBL = 133, OP2_RANDBETWEEN_DBL = 134, OP2_ROUND_DBL = 135,

		OP1_NOT_DBL = 139,

		OP1_ABS_DBL = 140, OP1_ACOS_DBL = 141, OP1_ASIN_DBL = 142, OP1_ATAN_DBL = 143, OP1_CEILING_DBL = 144, OP1_COS_DBL = 145, OP1_EVEN_DBL = 146, OP1_EXP_DBL = 147, OP1_FACT_DBL = 148, OP1_FLOOR_DBL = 149, OP1_INT_DBL = 150, OP1_LN_DBL = 151, OP1_LOG10_DBL = 152, OP1_ODD_DBL = 153, OP1_SIGN_DBL = 154, OP1_SIN_DBL = 155, OP1_SQRT_DBL = 156, OP1_TAN_DBL = 157, OP1_TRUNC_DBL = 158, OP1_WEEKDAY_DBL = 159,

		FUNC_CHAR = 160, FUNC_CLEAN = 161, FUNC_CODE = 162, FUNC_CONCATENATE = 163, FUNC_DATE = 164, FUNC_DATEFORMAT = 165, FUNC_DATEVALUE = 166, FUNC_EXACT = 167, FUNC_LEFT = 168, FUNC_LEN = 169, FUNC_LOWER = 170, FUNC_MID = 171, FUNC_NOW = 172, FUNC_PI = 173, FUNC_PROPER = 174, FUNC_RAND = 175, FUNC_REPLACE = 176, FUNC_REPT = 177, FUNC_RIGHT = 178, FUNC_SEARCH = 179, FUNC_STR = 180, FUNC_SUBSTITUTE = 181, FUNC_TRIM = 182, FUNC_UPPER = 183, FUNC_VALUE = 184,

		AGGR_SUM = 190, AGGR_PROD = 191, AGGR_MIN = 192, AGGR_MAX = 193, AGGR_COUNT = 194, AGGR_FIRST = 195, AGGR_LAST = 196, AGGR_AVG = 197, AGGR_AND = 198, AGGR_OR = 199,

		PALO_CUBEDIMENSION = 200, PALO_ECHILD = 201, PALO_ECHILDCOUNT = 202, PALO_ECOUNT = 203, PALO_EFIRST = 204, PALO_EINDENT = 205, PALO_EINDEX = 206, PALO_EISCHILD = 207, PALO_ELEVEL = 208, PALO_ENAME = 209, PALO_ENEXT = 210, PALO_EPARENT = 211, PALO_EPARENTCOUNT = 212, PALO_EPREV = 213, PALO_ESIBLING = 214, PALO_ETOPLEVEL = 215, PALO_ETYPE = 216, PALO_EWEIGHT = 217, PALO_EOFFSET = 218,

		FUNC_ISERROR = 220,

		RET_LD_SRC_DBL = 223, RET_CALL_DATA_DBL = 224, RET_LD_SRC_STR = 225, RET_CALL_DATA_STR = 226,

		INVALID = 235

	};
private:
	size_t ifLevel;
public:
	bytecode_generator(paloLegacy::ERule& a_rule) :
		m_rule(a_rule), ifLevel(0) {
	}
	;//shallow object. no copy constructor required
	paloLegacy::ERule* get_rule() {
		return &m_rule;
	}
	;
	bool EmitCode(paloLegacy::Bytecode bc);
	bool EmitId(IdentifierType id);
	bool EmitForceTypeCode(uint8_t have, uint8_t want);
	bool EmitNopCode();
	bool EmitHaltCode();
	bool EmitLdConstDblCode(double dbl);
	bool EmitLdConstStrCode(string str);
	bool EmitSourceCode(const IdentifiersType &elementIDs, const vector<uint8_t> &restriction, bool marker, uint8_t want);
	bool EmitVariableStrCode(uint8_t dim);
	bool EmitOp2DblCode(string name);
	bool EmitOp1DblCode(string name);
	bool EmitFuncCode(string name);
	bool EmitOp2ComparisonDblCode(string name);
	bool EmitOp2ComparisonStrCode(string name);
	bool EmitCallDataDblCode(uint8_t num, uint8_t dims, int16_t contextCubeIndex, const int16_t* dimensionOrder, uint32_t *dimensionElements);
	bool EmitCallDataStrCode(uint8_t num, uint8_t dims, int16_t contextCubeIndex, const int16_t* dimensionOrder, uint32_t *dimensionElements);
	bool EmitIfCondCode(uint32_t *end_of_cond, uint32_t *end_of_if);
	bool EmitIfIfCode(uint32_t *end_of_cond, uint32_t *end_of_if);
	bool EmitIfElseCode(uint32_t *end_of_cond, uint32_t *end_of_if);
	bool EmitContinue();
	bool EmitStet();
	bool EmitAggrCode(string name, uint8_t num);
	bool EmitIsError();
	size_t getIfLevel() const {return ifLevel;}
	void increaseIfLevel() {ifLevel++;}
	void decreaseIfLevel() {ifLevel--;}
};
}

#endif
