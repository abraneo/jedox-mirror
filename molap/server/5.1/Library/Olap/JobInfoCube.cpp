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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Engine/Area.h"
#include "Olap/JobInfoCube.h"
#include "Olap/PaloSession.h"
#include "InputOutput/FileUtils.h"
#include "PaloDispatcher/PaloJob.h"
#include "Thread/WriteLocker.h"
#include "Engine/DFilterProcessor.h"

namespace palo {

class SERVER_CLASS JobInfoProcessor : public ProcessorBase {
public:
	JobInfoProcessor(CPArea filter) : ProcessorBase(true, PEngineBase()), filter(filter), paloJobLock(&PaloJob::m_main_Lock) {}
	virtual ~JobInfoProcessor() {}

	virtual bool next();
	virtual const CellValue &getValue();
	virtual double getDouble();
	virtual void setValue(const CellValue &value);
	virtual const IdentifiersType &getKey() const {return key;}
	virtual const GpuBinPath &getBinKey() const;
	virtual void reset() {}
private:
	bool nextJob();
	CPArea filter;
	CellValue value;
	IdentifiersType key;
	map<IdentifierType, PaloJob *>::iterator jobIt;
	Set::Iterator propertyIt;
	WriteLocker paloJobLock;
};

JobInfoCube::JobInfoCube(PDatabase db, const string& name, const IdentifiersType* dimensions) :
	SystemCube(db, name, dimensions, Cube::JOBS)
{
}

ResultStatus JobInfoCube::setCellValue(PServer server, PDatabase db, PCubeArea cellPath, CellValue value, PLockedCells lockedCells, PUser user, boost::shared_ptr<PaloSession> session, bool checkArea, bool addValue, SplashMode splashMode, bool bWriteToJournal, User::RightSetting* checkRights, set<PCube> &changedCubes, bool possibleCommit, CubeArea::CellType ct)
{
	if (User::checkUser(user) && user->getRoleRight(User::sysOpRight) < RIGHT_DELETE) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights to terminate job!", "user", user ? (int)user->getId() : 0);
	}
	if (value.isString()) {
		ResultStatus result = RESULT_FAILED;
		WriteLocker paloSessionLock(&PaloJob::m_main_Lock);
		IdentifiersType key = *cellPath->pathBegin();
		// find job
		map<IdentifierType, PaloJob *>::iterator jobIt = PaloJob::jobs.find(key[1]);
		if (jobIt != PaloJob::jobs.end()) {
			PaloJob *job = jobIt->second;
			string command = value;
			if (command == string("stop")) {
				job->command = command;
				job->getContext()->stop(true);
			} else {
#ifdef _DEBUG
				job->command = "Invalid command:\""+command+"\"\nCommands: stop - stop this task";
#endif
				Logger::warning << "Invalid job command. User: " << (user ? user->getName() : "System") << " Command: '" << command << "'" << endl;
				throw ParameterException(ErrorException::ERROR_INVALID_COMMAND, "Invalid command received", "value", command);
			}
			Logger::info << "Job command accepted. User: " << (user ? user->getName() : "System") << " Command: '" << command << "'" << endl;
			result = RESULT_OK;
		}
		return result;
	} else {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "Job info cube cannot be modified", "user", user ? (int)user->getId() : 0);
	}
}

PCommitable JobInfoCube::copy() const
{
	checkNotCheckedOut();
	PJobInfoCube newCube(new JobInfoCube(*this));
	return newCube;
}

PProcessorBase JobInfoCube::evaluatePlan(PPlanNode plan, EngineBase::Type engineType, bool sortedOutput) const
{
	PProcessorBase result;
	if (plan->getType() == QUANTIFICATION) {
		result.reset(new DFilterQuantificationProcessor(PEngineBase(), plan));
	} else {
		result.reset(new JobInfoProcessor(plan->getArea()));
	}
	return result;
}

const CellValue &JobInfoProcessor::getValue()
{
	IdentifierType requestedMessagePart = getKey()[2];
//	// never change the order of the elements!
//	switch must be identical to SystemDatabase::JOB_PROPERTIES_ITEMS
//	{"Name",       Element::STRING},
//	{"Duration",   Element::NUMERIC},
//	{"Parameters", Element::STRING},
//	{"Progress", Element::NUMERIC}

	switch (requestedMessagePart) {
	case 0:	// {"Name",       Element::STRING}
		value = jobIt->second->getName();
		break;
	case 1:	// {"Duration",   Element::NUMERIC},
	{
		value = jobIt->second->getDuration();
		break;
	}
	case 2:	// {"Parameters", Element::STRING}
		value = CellValue(jobIt->second->getRequest());
		break;
	case 3: // {"Progress", Element::NUMERIC},
		value = CellValue(true);
		break;
	case 4: // {"Command",	Element::STRING}
		value = jobIt->second->command;
		break;
	default:
		value = CellValue();
		break;
	}
	return value;
}

double JobInfoProcessor::getDouble()
{
	return 0.0;
}

bool JobInfoProcessor::nextJob()
{
	bool result = false;
	if (key.empty()) {
		jobIt = PaloJob::jobs.begin();
		key.resize(3);
	} else {
		++jobIt;
	}
	while (!result && jobIt != PaloJob::jobs.end()) {
		if (jobIt->second->getDuration() < 1.0) {
			// skip job running for less than 1 second
			++jobIt;
			continue;
		}
		if (filter->getDim(1)->find(jobIt->first) == filter->getDim(1)->end()) {
			++jobIt;
			continue;	// job Id not requested
		}
		if (filter->getDim(0)->find(jobIt->second->getSessionInternalId()) == filter->getDim(0)->end()) {
			++jobIt;
			continue;	// session Id not requested
		}
		result = true;
		propertyIt = filter->getDim(2)->begin();
		key[0] = jobIt->second->getSessionInternalId();
		key[1] = jobIt->first;
		key[2] = *propertyIt;
		break;
	}
	return result;
}

bool JobInfoProcessor::next()
{
	bool result = false;
	if (key.empty()) {
		result = nextJob();
	} else {
		++propertyIt;
		if (propertyIt == filter->getDim(2)->end()) {
			result = nextJob();
		} else {
			key[2] = *propertyIt;
			result = true;
		}
	}
	return result;
}

void JobInfoProcessor::setValue(const CellValue &value)
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "JobInfoProcessor::setValue not implemented");
}

const GpuBinPath &JobInfoProcessor::getBinKey() const
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "JobInfoProcessor::getBinKey not implemented");
}

}
