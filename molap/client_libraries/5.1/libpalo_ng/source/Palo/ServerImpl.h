/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
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

#ifndef SERVERIMPL_H
#define SERVERIMPL_H

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/lexical_cast.hpp>

#include <libpalo_ng/Palo/types.h>
#include <libpalo_ng/Palo/Cache/CubesCache.h>
#include <libpalo_ng/Palo/Cache/DimensionsCache.h>
#include <libpalo_ng/Palo/Cache/DimensionCache.h>
#include <libpalo_ng/Palo/Cache/ServerCache.h>
#include <libpalo_ng/Palo/Cache/DatabaseCache.h>
#include <libpalo_ng/Util/StringUtils.h>

#include <map>
#include <boost/thread/thread_time.hpp>

namespace jedox {
namespace palo {

class PaloClient;

template<class C, class Ex> class LIBPALO_NG_CLASS_EXPORT SIBase : public CacheItemBase {
	friend class ServerImpl;
public:
	SIBase(unsigned int sequenceNumber) : CacheItemBase(sequenceNumber) {}
	virtual const C &operator[](const std::string &name) const
	{
		std::map<std::string, size_t, jedox::util::UTF8Comparer>::const_iterator it = m_Names.find(name);
		if (it == m_Names.end()) {
			throw Ex(getErrorMessage(name));
		}
		return m_List[it->second];
	}

	virtual const C &operator[](unsigned int id) const
	{
		std::map<unsigned int, size_t>::const_iterator it = m_Ids.find(id);
		if (it == m_Ids.end()) {
			throw Ex(getErrorMessage(id));
		}
		return m_List[it->second];
	}

	virtual bool exist(const std::string &name) const
	{
		std::map<std::string, size_t, jedox::util::UTF8Comparer>::const_iterator it = m_Names.find(name);
		return it != m_Names.end();
	}

	virtual bool exist(unsigned int id) const
	{
		std::map<unsigned int, size_t>::const_iterator it = m_Ids.find(id);
		return it != m_Ids.end();
	}

	virtual void forceNextUpdate() {
		update_time = boost::posix_time::ptime(boost::posix_time::min_date_time);
	}

	typename std::vector<C>::const_iterator begin() const {return m_List.begin();}
	typename std::vector<C>::const_iterator end() const {return m_List.end();}

protected:
	virtual std::string getErrorMessage(const std::string &name) const = 0;
	virtual std::string getErrorMessage(unsigned int id) const = 0;

	std::vector<C> m_List;
	std::map<std::string, size_t, jedox::util::UTF8Comparer> m_Names;
	std::map<unsigned int, size_t> m_Ids;
	boost::posix_time::ptime update_time;
};

class LIBPALO_NG_CLASS_EXPORT SIDatabases : public SIBase<DatabaseCacheB, DatabaseNotFoundException> {
public:
	SIDatabases(unsigned int sequenceNumber) : SIBase<DatabaseCacheB, DatabaseNotFoundException>(sequenceNumber) {}

private:
	virtual std::string getErrorMessage(const std::string &name) const
	{
		std::stringstream str;
		str << "Couldn't resolve database name \"" << name << "\".";
		return str.str();
	}

	virtual std::string getErrorMessage(unsigned int id) const
	{
		std::stringstream str;
		str << "Couldn't resolve database ID " << id << ".";
		return str.str();
	}
};

class LIBPALO_NG_CLASS_EXPORT SICubes : public SIBase<CubeCache, CubeNotFoundException> {
public:
	SICubes(unsigned int sequenceNumber, const std::string name) : SIBase<CubeCache, CubeNotFoundException>(sequenceNumber), m_name(name) {}

private:
	virtual std::string getErrorMessage(const std::string &name) const
	{
		std::stringstream str;
		str << "Couldn't resolve cube name \"" << name << "\" in database \"" << m_name << "\".";
		return str.str();
	}

	virtual std::string getErrorMessage(unsigned int id) const
	{
		std::stringstream str;
		str << "Couldn't resolve cube ID " << id << " in database \"" << m_name << "\".";
		return str.str();
	}

	std::string m_name;
};

class LIBPALO_NG_CLASS_EXPORT SIDimensions : public SIBase<DimensionCacheB, DimensionNotFoundException> {
public:
	SIDimensions(unsigned int sequenceNumber, const std::string name) : SIBase<DimensionCacheB, DimensionNotFoundException>(sequenceNumber), m_name(name) {}

private:
	virtual std::string getErrorMessage(const std::string &name) const
	{
		std::stringstream str;
		str << "Couldn't resolve dimension name \"" << name << "\" in database \"" << m_name << "\".";
		return str.str();
	}

