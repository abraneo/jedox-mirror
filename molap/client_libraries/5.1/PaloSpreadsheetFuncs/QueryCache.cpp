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
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

#include <libpalo_ng/Palo/Database.h>
#include <libpalo_ng/Palo/ServerPool.h>
#include <libpalo_ng/Network/SocketException.h>

#include <PaloSpreadsheetFuncs/StringArrayArray.h>
#include <PaloSpreadsheetFuncs/CellValueArray.h>
#include <PaloSpreadsheetFuncs/QueryCache.h>
#include <PaloSpreadsheetFuncs/SpreadsheetFuncsException.h>

#include "QueryCacheEntryNotFoundException.h"

using namespace Palo::SpreadsheetFuncs;
using namespace Palo::Types;
using namespace Palo::Util;
using namespace jedox::palo;
using namespace std;

bool QueryCache::QueryCacheIndex::operator<(const QueryCache::QueryCacheIndex &r) const
{
	if (server_srv.get() == r.server_srv.get()) {
		if (jedox::util::UTF8Comparer::compare(database, r.database) == 0) {
			return jedox::util::UTF8Comparer::compare(cube, r.cube) < 0;
		} else {
			return jedox::util::UTF8Comparer::compare(database, r.database) < 0;
		}
	} else {
		return server_srv.get() < r.server_srv.get();
	}
}

bool QueryCache::QueryCacheInnerIndex::operator<(const QueryCache::QueryCacheInnerIndex &r) const
{
	if (pathOrigcase.size() != r.pathOrigcase.size()) {
		return pathOrigcase.size() < r.pathOrigcase.size();
	}
	StringArray::const_iterator it1 = pathOrigcase.begin();
	StringArray::const_iterator it2 = r.pathOrigcase.begin();
	for (; it1 != pathOrigcase.end(); ++it1, ++it2) {
        int res = jedox::util::UTF8Comparer::compare(*it1, *it2);
        if (res != 0) {
			return res < 0;
        }
	}
	return false;
}

QueryCache::QueryCacheIndex::QueryCacheIndex(boost::shared_ptr<jedox::palo::Server> s, const std::string& db, const std::string& c) :
		server_srv(s), database(db), cube(c)
{
}

QueryCache::QueryCacheInnerIndex::QueryCacheInnerIndex(const StringArray& p)
{
	pathOrigcase.reserve(p.size());

	for (StringArray::const_iterator i = p.begin(); i != p.end(); i++) {
		pathOrigcase.push_back(*i);
	}
}

bool QueryCache::QueryCacheIndex::operator==(const QueryCacheIndex &r) const
{
	if (server_srv == r.server_srv) {
		if (jedox::util::UTF8Comparer::compare(database, r.database) == 0) {
			return jedox::util::UTF8Comparer::compare(cube, r.cube) == 0;
		}
	}
	return false;
}

bool QueryCache::QueryCacheInnerIndex::operator==(const QueryCacheInnerIndex &r) const
{
	if (pathOrigcase.size() != r.pathOrigcase.size()) {
		return false;
	}

	StringArray::const_iterator lit = pathOrigcase.begin();
	StringArray::const_iterator rit = r.pathOrigcase.begin();

	for (; lit != pathOrigcase.end(); ++lit, ++rit) {
		if (jedox::util::UTF8Comparer::compare(*lit, *rit) != 0) {
			return false;
		}
	}

	return true;
}

QueryCache::QueryCacheEntry::QueryCacheEntry() :
		result(new (CellValue::Pool) CellValue()), obsolete(false)
{
}

QueryCache::QueryCacheEntry::QueryCacheEntry(const CellValue& cv) :
		result(new (CellValue::Pool) CellValue(cv)), obsolete(false)
{
}

QueryCache::QueryCacheEntry::~QueryCacheEntry()
{
	if (result) {
		CellValue::operator delete(result, CellValue::Pool);
		result = NULL;
	}
}

QueryCache::QueryCacheEntry::QueryCacheEntry(const QueryCacheEntry& other) :
		result(new (CellValue::Pool) CellValue(*other.result))
{
}

QueryCache::QueryCacheEntry& QueryCache::QueryCacheEntry::operator=(const QueryCacheEntry& other)
{
	obsolete = other.obsolete;
	*result = *other.result;

	return *this;
}

QueryCache::QueryCache() :
		status(Collect)
{
}
;

QueryCache::Status QueryCache::getStatus() const
{
	return status;
}

bool QueryCache::setStatus(Status s, bool check_locks)
{
	bool result = false;
	bool lock_check;

	if ((status == Collect) && (s == Return)) {
		result = exec(&lock_check);
		result = check_locks ? (lock_check || result) : result;
	} else {
		if ((status == Return) && (s == Collect)) {
			for (QueryCacheMap::iterator i = cache.begin(); i != cache.end(); ++i) {
				for (QueryCacheInnerMap::iterator j = i->second.begin(); j != i->second.end(); ++j) {
					j->second.obsolete = true;
				}
			}

			result = false;
		}
	}

	status = s;

	return result;
}

