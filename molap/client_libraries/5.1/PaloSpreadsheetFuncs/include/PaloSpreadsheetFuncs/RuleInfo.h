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
 * \author Marek Pikulski <marek.pikulski@jedox.com>
 * \author Jiri Junek <jiri.junek@qbicon.cz>
 * 
 *
 */

#ifndef RULE_INFO_H
#define RULE_INFO_H

#include <vector>

#include <libpalo_ng/Palo/types.h>

namespace Palo {
namespace Types {
/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Stores information about a cube rule.
 */
struct RuleInfo {
	RuleInfo()
	{
	}

	RuleInfo(const jedox::palo::RULE_INFO& ri) :
			identifier(ri.identifier), definition(ri.definition), extern_id(ri.extern_id), comment(ri.comment),
			timestamp(ri.timestamp), activated(ri.activated), position(ri.position)
	{
	}

	long int identifier;
	std::string definition;
	std::string extern_id;
	std::string comment;
	unsigned long long timestamp;
	bool activated;
	double position;
};

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief RuleInfoArray.
 */
class RuleInfoArray : public std::vector<RuleInfo> {
public:
	RuleInfoArray(std::vector<jedox::palo::RULE_INFO> ris)
	{
		for (std::vector<jedox::palo::RULE_INFO>::const_iterator i = ris.begin(); i != ris.end(); i++)
			push_back(RuleInfo(*i));
	}
};
}
}
#endif
