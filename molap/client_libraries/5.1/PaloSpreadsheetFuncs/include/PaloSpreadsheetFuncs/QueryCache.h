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

#ifndef QUERY_CACHE_H
#define QUERY_CACHE_H

#include <string>

#include <boost/functional/hash.hpp>

#include <libpalo_ng/Palo/Server.h>
#include <libpalo_ng/Palo/Cube.h>

#include <PaloSpreadsheetFuncs/StringArray.h>
#include <PaloSpreadsheetFuncs/CellValue.h>

#include <unordered_map>

namespace Palo {
namespace SpreadsheetFuncs {
using namespace Types;

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Query cache.
 *
 *  This class provides the functionality needed by DATAC. Set the cache status to "Collect"
 *  in order to add requests to it using addRequest().
 *  Changing it to "Return" later will execute all stored queries and you will be able to retrieve the
 *  results using getResult().
 */
class QueryCache {
public:
	QueryCache();

	enum Status {
		Collect, Return
	};

	Status getStatus() const;
	/*! \return true if cache was not empty and data (and/or locks - if requested) changed */
	bool setStatus(Status s, bool check_locks = false);
	/*! \return true if cache was changed. */
	bool removeObsolete();

	void addRequest(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const StringArray& path, const CellValue* * const ptr = 0);
	const CellValue& getResult(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const StringArray& path) const;

	struct QueryCacheIndex {
		QueryCacheIndex(boost::shared_ptr<jedox::palo::Server> s, const std::string& db, const std::string& c);
		bool operator==(const QueryCacheIndex &r) const;
		bool operator<(const QueryCacheIndex &r) const;

		boost::shared_ptr<jedox::palo::Server> server_srv;
		const std::string database;
		const std::string cube;

	private:
		QueryCacheIndex();
	};

	struct QueryCacheInnerIndex {
		QueryCacheInnerIndex(const StringArray& p);
		bool operator==(const QueryCacheInnerIndex &r) const;
		bool operator<(const QueryCacheInnerIndex &r) const;

		StringArray pathOrigcase;

	private:
		QueryCacheInnerIndex();
	};

	struct QueryCacheEntry {
		QueryCacheEntry();
		QueryCacheEntry(const CellValue& cv);
		QueryCacheEntry(const QueryCacheEntry& other);
		~QueryCacheEntry();

		QueryCacheEntry& operator=(const QueryCacheEntry& other);

		CellValue* result;
		bool obsolete;
	};

private:
	typedef std::map<QueryCacheInnerIndex, QueryCacheEntry> QueryCacheInnerMap;
	typedef std::map<QueryCacheIndex, QueryCacheInnerMap> QueryCacheMap;

	bool exec(bool *lock_changed = 0);

	Status status;
	QueryCacheMap cache;

	jedox::palo::Cell_Values_Coordinates _Make_Cell_Value_Coordinates(jedox::palo::Cube c, QueryCacheInnerMap& im);
};
}
}
#endif