bool QueryCache::removeObsolete()
{
	bool changed = false;

	for (QueryCacheMap::iterator i = cache.begin(); i != cache.end();) {
		for (QueryCacheInnerMap::iterator j = i->second.begin(); j != i->second.end();) {
			if (j->second.obsolete) {
				changed = true;

				QueryCacheInnerMap::iterator tmp = j;

				++j;

				i->second.erase(tmp);
			} else {
				++j;
			}
		}

		// drop empty
		if (i->second.empty()) {
			QueryCacheMap::iterator tmp = i;

			++i;

			cache.erase(tmp);
		} else {
			++i;
		}
	}

	return changed;
}

jedox::palo::Cell_Values_Coordinates QueryCache::_Make_Cell_Value_Coordinates(jedox::palo::Cube c, QueryCacheInnerMap& im)
{
	Cell_Values_Coordinates cvc;

	cvc.rows = im.size();
	cvc.cols = c.getCacheData().number_dimensions;

	unique_ptr<char*> strings;
	strings.reset(new char*[cvc.rows * cvc.cols]);

size_t 	count;
	for (QueryCacheInnerMap::const_iterator i = im.begin(); i != im.end(); i++) {
		if (i->first.pathOrigcase.size() != cvc.cols) {
			throw SpreadsheetFuncsException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_NUMBER_OF_DIMENSIONS);
		}

		count = 0;
		for (StringArray::const_iterator j = i->first.pathOrigcase.begin(); j != i->first.pathOrigcase.end(); j++, count++) {
			strings.get()[count * cvc.cols + (j - i->first.pathOrigcase.begin())] = (char*)j->c_str();
		}
	}

	cvc.a = strings.release();
	return cvc;
}

bool QueryCache::exec(bool *lock_changed)
{
	bool changed = false;

	if (lock_changed) {
		*lock_changed = false;
	}

	QueryCacheMap::iterator begin = cache.begin(), end = cache.end();
	std::vector<CELL_VALUE> cv;
	QueryCacheInnerMap::iterator k;
	for (QueryCacheMap::iterator i = begin; i != end; i++) {
		const QueryCacheIndex& idx = i->first;
		QueryCacheInnerMap& im = i->second;

		try {
			StringArrayArray sa;
			sa.reserve(im.size());
			for (QueryCacheInnerMap::const_iterator j = im.begin(); j != im.end(); j++) {
				sa.push_back(j->first.pathOrigcase); // TODO: solve "scharfes s" problem better way, hash-index calculation, etc.
			}

			boost::shared_ptr < jedox::palo::Server > server = idx.server_srv;

			// assure that we are getting up-to-date data
			cv.clear();
			server->forceNextCacheUpdate();

			(*server)[idx.database].cube[idx.cube].CellValues(cv, sa, 0, 1);

			std::vector<CELL_VALUE>::const_iterator l = cv.begin();
			for (k = im.begin(); l != cv.end() && k != im.end(); l++, k++) {
				CellValue lcv(*l);
				if (*(k->second.result) != lcv) {
					changed = true;
				}

				if (lock_changed && (k->second.result->lock_status != l->lock_status)) {
					*lock_changed = true;
				}

				*(k->second.result) = lcv;
			}
			while (k != im.end()) {
				changed = true;
				*(k->second.result) = ErrorInfo(XLError::NAxl, 0, "No data returned!");
				++k;
			}
		} catch (const jedox::palo::PaloServerException& e) {
			for (k = im.begin(); k != im.end(); k++) {
				changed = true;
				*(k->second.result) = ErrorInfo(XLError::VALUExl, e.code(), "Palo returned error: " + e.longDescription());
			}
		} catch (const jedox::palo::PaloException& e) {
			for (k = im.begin(); k != im.end(); k++) {
				changed = true;
				*(k->second.result) = ErrorInfo(XLError::VALUExl, e.code(), "libpalo_ng returned error: " + e.longDescription());
			}
		} catch (const jedox::palo::SocketException& e) {
			for (k = im.begin(); k != im.end(); k++) {
				changed = true;
				*(k->second.result) = ErrorInfo(XLError::VALUExl, -1, std::string("libpalo_ng returned error: ") + e.what());
			}
		} catch (const PSFException& e) {
			for (k = im.begin(); k != im.end(); k++) {
				changed = true;
				*(k->second.result) = ErrorInfo(XLError::VALUExl, -1, std::string("PaloXLL error: ") + e.what());
			}
		}
	}

//  Try again with optimization
	return changed;

//	return begin != end;
}

void QueryCache::addRequest(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const StringArray& path, const CellValue* * const ptr)
{
	QueryCacheEntry& qce = cache[QueryCacheIndex(s, database, cube)][QueryCacheInnerIndex(path)];

	qce.obsolete = false;

	if (ptr) {
		*ptr = qce.result;
	}
}

const CellValue& QueryCache::getResult(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const StringArray& path) const
{
	QueryCacheMap::const_iterator i = cache.find(QueryCacheIndex(s, database, cube));
	if (i == cache.end()) {
		throw QueryCacheEntryNotFoundException(CurrentSourceLocation);
	}

	QueryCacheInnerMap::const_iterator j = i->second.find(QueryCacheInnerIndex(path));
	if (j == i->second.end()) {
		throw QueryCacheEntryNotFoundException(CurrentSourceLocation);
	}

	return *(j->second.result);
}
