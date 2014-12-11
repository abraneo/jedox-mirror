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
 * \author Christoffer Anselm, Jedox AG, Freiburg, Germany
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "Exceptions/WorkerException.h"

namespace palo {

WorkerException::WorkerException(const string& msg, bool typeMessageFromSVS) :
	ErrorException(ERROR_WORKER_MESSAGE)
{
	if (typeMessageFromSVS) { // "xxx";"yyy"
		size_t i = msg.find(';');
		if (i != string::npos && i >= 2 && (msg.size() - i >= 3)) {
			message = msg.substr(1, i - 2);
			details = msg.substr(i + 2, msg.size() - i - 3);
			return;
		}
	}

	// else
	message = getDescriptionErrorType(ERROR_WORKER_MESSAGE);
	details = msg;
}

}
