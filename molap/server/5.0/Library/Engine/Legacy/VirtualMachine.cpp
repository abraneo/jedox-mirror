/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Engine/Legacy/VirtualMachine.h"
#include "Engine/EngineCpu.h"
#include "VirtualMachine/BytecodeGenerator.h"
#include "Olap/Rule.h"
#include "Parser/FunctionNodeDateformat.h"

using namespace palo;

namespace paloLegacy {

void virtual_machine::compute(EngineBase *engine, IdentifierType *path, ERule* rule, double defValue, bool notFoundStatus)
{
	pc = rule->bytecode;

	Bytecode crt_pc;
	Bytecode ret_pc = bytecode_generator::INVALID;

	context->check();

	while (1) {
		if (bytecode_generator::INVALID == ret_pc) {
			crt_pc = *pc++;
		} else {
			crt_pc = ret_pc;
			ret_pc = bytecode_generator::INVALID;
		}

		switch (crt_pc) {

		case bytecode_generator::NOP:
			break;

		case bytecode_generator::HALT: {
			*ptrValue = val_0;
			if (val_0.isNumeric()) {
				m_mem_context->writeQueryCache(CONST_COMMITABLE_CAST(Cube, rule->cube->acube->shared_from_this()), path, val_0.getNumeric(), false);
				rule->arule->increaseEvalCounter(val_0.getNumeric() == 0.0);
			}
			RETURN()
		}

		case bytecode_generator::FUNC_ISERROR_DBL:
		case bytecode_generator::FUNC_ISERROR_STR:
			val_0 = 0.0;
			break;

		case bytecode_generator::PUSH_DBL:
		case bytecode_generator::PUSH_STR:
			*sp++ = val_0;
			break;

		case bytecode_generator::PULL_DBL:
		case bytecode_generator::PULL_STR:
			val_0 = *--sp;
			break;

		case bytecode_generator::SWAP_DBL:
		case bytecode_generator::SWAP_STR:
			str_t = *(sp - 1);
			*(sp - 1) = val_0;
			val_0 = str_t;
			break;

		case bytecode_generator::DBL_2_STR:
			break;

		case bytecode_generator::STR_2_DBL:
			break;

		case bytecode_generator::LD_CONST_DBL:
			*sp++ = val_0;
			val_0 = rule->dbl_consts[*pc++];
			break;

		case bytecode_generator::LD_CONST_STR:
			*sp++ = val_0;
			val_0 = rule->str_consts[*pc++];
			if (val_0.empty()) {
				val_0 = CellValue::NullString;
			}
			break;

		case bytecode_generator::LD_SRC_HIT_DBL:
			if (m_stack.is_empty() && !notFoundStatus) {
				i = *pc++;
				*sp++ = val_0;
				val_0 = defValue;
				break;
			} else {
				// else - evaluate LD_SRC_HIT_DBL as normal LD_SRC_DBL
			}
		case bytecode_generator::LD_SRC_STR:
		case bytecode_generator::LD_SRC_DBL: {
			CellValue str_result;
			i = *pc++;

			if (m_stack.is_empty() && sourceStreams && rule->source_precalc[i]) {
				if (sourceStreams->at(i)) {
					str_result = sourceStreams->at(i)->getValue();
				} else {
					str_result = CellValue::NullNumeric;
				}
				*sp++ = val_0;
				val_0 = str_result;
			} else {
				bool invalidCell = false;
				for (j = 0; j < rule->cube->nrDimensions; j++) {
					if (rule->copy_mask[i][j] == AreaNode::ABS_RESTRICTION) {
						path_t[j] = rule->copy_source[i][j];
					} else if (rule->copy_mask[i][j] == AreaNode::OFFSET_RESTRICTION) {
						// Id => position => +offset => Id
						PositionType pos = rule->cube->dimensions[j]->adimension->lookupElement((IdentifierType)path[j], false)->getPosition();
						int32_t offset = (int32_t)rule->copy_source[i][j];
						if ((int32_t)pos + offset < 0 || pos + offset >= rule->cube->dimensions[j]->size) {
							// out of range -> return null value
							path_t[j] = NO_IDENTIFIER;
							invalidCell = true;
						} else {
							Element *aelm = rule->cube->dimensions[j]->adimension->lookupElementByPosition(pos + offset, false);
							if (!aelm) {
								throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
							}
							path_t[j] = aelm->getIdentifier();
						}
					} else {
						path_t[j] = path[j];
					}
				}


				bool isStr = is_string(path_t, *rule->cube);
				CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
				CPCube acube = CONST_COMMITABLE_CAST(Cube, rule->cube->acube->shared_from_this());

				if (invalidCell) {
					dbl_t = 0;
					str_result = isStr ? CellValue::NullString : CellValue::NullNumeric;
				} else {
					bool isString;
					if (m_mem_context->readQueryCache(acube, path_t, dbl_t, isString)) {
						if (isString) {
							StringStorageCpu *st = dynamic_cast<StringStorageCpu*>(engine->getStorage(acube->getStringStorageId()).get());
							st->convertToCellValue(str_result, dbl_t);
						}
					} else {
						if (isStr) {
							// check rule recursion
							m_mem_context->m_recursion_stack.push(rule->cube->acube, rule->arule, path_t);
							const IdentifiersType cp(path_t, path_t + rule->cube->acube->getDimensions()->size());
							PCubeArea calcArea(new CubeArea(adb, acube, cp));
							PCellStream cs = rule->cube->acube->calculateArea(calcArea, CubeArea::BASE_STRING, ALL_RULES, true, UNLIMITED_UNSORTED_PLAN);
							if (cs->next()) {
								str_result = cs->getValue();
								if (str_result.isError()) {
									throw ErrorException(str_result.getError(), "", rule->nr_rule);
								}
							} else {
								str_result = CellValue::NullString;
							}
							m_mem_context->m_recursion_stack.pop();
						} else {
							bool new_call = false;

							if (is_consolidation(path_t, *rule->cube)) {
								for (list<ERule*>::iterator it = rule->cube->rules_c.begin(); it != rule->cube->rules_c.end(); ++it) {
									if (IsPathInArea((*it)->dest_area, path_t)) {
										/* rules that apply to cons cells should never have marker */
										/* even if the code does have a marker the following code  */
										/* will not crash, but simply return 0 for the marker      */

										if (crt_pc == bytecode_generator::LD_SRC_STR) {
											PUSH(bytecode_generator::RET_LD_SRC_STR)
										} else {
											PUSH(bytecode_generator::RET_LD_SRC_DBL)
										}

										rule = *it;
										for (j = 0; j < rule->cube->nrDimensions; j++) {
											path[j] = path_t[j];
										}
										pc = rule->bytecode;
										defValue = 0;
										notFoundStatus = false;
										new_call = true;
										break;

									}
								}
								if (new_call) {
									continue;
								}

								/* if no applicable rule is found, treat this as an area           */
								m_mem_context->m_recursion_stack.push(rule->cube->acube, 0, path_t);
								const IdentifiersType cp(path_t, path_t + rule->cube->acube->getDimensions()->size());
								PCubeArea calcArea(new CubeArea(adb, acube, cp));
								PCellStream cs = rule->cube->acube->calculateArea(calcArea, CubeArea::CONSOLIDATED, INDIRECT_RULES, true, 0);
								CellValue result;
								if (cs->next()) {
									result = cs->getValue();
									if (result.isError()) {
										throw ErrorException(result.getError(), "", rule->nr_rule);
									}
								} else {
									result = CellValue::NullNumeric;
								}
								dbl_t = result.getNumeric();
								m_mem_context->m_recursion_stack.pop();
								m_mem_context->writeQueryCache(acube, path_t, dbl_t, false);
							} else { // cell is base
								// loop over the rules
								for (list<ERule*>::iterator it = rule->cube->rules_n.begin(); it != rule->cube->rules_n.end(); ++it) {
									if (IsBasePathInArea((*it)->dest_area, path_t)) {
										if ((*it)->ubm_flag) {
											if (crt_pc == bytecode_generator::LD_SRC_STR) {
												PUSH(bytecode_generator::RET_LD_SRC_STR)
											} else {
												PUSH(bytecode_generator::RET_LD_SRC_DBL)
											}

											rule = *it;
											for (j = 0; j < rule->cube->nrDimensions; j++) {
												path[j] = path_t[j];
											}
											pc = rule->bytecode;

											defValue = 0;	// better to return nothing than random values
											notFoundStatus = false;
											new_call = true;
											break;
										} else {
											if (crt_pc == bytecode_generator::LD_SRC_STR) {
												PUSH(bytecode_generator::RET_LD_SRC_STR)
											} else {
												PUSH(bytecode_generator::RET_LD_SRC_DBL)
											}

											rule = *it;
											for (j = 0; j < rule->cube->nrDimensions; j++) {
												path[j] = path_t[j];
											}
											pc = rule->bytecode;
											defValue = 0;
											notFoundStatus = false;
											new_call = true;
											break;
										}
									}
								}
								if (new_call) {
									continue;
								}
								// if no applicable rule is found, try a base lookup
								const IdentifiersType apath(path_t, path_t + rule->cube->acube->getDimensions()->size());
								CellValue value = rule->cube->numStorage->valuesCount() ? rule->cube->numStorage->getCellValue(apath) : CellValue();

								if (!value.isEmpty()) {
									dbl_t = value.getNumeric();
								} else {
									dbl_t = 0.0;
								}
							}
						}
					}
				}

				if (isStr) {
					*sp++ = val_0;
					val_0 = str_result;
				} else {
					*sp++ = val_0;
					val_0 = dbl_t;
				}
			}
		}
		break;

		case bytecode_generator::RET_LD_SRC_STR:
		case bytecode_generator::RET_LD_SRC_DBL:
			sp++;
			val_0 = *ptrValue;
			break;

		case bytecode_generator::LD_VAR_STR:
			i = *pc++;
			*sp++ = val_0;
			val_0 = rule->cube->dimensions[i]->adimension->lookupElement((IdentifierType)path[i], false)->getName(rule->cube->dimensions[i]->adimension->getElemNamesVector());
			break;

		case bytecode_generator::OP2_SUM_DBL:
			val_0 = (--sp)->getNumeric() + val_0.getNumeric();
			break;

		case bytecode_generator::OP2_DIFF_DBL:
			val_0 = (--sp)->getNumeric() - val_0.getNumeric();
			break;

		case bytecode_generator::OP2_PROD_DBL:
			val_0 = (--sp)->getNumeric() * val_0.getNumeric();
			break;

		case bytecode_generator::OP2_QUO_DBL:
			if (0 == val_0.getNumeric()) {
				if (ignoreRuleError) {
					val_0 = 0.0;
				} else {
					throw ErrorException(ErrorException::ERROR_RULE_DIVISION_BY_ZERO, "Division by zero.", rule->nr_rule);
				}
			} else {
				val_0 = (--sp)->getNumeric() / val_0.getNumeric();
			}
			break;

		case bytecode_generator::OP2_EQ_DBL:
		case bytecode_generator::OP2_EQ_STR:
			{
				CellValue *tmp = --sp;
				if (tmp->isEmpty() && val_0.isEmpty()) {
					val_0 = 1.0;
				} else if (tmp->isNumeric() && val_0.isNumeric()) {
					val_0 = (tmp->getNumeric() == val_0.getNumeric() ? 1.0 : 0.0);
				} else {
					val_0 = (*tmp == val_0 ? 1.0 : 0.0);
				}
			}
			break;

		case bytecode_generator::OP2_NE_DBL:
		case bytecode_generator::OP2_NE_STR:
			{
				CellValue *tmp = --sp;
				if (tmp->isEmpty() && val_0.isEmpty()) {
					val_0 = 0.0;
				} else if (tmp->isNumeric() && val_0.isNumeric()) {
					val_0 = (tmp->getNumeric() != val_0.getNumeric() ? 1.0 : 0.0);
				} else {
					val_0 = (*tmp != val_0 ? 1.0 : 0.0);
				}
			}
			break;

		case bytecode_generator::OP2_LT_DBL:
		case bytecode_generator::OP2_LT_STR:
			{
				CellValue *tmp = --sp;
				if (tmp->isNumeric() && val_0.isNumeric()) {
					val_0 = (tmp->getNumeric() < val_0.getNumeric() ? 1.0 : 0.0);
				} else {
					val_0 = (*tmp < val_0 ? 1.0 : 0.0);
				}
			}
			break;

		case bytecode_generator::OP2_LE_DBL:
		case bytecode_generator::OP2_LE_STR:
			{
				CellValue *tmp = --sp;
				if (tmp->isNumeric() && val_0.isNumeric()) {
					val_0 = (tmp->getNumeric() <= val_0.getNumeric() ? 1.0 : 0.0);
				} else {
					val_0 = (*tmp <= val_0 ? 1.0 : 0.0);
				}
			}
			break;

		case bytecode_generator::CALL_GEN_DBL:
		case bytecode_generator::CALL_GEN_STR:
			break;

		case bytecode_generator::CALL_DATA_STR:
		case bytecode_generator::CALL_DATA_DBL: {
			i = *pc++;
			CPCube acube;
			int16_t cubeIndex = *pc++;
			uint32_t dims;
			int16_t *dimensionOrdinal = 0;
			IdentifierType *dimensionElements = 0;
			CPDatabase adb;

			if (cubeIndex == -1) {
				adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
				if (adb->getName() != val_0) {
					CPServer asrv = context->getServer();
					adb = asrv->lookupDatabaseByName(val_0, false);
				} else {
					// identical database -> try identical cube
					acube = CONST_COMMITABLE_CAST(Cube, rule->cube->acube->shared_from_this());
				}
				if (!adb) {
					throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
				}
				val_0 = *--sp;
				i--;
				if (!acube || acube->getName() != val_0) {
					acube = adb->findCubeByName(val_0, PUser(), true, false);
				}
				val_0 = *--sp;
				i--;
				dims = i;
			} else {
				// we do not have to resolve database, cube
				context->getPaloDataCube(cubeIndex, acube, adb);
				dims = *pc++;
				dimensionOrdinal = (int16_t*)pc;
				pc += dims;
				dimensionElements = (IdentifierType*)pc;
				pc += dims*2;
			}
			if (!acube || !adb) {
				throw ErrorException(ErrorException::ERROR_CUBE_NOT_FOUND, "", rule->nr_rule);
			}
			if (dims != acube->getDimensions()->size()) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			context->setCacheDependence(dbID_cubeID(adb->getId(), acube->getId()));
			for (j = 0; j < dims; j++) {
				if (dimensionOrdinal && dimensionOrdinal[j] != -1) {
					path_t[j] = path[dimensionOrdinal[j]];
				} else if (dimensionElements && dimensionElements[j] != NO_IDENTIFIER) {
					path_t[j] = dimensionElements[j];
				} else {
					path_t[j] = NO_IDENTIFIER;
					if (vmCache) {
						ElemNameCache::iterator enc = vmCache->find(acube);
						if (enc != vmCache->end() && !enc->second[j].first.empty() && !enc->second[j].first.compare(val_0)) {
							path_t[j] = enc->second[j].second;
						}
					}
					if (path_t[j] == NO_IDENTIFIER) {
						CPDimension adim = adb->lookupDimension(acube->getDimensions()->at(j), false);
						Element * aelm = adim->lookupElementByName(val_0, false);
						if (!aelm) {
							throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
						}
						path_t[j] = aelm->getIdentifier();
						if (vmCache) {
							CubeElemNameCache &cec = (*vmCache)[acube];
							if (cec.empty()) {
								cec.resize(dims);
							}
							cec[j] = make_pair((string)val_0, path_t[j]);
						}
					}
					val_0 = *--sp;
					i--;
				}
			}
			ECube * cube = NewEntryCube(*acube, *adb, *engine, context);

			CellValue result;
			bool isStr = is_string(path_t, *cube);

			bool isString;
			if (m_mem_context->readQueryCache(acube, path_t, dbl_t, isString)) {
				if (isString) {
					StringStorageCpu *st = dynamic_cast<StringStorageCpu*>(engine->getStorage(acube->getStringStorageId()).get());
					st->convertToCellValue(result, dbl_t);
				} else {
					result = dbl_t;
				}
			} else {
				if (isStr) {
					bool rulecalculated = false;
					// loop over the rules
					for (list<ERule*>::iterator it = cube->rules_n.begin(); it != cube->rules_n.end(); ++it) {
						if (IsPathInArea((*it)->dest_area, path_t)) {
							// check rule recursion
							m_mem_context->m_recursion_stack.push(cube->acube, rule->arule, path_t);
							const IdentifiersType cp(path_t, path_t + cube->acube->getDimensions()->size());
							PCubeArea calcArea(new CubeArea(adb, acube, cp));
							PCellStream cs = cube->acube->calculateArea(calcArea, CubeArea::BASE_STRING, ALL_RULES, true, 0);
							if (cs->next()) {
								result = cs->getValue();
								if (result.isError()) {
									throw ErrorException(result.getError(), "", rule->nr_rule);
								}
							} else {
								result = CellValue::NullString;
							}
							m_mem_context->m_recursion_stack.pop();
							rulecalculated = true;

							break;
						}
					}
					if (!rulecalculated) {
						// if no applicable rule is found, try a base lookup
						IdentifiersType apath(path_t, path_t + cube->acube->getDimensions()->size());
						PStorageBase st = engine->getStorage(cube->acube->getStringStorageId());
						result = st->valuesCount() ? st->getCellValue(apath) : CellValue();
						if (!result.isEmpty()) {
							m_mem_context->writeQueryCache(acube, path_t, result.getNumeric(), true);
						}
					}
				} else {
					bool new_call = false;

					if (is_consolidation(path_t, *cube)) {
						/* loop over the rules                                             */
						for (list<ERule*>::iterator it = cube->rules_c.begin(); it != cube->rules_c.end(); ++it) {
							if (IsPathInArea((*it)->dest_area, path_t)) {
								/* rules that apply to cons cells should never have marker */
								/* even if the code does have a marker the following code  */
								/* will not crash, but simply return 0 for the marker      */

								if (crt_pc == bytecode_generator::CALL_DATA_STR) {
									PUSH(bytecode_generator::RET_CALL_DATA_STR);
									*sp++ = val_0;
								} else {
									PUSH(bytecode_generator::RET_CALL_DATA_DBL)
									*sp++ = val_0;
								}

								rule = *it;
								for (j = 0; j < rule->cube->nrDimensions; j++) {
									path[j] = path_t[j];
								}
								pc = rule->bytecode;
								defValue = 0;
								notFoundStatus = false;
								new_call = true;
								break;
							}
						}
						if (new_call) {
							continue;
						}

						/* if no applicable rule is found, treat this as an area           */
						m_mem_context->m_recursion_stack.push(cube->acube, 0, path_t);
						const IdentifiersType cp(path_t, path_t + cube->acube->getDimensions()->size());
						PCubeArea calcArea(new CubeArea(adb, acube, cp));
						PCellStream cs = cube->acube->calculateArea(calcArea, CubeArea::CONSOLIDATED, INDIRECT_RULES, true, 0);
						if (cs->next()) {
							result = cs->getValue();
							if (result.isError()) {
								throw ErrorException(result.getError(), "", rule->nr_rule);
							}
						} else {
							result = CellValue::NullNumeric;
						}
						dbl_t = result.getNumeric();
						m_mem_context->m_recursion_stack.pop();
						m_mem_context->writeQueryCache(acube, path_t, dbl_t, false);
					} else { // cell is base
						// loop over the rules
						for (list<ERule*>::iterator it = cube->rules_n.begin(); it != cube->rules_n.end(); ++it) {
							if (IsBasePathInArea((*it)->dest_area, path_t)) {
								if ((*it)->ubm_flag) {
									if (crt_pc == bytecode_generator::CALL_DATA_STR) {
										PUSH(bytecode_generator::RET_CALL_DATA_STR);
										*sp++ = val_0;
									} else {
										PUSH(bytecode_generator::RET_CALL_DATA_DBL)
										*sp++ = val_0;
									}

									rule = *it;
									for (j = 0; j < rule->cube->nrDimensions; j++) {
										path[j] = path_t[j];
									}
									pc = rule->bytecode;
									new_call = true;
									break;
								} else {
									if (crt_pc == bytecode_generator::CALL_DATA_STR) {
										PUSH(bytecode_generator::RET_CALL_DATA_STR);
										*sp++ = val_0;
									} else {
										PUSH(bytecode_generator::RET_CALL_DATA_DBL)
										*sp++ = val_0;
									}

									rule = *it;
									for (j = 0; j < rule->cube->nrDimensions; j++) {
										path[j] = path_t[j];
									}
									pc = rule->bytecode;
									defValue = 0;
									notFoundStatus = false;
									new_call = true;
									break;
								}
							}
						}
						if (new_call) {
							continue;
						}
						// if no applicable rule is found, try a base lookup
						IdentifiersType apath(path_t, path_t + cube->acube->getDimensions()->size());
						PStorageBase st = engine->getStorage(cube->acube->getNumericStorageId());
						result = st->getCellValue(apath);
					}
				}
			}

			*sp++ = val_0;
			val_0 = result;
		}
		break;

		case bytecode_generator::RET_CALL_DATA_DBL:
		case bytecode_generator::RET_CALL_DATA_STR:
			sp++;
			val_0 = *ptrValue;
			break;

		case bytecode_generator::JUMP:
			i = *pc++;
			pc += i;
			break;

		case bytecode_generator::JUMP_IF_NOT:
			i = *pc++;
			if (val_0 == 0.0) {
				val_0 = *--sp;
				pc += i;
			} else {
				val_0 = *--sp;
			}
			break;

		case bytecode_generator::CONTINUE:
			/* XXX or could we need to call a C-Rule here too ? */
		{
			list<ERule*>* plist;
			if (is_consolidation(path, *rule->cube)) {
				plist = &rule->cube->rules_c;
			} else {
				plist = &rule->cube->rules_n;
			}
			list<ERule*>::iterator end_it = plist->end();
			list<ERule*>::iterator it = find(plist->begin(), end_it, rule);
			do {
				++it;
			} while (it != end_it && !IsPathInArea((*it)->dest_area, path));
			if (it != end_it) {
				rule = *it;
				pc = rule->bytecode;
				sourceStreams = 0;
				notFoundStatus = true;
			} else {
				CellValue val;
				CPDatabase adb = CONST_COMMITABLE_CAST(Database, context->getParent(rule->cube->acube->shared_from_this()));
				const IdentifiersType cp(path, path + rule->cube->acube->getDimensions()->size());
				PCubeArea calcArea(new CubeArea(adb, CONST_COMMITABLE_CAST(Cube, rule->cube->acube->shared_from_this()), cp));
				PCellStream cs = rule->cube->acube->calculateArea(calcArea, CubeArea::ALL, INDIRECT_RULES, true, 0);
				if (cs->next()) {
					val = cs->getValue();
				}
				*ptrValue = val;
				RETURN()
			}
		}
		break;

		case bytecode_generator::STET:
			*sp = val_0;
			if (m_stack.is_empty() && sourceStreams && rule->precalcStet) {
				if (sourceStreams->at(rule->gc_copy_nr)) {
					*ptrValue = sourceStreams->at(rule->gc_copy_nr)->getValue();
				} else {
					*ptrValue = CellValue::NullNumeric;
				}
			} else {
				const IdentifiersType apath(path, path + rule->cube->acube->getDimensions()->size());
				if (is_string(path, *rule->cube)) {
					*ptrValue = rule->cube->strStorage->valuesCount() ? rule->cube->strStorage->getCellValue(apath) : CellValue(false);
				} else if (is_consolidation(path, *rule->cube)) {
					CellValue val;
					CPDatabase adb = CONST_COMMITABLE_CAST(Database, context->getParent(rule->cube->acube->shared_from_this()));
					PCubeArea calcArea(new CubeArea(adb, CONST_COMMITABLE_CAST(Cube, rule->cube->acube->shared_from_this()), apath));
					PCellStream cs = rule->cube->acube->calculateArea(calcArea, CubeArea::ALL, RulesType(INDIRECT_RULES | NOCACHE), true, 0);
					if (cs->next()) {
						val = cs->getValue();
					}
					*ptrValue = val;
				} else {
					// base numeric cell
					*ptrValue = rule->cube->numStorage->valuesCount() ? rule->cube->numStorage->getCellValue(apath) : CellValue();
				}
			}
			RETURN()

		case bytecode_generator::OP2_LOG_DBL:
			dbl_t = (--sp)->getNumeric();
			if (0 < dbl_t && 0 < val_0.getNumeric()) {
				val_0 = log(dbl_t) / log(val_0.getNumeric());
			} else {
				val_0 = 0.0;
			}
			break;

		case bytecode_generator::OP2_MOD_DBL:
			dbl_t = (--sp)->getNumeric();
			if (val_0.getNumeric() != 0.0) {
				val_0 = dbl_t - floor(dbl_t / val_0.getNumeric()) * val_0.getNumeric();
			} else {
				val_0 = 0.0;
			}
			break;

		case bytecode_generator::OP2_POWER_DBL:
			dbl_t = (--sp)->getNumeric();
			val_0 = pow(dbl_t, val_0.getNumeric());
			break;

		case bytecode_generator::OP2_QUOTIENT_DBL:
			dbl_t = (--sp)->getNumeric();
			if (val_0.getNumeric() != 0.0) {
				val_0 = trunc(dbl_t / val_0.getNumeric());
			} else {
				val_0 = 0.0;
			}
			break;

		case bytecode_generator::OP2_RANDBETWEEN_DBL:
			dbl_t = (--sp)->getNumeric();
			if (dbl_t < val_0.getNumeric()) {
				val_0 = dbl_t + ((double)rand()) / double(RAND_MAX) * (val_0.getNumeric() - dbl_t);
			} else {
				val_0 = 0.0;
			}
			break;

		case bytecode_generator::OP2_ROUND_DBL:
			dbl_t = (--sp)->getNumeric();
			if (0 < val_0.getNumeric()) {
				val_0 = pow((double)10, int(val_0.getNumeric()));
				val_0 = round(dbl_t * val_0.getNumeric()) / val_0.getNumeric();
			} else if (val_0.getNumeric() < 0) {
				val_0 = pow((double)10, -int(val_0.getNumeric()));
				val_0 = round(dbl_t / val_0.getNumeric()) * val_0.getNumeric();
			} else {
				val_0 = round(dbl_t);
			}
			break;

		case bytecode_generator::OP1_NOT_DBL:
			val_0 = (0 == val_0.getNumeric() ? 1.0 : 0.0);
			break;

		case bytecode_generator::OP1_ABS_DBL:
			val_0 = (0 <= val_0.getNumeric() ? val_0.getNumeric() : -val_0.getNumeric());
			break;

		case bytecode_generator::OP1_ACOS_DBL:
			if (-1.0 <= val_0.getNumeric() && val_0.getNumeric() <= 1.0) {
				val_0 = acos(val_0.getNumeric());
			} else {
				val_0 = 0.0;
			}
			break;

		case bytecode_generator::OP1_ASIN_DBL:
			if (-1.0 <= val_0.getNumeric() && val_0.getNumeric() <= 1.0) {
				val_0 = asin(val_0.getNumeric());
			} else {
				val_0 = 0.0;
			}
			break;

		case bytecode_generator::OP1_ATAN_DBL:
			val_0 = atan(val_0.getNumeric());
			break;

		case bytecode_generator::OP1_CEILING_DBL:
			val_0 = ceil(val_0.getNumeric());
			break;

		case bytecode_generator::OP1_COS_DBL:
			val_0 = cos(val_0.getNumeric());
			break;

		case bytecode_generator::OP1_EVEN_DBL:
			val_0 = round(val_0.getNumeric() / 2) * 2;
			break;

		case bytecode_generator::OP1_EXP_DBL:
			val_0 = exp(val_0.getNumeric());
			break;

		case bytecode_generator::OP1_FACT_DBL:
			i = (int)val_0.getNumeric();
			val_0 = 1.0;
			for (j = 2; j <= i; j++) {
				val_0 = val_0.getNumeric() * j;
			}
			break;

		case bytecode_generator::OP1_FLOOR_DBL:
		case bytecode_generator::OP1_INT_DBL:
			val_0 = floor(val_0.getNumeric());
			break;

		case bytecode_generator::OP1_LN_DBL:
			if (0.0 < val_0.getNumeric()) {
				val_0 = log(val_0.getNumeric());
			} else {
				val_0 = 0.0;
			}
			break;

		case bytecode_generator::OP1_LOG10_DBL:
			if (0.0 < val_0.getNumeric()) {
				val_0 = log10(val_0.getNumeric());
			} else {
				val_0 = 0.0;
			}
			break;

		case bytecode_generator::OP1_ODD_DBL:
			val_0 = round((val_0.getNumeric() - 1) / 2) * 2 + 1;
			break;

		case bytecode_generator::OP1_SIGN_DBL:
			if (0 < val_0.getNumeric()) {
				val_0 = 1.0;
			} else if (val_0.getNumeric() < 0) {
				val_0 = -1.0;
			} else {
				val_0 = 0.0;
			}
			break;

		case bytecode_generator::OP1_SIN_DBL:
			val_0 = sin(val_0.getNumeric());
			break;

		case bytecode_generator::OP1_SQRT_DBL:
			if (0.0 <= val_0.getNumeric()) {
				val_0 = sqrt(val_0.getNumeric());
			} else {
				val_0 = 0.0;
			}
			break;

		case bytecode_generator::OP1_TAN_DBL:
			val_0 = tan(val_0.getNumeric());
			break;

		case bytecode_generator::OP1_TRUNC_DBL:
			val_0 = floor(val_0.getNumeric());
			break;

		case bytecode_generator::OP1_WEEKDAY_DBL: {
			time_t tt = (time_t)val_0.getNumeric();
			struct tm* t = gmtime(&tt);
			if (t) {
				val_0 = (double)t->tm_wday + 1.0;
			} else {
				val_0 = 0.0;
			}
		}
		break;

		case bytecode_generator::FUNC_CHAR: { /* S <- D */
			if (val_0.getNumeric()) {
				string str_c(1, (char)val_0.getNumeric());
				val_0 = str_c;
			} else {
				val_0 = "";
			}
		}
		break;

		case bytecode_generator::FUNC_CLEAN: { /* S <- S */
			size_t len = val_0.size();
			char * str_c = new char[len + 1];
			const char * p = val_0.c_str();
			char * q = str_c;
			for (i = 0; i < len; i++) {
				if (' ' <= *p) {
					*q++ = *p;
				}
				p++;
			}
			*q = '\0';
			val_0 = str_c;
			delete[] str_c;
		}
		break;

		case bytecode_generator::FUNC_CODE: /* D <- S */
			val_0 = (double)val_0[0];
			break;

		case bytecode_generator::FUNC_CONCATENATE: /* S <- S,S */
			val_0 = *(--sp) + val_0;
			break;

		case bytecode_generator::FUNC_DATE: /* D <- D,D,D */
			t.tm_mday = (int)val_0.getNumeric();
			t.tm_mon = (int)(--sp)->getNumeric() - 1;
			t.tm_year = (int)(--sp)->getNumeric() - 1900;
			t.tm_hour = 12;
			t.tm_min = 0;
			t.tm_sec = 0;
			t.tm_wday = 0;
			t.tm_yday = 0;
			t.tm_isdst = -1;
			val_0 = (double)(mktime(&t) / 86400 * 86400);
			break;

		case bytecode_generator::FUNC_DATEFORMAT: /* S <- D,S */
			val_0 = palo::FunctionNodeDateformat::Format((--sp)->getNumeric(), val_0);
			break;

		case bytecode_generator::FUNC_DATEVALUE: /* D <- S */
			if (val_0.length() > 6) {
				t.tm_mday = StringToInt(val_0.substr(3, 2));
				t.tm_mon = StringToInt(val_0.substr(0, 2)) - 1;
				t.tm_year = StringToInt(val_0.substr(6, 4));
				if (t.tm_year >= 1970 && t.tm_year < 2038) {
					t.tm_year -= 1900;
				} else if (t.tm_year >= 0 && t.tm_year < 100) {
					t.tm_year += 100;
				} else {
					val_0 = 0.0;
					break;
				}
				t.tm_hour = 12;
				t.tm_min = 0;
				t.tm_sec = 0;
				t.tm_wday = 0;
				t.tm_yday = 0;
				t.tm_isdst = -1;
				time_t dtm = mktime(&t);
				val_0 = (double)(dtm / 86400 * 86400);
			} else {
				val_0 = 0.0;
			}
			break;

		case bytecode_generator::FUNC_EXACT: /* D <- S,S */
			val_0 = (*--sp == val_0 ? 1.0 : 0.0);
			break;

		case bytecode_generator::FUNC_LEFT: /* S <- S,D */
			val_0 = (--sp)->substr(0, (int)val_0.getNumeric());
			break;

		case bytecode_generator::FUNC_LEN: /* D <- S */
			val_0 = (double)(val_0.length());
			break;

		case bytecode_generator::FUNC_LOWER: /* S <- S */
			val_0 = StringUtils::tolower(val_0);
			break;

		case bytecode_generator::FUNC_MID: /* S <- S,D,D */
		{
			dbl_t = (--sp)->getNumeric(); /* pos */
			string str(*(--sp));
			if (0 < (int)dbl_t && (unsigned int)dbl_t < str.length() && 0 < (int)val_0.getNumeric()) {
				val_0 = str.substr(((int)dbl_t) - 1, (int)val_0.getNumeric());
			} else {
				val_0 = "";
			}
			break;
		}
		case bytecode_generator::FUNC_NOW: /* D <- */
			*sp++ = val_0;
			val_0 = (double)time(0);
			break;

		case bytecode_generator::FUNC_PI: /* D <- */
			*sp++ = val_0;
			val_0 = M_PI;
			break;

		case bytecode_generator::FUNC_PROPER: /* S <- S */
			val_0 = StringUtils::capitalization(val_0);
			break;

		case bytecode_generator::FUNC_RAND: /* D <- */
			*sp++ = val_0;
			val_0 = ((double)rand()) / ((double)RAND_MAX);
			break;

		case bytecode_generator::FUNC_REPLACE: /* S <- S,D,D,S */
		{
			double d = (--sp)->getNumeric(); /* str */
			dbl_t = (--sp)->getNumeric(); /* pos */
			str_t = *--sp; /* str */
			if (0 <= ((int)dbl_t) && ((unsigned int)dbl_t) < str_t.length() && 0 <= ((int)d)) {
				if (((unsigned int)dbl_t) + ((unsigned int)d) <= str_t.length()) {
					val_0 = str_t.substr(0, ((int)dbl_t)) + val_0 + str_t.substr(((int)dbl_t) + ((int)d));
				} else {
					val_0 = str_t.substr(0, ((int)dbl_t)) + val_0;
				}
			} else {
				val_0 = "";
			}
			break;
		}
		case bytecode_generator::FUNC_REPT: /* S <- S,D */
			str_t = *--sp;
			dbl_t = val_0.getNumeric();
			val_0 = "";
			for (i = 0; i < ((unsigned int)dbl_t); i++) {
				val_0 += str_t;
			}
			break;

		case bytecode_generator::FUNC_RIGHT: /* S <- S,D */
			str_t = *--sp;
			if (((unsigned int)val_0.getNumeric()) < str_t.length()) {
				val_0 = str_t.substr(str_t.length() - ((int)val_0.getNumeric()), ((int)val_0.getNumeric()));
			} else {
				val_0 = "";
			}
			break;

		case bytecode_generator::FUNC_SEARCH: /* D <- S,S */
			str_t = *--sp;
			val_0 = (double)StringUtils::simpleSearch(val_0, str_t);
			break;

		case bytecode_generator::FUNC_STR: /* S <- D */
		{
			int d = (int)val_0.getNumeric();
			int l = (int)(--sp)->getNumeric();
			double v = (--sp)->getNumeric();
			val_0 = UTF8Comparer::doubleToString(v, l, d);
			break;
		}

		case bytecode_generator::FUNC_SUBSTITUTE: /* S <- S,S,S */
			str_s = *--sp; /* repl */
			str_t = *--sp; /* str */
			if (!str_s.empty()) {
				size_t i = 0;
				while ((i = str_t.find(str_s, i)) != string::npos) {
					str_t = str_t.substr(0, i) + val_0 + str_t.substr(i + str_s.size());
					i += val_0.size();
				}
			}
			val_0 = str_t;
			break;

		case bytecode_generator::FUNC_TRIM: { /* S <- S */
			size_t i = val_0.find_first_not_of(" \t\n\r");
			size_t j = val_0.find_last_not_of(" \t\n\r");
			if (i != std::string::npos) {
				val_0 = val_0.substr(i, j - i + 1);
			} else {
				val_0 = "";
			}
			break;
		}

		case bytecode_generator::FUNC_UPPER: /* S <- S */
			val_0 = StringUtils::toupper(val_0);
			break;

		case bytecode_generator::FUNC_VALUE: /* D <- S */
			val_0 = strtod(val_0.c_str(), &p);
			if (p && *p != 0) {
				val_0 = 0.0;
			}
			break;

		case bytecode_generator::AGGR_SUM: /* D <- D* */
			i = *pc++;
			for (j = 1; j < i; j++) {
				--sp;
				val_0 = val_0.getNumeric() + sp->getNumeric();
			}
			break;

		case bytecode_generator::AGGR_PROD: /* D <- D* */
			i = *pc++;
			for (j = 1; j < i; j++) {
				--sp;
				val_0 = val_0.getNumeric() * sp->getNumeric();
			}
			break;

		case bytecode_generator::AGGR_MIN: /* D <- D* */
			i = *pc++;
			for (j = 1; j < i; j++) {
				--sp;
				if (sp->getNumeric() < val_0.getNumeric()) {
					val_0 = *sp;
				}
			}
			break;

		case bytecode_generator::AGGR_MAX: /* D <- D* */
			i = *pc++;
			for (j = 1; j < i; j++) {
				--sp;
				if (val_0.getNumeric() < sp->getNumeric()) {
					val_0 = *sp;
				}
			}
			break;

		case bytecode_generator::AGGR_COUNT: /* D <- D* */
			i = *pc++;
			for (j = 1; j < i; j++) {
				--sp;
			}
			val_0 = (double)i;
			break;

		case bytecode_generator::AGGR_FIRST: /* D <- D* */
			i = *pc++;
			for (j = 1; j < i; j++) {
				--sp;
				val_0 = *sp;
			}
			break;

		case bytecode_generator::AGGR_LAST: /* D <- D* */
			i = *pc++;
			for (j = 1; j < i; j++) {
				--sp;
			}
			break;

		case bytecode_generator::AGGR_AVG: /* D <- D* */
			i = *pc++;
			for (j = 1; j < i; j++) {
				--sp;
				val_0 = val_0.getNumeric() + sp->getNumeric();
			}
			val_0 = val_0.getNumeric() / i;
			break;

		case bytecode_generator::AGGR_AND:
			i = *pc++;
			for (j = 1; j < i; j++) {
				--sp;
				if (*sp == 0.0) {
					val_0 = 0.0;
				}
			}
			break;

		case bytecode_generator::AGGR_OR:
			i = *pc++;
			for (j = 1; j < i; j++) {
				--sp;
				if (*sp != 0.0) {
					val_0 = 1.0;
				}
			}
			break;

		case bytecode_generator::PALO_CUBEDIMENSION: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			CPCube acube;
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			} else {
				// identical database -> try identical cube
				acube = CONST_COMMITABLE_CAST(Cube, rule->cube->acube->shared_from_this());
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			if (!acube || acube->getName() != val_0) {
				acube = adb->findCubeByName(val_0, PUser(), true, false);
			}
			if (!acube) {
				throw ErrorException(ErrorException::ERROR_CUBE_LOCK_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			i = (int)val_0.getNumeric();
			val_0 = "";
			if (0 < i && i <= acube->getDimensions()->size()) {
				val_0 = adb->lookupDimension(acube->getDimensions()->at(i - 1), false)->getName();
			}
		}
		break;

		case bytecode_generator::PALO_ECHILD: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm = adim->lookupElementByName(val_0, false);
			if (!aelm) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			i = (int)val_0.getNumeric();
			val_0 = "";
			if (0 < i && i <= aelm->getChildrenCount()) {
				val_0 = adim->lookupElement(aelm->getChildren()->at(i - 1).first, false)->getName(adim->getElemNamesVector());
			}
		}
		break;

		case bytecode_generator::PALO_ECHILDCOUNT: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm = adim->lookupElementByName(val_0, false);
			if (!aelm) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = (double)(aelm->getChildrenCount());
		}
		break;

		case bytecode_generator::PALO_ECOUNT: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = (double)(adim->sizeElements());

		}
		break;

		case bytecode_generator::PALO_EFIRST: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = "";
			if (0 < adim->sizeElements()) {
				val_0 = adim->getElements(PUser(), false)[0]->getName(adim->getElemNamesVector());
			}

		}
		break;

