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
#include "Olap/SessionInfoCube.h"
#include "Olap/PaloSession.h"
#include "Olap/Server.h"
#include "PaloDispatcher/PaloJob.h"
#include "InputOutput/FileUtils.h"
#include "Thread/WriteLocker.h"
#include "Engine/DFilterProcessor.h"

namespace palo {

class SERVER_CLASS SessionInfoProcessor : public ProcessorBase {
public:
	SessionInfoProcessor(CPArea filter) : ProcessorBase(true, PEngineBase()), filter(filter), paloSessionLock(&PaloSession::m_main_Lock), context(Context::getContext()) {}
	virtual ~SessionInfoProcessor() {}

	virtual bool next();
	virtual const CellValue &getValue();
	virtual double getDouble();
	virtual void setValue(const CellValue &value);
	virtual const IdentifiersType &getKey() const {return key;}
	virtual const GpuBinPath &getBinKey() const;
	virtual void reset() {}
private:
	bool nextSession();
	CPArea filter;
	CellValue value;
	IdentifiersType key;
	map<IdentifierType, boost::shared_ptr<PaloSession> >::iterator sessionIt;
	Set::Iterator propertyIt;
	WriteLocker paloSessionLock;
	Context *context;
};

SessionInfoCube::SessionInfoCube(PDatabase db, const string& name, const IdentifiersType* dimensions) :
	SystemCube(db, name, dimensions, Cube::SESSIONS)
{
}

ResultStatus SessionInfoCube::setCellValue(PServer server, PDatabase db, PCubeArea cellPath, CellValue value, PLockedCells lockedCells, PUser user, boost::shared_ptr<PaloSession> session, bool checkArea, bool addValue, SplashMode splashMode, bool bWriteToJournal, User::RightSetting* checkRights, set<PCube> &changedCubes, bool possibleCommit, CubeArea::CellType ct)
{
	if (User::checkUser(user) && user->getRoleRight(User::sysOpRight) < RIGHT_DELETE) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights to terminate session!", "user", user ? (int)user->getId() : 0);
	}
	if (value.isString()) {
		ResultStatus result = RESULT_FAILED;
		WriteLocker paloSessionLock(&PaloSession::m_main_Lock);
		IdentifiersType key = *cellPath->pathBegin();
		// find session
		map<IdentifierType, boost::shared_ptr<PaloSession> >::iterator sessionIt = PaloSession::sessionIds.find(key[0]);
		if (sessionIt != PaloSession::sessionIds.end()) {
			string command = value;
			if (command == string("close")) {
				boost::shared_ptr<PaloSession> session = sessionIt->second;
				if (session->isWorker()) {
					session->command = "Invalid command:\""+command+"\"\nSVS session can't be closed";
					throw ErrorException(ErrorException::ERROR_NOT_AUTHORIZED, "SVS session can't be closed");
				}
				session->command = command;
				// terminate all session jobs
				WriteLocker wl(&sessionIt->second->thisLock);
				for (set<const PaloJob *>::iterator jit = sessionIt->second->activeJobs.begin(); jit != sessionIt->second->activeJobs.end(); ++jit) {
					if (*jit) {
						Context *jobContext = const_cast<Context *>((*jit)->getContext());
						if (jobContext) {
							jobContext->stop(true);
						}
					}
				}
				PaloSession::deleteSession(session, true);
			} else {
#ifdef _DEBUG
				session->command = "Invalid command:\""+command+"\"\nCommands: close - close this session";
#endif
				Logger::warning << "Invalid session command. User: " << (user ? user->getName() : "System") << " Command: '" << command << "'" << endl;
				throw ParameterException(ErrorException::ERROR_INVALID_COMMAND, "Invalid command received", "value", command);
			}
			Logger::info << "Session command accepted. User: " << (user ? user->getName() : "System") << " Command: '" << command << "'" << endl;
			result = RESULT_OK;
		} else {
			throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "session element not found", "element", key[0]);
		}
		return result;
	} else {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "session info cube cannot be modified", "user", user ? (int)user->getId() : 0);
	}
}

PCommitable SessionInfoCube::copy() const
{
	checkNotCheckedOut();
	PSessionInfoCube newCube(new SessionInfoCube(*this));
	return newCube;
}

PProcessorBase SessionInfoCube::evaluatePlan(PPlanNode plan, EngineBase::Type engineType, bool sortedOutput) const
{
	PProcessorBase result;
	if (plan->getType() == QUANTIFICATION) {
		result.reset(new DFilterQuantificationProcessor(PEngineBase(), plan));
	} else {
		result.reset(new SessionInfoProcessor(plan->getArea()));
	}
	return result;
}

