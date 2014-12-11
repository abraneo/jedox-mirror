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
 * 
 *
 */

#ifndef PALO_BROWSER_CUBE_BROWSER_JOB_H
#define PALO_BROWSER_CUBE_BROWSER_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/PaloBrowserJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief cube browser
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS CubeBrowserJob : public PaloBrowserJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new CubeBrowserJob(jobRequest);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	CubeBrowserJob(PaloJobRequest* jobRequest) : PaloBrowserJob(jobRequest), jobType(READ_JOB), pdims(0) {}

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return jobType;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	bool initialize();

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	void compute();

private:
	virtual void appendValue(const IdentifiersType &key, const CellValue &value, const vector<CellValue> &prop_vals);
	void fillCubeRights(vector<RightsType>& cubeRights) const;

	string action;
	JobType jobType;

	vector<string> identifiers;
	vector<string> elemNames;
	vector<string> type;
	vector<string> rule;
	vector<string> value;

	vector<CPDimension> *pdims;
};

}

#endif
