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

#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include "palo.h"
#include "VirtualMachine/BytecodeGenerator.h"
#include "Thread/WriteLocker.h"
#include "Olap/Rule.h"

using namespace palo;

namespace paloLegacy {

typedef vector< pair<string, IdentifierType> > CubeElemNameCache;
typedef map<CPCube, CubeElemNameCache> ElemNameCache;
class VMCache : public ElemNameCache
{
};

class virtual_machine {
private:

	class preallocated_stacks {
	public:
		typedef CellValue* preallocated_stack_type;
	private:
		static const size_t INITIAL_PREALLOCATED_SIZE = 10;
		static const int32_t STACK_BUFF_SIZE = 64000;
		std::vector<preallocated_stack_type> stacks;
		std::vector<bool> marks;
		Mutex lock;

		size_t size;
	public:

		const preallocated_stack_type& get(size_t& handle) {
			WriteLocker w(&lock);
			size_t enter_size = marks.size();
			handle = enter_size;
			for (size_t i = 0; i < enter_size; i++) {
				if (!marks[i]) {
					marks[i] = true;
					handle = i;
					break;
				}
			}

			if (handle == enter_size) {
				stacks.push_back(preallocated_stack_type(new CellValue[STACK_BUFF_SIZE]));
				marks.push_back(true);
			}

			return stacks[handle];
		}

		void release(size_t& handle) {
			WriteLocker w(&lock);
			marks[handle] = false;
		}

		preallocated_stacks() :
			size(INITIAL_PREALLOCATED_SIZE) {
			stacks.reserve(INITIAL_PREALLOCATED_SIZE);
			for (size_t i = 0; i < INITIAL_PREALLOCATED_SIZE; i++) {
				stacks.push_back(preallocated_stack_type(new CellValue[STACK_BUFF_SIZE]));
				marks.push_back(false);
			}
		}

		~preallocated_stacks() {
			for (size_t i = 0; i < stacks.size(); i++) {
				CellValue* crt = stacks[i];
				delete[] crt;
			}
		}

		static preallocated_stacks& get_instance() {
			static preallocated_stacks instance;
			return instance;
		}
	};
private:
	//volatile general registers (integer).
	uint32_t i;
	uint32_t j;
	uint32_t k;
	int32_t s;

	//volatile time register
	struct tm t;

	//volatile string register
	CellValue str_t;
	CellValue str_s;

	//volatile Value register
	double dbl_t;

	//volatile char* register
	char * p;

	//volatile char array register
	char * str_buf;

	//volatile EPath register
	EPath path_t;

	//program counter
	Bytecode * pc;

	//invariants.
	RulesContext* m_mem_context;

	//stack pointers
	CellValue *sp;
	CellValue *stack;

	// active rule
	ERule* rule;

	//handle of preallocated mem
	size_t handle;

	double dummy41;

	//string buffer
	char str_bp[256];

	//registers holding non-volatile values
	CellValue val_0;

	//placeholders for return values
	CellValue * ptrValue;

	Context *context;

	vector<CellValueStream *> *sourceStreams;
	VMCache *vmCache;
	PUser user;

	struct machine_state {
		EPath path;
		ERule* rule;
		double defValue;
		bool notFoundStatus;
		Bytecode * pc;
		CellValue* sp;
		palo::bytecode_generator::WM_OPCODES return_pos;
	};

	class machine_stack : private std::vector<machine_state> {
	private:
		static const unsigned int MAX_STACK_SIZE = 50 * 1024;
	public:
		bool is_empty() {
			return empty();
		}
		void push_state(EPath path, ERule* rule, double defValue, bool notFoundStatus, Bytecode *pc, bytecode_generator::WM_OPCODES return_pos, CellValue* sp) {
			machine_state state;
			for (size_t i = 0; i < rule->cube->nrDimensions; i++) {
				state.path[i] = path[i];
			}
			state.rule = rule;
			state.defValue = defValue;
			state.notFoundStatus = notFoundStatus;
			state.pc = pc;
			state.return_pos = return_pos;
			state.sp = sp;

			push_back(state);
		}

