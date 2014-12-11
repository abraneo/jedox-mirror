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
 * 
 *
 */

#include <sstream>
#include "StringTokenizer.h"

using namespace Palo::Util;
using namespace Palo::Types;
using namespace std;

StringTokenizer::StringTokenizer(string s, string::value_type delim) :
	s(s), delim(delim)
{
}

StringTokenizer::~StringTokenizer()
{
}

StringArray StringTokenizer::getTokens()
{
	StringArray sa;

	bool inquote = false;
	string::const_iterator curr = s.begin();
	for (string::const_iterator it = s.begin(); it != s.end(); ++it) {
		if (*it == delim) {
			if (!inquote) {
				string sub(curr, it);
				sa.push_back(sub);
				curr = it + 1;
			}
		}
		if (*it == '\"') {
			if (it + 1 != s.end() && *(it + 1) == '\"') {
				++it;
			} else {
				inquote = !inquote;
			}
		}
	}
	string sub(curr, string::const_iterator(s.end()));
	sa.push_back(sub);
	return sa;
}

string StringTokenizer::unQuote(const string &s)
{
	stringstream res;
	for (string::const_iterator it = s.begin(); it != s.end(); ++it) {
		if (*it == '\"') {
			if (it + 1 != s.end() && *(it + 1) == '\"') {
				res << *(it++);
			}
		} else {
			res << *it;
		}
	}
	return res.str();
}
