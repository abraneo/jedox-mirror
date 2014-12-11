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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef __TEXTFILTER_H_INCL__
#define __TEXTFILTER_H_INCL__

#include <list>

#include <boost/regex.hpp>
#include <boost/scoped_ptr.hpp>

#include "Filter.h"
#include "PaloDispatcher/PaloJobRequest.h"

namespace palo {

class TextFilter : public TextFilterBase, public Filter {

public:
	enum ColumnType {
		BASIC = 0, // use only the basic dimension for searching the regex (default)
		ALIAS = 1, // use aliases as defined by the alias filter for searching the regex
		// use the attribute columns for searching -- ADDITIONAL_FIELDS must be != 0
		ATTRIBUTE = 2
	};

	TextFilter(SubSet &s, TextFilterSettings &settings, ColumnType type, const string &attribute);
	ElementsType apply();
private:
	typedef std::vector<bool> match_vec_type;

	class internal_regex {
	public:
		ColumnType type;
		long attr_id;
		boost::regex rex;

		internal_regex(std::string descr, ColumnType t, const long attr_id) :
			type(t), attr_id(attr_id), rex(boost::regex(descr))
		{
		}
	};

	void findRegexInAttribute(const boost::regex& rx, const long attr_id);
	void findRegexInAlias(const boost::regex& rx);
	void findRegexInBasic(const boost::regex& rx);
	void validateRegex(const std::string& s) const;
	void prepareRegex(string& rex);
	void applySettings();

	TextFilterSettings &m_settings;
	list<internal_regex> regular_expressions;
	match_vec_type m_matches;
	ColumnType m_type;
	string m_attr;
};

} //palo
#endif
