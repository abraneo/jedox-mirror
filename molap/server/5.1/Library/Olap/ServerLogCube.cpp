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
#include "Olap/ServerLogCube.h"
#include "InputOutput/FileUtils.h"
#include "Engine/DFilterProcessor.h"

namespace palo {

class SERVER_CLASS ServerLogProcessor : public ProcessorBase {
public:
	ServerLogProcessor(CPArea filter) : ProcessorBase(true, PEngineBase()), filter(filter), paloLog(0), counter(NO_IDENTIFIER), pathIt(filter->pathBegin()) {}
	virtual ~ServerLogProcessor() {delete paloLog;}

	virtual bool next();
	virtual const CellValue &getValue();
	virtual double getDouble();
	virtual void setValue(const CellValue &value);
	virtual const IdentifiersType &getKey() const {return *pathIt;}
	virtual const GpuBinPath &getBinKey() const;
	virtual void reset() {delete paloLog; paloLog = 0; counter = NO_IDENTIFIER; pathIt = filter->pathBegin();}
private:
	bool nextLine();
	CPArea filter;
	CellValue value;
	istream *paloLog;
	IdentifierType counter;
	string nextLineStart;
	string dateText;
	string timeText;
	string levelText; // "INFO: ", "DEBUG: ", "WARNING: ", "TRACE: ", "ERROR: "
	string messageText;
	Area::PathIterator pathIt;
};

ServerLogCube::ServerLogCube(PDatabase db, const string& name, const IdentifiersType* dimensions) :
	SystemCube(db, name, dimensions, Cube::LOG)
{
	setDeletable(false);
	setRenamable(false);
}

ResultStatus ServerLogCube::setCellValue(PServer server, PDatabase db, PCubeArea cellPath, CellValue value, PLockedCells lockedCells, PUser user, boost::shared_ptr<PaloSession> session, bool checkArea, bool addValue, SplashMode splashMode, bool bWriteToJournal, User::RightSetting* checkRights, set<PCube> &changedCubes, bool possibleCommit, CubeArea::CellType ct)
{
	throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "server log cube cannot be modified", "user", (int)user->getId());
}

PCommitable ServerLogCube::copy() const
{
	checkNotCheckedOut();
	PCommitable newCube(new ServerLogCube(*this));
	return newCube;
}

PProcessorBase ServerLogCube::evaluatePlan(PPlanNode plan, EngineBase::Type engineType, bool sortedOutput) const
{
	PProcessorBase result;
	if (plan->getType() == QUANTIFICATION) {
		result.reset(new DFilterQuantificationProcessor(PEngineBase(), plan));
	} else {
		result.reset(new ServerLogProcessor(plan->getArea()));
	}
	return result;
}

const CellValue &ServerLogProcessor::getValue()
{
	IdentifierType requestedMessagePart = getKey()[1];
	switch (requestedMessagePart) {
	case 0:	// date
		value = dateText;
		break;
	case 1:	// time
		value = timeText;
		break;
	case 2:	// level
		value = levelText;
		break;
	case 3:	// message
		value = messageText;
		break;
	default:
		value = "";
		break;
	}
	return value;
}

double ServerLogProcessor::getDouble()
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "ServerLogProcessor::getDouble not implemented");
}

bool ServerLogProcessor::next()
{
	bool result = false;
	if (!paloLog) {
		string fileName = Logger::getLogFile();
		paloLog = new FileUtils::paloifstream(fileName.c_str());
		if (paloLog) {
			paloLog->seekg(Logger::getZeroPos(), ios::beg);
		}
	} else {
		IdentifierType oldLine = getKey()[0];
		++pathIt;
		if (pathIt == filter->pathEnd()) {
			return false;
		}
		IdentifierType requestedLine = getKey()[0];
		if (oldLine != requestedLine) {
			dateText.clear();
			timeText.clear();
			levelText.clear();
			messageText.clear();
		} else if (counter == requestedLine) {
			return true;
		}
	}
	if (paloLog && pathIt != filter->pathEnd()) {
		string completeLine = nextLineStart;
		nextLineStart.clear();
		// read while line is not complete (dateText is not empty)
		while (!result && paloLog->good()) {
			string line;
			getline ( *paloLog, line );
			if (line.size() > 26) {	// date(11), time(9), and shortest level(6)
				if (line[4] == '-' && line[7] == '-' && line[10] == ' ' && line[13] == ':' && line[16] == ':' && line[19] == ' ') {
					size_t found = line.find(": ", 20);
					if (found != string::npos) {
						// start of new line => close the old one and remember this
						nextLineStart = line;
					}
				}
			}
			if (nextLineStart.empty()) {
				completeLine += "\n"+line;
			} else if (!completeLine.empty()) {
				// completeLine read
				result = nextLine();
				if (result) {
					dateText = completeLine.substr(0, 10);
					timeText = completeLine.substr(11, 8);
					size_t found = completeLine.find(": ", 20);
					levelText = completeLine.substr(20, found-20);
					messageText = completeLine.substr(found+2);
				} else {
					completeLine = line;
				}
			} else {
				completeLine = nextLineStart;
			}
		}
		if (!result && !completeLine.empty()) {
			result = nextLine();
			if (result) {
				dateText = completeLine.substr(0, 10);
				timeText = completeLine.substr(11, 8);
				size_t found = completeLine.find(": ", 20);
				levelText = completeLine.substr(20, found-20);
				messageText = completeLine.substr(found+2);
			}
		}
	}
//	if (result) {
//		IdentifierType requestedLine = getKey()[0];
//		cout << requestedLine << endl;
//	}
	return result;
}

void ServerLogProcessor::setValue(const CellValue &value)
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "ServerLogProcessor::setValue not implemented");
}

bool ServerLogProcessor::nextLine()
{
	counter++;
	IdentifierType requestedLine = getKey()[0];
	if (counter == requestedLine) {
		return true;
	} else {
		return false;
	}
}

const GpuBinPath &ServerLogProcessor::getBinKey() const
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "ServerLogProcessor::getBinKey not implemented");
}

}
