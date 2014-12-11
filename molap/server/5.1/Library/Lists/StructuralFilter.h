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
 * \author Frieder Hofmann , Jedox AG, Freiburg, Germany
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef __STRUCTURALFILTER_H_INCL__
#define __STRUCTURALFILTER_H_INCL__

#include <string>
# include <boost/scoped_ptr.hpp>

#include "Filter.h"
#include "PaloDispatcher/PaloJobRequest.h"
#include "SubSet.h"

namespace palo {

class StructuralFilter : public StructuralFilterBase, public Filter {
public:
	class InRange;

	StructuralFilter(SubSet&, StructuralFilterSettings &settings);

	vector<SubElem> apply(bool &worked);
	Element *getElemBound() const {return m_bound;}

	unsigned int m_level_begin, m_level_end;
	unsigned int m_indent;
private:
	class InLevelRange;
	class InDepthRange;
	class InIndentRange;
	class CheckIsNotParentLevel;
	class CheckIsNotChildLevel;
	class CheckIsNotLevel;

	Element *m_bound;
	Element *m_revolve;

	unsigned int m_revolve_count;
	StructuralFilterSettings &m_settings;

	vector<SubElem> CheckNotInRange(SubSet::Iterator &beg);
	vector<SubElem> CheckNotInLevelRange(SubSet::Iterator &beg);
	vector<SubElem> CheckNotInDepthRange(SubSet::Iterator &beg);
	vector<SubElem> CheckNotInIndentRange(SubSet::Iterator &beg);

	void applySettings();
};

class StructuralFilter::InRange {
public:

	InRange(unsigned int lower_bound, unsigned int upper_bound) :
		m_upper(upper_bound), m_lower(lower_bound)
	{
	}

	virtual const bool do_check(IdentifierType level, IdentifierType indent, IdentifierType depth) const
	{
		return true;
	}

protected:
	unsigned int m_upper;
	unsigned int m_lower;
};

class StructuralFilter::InLevelRange : public StructuralFilter::InRange {
public:

	InLevelRange(unsigned int lower_bound, unsigned int upper_bound) :
		InRange(lower_bound, upper_bound)
	{
	}

	const bool do_check(IdentifierType level, IdentifierType indent, IdentifierType depth) const
	{
		if ((level < m_lower) || (level > m_upper)) {
			return false;
		}
		return true;
	}
};

class StructuralFilter::InIndentRange : public StructuralFilter::InRange {
public:

	InIndentRange(unsigned int lower_bound, unsigned int upper_bound) :
		InRange(lower_bound, upper_bound)
	{
	}

	const bool do_check(IdentifierType level, IdentifierType indent, IdentifierType depth) const
	{
		if ((indent < m_lower) || (indent > m_upper)) {
			return false;
		}
		return true;
	}
};

class StructuralFilter::InDepthRange : public StructuralFilter::InRange {
public:

	InDepthRange(unsigned int lower_bound, unsigned int upper_bound) :
		InRange(lower_bound, upper_bound)
	{
	}

	const bool do_check(IdentifierType level, IdentifierType indent, IdentifierType depth) const
	{
		if ((depth < m_lower) || (depth > m_upper)) {
			return false;
		}
		return true;
	}
};

} //palo
#endif
