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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef WORKER_DIMENSION_WORKER_H
#define WORKER_DIMENSION_WORKER_H 1

#include "palo.h"

#include "Worker/Worker.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief login worker
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS DimensionWorker : public Worker {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	DimensionWorker(const string& session) :
		Worker(session), shutdownInProgress(false) {
	}

	virtual ~DimensionWorker();

public:

	typedef map<IdentifierType, long> MapDimension2FunctionId;
	typedef map<IdentifierType, MapDimension2FunctionId> MapDatabase2WatchDef;

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	bool start();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief informs worker about server shutdown
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus notifyShutdown();

	bool triggerDestroyElement(IdentifierType database, IdentifierType dimension, long &function);
	bool triggerRenameElement(IdentifierType database, IdentifierType dimension, long &function);
	bool triggerCreateElement(IdentifierType database, IdentifierType dimension, long &function);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief defines dimensions to watch for element related events inside worker
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus getDimensionsToWatch();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief informs worker about element removal
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus notifyElementDestroyed(IdentifierType database, IdentifierType dimension, const string &elementName, long function, string session);
	ResultStatus notifyElementRenamed(IdentifierType database, IdentifierType dimension, const string &oldName, const string &newName, long function, string session);
	ResultStatus notifyElementCreated(IdentifierType database, IdentifierType dimension, IdentifierType element, long function, string session);


private:

	ResultStatus getDimensionsToWatch(MapDatabase2WatchDef &mDims, string command);
	ResultStatus readDimensionLines(const vector<string> &result, MapDatabase2WatchDef &mDims);
	bool triggerElement(IdentifierType database, IdentifierType dimension, long &function, const MapDatabase2WatchDef &v);

	bool shutdownInProgress;
	string message;

	MapDatabase2WatchDef mapElementDestroy;
	MapDatabase2WatchDef mapElementRename;
	MapDatabase2WatchDef mapElementCreate;
};

}
#endif
