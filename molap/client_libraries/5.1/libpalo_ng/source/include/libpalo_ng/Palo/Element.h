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

#ifndef ELEMENT_H
#define ELEMENT_H

#include <string>

#include "types.h"

namespace jedox {
namespace palo {

class ServerImpl;
class SIElements;
class PaloClient;

/** @brief
 *  Class for performing element operations.
 *  All Strings are UTF8.
 */
class LIBPALO_NG_CLASS_EXPORT Element {
public:

	/** @brief
	 *  Do NOT use explicitly.
	 *  Use Dimension::CreateElement
	 */
	Element(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, IdentifierType db, IdentifierType dim, IdentifierType element, boost::shared_ptr<const SIElements> cache);

	/** @brief
	 *  Do NOT use explicitly.
	 *  Use destroy()
	 */
	~Element();

	const ELEMENT_INFO& getCacheData() const;

	const ELEMENT_INFO getCacheDataCopy() const;

	/** @brief
	 *  Add new children to the element
	 *
	 *  @param children : list of the new children.
	 *  @param weights : Weights of the new children
	 */
	void append(const std::vector<std::string>& children, const std::vector<double>& weights);
	
	/** @brief
	 *  Add new children to the element
	 *
	 *  @param children : list of the new children.
	 *  @param weights : Weights of the new children
	 */
	void append(const ELEMENT_LIST& children, const std::vector<double>& weights);
	
	/** @brief
	 *  Updates the structur of an existing element.
	 *
	 *  @param type : type of the element
	 *  @param children : List of children
	 *  @param weights : Weights of children
	 */
	void replace(ELEMENT_INFO::TYPE type, const std::vector<std::string>& children, const std::vector<double>& weights);

	/** @brief
	 * rename this element.
	 */
	void rename(const std::string& newName);

	/** @brief
	 *  Move this element to a specific position
	 */
	void move(IdentifierType position);

	/** @brief
	 *  Destroy this element, removing it from the dimension
	 */
	bool destroy();

private:
	boost::shared_ptr<ServerImpl> m_ServerImpl;
	boost::shared_ptr<PaloClient> m_PaloClient;
	IdentifierType m_Database;
	IdentifierType m_Dimension;
	IdentifierType m_Element;
	boost::shared_ptr<const SIElements> m_Cache;
};

} /* palo */
} /* jedox */

#endif							 // ELEMENT_H
