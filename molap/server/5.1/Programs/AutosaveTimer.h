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
 * \author Radu Ialovoi, Yalos Solutions, Bucharest, Romania
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef PROGRAMS_AUTOSAVE_TIMER_H
#define PROGRAMS_AUTOSAVE_TIMER_H 1

#include <boost/thread/thread.hpp>
#include <boost/thread/thread_time.hpp>

#include "AutosaveType.h"
#include "PaloHttpInterface.h"

namespace palo {

class AutosaveTimer {
private:
	class SaveWorker : private autosave_type {
	private:
		typedef boost::posix_time::ptime system_time;

		system_time nextSave;
		system_time nextSessionClear;
		system_time nextRequestRecord;

		bool* active;

		PaloHttpInterface* iface;

		void nextAutoSaveTime() {
			system_time now = boost::posix_time::second_clock::local_time();

			switch (mode) {
			case AS_LOOP:
				nextSave = now + boost::posix_time::hours(hour) + boost::posix_time::minutes(minute);
				break;
			case AS_EXACT_TIME:
				nextSave = system_time(now.date(), boost::posix_time::hours(hour) + boost::posix_time::minutes(minute));
				if (nextSave < now) {
					nextSave = nextSave + boost::gregorian::days(1);
				}
				break;
			default:
				break;//do nothing
			}
		}

		void nextSessionClearTime() {
			system_time now = boost::posix_time::second_clock::local_time();
			nextSessionClear = now + boost::posix_time::hours(0) + boost::posix_time::minutes(1);
		}

		void nextRequestRecordTime() {
			system_time now = boost::posix_time::second_clock::local_time();
			nextRequestRecord = now + boost::posix_time::hours(0) + boost::posix_time::minutes(0) + boost::posix_time::seconds(30);
		}

		void doAutoSave() {
			Logger::info << "autosaving: begin" << std::endl;
			iface->commitAndSave();
			Logger::info << "autosaving: end" << std::endl;
		}

		void doSessionClear() {
			iface->clearOldSessions();
		}

		void doRequestRecord() {
			iface->requestRecord();
		}

	public:
		void operator()() {
			nextAutoSaveTime();
			nextSessionClearTime();
			nextRequestRecordTime();

			while (*active) {
				boost::this_thread::sleep(boost::posix_time::seconds(1));
				if (mode != autosave_type::AS_DISABLE && nextSave < boost::posix_time::second_clock::local_time()) {
					doAutoSave();
					nextAutoSaveTime();
				}
				if (nextSessionClear < boost::posix_time::second_clock::local_time()) {
					doSessionClear();
					nextSessionClearTime();
				}
				if (/*requestRecordFile != "" && */nextRequestRecord < boost::posix_time::second_clock::local_time()) {
					doRequestRecord();
					nextRequestRecordTime();
				}
			}
		}

		SaveWorker(const autosave_type& params, PaloHttpInterface* iface, bool* active) :
			autosave_type(params), active(active), iface(iface) {
		}
	};

	boost::thread* timer;
	bool active;

public:

	AutosaveTimer(const autosave_type& params, PaloHttpInterface* iface) {
		active = true;
		timer = new boost::thread(SaveWorker(params, iface, &active));
	}

	~AutosaveTimer() {
		stop();
	}

	void stop() {
		if (NULL == timer)
			return;
		active = false;
		timer->join();
		delete timer;
		timer = NULL;
	}
};

}
;

#endif
