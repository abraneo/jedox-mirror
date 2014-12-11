/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

#include "ServerImpl.h"
#include "libpalo_ng/Palo/Network/PaloClient.h"
#include "../Palo/Exception/PaloNGGeneralException.h"
#include "../Util/CsvTokenFromStream.h"

#include <set>

namespace jedox {

namespace palo {

bool SIElements::updateElement(const ElementCache &elem)
{
	std::map<unsigned int, size_t>::iterator iter = m_Ids.find(elem.element);
	if (iter == m_Ids.end()) {
		size_t pos = m_List.size();
		m_Names.insert(std::make_pair(elem.nelement, pos));
		m_Ids.insert(std::make_pair(elem.element, pos));
		m_List.push_back(elem);
		for (ELEMENT_LIST::const_iterator it = elem.children.begin(); it != elem.children.end(); ++it) {
			ElementCache &child = const_cast<ElementCache &>((*this)[*it]);
			ELEMENT_LIST::iterator res = std::find(child.parents.begin(), child.parents.end(), elem.element);
			if (res == child.parents.end()) {
				++child.number_parents;
				child.parents.push_back(elem.element);
			}
		}
	} else {
		ELEMENT_INFO &old = m_List[iter->second];
		m_Names.erase(old.nelement);
		m_Names.insert(std::make_pair(elem.nelement, iter->second));

		std::set<unsigned int> oldchildren, newchildren;
		ELEMENT_LIST added, removed;
		added.resize(std::max(old.children.size(), elem.children.size()));
		removed.resize(std::max(old.children.size(), elem.children.size()));
		for (ELEMENT_LIST::const_iterator it = old.children.begin(); it != old.children.end(); ++it) oldchildren.insert(*it);
		for (ELEMENT_LIST::const_iterator it = elem.children.begin(); it != elem.children.end(); ++it) newchildren.insert(*it);
		ELEMENT_LIST::iterator remend = std::set_difference(oldchildren.begin(), oldchildren.end(), newchildren.begin(), newchildren.end(), removed.begin());
		ELEMENT_LIST::iterator addend = std::set_difference(newchildren.begin(), newchildren.end(), oldchildren.begin(), oldchildren.end(), added.begin());

		for (ELEMENT_LIST::const_iterator it = added.begin(); it != addend; ++it) {
			ElementCache &child = const_cast<ElementCache &>((*this)[*it]);
			++child.number_parents;
			child.parents.push_back(elem.element);
		}

		for (ELEMENT_LIST::const_iterator it = removed.begin(); it != remend; ++it) {
			ElementCache &child = const_cast<ElementCache &>((*this)[*it]);
			ELEMENT_LIST::iterator res = std::find(child.parents.begin(), child.parents.end(), elem.element);
			--child.number_parents;
			child.parents.erase(res);
		}

		if (old.position != elem.position) {
			unsigned int start = 0;
			unsigned int end = 0;
			int step = 0;

			if (old.position > elem.position) {
				start = elem.position;
				end = old.position;
				step = 1;
			} else {
				start = old.position;
				end = elem.position;
				step = -1;
			}

			for (std::vector<ElementCache>::iterator it = m_List.begin(); it != m_List.end(); ++it) {
				if (it->position >= start && it->position <= end) {
					it->position += step;
				}
			}
		}

		old = elem;
	}
	return iter == m_Ids.end();
}

ServerImpl::ServerImpl() : m_svsMode(false)
{
}

boost::shared_ptr<const ServerCacheB> ServerImpl::getServerCache()
{
	return m_ServerCache;
}

void ServerImpl::updateServerCache(boost::shared_ptr<PaloClient> paloClient)
{
	unsigned int dummy, sequencenumber = 0;
	SERVER_TOKEN servertoken(sequencenumber);

	std::unique_ptr<std::istringstream> stream = paloClient->request("/server/info", servertoken, sequencenumber, dummy);
	boost::shared_ptr<ServerCacheB> sc(new ServerCacheB());
	(*stream) >> csv >> *sc;
	sc->setSequenceNumber(sequencenumber);
	boost::unique_lock<boost::mutex> lock(m_Lock);
	if (m_ServerCache && m_ServerCache->getSequenceNumber() != sc->getSequenceNumber()) {
		m_Caches.clear();
	}
	m_ServerCache = sc;
	paloClient->SetHttpsParams(m_ServerCache->encryption, m_ServerCache->httpsPort);
}

void ServerImpl::setSvsMode(bool enabled)
{
	m_svsMode = enabled;
}

DatabaseCache ServerImpl::getDatabase(boost::shared_ptr<PaloClient> paloClient, const std::string &name)
{
	boost::shared_ptr<const SIDatabases> dbs = getDatabases(paloClient, false);
	const DatabaseCacheB &db = (*dbs)[name];
	return DatabaseCache(db, boost::shared_ptr<DimensionsCache>(new DimensionsCache(shared_from_this(), paloClient, db.database)),boost::shared_ptr<CubesCache>(new CubesCache(shared_from_this(), paloClient, db.database)));
}

DatabaseCache ServerImpl::getDatabase(boost::shared_ptr<PaloClient> paloClient, unsigned int id)
{
	boost::shared_ptr<const SIDatabases> dbs = getDatabases(paloClient, false);
	const DatabaseCacheB &db = (*dbs)[id];
	return DatabaseCache(db, boost::shared_ptr<DimensionsCache>(new DimensionsCache(shared_from_this(), paloClient, db.database)),boost::shared_ptr<CubesCache>(new CubesCache(shared_from_this(), paloClient, db.database)));
}

bool ServerImpl::existDatabase(boost::shared_ptr<PaloClient> paloClient, const std::string &name)
{
	boost::shared_ptr<const SIDatabases> dbs = getDatabases(paloClient, false);
	return dbs->exist(name);
}

bool ServerImpl::existDatabase(boost::shared_ptr<PaloClient> paloClient, unsigned int id)
{
	boost::shared_ptr<const SIDatabases> dbs = getDatabases(paloClient, false);
	return dbs->exist(id);
}

std::unique_ptr<ServerCache::CacheIterator> ServerImpl::getDatabaseIterator(boost::shared_ptr<PaloClient> paloClient)
{
	boost::shared_ptr<const SIDatabases> dbs = getDatabases(paloClient, true);
	ServerCache c(shared_from_this(), paloClient);
	return std::unique_ptr<ServerCache::CacheIterator>(new ServerCache::CacheIterator(ServerCacheBase::CacheIterator(dbs->begin(), dbs->end(), dbs), c.m_Base));
}

void ServerImpl::invalidateDatabases()
{
	boost::unique_lock<boost::mutex> lock(m_Lock);
	invalidateDatabasesIntern(0, false);
}

void ServerImpl::invalidateDatabases(unsigned int sequenceNumber)
{
	boost::unique_lock<boost::mutex> lock(m_Lock);
	invalidateDatabasesIntern(sequenceNumber, true);
}

unsigned int ServerImpl::getDatabasesSN(boost::shared_ptr<PaloClient> paloClient)
{
	boost::shared_ptr<const SIDatabases> dbs = getDatabases(paloClient, false);
	return dbs->getSequenceNumber();
}

boost::shared_ptr<const SIDatabases> ServerImpl::getDatabases(boost::shared_ptr<PaloClient> paloClient, bool update)
{
	boost::unique_lock<boost::mutex> lock(m_Lock);
	std::map<std::pair<unsigned int, unsigned int>, boost::shared_ptr<CacheItemBase> >::iterator it = m_Caches.find(std::make_pair(-1, -1));
	boost::shared_ptr<SIDatabases> res;
	if (it == m_Caches.end()) {
		m_Caches[std::make_pair(-1, -1)] = res = readDatabases(paloClient);
		res->update_time = boost::posix_time::microsec_clock::universal_time();
	} else {
		res = boost::dynamic_pointer_cast<SIDatabases, CacheItemBase>(it->second);
		if (update && (boost::posix_time::microsec_clock::universal_time() - res->update_time) > boost::posix_time::milliseconds(checkinterval)) {
			if (it->second->getSequenceNumber() != getDatabasesSequenceNumber(paloClient)) {
				m_Caches[std::make_pair(-1, -1)] = res = readDatabases(paloClient);
			}
			res->update_time = boost::posix_time::microsec_clock::universal_time();
		}
	}
	return res;
}

CubeCache ServerImpl::getCube(boost::shared_ptr<PaloClient> paloClient, unsigned int db, const std::string &name)
{
	boost::shared_ptr<const SICubes> cubes = getCubes(paloClient, db, false);
	return (*cubes)[name];
}

CubeCache ServerImpl::getCube(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int id)
{
	boost::shared_ptr<const SICubes> cubes = getCubes(paloClient, db, false);
	return (*cubes)[id];
}

bool ServerImpl::existCube(boost::shared_ptr<PaloClient> paloClient, unsigned int db, const std::string &name)
{
	boost::shared_ptr<const SICubes> cubes = getCubes(paloClient, db, false);
	return cubes->exist(name);
}

bool ServerImpl::existCube(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int id)
{
	boost::shared_ptr<const SICubes> cubes = getCubes(paloClient, db, false);
	return cubes->exist(id);
}

std::unique_ptr<CubesCache::CacheIterator> ServerImpl::getCubeIterator(boost::shared_ptr<PaloClient> paloClient, unsigned int db)
{
	boost::shared_ptr<const SICubes> cubes = getCubes(paloClient, db, true);
	return std::unique_ptr<CubesCache::CacheIterator>(new CubesCache::CacheIterator(cubes->begin(), cubes->end(), cubes));
}

void ServerImpl::invalidateCubes(unsigned int db)
{
	boost::unique_lock<boost::mutex> lock(m_Lock);
	invalidateCubesIntern(0, false, db);
}

void ServerImpl::invalidateCubes(unsigned int sequenceNumber, unsigned int db)
{
	boost::unique_lock<boost::mutex> lock(m_Lock);
	invalidateCubesIntern(sequenceNumber, true, db);
}

unsigned int ServerImpl::getCubesSN(boost::shared_ptr<PaloClient> paloClient, unsigned int db)
{
	boost::shared_ptr<const SICubes> cubes = getCubes(paloClient, db, false);
	return cubes->getSequenceNumber();
}

boost::shared_ptr<const SICubes> ServerImpl::getCubes(boost::shared_ptr<PaloClient> paloClient, unsigned int db, bool update)
{
	boost::unique_lock<boost::mutex> lock(m_Lock);
	std::map<std::pair<unsigned int, unsigned int>, boost::shared_ptr<CacheItemBase> >::iterator it = m_Caches.find(std::make_pair(db, -1));
	boost::shared_ptr<SICubes> res;
	if (it == m_Caches.end()) {
		m_Caches[std::make_pair(db, -1)] = res = readCubes(paloClient, db);
		res->update_time = boost::posix_time::microsec_clock::universal_time();
	} else {
		res = boost::dynamic_pointer_cast<SICubes, CacheItemBase>(it->second);
		if (update && (boost::posix_time::microsec_clock::universal_time() - res->update_time) > boost::posix_time::milliseconds(checkinterval)) {
			if (it->second->getSequenceNumber() != getCubesSequenceNumber(paloClient, db)) {
				m_Caches[std::make_pair(db, -1)] = res = readCubes(paloClient, db);
			}
			res->update_time = boost::posix_time::microsec_clock::universal_time();
		}
	}
	return res;
}

DimensionCache ServerImpl::getDim(boost::shared_ptr<PaloClient> paloClient, unsigned int db, const std::string &name)
{
	boost::shared_ptr<const SIDimensions> dims = getDimensions(paloClient, db, false);
	return DimensionCache((*dims)[name], shared_from_this(), paloClient, db);
}

DimensionCache ServerImpl::getDim(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int id)
{
	boost::shared_ptr<const SIDimensions> dims = getDimensions(paloClient, db, false);
	return DimensionCache((*dims)[id], shared_from_this(), paloClient, db);
}

bool ServerImpl::existDim(boost::shared_ptr<PaloClient> paloClient, unsigned int db, const std::string &name)
{
	boost::shared_ptr<const SIDimensions> dims = getDimensions(paloClient, db, false);
	return dims->exist(name);
}

bool ServerImpl::existDim(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int id)
{
	boost::shared_ptr<const SIDimensions> dims = getDimensions(paloClient, db, false);
	return dims->exist(id);
}

std::unique_ptr<DimensionsCache::CacheIterator> ServerImpl::getDimIterator(boost::shared_ptr<PaloClient> paloClient, unsigned int db)
{
	boost::shared_ptr<const SIDimensions> dims = getDimensions(paloClient, db, true);
	DimensionsCache c(shared_from_this(), paloClient, db);
	return std::unique_ptr<DimensionsCache::CacheIterator>(new DimensionsCache::CacheIterator(DimensionsCacheB::CacheIterator(dims->begin(), dims->end(), dims), c.m_Base));
}

void ServerImpl::invalidateDimensions(unsigned int db)
{
	boost::unique_lock<boost::mutex> lock(m_Lock);
	invalidateDimensionsIntern(0, false, db);
}

void ServerImpl::invalidateDimensions(unsigned int sequenceNumber, unsigned int db)
{
	boost::unique_lock<boost::mutex> lock(m_Lock);
	invalidateDimensionsIntern(sequenceNumber, true, db);
}

unsigned int ServerImpl::getDimensionsSN(boost::shared_ptr<PaloClient> paloClient, unsigned int db)
{
	boost::shared_ptr<const SIDimensions> dims = getDimensions(paloClient, db, false);
	return dims->getSequenceNumber();
}

boost::shared_ptr<const SIDimensions> ServerImpl::getDimensions(boost::shared_ptr<PaloClient> paloClient, unsigned int db, bool update)
{
	boost::unique_lock<boost::mutex> lock(m_Lock);
	std::map<std::pair<unsigned int, unsigned int>, boost::shared_ptr<CacheItemBase> >::iterator it = m_Caches.find(std::make_pair(db, -2));
	boost::shared_ptr<SIDimensions> res;
	if (it == m_Caches.end()) {
		m_Caches[std::make_pair(db, -2)] = res = readDimensions(paloClient, db);
		res->update_time = boost::posix_time::microsec_clock::universal_time();
	} else {
		res = boost::dynamic_pointer_cast<SIDimensions, CacheItemBase>(it->second);
		if (update && (boost::posix_time::microsec_clock::universal_time() - res->update_time) > boost::posix_time::milliseconds(checkinterval)) {
			if (it->second->getSequenceNumber() != getDimensionsSequenceNumber(paloClient, db)) {
				m_Caches[std::make_pair(db, -2)] = res = readDimensions(paloClient, db);
			}
			res->update_time = boost::posix_time::microsec_clock::universal_time();
		}
	}
	return res;
}

ElementCache ServerImpl::getElement(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim, const std::string &name)
{
	boost::shared_ptr<const SIElements> elems = getElements(paloClient, db, dim, false);
	return (*elems)[name];
}

ElementCache ServerImpl::getElement(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim, unsigned int id)
{
	boost::shared_ptr<const SIElements> elems = getElements(paloClient, db, dim, false);
	return (*elems)[id];
}

bool ServerImpl::existElement(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim, const std::string &name)
{
	boost::shared_ptr<const SIElements> elems = getElements(paloClient, db, dim, false);
	return elems->exist(name);
}

bool ServerImpl::existElement(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim, unsigned int id)
{
	boost::shared_ptr<const SIElements> elems = getElements(paloClient, db, dim, false);
	return elems->exist(id);
}

std::unique_ptr<DimensionCache::CacheIterator> ServerImpl::getElemIterator(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim)
{
	boost::shared_ptr<const SIElements> elems = getElements(paloClient, db, dim, true);
	return std::unique_ptr<DimensionCache::CacheIterator>(new DimensionCache::CacheIterator(elems->begin(), elems->end(), elems));
}

void ServerImpl::invalidateElements(unsigned int db, unsigned int dim)
{
	boost::unique_lock<boost::mutex> lock(m_Lock);
	invalidateElementsIntern(0, false, db, dim);
}

void ServerImpl::invalidateElements(unsigned int sequenceNumber, unsigned int db, unsigned int dim)
{
	boost::unique_lock<boost::mutex> lock(m_Lock);
	invalidateElementsIntern(sequenceNumber, true, db, dim);
}

unsigned int ServerImpl::getElementsSN(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim)
{
	boost::shared_ptr<const SIElements> elems = getElements(paloClient, db, dim, false);
	return elems->getSequenceNumber();
}

boost::shared_ptr<const SIElements> ServerImpl::getElements(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim, bool update)
{
	DimKey dimKey = std::make_pair(db, dim);
	boost::unique_lock<boost::mutex> lock(m_Lock);
	std::map<std::pair<unsigned int, unsigned int>, boost::shared_ptr<CacheItemBase> >::iterator it = m_Caches.find(dimKey);
	boost::shared_ptr<SIElements> res;
	if (it == m_Caches.end()) {
		bool cachable = true;
		res = readElements(paloClient, db, dim, cachable);
		if (cachable) {
			m_Caches[dimKey] = res;
			res->update_time = boost::posix_time::microsec_clock::universal_time();
		}
	} else {
		res = boost::dynamic_pointer_cast<SIElements, CacheItemBase>(it->second);
		if (update && (boost::posix_time::microsec_clock::universal_time() - res->update_time) > boost::posix_time::milliseconds(checkinterval)) {
			if (it->second->getSequenceNumber() != getElementsSequenceNumber(paloClient, db, dim)) {
				bool cachable = true;
				res = readElements(paloClient, db, dim, cachable);
				if (cachable) {
					m_Caches[dimKey] = res;
				}
			}
			res->update_time = boost::posix_time::microsec_clock::universal_time();
		}
	}
	return res;
}

boost::shared_ptr<const SIElements> ServerImpl::updateElement(boost::shared_ptr<PaloClient> paloClient, unsigned int sequenceNumber, unsigned int db, unsigned int dim, const ElementCache &elem)
{
	boost::unique_lock<boost::mutex> lock(m_Lock);
	std::map<std::pair<unsigned int, unsigned int>, boost::shared_ptr<CacheItemBase> >::iterator it = m_Caches.find(std::make_pair(db, dim));
	boost::shared_ptr<SIElements> newCache;
	if (it != m_Caches.end()) {
		if (it->second->getSequenceNumber() < sequenceNumber || m_svsMode) {
			newCache.reset(new SIElements(*boost::dynamic_pointer_cast<SIElements, CacheItemBase>(it->second)));
			newCache->setSequenceNumber(sequenceNumber);
			bool newElem = newCache->updateElement(elem);
			m_Caches[std::make_pair(db, dim)] = newCache;
			if (newElem) {
				it = m_Caches.find(std::make_pair(db, -2));
				if (it != m_Caches.end()) {
					boost::shared_ptr<SIDimensions> nd(new SIDimensions(*boost::dynamic_pointer_cast<SIDimensions, CacheItemBase>(it->second)));
					++const_cast<DimensionCacheB &>((*nd)[dim]).number_elements;
					m_Caches[std::make_pair(db, -2)] = nd;
				}
			}
		}
	} else {
		bool cachable = true;
		newCache = readElements(paloClient, db, dim, cachable);
		if (cachable) {
			m_Caches[std::make_pair(db, dim)] = newCache;
		}
	}
	return newCache;
}

void ServerImpl::forceNextUpdate()
{
	boost::unique_lock<boost::mutex> lock(m_Lock);
	for (std::map<std::pair<unsigned int, unsigned int>, boost::shared_ptr<CacheItemBase> >::iterator it = m_Caches.begin(); it != m_Caches.end(); ++it) {
		it->second->forceNextUpdate();
	}
}

void ServerImpl::invalidateDatabasesIntern(unsigned int sequenceNumber, bool check)
{
	std::map<std::pair<unsigned int, unsigned int>, boost::shared_ptr<CacheItemBase> >::iterator it = m_Caches.find(std::make_pair(-1, -1));
	if (it != m_Caches.end()) {
		if (!check || it->second->getSequenceNumber() != sequenceNumber) {
			m_Caches.clear();
		}
	}
}

void ServerImpl::invalidateCubesIntern(unsigned int sequenceNumber, bool check, unsigned int db)
{
	std::map<std::pair<unsigned int, unsigned int>, boost::shared_ptr<CacheItemBase> >::iterator it = m_Caches.find(std::make_pair(db, -1));
	if (it != m_Caches.end()) {
		if (!check || it->second->getSequenceNumber() != sequenceNumber) {
			m_Caches.erase(it);
		}
	}
}

void ServerImpl::invalidateDimensionsIntern(unsigned int sequenceNumber, bool check, unsigned int db)
{
	std::map<std::pair<unsigned int, unsigned int>, boost::shared_ptr<CacheItemBase> >::iterator it = m_Caches.find(std::make_pair(db, -2));
	if (it != m_Caches.end()) {
		if (!check || it->second->getSequenceNumber() != sequenceNumber) {
			boost::shared_ptr<SIDimensions> dims = boost::dynamic_pointer_cast<SIDimensions, CacheItemBase>(it->second);
			m_Caches.erase(it);
			for (std::vector<DimensionCacheB>::const_iterator i = dims->begin(); i != dims->end(); ++i) {
				invalidateElementsIntern(0, false, db, i->dimension);
			}
		}
	}
}

void ServerImpl::invalidateElementsIntern(unsigned int sequenceNumber, bool check, unsigned int db, unsigned int dim)
{
	std::map<std::pair<unsigned int, unsigned int>, boost::shared_ptr<CacheItemBase> >::iterator it = m_Caches.find(std::make_pair(db, dim));
	if (it != m_Caches.end()) {
		if (!check || it->second->getSequenceNumber() != sequenceNumber) {
			m_Caches.erase(it);
		}
	}
}

boost::shared_ptr<SIDatabases> ServerImpl::readDatabases(boost::shared_ptr<PaloClient> paloClient)
{
	std::stringstream query;
	unsigned int dummy, sequencenumber = 0;
	SERVER_TOKEN sequenceToken(sequencenumber);

	query << "show_normal=1&show_system=1&show_user_info=1";

	std::unique_ptr<std::istringstream> stream = paloClient->request("/server/databases", query.str(), sequenceToken, sequencenumber, dummy);
	jedox::util::CsvTokenFromStream tfs(*stream);
	boost::shared_ptr<SIDatabases> ret(new SIDatabases(sequencenumber));
	while ((*stream).eof() == false) {
		DatabaseCacheB db;
		(*stream) >> csv >> db;
		unsigned int seq_nr;
		tfs.get(seq_nr, 0);
		db.setSequenceNumber(seq_nr);
		ret->m_Ids.insert(std::make_pair(db.database, ret->m_List.size()));
		ret->m_Names.insert(std::make_pair(db.ndatabase, ret->m_List.size()));
		ret->m_List.push_back(db);
		tfs.clear_done();
	}
	return ret;
}

boost::shared_ptr<SICubes> ServerImpl::readCubes(boost::shared_ptr<PaloClient> paloClient, unsigned int db)
{
	std::stringstream query;
	unsigned int dummy, sequencenumber = 0;
	DATABASE_TOKEN sequenceToken(sequencenumber);

	query << "database=" << db << "&show_normal=1&show_system=1&show_attribute=1&show_info=1&show_gputype=1";

	std::unique_ptr<std::istringstream> stream = paloClient->request("/database/cubes", query.str(), sequenceToken, sequencenumber, dummy);
	jedox::util::CsvTokenFromStream tfs(*stream);
	std::string dbname;
	std::map<std::pair<unsigned int, unsigned int>, boost::shared_ptr<CacheItemBase> >::iterator it = m_Caches.find(std::make_pair(-1, -1));
	if (it != m_Caches.end()) {
		dbname = (*boost::dynamic_pointer_cast<SIDatabases, CacheItemBase>(it->second))[db].ndatabase;
	}
	boost::shared_ptr<SICubes> ret(new SICubes(sequencenumber, dbname));
	while ((*stream).eof() == false) {
		CubeCache cube;
		(*stream) >> csv >> cube;
		unsigned int seq_nr;
		tfs.get(seq_nr, 0);
		cube.setSequenceNumber(seq_nr);
		ret->m_Ids.insert(std::make_pair(cube.cube, ret->m_List.size()));
		ret->m_Names.insert(std::make_pair(cube.ncube, ret->m_List.size()));
		ret->m_List.push_back(cube);
		tfs.clear_done();
	}
	return ret;
}

boost::shared_ptr<SIDimensions> ServerImpl::readDimensions(boost::shared_ptr<PaloClient> paloClient, unsigned int db)
{
	std::stringstream query;
	unsigned int dummy, sequencenumber = 0;
	DATABASE_TOKEN sequenceToken(sequencenumber);

	query << "database=" << db << "&show_normal=1&show_system=1&show_attribute=1&show_info=1";

	std::unique_ptr<std::istringstream> stream = paloClient->request("/database/dimensions", query.str(), sequenceToken, sequencenumber, dummy);
	jedox::util::CsvTokenFromStream tfs(*stream);
	std::string dbname;
	std::map<std::pair<unsigned int, unsigned int>, boost::shared_ptr<CacheItemBase> >::iterator it = m_Caches.find(std::make_pair(-1, -1));
	if (it != m_Caches.end()) {
		dbname = (*boost::dynamic_pointer_cast<SIDatabases, CacheItemBase>(it->second))[db].ndatabase;
	}
	boost::shared_ptr<SIDimensions> ret(new SIDimensions(sequencenumber, dbname));
	while ((*stream).eof() == false) {
		DimensionCacheB dim;
		(*stream) >> csv >> dim;
		unsigned int seq_nr;
		tfs.get(seq_nr, 0);
		dim.setSequenceNumber(seq_nr);
		ret->m_Ids.insert(std::make_pair(dim.dimension, ret->m_List.size()));
		ret->m_Names.insert(std::make_pair(dim.ndimension, ret->m_List.size()));
		ret->m_List.push_back(dim);
		tfs.clear_done();
	}
	return ret;
}

boost::shared_ptr<SIElements> ServerImpl::readElements(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim, bool &cachable)
{
	std::stringstream query;
	unsigned int dummy, sequencenumber = 0;
	DIMENSION_TOKEN sequenceToken(sequencenumber);

	query << "show_lock_info=1&database=" << db << "&dimension=" << dim;

	std::unique_ptr<std::istringstream> stream = paloClient->request("/dimension/elements", query.str(), sequenceToken, sequencenumber, dummy);
	std::string dimname;
	std::map<std::pair<unsigned int, unsigned int>, boost::shared_ptr<CacheItemBase> >::iterator it = m_Caches.find(std::make_pair(db, -2));
	DimensionCacheB dimCache;
	dimCache.type = DIMENSION_INFO::NORMAL;

	if (it != m_Caches.end()) {
		dimCache = (*boost::dynamic_pointer_cast<SIDimensions, CacheItemBase>(it->second))[dim];
	} else {
		dimCache = (*readDimensions(paloClient, db))[dim];
	}
	dimname = dimCache.ndimension;
	if (dimCache.type == DIMENSION_INFO::SYSTEM_ID) {
		boost::shared_ptr<SIElements> ret(new SISystemIdElements(sequencenumber, dimname));
		cachable = false;
		return ret;
	}

	boost::shared_ptr<SIElements> ret(new SIElements(sequencenumber, dimname));
	cachable = true;
	while ((*stream).eof() == false) {
		ElementCache element;
		(*stream) >> csv >> element;
		ret->m_Ids.insert(std::make_pair(element.element, ret->m_List.size()));
		ret->m_Names.insert(std::make_pair(element.nelement, ret->m_List.size()));
		ret->m_List.push_back(element);
	}
	return ret;
}

unsigned int ServerImpl::getDatabasesSequenceNumber(boost::shared_ptr<PaloClient> paloClient)
{
	unsigned int dummy, sequencenumber = 0;
	SERVER_TOKEN sequenceToken(sequencenumber);
	std::unique_ptr<std::istringstream> stream = paloClient->request("/server/info", sequenceToken, sequencenumber, dummy);
	return sequencenumber;
}

unsigned int ServerImpl::getCubesSequenceNumber(boost::shared_ptr<PaloClient> paloClient, unsigned int db)
{
	unsigned int dummy, sequencenumber = 0;
	SERVER_TOKEN sequenceToken(sequencenumber);
	std::stringstream query;
	query << "database=" << db;
	std::unique_ptr<std::istringstream> stream = paloClient->request("/database/info", query.str(), sequenceToken, sequencenumber, dummy);
	jedox::util::CsvTokenFromStream tfs(*stream);
	DATABASE_INFO element;
	(*stream) >> csv >> element;
	tfs.get(sequencenumber, 0);
	return sequencenumber;
}

unsigned int ServerImpl::getDimensionsSequenceNumber(boost::shared_ptr<PaloClient> paloClient, unsigned int db)
{
	return getCubesSequenceNumber(paloClient, db);
}

unsigned int ServerImpl::getElementsSequenceNumber(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim)
{
	unsigned int dummy, sequencenumber = 0;
	SERVER_TOKEN sequenceToken(sequencenumber);
	std::stringstream query;
	query << "show_lock_info=1&database=" << db << "&dimension=" << dim;
	std::unique_ptr<std::istringstream> stream = paloClient->request("/dimension/info", query.str(), sequenceToken, sequencenumber, dummy);
	DIMENSION_INFO element;
	jedox::util::CsvTokenFromStream tfs(*stream);
	(*stream) >> csv >> element;
	tfs.get(sequencenumber, 0);
	return sequencenumber;
}

std::unique_ptr<CubesCache::CacheIterator> CubesCache::getIterator() const
{
	return m_ServerImpl->getCubeIterator(m_PaloClient, m_db);
}

std::unique_ptr<DimensionsCacheB::CacheIterator> DimensionsCacheB::getIterator() const
{
	return std::unique_ptr<DimensionsCacheB::CacheIterator>();
}

std::unique_ptr<DimensionsCache::CacheIterator> DimensionsCache::getIterator() const
{
	return m_Base->m_ServerImpl->getDimIterator(m_Base->m_PaloClient, m_Base->m_db);
}

std::unique_ptr<ServerCacheBase::CacheIterator> ServerCacheBase::getIterator() const
{
	return std::unique_ptr<ServerCacheBase::CacheIterator>();
}

std::unique_ptr<ServerCache::CacheIterator> ServerCache::getIterator() const
{
	return m_Base->m_ServerImpl->getDatabaseIterator(m_Base->m_PaloClient);
}

std::unique_ptr<DimensionCache::CacheIterator> DimensionCache::getIterator() const
{
	return m_ServerImpl->getElemIterator(m_PaloClient, m_db, dimension);
}

boost::shared_ptr<CubeCache> CubesCache::operator[](const std::string &name) const
{
	return boost::shared_ptr<CubeCache>(new CubeCache(m_ServerImpl->getCube(m_PaloClient, m_db, name)));
}

boost::shared_ptr<CubeCache> CubesCache::operator[](unsigned int id) const
{
	return boost::shared_ptr<CubeCache>(new CubeCache(m_ServerImpl->getCube(m_PaloClient, m_db, id)));
}

boost::shared_ptr<DimensionCacheB> DimensionsCacheB::operator[](const std::string &name) const
{
	return boost::shared_ptr<DimensionCacheB>();
}

boost::shared_ptr<DimensionCacheB> DimensionsCacheB::operator[](unsigned int id) const
{
	return boost::shared_ptr<DimensionCacheB>();
}

boost::shared_ptr<DimensionCache> DimensionsCache::operator[](const std::string &name) const
{
	return boost::shared_ptr<DimensionCache>(new DimensionCache(m_Base->m_ServerImpl->getDim(m_Base->m_PaloClient, m_Base->m_db, name)));
}

boost::shared_ptr<DimensionCache> DimensionsCache::operator[](unsigned int id) const
{
	return boost::shared_ptr<DimensionCache>(new DimensionCache(m_Base->m_ServerImpl->getDim(m_Base->m_PaloClient, m_Base->m_db, id)));
}

boost::shared_ptr<DatabaseCacheB> ServerCacheBase::operator[](const std::string &name) const
{
	return boost::shared_ptr<DatabaseCacheB>();
}

boost::shared_ptr<DatabaseCacheB> ServerCacheBase::operator[](unsigned int id) const
{
	return boost::shared_ptr<DatabaseCacheB>();
}

boost::shared_ptr<DatabaseCache> ServerCache::operator[](const std::string &name) const
{
	return boost::shared_ptr<DatabaseCache>(new DatabaseCache(m_Base->m_ServerImpl->getDatabase(m_Base->m_PaloClient, name)));
}

boost::shared_ptr<DatabaseCache> ServerCache::operator[](unsigned int id) const
{
	return boost::shared_ptr<DatabaseCache>(new DatabaseCache(m_Base->m_ServerImpl->getDatabase(m_Base->m_PaloClient, id)));
}

boost::shared_ptr<ElementCache> DimensionCache::operator[](const std::string &name) const
{
	return boost::shared_ptr<ElementCache>(new ElementCache(m_ServerImpl->getElement(m_PaloClient, m_db, dimension, name)));
}

boost::shared_ptr<ElementCache> DimensionCache::operator[](unsigned int id) const
{
	return boost::shared_ptr<ElementCache>(new ElementCache(m_ServerImpl->getElement(m_PaloClient, m_db, dimension, id)));
}

}

}
