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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef PALO_JOBS_CELL_COPY_JOB_H
#define PALO_JOBS_CELL_COPY_JOB_H 1

#include "palo.h"

#include "Exceptions/ParameterException.h"
#include "Olap/Dimension.h"
#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief cell copy
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS CellCopyJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new CellCopyJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	CellCopyJob(PaloJobRequest* jobRequest) :
		DirectPaloJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets job type
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return WRITE_JOB;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start working
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		bool ret = false;
		for (int commitTry = 0; commitTry < Commitable::COMMIT_REPEATS_CELL_REPLACE; commitTry++) {
			Context *context = Context::getContext();

			bool optimistic = commitTry == 0;
			server = context->getServerCopy();
			findDatabase(true, true);
			findCube(true, true);
			findLockedPaths(0);
			if (!jobRequest->function) {
				findPath();
			}
			findPathTo();

			cube->disableTokenUpdate();

			User::RightSetting rs;
			if (User::checkUser(user)) {
				if (user->getRoleRight(User::cellDataRight) < RIGHT_SPLASH) {
					throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)user->getId());
				}
				rs.checkCells = User::checkCellDataRightCube(database, cube);
			}

			double dValue = 0;
			double *ptrValue = &dValue;
			if (jobRequest->function) {
				IdentifierType timeDim = checkArea();
				PCubeArea calcArea(new CubeArea(database, cube, *jobRequest->area));
				cube->checkAreaAccessRight(database, user, calcArea, rs, false, RIGHT_READ, 0);
				dValue = calculatePredictFunction(calcArea, timeDim);
			} else {
				cube->checkAreaAccessRight(database, user, cellPath, rs, false, RIGHT_READ, 0);
				if (jobRequest->value) {
					dValue = StringUtils::stringToDouble(*(jobRequest->value));
				} else {
					ptrValue = NULL;
				}
			}

			if (ptrValue) {
				cube->checkAreaAccessRight(database, user, cellPathTo, rs, dValue == 0.0, RIGHT_SPLASH, 0);
			}

			bool unused = false; //NULL is also an unused value
			PCellValueContext cvc = PCellValueContext(new CellValueContext(cube, CellValueContext::CELL_COPY_JOB, database, user, boost::shared_ptr<PaloSession>(), unused, unused, (int)unused, lockedPaths, cellPathTo, jobRequest->useRules, ptrValue));
			context->setCellValueContext(cvc);
			cvc->addPathAndValue(cellPath, CellValue(), unused, CubeArea::NONE);

			if (!optimistic) {
				context->setPesimistic();
			}

			updateLics = true;
			ret = server->commit();

			if (ret) {
				updateLics = false;
				break;
			}
			clear();
		}
		if (!ret) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "CellCopyJob failed. Internal error occured.");
		}

		server->invalidateCache();

		generateOkResponse(cube);
	}

private:
	static const uint32_t LINEAR_REGRESSION = 1;

	IdentifierType checkArea()
	{
		if (jobRequest->function != LINEAR_REGRESSION) {
			throw ParameterException(ErrorException::ERROR_INVALID_PREDICT_FUNCTION, "invalid predictive function", PaloRequestHandler::FUNCTION, "");
		}

		const IdentifiersType *dimensions = cube->getDimensions();
		size_t size = dimensions->size();
		if (size != jobRequest->area->size()) {
			throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of area elements", PaloRequestHandler::ID_AREA, "");
		}

		if (!jobRequest->path) {
			jobRequest->path = new IdentifiersType();
		}
		jobRequest->path->resize(size);

		IdentifierType timeDim = NO_IDENTIFIER;
		size_t timeDimCount = 0;
		for (size_t i = 0; i < size; i++) {
			const IdentifiersType &elems = jobRequest->area->at(i);
			if (elems.size()) {
				if (elems.size() > 1) {
					timeDimCount++;
					if (timeDimCount > 1) {
						throw ParameterException(ErrorException::ERROR_INVALID_PREDICT_AREA, "multiple time dimension", PaloRequestHandler::ID_AREA, "");
					}
					timeDim = (IdentifierType)i;
				}

				CPDimension dim = database->lookupDimension((*dimensions)[i], false);
				for (size_t j = 0; j < elems.size(); j++) {
					dim->findElement(elems[j], 0, false);
				}

				jobRequest->path->at(i) = elems[elems.size() - 1];
			} else {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "area is empty", PaloRequestHandler::ID_AREA, "");
			}
		}
		if (timeDim == NO_IDENTIFIER) {
			throw ParameterException(ErrorException::ERROR_INVALID_PREDICT_AREA, "missing time dimension", PaloRequestHandler::ID_AREA, "");
		}
		findPath();
		return timeDim;
	}

	double calculatePredictFunction(PCubeArea calcArea, IdentifierType timeDim)
	{
		double result = 0.0;
		if (calcArea->getSize()) {
			size_t size = calcArea->elemCount(timeDim);
			vector<double> X(size);
			vector<double> Y(size);

			CPDimension dim = database->findDimension(cube->getDimensions()->at(timeDim), PUser(), false);
			Area::ConstElemIter eit = cellPathTo->elemBegin(timeDim);
			Element *elem = dim->findElement(*eit, 0, false);
			double xPos = (double)elem->getPosition();

			size_t i = 0;
			eit = calcArea->elemBegin(timeDim);

			PCellStream cs = cube->calculateArea(calcArea, CubeArea::NUMERIC, jobRequest->useRules ? RulesType(ALL_RULES | NO_RULE_IDS) : NO_RULES, false, UNLIMITED_SORTED_PLAN);
			while (cs->next()) {
				elem = dim->findElement(*eit, 0, false);
				X[i] = (double)elem->getPosition();
				Y[i] = cs->getDouble();
				++eit;
				i++;
			}

			result = linearRegression(X, Y, xPos);
		}
		return result;
	}

	//returns y = ax + b, where a and b are calculated from X and Y
	double linearRegression(vector<double> &X, vector<double> &Y, double x)
	{
		double avgX = average(X);
		double avgY = average(Y);

		double bNumerator = 0;
		for (size_t i = 0; i < X.size(); i++) {
			bNumerator += (X[i] - avgX) * (Y[i] - avgY);
		}
		double bDenominator = 0;
		for (size_t i = 0; i < X.size(); i++) {
			bDenominator += (X[i] - avgX) * (X[i] - avgX);
		}

		double b = bDenominator == 0 ? 0 : bNumerator / bDenominator;
		double a = avgY - b * avgX;

		double y = a + b * x;
		return y;
	}

	double average(vector<double> &v)
	{
		double result = 0;
		if (v.size()) {
			for (size_t i = 0; i < v.size(); i++) {
				result += v[i];
			}
			result /= v.size();
		}
		return result;
	}

};

}

#endif