		Bytecode pop_state(IdentifierType *path, ERule** rule, double* defValue, bool *notFoundStatus, Bytecode **pc, CellValue** sp) {
			bytecode_generator::WM_OPCODES return_pos;
			machine_state state = back();
			for (size_t i = 0; i < state.rule->cube->nrDimensions; i++) {
				path[i] = state.path[i];
			}
			*rule = state.rule;
			*defValue = state.defValue;
			*notFoundStatus = state.notFoundStatus;
			*pc = state.pc;
			return_pos = state.return_pos;
			*sp = state.sp;

			pop_back();
			return return_pos;
		}

	};

	machine_stack m_stack;

private:
	long StringToInt(const std::string& str) {
		char *p;

		if (str.empty())
			return 0;

		long i = strtol(str.c_str(), &p, 10);

		if (0 != *p)
			return 0;
		return i;
	}

	inline bool is_consolidation(IdentifierType *path, ECube& cube) {
		bool result = false;
		if (!cube.acube->supportsAggregations()) {
			return false;
		}
		/* check if consolidated                                               */
		for (uint32_t d = 0; d < cube.nrDimensions; d++) {
			char ElemType = cube.dimensions[d]->type[path[d]];
			if (ElemType == 'C') {
				result = true;
			} else if (ElemType == 'S') {
				result = false;
				break;
			}
		}

		return result;
	}

	inline bool is_string(IdentifierType *path, ECube& cube) {
		/* check if consolidated                                               */
		for (uint32_t d = 0; d < cube.nrDimensions; d++) {
			if (path[d] != NO_IDENTIFIER && cube.dimensions[d]->type[path[d]] == 'S') {
				return true;
			}
		}

		return false;
	}

	ERule* get_matching_rule(EElementId* a_path, list<ERule*>& a_list) {
		/* loop over the rules                                             */
		for (list<ERule*>::iterator it = a_list.begin(); it != a_list.end(); ++it) {
			if (IsBasePathInArea((*it)->dest_area, a_path)) {
				return *it;
			}
		}

		return NULL;
	}

	inline void ruleError( ErrorException::ErrorType err_code, int stack_correction )
	{
		val_0 = CellValue(err_code);
		val_0.setRuleId(rule->nr_rule);
		sp -= stack_correction;
		if (sp <= stack) {
			Logger::error << "rule value stack underflow while processing rule: " << rule->arule->getId() << " of cube: " << rule->cube->acube->getName() << endl;
			sp = stack;
			throw ErrorException(ErrorException::ERROR_INTERNAL, "rule value stack underflow while processing rule", rule->nr_rule);
		}
	}

	inline bool checkStackErrors( int params_count )
	{
		if (params_count) {
			if (val_0.isError()) {
				sp -= params_count-1;
				return false;
			} else {
				for (int iparam = 1; iparam < (params_count); iparam++) {
					if ((sp-1)->isError()) {
						val_0 = sp[-1];
						sp -= params_count-1;
						return false;
					}
				}
			}
		}
		return true;
	}

public:
	void compute(EngineBase *engine, IdentifierType *path, ERule* rule, double defValue, bool notFoundStatus);

	virtual_machine(RulesContext* mem_context, CellValue* ptrValue) :
		m_mem_context(mem_context), ptrValue(ptrValue), context(mem_context->context), sourceStreams(0), vmCache(0)
	{
		stack = preallocated_stacks::get_instance().get(handle);

		sp = stack;

		str_buf = str_bp;
	}

	~virtual_machine() {
		preallocated_stacks::get_instance().release(handle);
	}

	void setSourceStreams(vector<CellValueStream *> *sourceStreams) {this->sourceStreams = sourceStreams;}
	void setCache(VMCache *vmCache) {this->vmCache = vmCache;}
	void setUser(PUser user) {this->user = user;}
};
}

#endif
