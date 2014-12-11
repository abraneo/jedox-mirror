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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Engine/Area.h"
#include "Olap/LicenseInfoCube.h"
#include "Olap/Server.h"
#include "Engine/DFilterProcessor.h"

namespace palo {

const CellValue &LicenseInfoProcessor::getValue()
{
	IdentifierType requestedMessagePart = getKey()[1];
	switch (requestedMessagePart) {
	case 0: // {"Code",    		Element::STRING}
		value = licIter->getKey();
		break;
	case 1: // {"Customer",     Element::STRING}
		value = licIter->getCustomer();
		break;
	case 2: // {"Features",    	Element::STRING}
		value = licIter->getFeatures();
		break;
	case 3: // {"Concurrent Seats",Element::NUMERIC}
		value = licIter->getLicenseCount();
		break;
	case 4: // {"Named Seats",		Element::NUMERIC}
		value = licIter->getNamedCount();
		break;
	case 5: // {"Sessions",	Element::STRING}
		value = licIter->getConcurrentSessions();
		break;
	case 6: // {"GPU Cards",		Element::NUMERIC}
		value = licIter->getGPUCount();
		break;
	case 7: // {"Free Concurrent",	Element::NUMERIC}
		value = licIter->getFreeConcurrent();
		break;
	case 8: // {"Free Named",		Element::NUMERIC}
		value = licIter->getFreeNamed();
		break;
	case 9: // {"Activation Time", Element::STRING}
		value = StringUtils::convertTimeToString(licIter->getActivationTime());
		break;
	case 10: // {"Expiration Time", Element::STRING}
		value = StringUtils::convertTimeToString(licIter->getExpirationTime());
		break;
	default:
		value = "";
		break;
	}
	return value;
}

double LicenseInfoProcessor::getDouble()
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "LicenseInfoProcessor::getDouble not implemented");
}

bool LicenseInfoProcessor::nextLicense()
{
	bool result = false;
	if (key.empty()) {
		i = 0;
		key.resize(2);
	} else {
		++i;
	}
	licCont = licIter->next();
	while (!result && licCont) {
		if (filter->getDim(0)->find((IdentifierType)i) != filter->getDim(0)->end()) {
			result = true;
			propertyIt = filter->getDim(1)->begin();
			key[0] = (unsigned int)i;
			key[1] = *propertyIt;
			break;
		}
		licCont = licIter->next();
		++i;
	}
	return result;
}

bool LicenseInfoProcessor::next()
{
	bool result = false;
	if (key.empty()) {
		result = nextLicense();
	} else {
		++propertyIt;
		if (propertyIt == filter->getDim(1)->end()) {
			result = nextLicense();
		} else {
			key[1] = *propertyIt;
			result = true;
		}
	}
	return result;
}

void LicenseInfoProcessor::setValue(const CellValue &value)
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "LicenseInfoProcessor::setValue not implemented");
}

const GpuBinPath &LicenseInfoProcessor::getBinKey() const
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "LicenseInfoProcessor::getBinKey not implemented");
}

LicenseInfoCube::LicenseInfoCube(PDatabase db, const string& name, const IdentifiersType* dimensions) :
	SystemCube(db, name, dimensions, Cube::LICENSES)
{
}

ResultStatus LicenseInfoCube::setCellValue(PServer server, PDatabase db, PCubeArea cellPath, CellValue value, PLockedCells lockedCells, PUser user, boost::shared_ptr<PaloSession> session, bool checkArea, bool addValue, SplashMode splashMode, bool bWriteToJournal, User::RightSetting* checkRights, set<PCube> &changedCubes, bool possibleCommit, CubeArea::CellType ct)
{
	throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "license info cube cannot be modified", "user", user ? (int)user->getId() : 0);
}

PCommitable LicenseInfoCube::copy() const
{
	checkNotCheckedOut();
	boost::shared_ptr<LicenseInfoCube> newCube(new LicenseInfoCube(*this));
	return newCube;
}

PProcessorBase LicenseInfoCube::evaluatePlan(PPlanNode plan, EngineBase::Type engineType, bool sortedOutput) const
{
	PProcessorBase result;
	if (plan->getType() == QUANTIFICATION) {
		result.reset(new DFilterQuantificationProcessor(PEngineBase(), plan));
	} else {
		result.reset(new LicenseInfoProcessor(plan->getArea(), Context::getContext()->getServer()->getLicenseIterator(false)));
	}
	return result;
}

}