	virtual std::string getErrorMessage(unsigned int id) const
	{
		std::stringstream str;
		str << "Couldn't resolve dimension ID " << id << " in database \"" << m_name << "\".";
		return str.str();
	}

	std::string m_name;
};

class LIBPALO_NG_CLASS_EXPORT SIElements : public SIBase<ElementCache, ElementNotFoundException> {
public:
	SIElements(unsigned int sequenceNumber, const std::string name) : SIBase<ElementCache, ElementNotFoundException>(sequenceNumber), m_name(name) {}
	bool updateElement(const ElementCache &elem);

private:
	virtual std::string getErrorMessage(const std::string &name) const
	{
		std::stringstream str;
		str << "Couldn't resolve element name \"" << name << "\" in dimension \"" << m_name << "\".";
		return str.str();
	}

	virtual std::string getErrorMessage(unsigned int id) const
	{
		std::stringstream str;
		str << "Couldn't resolve element ID " << id << " in dimension \"" << m_name << "\".";
		return str.str();
	}

	void updateChildrenIndent(const ELEMENT_INFO &el, bool updateParents, const ELEMENT_LIST &list, bool remove, bool find);
	void updateParentLevel(const ELEMENT_INFO &el, bool updateChildren);

	std::string m_name;
};

class LIBPALO_NG_CLASS_EXPORT SISystemIdElements : public SIElements {
public:
	SISystemIdElements(unsigned int sequenceNumber, const std::string name) : SIElements(sequenceNumber, name) {}
	virtual const ElementCache &operator[](const std::string &name) const
	{
		std::map<std::string, size_t, jedox::util::UTF8Comparer>::const_iterator it = m_Names.find(name);
		if (it == m_Names.end()) {
			try {
				unsigned int id = boost::lexical_cast<unsigned int>(name);
				ElementCache elem(id);
				const_cast<SISystemIdElements *>(this)->updateElement(elem);
				return this->operator[](id);
			}
			catch (boost::bad_lexical_cast &) {
				// no such element
				throw ElementNotFoundException(name);
			}
		}
		return m_List[it->second];
	}

	virtual const ElementCache &operator[](unsigned int id) const
	{
		std::map<unsigned int, size_t>::const_iterator it = m_Ids.find(id);
		if (it == m_Ids.end()) {
			ElementCache elem(id);
			const_cast<SISystemIdElements *>(this)->updateElement(elem);
			return this->operator[](id);
		}
		return m_List[it->second];
	}

	virtual bool exist(const std::string &name) const
	{
		bool result = false;
		try {
			unsigned int id = boost::lexical_cast<unsigned int>(name);
			id++; // id valid
			result = true;
		}
		catch (boost::bad_lexical_cast &) {
			// no such element
		}
		return result;
	}

	virtual bool exist(unsigned int id) const {return true;}
};

class LIBPALO_NG_CLASS_EXPORT ServerImpl : public boost::enable_shared_from_this<ServerImpl> {
public:
	ServerImpl();

	boost::shared_ptr<const ServerCacheB> getServerCache();
	void updateServerCache(boost::shared_ptr<PaloClient> paloClient);
	void setSvsMode(bool enabled);

	DatabaseCache getDatabase(boost::shared_ptr<PaloClient> paloClient, const std::string &name);
	DatabaseCache getDatabase(boost::shared_ptr<PaloClient> paloClient, unsigned int id);
	bool existDatabase(boost::shared_ptr<PaloClient> paloClient, const std::string &name);
	bool existDatabase(boost::shared_ptr<PaloClient> paloClient, unsigned int id);
	std::unique_ptr<ServerCache::CacheIterator> getDatabaseIterator(boost::shared_ptr<PaloClient> paloClient);
	void invalidateDatabases();
	void invalidateDatabases(unsigned int sequenceNumber);
	unsigned int getDatabasesSN(boost::shared_ptr<PaloClient> paloClient);
	boost::shared_ptr<const SIDatabases> getDatabases(boost::shared_ptr<PaloClient> paloClient);