const CellValue &SessionInfoProcessor::getValue()
{
	IdentifierType requestedMessagePart = getKey()[1];
	// switch must be identical to SystemDatabase::SESSION_PROPERTIES_ITEMS
//	{"User",	Element::STRING},
//	{"Jobs",	Element::NUMERIC},
//	{"Login Time",	Element::STRING},
//	{"Expiration Time", Element::STRING},
//	{"Time",	Element::NUMERIC},
//	{"Active Jobs", Element::STRING}
//	{"License",	Element::NUMERIC},
//	{"Address",	Element::STRING},
//	{"Command",	Element::STRING},
//	{"Description",	Element::STRING},
//	{"MachineId", Element::STRING}
	switch (requestedMessagePart) {
	case 0:	// 	{"User",     Element::STRING},
	{
		try {
			PUser user = sessionIt->second->getUser();
			if (user) {
				value = user->getName();
			} else if (sessionIt->second->isWorker()) {
				value = "<SupervisionServer>";
			} else {
				value = "<FAKE>";
			}
		} catch (ParameterException&) {
			value = "<UNKNOWN USER>"; // deleted user
		}
		break;
	}
	case 1:	// {"Jobs",	Element::NUMERIC},
		value = double(sessionIt->second->requestCounter);
		break;
	case 2:	// {"Login Time", Element::STRING},
		value = StringUtils::convertTimeToString(sessionIt->second->getLoginTime());
		break;
	case 3:	// {"Expiration Time", Element::STRING},
	{
		time_t ttl = sessionIt->second->getTtl();
		value = StringUtils::convertTimeToString(ttl);
		break;
	}
	case 4:	// {"Time", Element::NUMERIC},
		{
			double totalTime = sessionIt->second->totalTime;
			WriteLocker wl(&sessionIt->second->thisLock);
			for (set<const PaloJob *>::iterator jit = sessionIt->second->activeJobs.begin(); jit != sessionIt->second->activeJobs.end(); ++jit) {
				if (*jit) {
					totalTime += (*jit)->getDuration();
				}
			}
			value = totalTime;
		}
		break;
	case 5:	// {"Active Jobs", Element::STRING}
	{
		WriteLocker wl(&sessionIt->second->thisLock);
		string jobsNames;
		Context *context = Context::getContext();
		for (set<const PaloJob *>::iterator jit = sessionIt->second->activeJobs.begin(); jit != sessionIt->second->activeJobs.end(); ++jit) {
			if (*jit && (*jit)->getContext() != context) {
				if (!jobsNames.empty()) {
					jobsNames += ", ";
				}
				jobsNames += StringUtils::convertToString((*jit)->getId());
			}
		}
		value = jobsNames;
		break;
	}
	case 6:	// {"License",  Element::STRING},
		value = Context::getContext()->getServer()->getSessionLicense(sessionIt->second);
		break;
	case 7:	// {"Address",  Element::STRING},
		value = sessionIt->second->peerName;
		break;
	case 8:	// {"Command",	Element::STRING}
		value = sessionIt->second->command;
		break;
	case 9:	// {"Description",	Element::STRING}
		value = sessionIt->second->description;
		break;
	case 10:	// {"MachineId", Element::STRING}
		value = sessionIt->second->machineId;
		break;
	case 11:	// {"CurrentSession", Element::NUMERIC}
		value = CellValue();
		if (context && context->getSession() == sessionIt->second) {
			value = 1.0;
		}
		break;
	default:
		value = CellValue();
		break;
	}
	return value;
}

double SessionInfoProcessor::getDouble()
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "SessionInfoProcessor::getDouble not implemented");
}

bool SessionInfoProcessor::nextSession()
{
	bool result = false;
	if (key.empty()) {
		sessionIt = PaloSession::sessionIds.begin();
		key.resize(2);
	} else {
		++sessionIt;
	}
	while (!result && sessionIt != PaloSession::sessionIds.end()) {
		if (filter->getDim(0)->find(sessionIt->first) != filter->getDim(0)->end()) {
			result = true;
			propertyIt = filter->getDim(1)->begin();
			key[0] = sessionIt->first;
			key[1] = *propertyIt;
			break;
		}
		++sessionIt;
	}
	return result;
}

bool SessionInfoProcessor::next()
{
	bool result = false;
	if (key.empty()) {
		result = nextSession();
	} else {
		++propertyIt;
		if (propertyIt == filter->getDim(1)->end()) {
			result = nextSession();
		} else {
			key[1] = *propertyIt;
			result = true;
		}
	}
	return result;
}

void SessionInfoProcessor::setValue(const CellValue &value)
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "SessionInfoProcessor::setValue not implemented");
}

const GpuBinPath &SessionInfoProcessor::getBinKey() const
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "SessionInfoProcessor::getBinKey not implemented");
}

}
