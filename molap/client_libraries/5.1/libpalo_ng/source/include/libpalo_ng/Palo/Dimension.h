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
 * \author Florian Schaper <florian.schaper@jedox.com>
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4251)
#endif

#ifndef DIMENSION_H
#define DIMENSION_H

#include <string>

#include <libpalo_ng/Palo/types.h>
#include <libpalo_ng/Palo/Server.h>
#include <libpalo_ng/Palo/Element.h>
#include <libpalo_ng/Palo/Cache/DimensionCache.h>

namespace jedox {
namespace palo {

class SIDimensions;
class ServerImpl;

/** @brief
 *  Class for performing dimension operations.
 *  All Strings are UTF8.
 */
class LIBPALO_NG_CLASS_EXPORT Dimension {
public:
	Dimension(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, IdentifierType db, IdentifierType dim);

	/** @brief
	 *  get an Element Object by its name
	 *
	 *  @return Element Object with name "name"
	 */
	Element operator[](const std::string& name);

	/** @brief
	 *  get an Element Object by its id
	 *
	 *  @return Element Object with id "id"
	 */
	Element operator[](IdentifierType id);

	/** @brief
	 *  Check the existence of the element with name 'name'
	 */
	bool Exists(const std::string& name);

	/** @brief
	 *  Check the existence of the element with id 'id'
	 */
	bool Exists(IdentifierType id, bool cacheonly = false);

	/** @brief
	 *  Get information about this dimension in a structure of type DIMENSION_INFO.
	 *
	 *  @return DIMENSION_INFO for this dimension.
	 */
	const DIMENSION_INFO& getCacheData() const;

	/** @brief
	 *  Get information about this dimension in a structure of type DIMENSION_INFO.
	 *
	 *  @return DIMENSION_INFO for this dimension.
	 */
	const DIMENSION_INFO getCacheDataCopy() const;

	/** @brief
	 *  Add a new element to this dimenison.
	 *
	 *  @param Name : Name of the new element.
	 *  @param type : ELEMENT_INFO::TYPE of the new element
	 *  @param children : List of children
	 *  @param weights : Weights of children
	 */
	void createElement(const std::string& Name, ELEMENT_INFO::TYPE type, const std::vector<std::string> &children, const std::vector<double> &weights);

	/** @brief
	 *  Add a new element to this dimenison.
	 *
	 *  @param result : Newly created element.
	 *  @param Name : Name of the new element.
	 *  @param type : ELEMENT_INFO::TYPE of the new element
	 *  @param children : List of children
	 *  @param weights : Weights of children
	 */
	void createElement(ELEMENT_INFO& result, const std::string& Name, ELEMENT_INFO::TYPE type, const std::vector<std::string> &children, const std::vector<double> &weights);

	/** @brief
	 *  Add a new element to this dimenison.
	 *
	 *  @param Name : Name of the new element.
	 *  @param type : ELEMENT_INFO::TYPE of the new element
	 *  @param children : List of children
	 *  @param weights : List of weights
	 */
	void createElement(ELEMENT_INFO& result, const std::string& Name, ELEMENT_INFO::TYPE type, const ELEMENT_LIST &children, const std::vector<double> &weights);

	/** @brief
	 *  Add a new element to this dimenison.
	 *
	 *  @param result : Newly created element.
	 *  @param Name : Name of the new element.
	 *  @param type : ELEMENT_INFO::TYPE of the new element
	 *  @param children : List of children
	 *  @param weights : List of weights
	 */
	void createElement(const std::string& Name, ELEMENT_INFO::TYPE type, const ELEMENT_LIST &children, const std::vector<double> &weights);

	/** @brief
	 *  Create list of elements
	 *
	 *  @param elements : List of element names.
	 *  @param type : ELEMENT_INFO::TYPE of the new element.
	 *  @param children : List of children names.
	 *  @param weights : List of children weight lists.
	 */
	void bulkCreateElements(const std::vector<std::string> &elements, ELEMENT_INFO::TYPE type, const std::vector<std::vector<std::string> > &children, const std::vector<std::vector<double> > &weights);