	CubeCache getCube(boost::shared_ptr<PaloClient> paloClient, unsigned int db, const std::string &name);
	CubeCache getCube(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int id);
	bool existCube(boost::shared_ptr<PaloClient> paloClient, unsigned int db, const std::string &name);
	bool existCube(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int id);
	std::unique_ptr<CubesCache::CacheIterator> getCubeIterator(boost::shared_ptr<PaloClient> paloClient, unsigned int db);
	void invalidateCubes(unsigned int db);
	void invalidateCubes(unsigned int sequenceNumber, unsigned int db);
	unsigned int getCubesSN(boost::shared_ptr<PaloClient> paloClient, unsigned int db);
	boost::shared_ptr<const SICubes> getCubes(boost::shared_ptr<PaloClient> paloClient, unsigned int db);

	DimensionCache getDim(boost::shared_ptr<PaloClient> paloClient, unsigned int db, const std::string &name);
	DimensionCache getDim(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int id);
	bool existDim(boost::shared_ptr<PaloClient> paloClient, unsigned int db, const std::string &name);
	bool existDim(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int id);
	std::unique_ptr<DimensionsCache::CacheIterator> getDimIterator(boost::shared_ptr<PaloClient> paloClient, unsigned int db);
	void invalidateDimensions(unsigned int db);
	void invalidateDimensions(unsigned int sequenceNumber, unsigned int db);
	unsigned int getDimensionsSN(boost::shared_ptr<PaloClient> paloClient, unsigned int db);
	boost::shared_ptr<const SIDimensions> getDimensions(boost::shared_ptr<PaloClient> paloClient, unsigned int db);
	boost::shared_ptr<const SIDimensions> getDimensionsInt(boost::shared_ptr<PaloClient> paloClient, unsigned int db);

	ElementCache getElement(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim, const std::string &name);
	ElementCache getElement(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim, unsigned int id);
	bool existElement(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim, const std::string &name);
	bool existElement(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim, unsigned int id);
	std::unique_ptr<DimensionCache::CacheIterator> getElemIterator(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim);
	void invalidateElements(unsigned int db, unsigned int dim);
	void invalidateElements(unsigned int sequenceNumber, unsigned int db, unsigned int dim);
	unsigned int getElementsSN(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim);
	boost::shared_ptr<const SIElements> getElements(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim);
	boost::shared_ptr<const SIElements> updateElement(boost::shared_ptr<PaloClient> paloClient, unsigned int sequenceNumber, unsigned int db, unsigned int dim, const ElementCache &elem);

	void forceNextUpdate();

private:
	typedef std::pair<unsigned int, unsigned int> DimKey;

	void invalidateDatabasesIntern(unsigned int sequenceNumber, bool check);
	void invalidateCubesIntern(unsigned int sequenceNumber, bool check, unsigned int db);
	void invalidateDimensionsIntern(unsigned int sequenceNumber, bool check, unsigned int db);
	void invalidateElementsIntern(unsigned int sequenceNumber, bool check, unsigned int db, unsigned int dim);
	boost::shared_ptr<SIDatabases> readDatabases(boost::shared_ptr<PaloClient> paloClient);
	boost::shared_ptr<SICubes> readCubes(boost::shared_ptr<PaloClient> paloClient, unsigned int db);
	boost::shared_ptr<SIDimensions> readDimensions(boost::shared_ptr<PaloClient> paloClient, unsigned int db);
	boost::shared_ptr<SIElements> readElements(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim, bool &cachable);
	unsigned int getDatabasesSequenceNumber(boost::shared_ptr<PaloClient> paloClient);
	unsigned int getCubesSequenceNumber(boost::shared_ptr<PaloClient> paloClient, unsigned int db);
	unsigned int getDimensionsSequenceNumber(boost::shared_ptr<PaloClient> paloClient, unsigned int db);
	unsigned int getElementsSequenceNumber(boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim);
	template<typename T> void checkCache(CacheItemBase *cache, const std::string &expected, const DimKey &key)
	{
		std::string name = typeid(*cache).name();
		if (!dynamic_cast<T *>(cache)) {
			std::stringstream str;
			str << "Wrong value for cache " << key.first << ", " << key.second << ". ";
			if (!cache) {
				str << "NULL in cache.";
			} else {
				str << "Cache type mismatch. Expecting: " << expected << " Got: " << typeid(*cache).name();
			}
			std::string s(str.str());
			LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_OFFLINECACHE, &s);
		}
	}

	boost::shared_ptr<ServerCacheB> m_ServerCache;

	std::map<DimKey, boost::shared_ptr<CacheItemBase> > m_Caches;
	boost::mutex m_Lock;
	bool m_svsMode;
	static const int checkinterval = 500;
};

}
}

#endif