		case bytecode_generator::PALO_EINDENT: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm = adim->lookupElementByName(val_0, false);
			if (!aelm) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = (double)(aelm->getIndent());

		}
		break;

		case bytecode_generator::PALO_EINDEX: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm = adim->lookupElementByName(val_0, false);
			if (!aelm) {
				if (*pc == bytecode_generator::FUNC_ISERROR_DBL) {
					val_0 = 1.0;
					pc++;
				} else {
					val_0 = 0.0;
				}
//				--sp;
//				val_0 = 0.0;
				break;
			}

			val_0 = (double)aelm->getPosition()+1;
		}
		break;

		case bytecode_generator::PALO_EISCHILD: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm = adim->lookupElementByName(val_0, false);
			if (!aelm) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm2 = adim->lookupElementByName(val_0, false);
			if (!aelm2) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = 0.0;
			const IdentifiersWeightType *aelms = aelm->getChildren();
			for (IdentifiersWeightType::const_iterator aelmsi = aelms->begin(); aelmsi != aelms->end(); ++aelmsi) {
				if (aelmsi->first == aelm2->getIdentifier()) {
					val_0 = 1.0;
					break;
				}
			}

		}
		break;

		case bytecode_generator::PALO_ELEVEL: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm = adim->lookupElementByName(val_0, false);
			if (!aelm) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = (double)(aelm->getLevel());

		}
		break;

		case bytecode_generator::PALO_ENAME: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			i = (0 <= val_0.getNumeric() ? (uint32_t)(val_0.getNumeric() + 0.5) : (uint32_t)(val_0.getNumeric() - 0.5));
			val_0 = "";
			if (0 < i && i <= adim->sizeElements()) {
				val_0 = adim->getElements(PUser(), false)[i - 1]->getName(adim->getElemNamesVector());
			}

		}
		break;

		case bytecode_generator::PALO_ENEXT: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm = adim->lookupElementByName(val_0, false);
			if (!aelm) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}
			ElementsType aelms = adim->getElements(PUser(), false);
			ElementsType::iterator aelmsi = find(aelms.begin(), aelms.end(), aelm);

			val_0 = "";
			if (aelmsi != aelms.end()) {
				++aelmsi;
				if (aelmsi != aelms.end()) {
					val_0 = (*aelmsi)->getName(adim->getElemNamesVector());
				}
			}

		}
		break;

		case bytecode_generator::PALO_EPARENT: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm = adim->lookupElementByName(val_0, false);
			if (!aelm) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			i = (int)val_0.getNumeric();
			val_0 = "";
			if (0 < i && i <= aelm->getParentsCount()) {
				val_0 = adim->lookupElement(aelm->getParents()->at(i - 1), false)->getName(adim->getElemNamesVector());
			}

		}
		break;

		case bytecode_generator::PALO_EPARENTCOUNT: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm = adim->lookupElementByName(val_0, false);
			if (!aelm) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = (double)(aelm->getParentsCount());
		}
		break;

		case bytecode_generator::PALO_EPREV: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm = adim->lookupElementByName(val_0, false);
			if (!aelm) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}

			ElementsType aelms = adim->getElements(PUser(), false);
			ElementsType::iterator aelmsi = find(aelms.begin(), aelms.end(), aelm);

			val_0 = "";
			if ((aelmsi != aelms.end()) && (aelmsi != aelms.begin())) {
				--aelmsi;
				if (aelmsi >= aelms.begin()) {
					val_0 = (*aelmsi)->getName(adim->getElemNamesVector());
				}
			}

		}
		break;

		case bytecode_generator::PALO_ESIBLING:
		{
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm = adim->lookupElementByName(val_0, false);
			if (!aelm) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			s = (int32_t)val_0.getNumeric();
			val_0 = "";

			// return element name for num == 0
			if (s == 0) {
				val_0 = aelm->getName(adim->getElemNamesVector());
			} else {
				// get parents
				CPParents parentsList = aelm->getParents();
				if (parentsList->size() > 0) {
					// get first parent
					Element* parent = adim->lookupElement(parentsList->at(0), false);
					const IdentifiersWeightType *childrenList = parent->getChildren();
					ssize_t childPos = 0;

					for (IdentifiersWeightType::const_iterator iter = childrenList->begin(); iter != childrenList->end(); childPos++, iter++) {
						if ((*iter).first == aelm->getIdentifier()) {
							break;
						}
					}

					if (childPos < (ssize_t)childrenList->size()) {
						// in case of negative offset, try to find element left of the given element
						if (s < 0) {
							if (childPos + s >= 0) {
								Element *resultElement = adim->lookupElement((*childrenList)[childPos + s].first, false);
								val_0 = resultElement->getName(adim->getElemNamesVector());
							}
						} else {
							// in case of positive offset, try to find element right of the given element
							if (childPos + (ssize_t)s < (ssize_t)childrenList->size()) {
								Element *resultElement = adim->lookupElement((*childrenList)[childPos + s].first, false);
								val_0 = resultElement->getName(adim->getElemNamesVector());
							} else {
								s -= (ssize_t)(childrenList->size() - childPos);

								// try other parents
								bool br = false;
								for (Parents::const_iterator iter = parentsList->begin() + 1; iter != parentsList->end(); ++iter) {
									childrenList = adim->lookupElement(*iter, false)->getChildren();

									for (IdentifiersWeightType::const_iterator c = childrenList->begin(); c != childrenList->end(); ++c) {
										if ((*c).first != aelm->getIdentifier()) {
											if (s == 0) {
												val_0 = adim->lookupElement(c->first, false)->getName(adim->getElemNamesVector());
												br = true;
												break;
											}
											s--;
										}
									}
									if (br) {
										break;
									}
								}
							}
						}
					}
				}
			}
			break;
		}
		case bytecode_generator::PALO_EOFFSET:
		{
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element *aelm = adim->lookupElementByName(val_0, false);
			if (!aelm) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			s = (int32_t)val_0.getNumeric();
			val_0 = "";

			// return element name for num == 0
			if (s == 0) {
				val_0 = aelm->getName(adim->getElemNamesVector());
			} else {
				// get position
				PositionType pos = aelm->getPosition();
				// check ranges
				if ((int32_t)pos + s >= 0 && (int32_t)pos + s < (int32_t)adim->sizeElements()) {
					Element *offsetElement = adim->lookupElementByPosition(pos + s, false);
					if (offsetElement) {
						val_0 = offsetElement->getName(adim->getElemNamesVector());
					}
				}
			}
			break;
		}

		case bytecode_generator::PALO_ETOPLEVEL: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = (double)(adim->getLevel());

		}
		break;

		case bytecode_generator::PALO_ETYPE: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm = adim->lookupElementByName(val_0, false);
			if (!aelm) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = "";
			if (aelm->getElementType() == Element::CONSOLIDATED) {
				val_0 = "consolidated";
			} else if (aelm->getElementType() == Element::NUMERIC) {
				val_0 = "numeric";
			} else if (aelm->getElementType() == Element::STRING) {
				val_0 = "string";
			}

		}
		break;

		case bytecode_generator::PALO_EWEIGHT: {
			CPDatabase adb = CONST_COMMITABLE_CAST(Database, rule->cube->database->shared_from_this());
			if (adb->getName() != val_0) {
				CPServer asrv = context->getServer();
				adb = asrv->lookupDatabaseByName(val_0, false);
			}
			if (!adb) {
				throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			PDimension adim = adb->findDimensionByName(val_0, PUser(), false);
			if (!adim) {
				throw ErrorException(ErrorException::ERROR_INVALID_COORDINATES, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm = adim->lookupElementByName(val_0, false);
			if (!aelm) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = *--sp;

			Element * aelm2 = adim->lookupElementByName(val_0, false);
			if (!aelm2) {
				throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "", rule->nr_rule);
			}
			val_0 = 0.0;
			const IdentifiersWeightType *aelms = aelm->getChildren();
			for (IdentifiersWeightType::const_iterator aelmsi = aelms->begin(); aelmsi != aelms->end(); ++aelmsi) {
				if (aelmsi->first == aelm2->getIdentifier()) {
					val_0 = aelmsi->second;
					break;
				}
			}

		}
		break;
		}
	}
}
}