	/** @brief
	 *  Delete list of elements
	 *
	 *  @param elements : List of element names.
	 */
	void bulkDeleteElements(const std::vector<std::string> &elements);

	/** @brief
	 *  Delete list of elements
	 *
	 *  @param elements : List of element names.
	 */
	void bulkDeleteElements(const ELEMENT_LIST &elements);
	
	/** @brief
	 *  Changes list of elements
	 *
	 *  @param elements : List of element names.
	 *  @param type : ELEMENT_INFO::TYPE of the new element.
	 *  @param children : List of children names.
	 *  @param weights : List of children weight lists.
	 */
	void bulkReplaceElements(const std::vector<std::string> &elements, ELEMENT_INFO::TYPE type, const std::vector<std::vector<std::string> > &children, const std::vector<std::vector<double> > &weights);

	/** @brief
	 *  Move elements to the specified positions
	 *
	 *  @param elements : List of element names.
	 *  @param positions: List of positions.
	 */
	void bulkMoveElements(const std::vector<std::string> &elements, const std::vector<int> &positions);

	/** @brief
	 *  Move elements to the specified positions
	 *
	 *  @param elements : List of elements.
	 *  @param positions: List of positions.
	 */
	void bulkMoveElements(const ELEMENT_LIST &elements, const std::vector<int> &positions);

	/** @brief
	 *  Rename this dimension
	 *
	 *  @param newName : new Name of the dimension.
	 */
	void rename(const std::string& newName);

	/** @brief
	 *  clear this dimension
	 */
	void clear();

	/** @brief
	 *  clear this dimension
	 */
	void clear(ELEMENT_INFO::TYPE type);

	/** @brief
	 *  Completely remove this dimension
	 *
	 *  @param type : type of elements to destroy.
	 */
	bool destroy();

	ElementExList subset(Server *server, const AliasFilterSettings& alias, const FieldFilterSettings& field, const BasicFilterSettings& basic, const DataFilterSettings& data, const SortingFilterSettings& sorting, const StructuralFilterSettings& structural, const TextFilterSettings& text);
	void subset(ElementExList& retval, Server *server, const AliasFilterSettings& alias, const FieldFilterSettings& field, const BasicFilterSettings& basic, const DataFilterSettings& data, const SortingFilterSettings& sorting, const StructuralFilterSettings& structural, const TextFilterSettings& text);

	/** @brief
	 *  Get an iterator for the elements of this dimension.
	 */
	std::unique_ptr<DimensionCache::CacheIterator> getIterator();

	unsigned int getSequenceNumberFromCache() const;

	void dfilter(ElementExList &subset, const std::string &cube, std::vector<std::vector<std::string> > &coords, unsigned long mode, const std::string &condition, int top, double highPer, double lowPer);

	std::list<ELEMENT_INFO> getElements(IdentifierType parentid = -1, IdentifierType start = 0, IdentifierType limit = -1) const;
	std::list<ELEMENT_INFO> getElements(IdentifierType parentid, IdentifierType &start, const std::string &element, IdentifierType limit) const;
	std::list<ELEMENT_INFO_PERM> getElementsPerm(bool showPermission) const;
	size_t getTopCount() const;
	size_t getFlatCount() const;

	void unlock();
	DIMENSION_INFO_PERMISSIONS getDimensionInfo(bool showPermission);

private:
	std::unique_ptr<std::istringstream> getElementsIntern(IdentifierType parentid, IdentifierType start, IdentifierType limit, IdentifierType element, bool showPermission) const;
	boost::shared_ptr<ServerImpl> m_ServerImpl;
	boost::shared_ptr<PaloClient> m_PaloClient;
	IdentifierType m_Database;
	IdentifierType m_Dimension;
	boost::shared_ptr<const SIDimensions> m_Cache;
};

} /* palo */
} /* jedox */

#endif							 // DIMENSION_H
