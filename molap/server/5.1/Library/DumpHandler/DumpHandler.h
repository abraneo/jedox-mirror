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
 * \author Florian Schaper, Jedox AG, Freiburg, Germany
 * \author Frieder Hofmann, Jedox AG, Freiburg, Germany
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef DUMP_HANDLER_H
#define DUMP_HANDLER_H 1

#include "palo.h"
#include "Thread/WriteLocker.h"

namespace palo {

class SERVER_CLASS DumpHandler
{
public:
    static void register_handler(string paloIniPath);
    static void enable_crash_reporting(bool enable);
    static void disarm();

    static bool m_enable;
    static bool m_initialized;
	static Mutex m_lock;
	static string m_palo_ini;
};

}

#endif
