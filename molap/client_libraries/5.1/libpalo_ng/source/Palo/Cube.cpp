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
 * \author Jiri Junek <jiri.junek@qbicon.cz>
 * 
 *
 */

#include <ios>

#include <boost/shared_ptr.hpp>

#include <libpalo_ng/Palo/Dimensions.h>
#include <libpalo_ng/Palo/Cube.h>
#include <libpalo_ng/Palo/Cache/DatabaseCache.h>
#include <libpalo_ng/Palo/Cache/CubesCache.h>
#include <libpalo_ng/Palo/Cache/ElementCache.h>
#include <libpalo_ng/Palo/Network/PaloClient.h>
#include <libpalo_ng/Util/StringUtils.h>

#include "Exception/TokenOutdatedException.h"
#include "Exception/CacheInvalidException.h"

#include "ServerImpl.h"

namespace jedox {
namespace palo {

/*!
 * \brief
 * little helper functions that return the id or ids of elements
 *
 * \author
 * Frieder Hofmann <frieder.hofmann@jedox.com>
 */
IdentifierType CubeHelper::getElementID(boost::shared_ptr<const SIElements> cache, const std::string& element_name)
{
	return (*cache)[element_name].element;
}

ELEMENT_LIST CubeHelper::getElementIDs(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db, const DIMENSION_LIST& dimension_ids, const ELEMENT_NAME_LIST& element_names)
{
	ELEMENT_LIST return_vec;
	DIMENSION_LIST::size_type dim_ids_size = dimension_ids.size();
	if (element_names.size() != dim_ids_size) {
		throw PaloServerException(WRONGDIMENSIONCOUNT, WRONGDIMENSIONCOUNT, PaloExceptionFactory::ERROR_INVALID_COORDINATES);
	}
	return_vec.reserve(dim_ids_size);
	for (DIMENSION_LIST::size_type i = 0; i < dim_ids_size; ++i) {
		return_vec.push_back(getElementID(serverImpl->getElements(paloClient, db, dimension_ids[i]), element_names[i]));
	}
	return return_vec;
}

ELEMENT_LIST CubeHelper::getElementIDs(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db, const unsigned int dimension_id, const ELEMENT_NAME_LIST& element_names)
{
	ELEMENT_LIST return_vec;
	ELEMENT_NAME_LIST::size_type names_size = element_names.size();
	return_vec.reserve(names_size);
	boost::shared_ptr<const SIElements> dim = serverImpl->getElements(paloClient, db, dimension_id);
	for (ELEMENT_NAME_LIST::size_type i = 0; i < names_size; ++i) {
		return_vec.push_back(getElementID(dim, element_names[i]));
	}
	return return_vec;
}

std::vector<ELEMENT_LIST> CubeHelper::getElementIDs(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db, const DIMENSION_LIST& dimension_ids, const std::vector<ELEMENT_NAME_LIST>& element_names)
{
	std::vector<ELEMENT_LIST> return_vec;
	DIMENSION_LIST::size_type dim_ids_size = dimension_ids.size();
	if (element_names.size() != dim_ids_size) {
		throw PaloServerException(WRONGDIMENSIONCOUNT, WRONGDIMENSIONCOUNT, PaloExceptionFactory::ERROR_INVALID_COORDINATES);
	}
	return_vec.reserve(dim_ids_size);
	for (DIMENSION_LIST::size_type i = 0; i < dim_ids_size; ++i) {
		return_vec.push_back(getElementIDs(serverImpl, paloClient, db, dimension_ids[i], element_names[i]));
	}
	return return_vec;
}

std::vector<ELEMENT_LIST> CubeHelper::getCoordIDs(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db, const DIMENSION_LIST& dimension_ids, const std::vector<std::vector<std::string> > &coords)
{
	std::vector<ELEMENT_LIST> return_vec;
	return_vec.reserve(coords.size());
	for (std::vector<std::vector<std::string> >::const_iterator it = coords.begin(); it != coords.end(); ++it) {
		return_vec.push_back(getElementIDs(serverImpl, paloClient, db, dimension_ids, *it));
	}
	return return_vec;
}

void CubeHelper::coordinates2query(const std::vector<std::vector<std::string> > & coordinates, std::stringstream &query, boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db, const DIMENSION_LIST& dimlist)
{
	const size_t dsize = dimlist.size(), coord_size = coordinates.size();

	std::vector<boost::shared_ptr<const SIElements> > elemlists;
	elemlists.reserve(dimlist.size());
	for (DIMENSION_LIST::const_iterator it = dimlist.begin(); it != dimlist.end(); ++it) {
		elemlists.push_back(serverImpl->getElements(paloClient, db, *it));
	}

	C_ARRAY_ARRAY_LONG coord(coord_size);
	for (size_t i = 0; i < coord_size; i++) {
		const size_t vsize2 = coordinates[i].size();

		if (vsize2 != dsize) {
			const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
			const std::string errormsg = WRONGDIMENSIONCOUNT;
			throw PaloServerException(errormsg, errormsg, errorcode);
		}

		(*(coord.m_a + i)).init(vsize2);

		for (size_t j = 0; j < vsize2; j++) {
			*((*(coord.m_a + i)).m_a + j) = CubeHelper::getElementID(elemlists[j], coordinates[i][j]);
		}
	}
	query << coord.ListeListe(',', ':', false);
}

const std::string Cube::CELL_PROPERTIES_DIM_NAME = "#_CELL_PROPERTIES_";

Cube::Cube(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int cube) :
	m_ServerImpl(serverImpl), m_PaloClient(paloClient), m_Dat(db), m_Cube(cube), m_Cache(serverImpl->getCubes(m_PaloClient, db))
{
}

void Cube::rename(const std::string& newName)
{
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&new_name=";
	jedox::util::URLencoder(query, newName);

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		m_PaloClient->request("/cube/rename", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

void Cube::ActualInfo(CUBE_INFO &ci)
{
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&complete=1";

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cube/info", query.str(), token, sequencenumber, dummy);
		(*stream) >> csv >> ci;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

void Cube::ActualInfoPerm(CUBE_INFO_PERMISSIONS &ci, bool showPermission)
{
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&complete=1";
	if (showPermission) {
		query << "&show_permission=1";
	} else {
		query << "&show_permission=0";
	}

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cube/info", query.str(), token, sequencenumber, dummy);
		(*stream) >> csv >> ci;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

void Cube::clear()
{
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&complete=1";

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		m_PaloClient->request("/cube/clear", query.str(), token, sequencenumber, dummy);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

void Cube::clear(const std::vector<std::vector<std::string> > & coordinates)
{
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;
	std::vector<long> elemids;

	if (coordinates.size() != dimlist.size()) {
		const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
		const std::string errormsg = WRONGDIMENSIONCOUNT;
		throw PaloServerException(errormsg, errormsg, errorcode);
	}

	std::vector<ELEMENT_LIST> coord(CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, coordinates));

	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&complete=0" << "&area=";
	jedox::util::ListeTListe(query, coord, ':', ',', true);

	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cube/clear", query.str(), token, sequencenumber, dummy);
		CUBE_INFO cubeinfo;
		(*stream) >> csv >> cubeinfo;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

bool Cube::CellReplace(const std::vector<std::string> & elements, const CELL_VALUE& cellvalue, SPLASH_MODE splashmode, bool add, bool eventprocessor)
{
	static std::vector<std::vector<std::string> > lockedCoordinates;
	return CellReplaceWithLock(elements, cellvalue, splashmode, add, eventprocessor, lockedCoordinates);
}

bool Cube::CellReplaceWithLock(const std::vector<std::string> & elements, const CELL_VALUE& cellvalue, SPLASH_MODE splashmode, bool add, bool eventprocessor, const std::vector<std::vector<std::string> > &lockedCoordinates)
{
	char result = 0;
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;

	if (elements.size() != dimlist.size()) {
		const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
		const std::string errormsg = WRONGDIMENSIONCOUNT;
		throw PaloServerException(errormsg, errormsg, errorcode);
	}

	ELEMENT_LIST elemids(CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, elements));

	std::stringstream query;

	query.setf(std::ios_base::fixed, std::ios_base::floatfield);
	query.precision(PRECISION);
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&path=";
	jedox::util::TListe(query, elemids, ',', false);
	query << "&splash=" << splashmode << "&value=";

	if (cellvalue.type == CELL_VALUE::NUMERIC) {
		query << jedox::util::StringUtils::Numeric2String(cellvalue.val.d);
	} else {
		jedox::util::URLencoder(query, cellvalue.val.s);
	}

	query << "&add=" << int(add) << "&event_processor=" << int(eventprocessor);

	if (!lockedCoordinates.empty()) {
		query << "&locked_paths=";
		CubeHelper::coordinates2query(lockedCoordinates, query, m_ServerImpl, m_PaloClient, m_Dat, dimlist);
	}

	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/replace", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
		return result == '1';
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

bool Cube::CellReplace(const std::vector<std::string> & elements, const CELL_VALUE& cellvalue, SPLASH_MODE splashmode)
{
	return Cube::CellReplace(elements, cellvalue, splashmode, false, true);
}

bool Cube::CellReplace(const ELEMENT_LIST & elements, const CELL_VALUE& cellvalue, SPLASH_MODE splashmode, bool add, bool eventprocessor)
{
	char result = 0;
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;

	if (elements.size() != dimlist.size()) {
		const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
		const std::string errormsg = WRONGDIMENSIONCOUNT;
		throw PaloServerException(errormsg, errormsg, errorcode);
	}

	std::stringstream query;

	query.setf(std::ios_base::fixed, std::ios_base::floatfield);
	query.precision(PRECISION);
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&path=";
	jedox::util::TListe(query, elements, ',', false);
	query << "&splash=" << splashmode << "&value=";

	if (cellvalue.type == CELL_VALUE::NUMERIC) {
		query << jedox::util::StringUtils::Numeric2String(cellvalue.val.d);
	} else {
		jedox::util::URLencoder(query, cellvalue.val.s);
	}

	query << "&add=" << int(add) << "&event_processor=" << int(eventprocessor);

	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/replace", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
		return result == '1';
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

bool Cube::CellReplace(const ELEMENT_LIST & elements, const CELL_VALUE& cellvalue, SPLASH_MODE splashmode)
{
	return Cube::CellReplace(elements, cellvalue, splashmode, false, true);
}

bool Cube::CellReplaceBulk(const std::vector<std::vector<std::string> > & coordinates, const std::vector<CELL_VALUE>& cellvalues, SPLASH_MODE splashmode, bool add, bool eventprocessor)
{
	static std::vector<std::vector<std::string> > emptyCoordinates;
	return CellReplaceBulkWithLock(coordinates, cellvalues, splashmode, add, eventprocessor, emptyCoordinates);
}

bool Cube::CellReplaceBulkWithLock(const std::vector<std::vector<std::string> > & coordinates, const std::vector<CELL_VALUE>& cellvalues, SPLASH_MODE splashmode, bool add, bool eventprocessor, const std::vector<std::vector<std::string> > &lockedCoordinates)
{
	char result = 0;
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&splash=" << splashmode << "&paths=";
	CubeHelper::coordinates2query(coordinates, query, m_ServerImpl, m_PaloClient, m_Dat, dimlist);
	query  << "&values=";
	jedox::util::TListe(query, cellvalues, ':');
	query << "&add=" << int(add) << "&event_processor=" << int(eventprocessor);
	if (!lockedCoordinates.empty()) {
		query << "&locked_paths=";
		CubeHelper::coordinates2query(lockedCoordinates, query, m_ServerImpl, m_PaloClient, m_Dat, dimlist);
	}

	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/replace_bulk", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
		return result == '1';
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

bool Cube::CellReplaceBulk(const std::vector<std::vector<std::string> > & coordinates, const std::vector<CELL_VALUE>& cellvalues, SPLASH_MODE splashmode)
{
	return Cube::CellReplaceBulk(coordinates, cellvalues, splashmode, false, true);
}

void Cube::CellValue(CELL_VALUE &result, const ELEMENT_LIST & elemids, unsigned short showRule, unsigned short showLockState, bool ForceServerCall)
{
	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&path=";
	jedox::util::TListe(query, elemids, ',', false);
	query << "&show_rule=" << ((showRule + showLockState > 0) ? 1 : 0) << "&show_lock_info=" << ((showLockState > 0) ? 1 : 0);
	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/value", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

void Cube::CellValue(CELL_VALUE &result, const std::vector<std::string> & elements, unsigned short showRule, unsigned short showLockState, bool ForceServerCall)
{
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;
	size_t vsize = elements.size();
	std::vector<long> elemids(vsize);

	if (vsize != dimlist.size()) {
		const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
		const std::string errormsg = WRONGDIMENSIONCOUNT;
		throw PaloServerException(errormsg, errormsg, errorcode);
	}

	Cube::CellValue(result, CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, elements), showRule, showLockState, ForceServerCall);
}

CELL_VALUE Cube::CellValue(const std::vector<std::string> & elements, unsigned short showRule, unsigned short showLockState)
{
	CELL_VALUE result;
	CellValue(result, elements, showRule, showLockState, false);
	return result;
}

void Cube::CellValueProps(CELL_VALUE_PROPS &result, const std::vector<std::string> &elements, const std::vector<std::string> &properties)
{
	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&path=";
	jedox::util::TListe(query, CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, cube.dimensions, elements), ',', false);
	query << "&show_rule=1&show_lock_info=1";
	if (m_ServerImpl->existDim(m_PaloClient, m_Dat, CELL_PROPERTIES_DIM_NAME)) {
		query << "&properties=";
		jedox::util::TListe(query, CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, m_ServerImpl->getDim(m_PaloClient, m_Dat, CELL_PROPERTIES_DIM_NAME).dimension, properties), ',', false);
	}
	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/value", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
		if (result.prop_vals.size() != properties.size()) {
			result.prop_vals.resize(properties.size());
		}
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

void Cube::CellArea(std::vector<CELL_VALUE_PATH>& res, const std::vector<ELEMENT_LIST> & coord, unsigned short showRule, unsigned short showLockState)
{
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&area=";
	jedox::util::ListeTListe(query, coord, ':', ',', true);
	query << "&show_rule=" << ((showRule + showLockState > 0) ? 1 : 0) << "&show_lock_info=" << ((showLockState > 0) ? 1 : 0);

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/area", query.str(), token, sequencenumber, dummy);
		while ((*stream).eof() == false) {
			CELL_VALUE_PATH tmp;
			(*stream) >> csv >> tmp;
			res.push_back(tmp);
		}
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

std::vector<CELL_VALUE_PATH> Cube::CellArea(const std::vector<std::vector<std::string> > & coordinates, unsigned short showRule, unsigned short showLockState)
{
	std::vector<CELL_VALUE_PATH> res;
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;

	if (coordinates.size() != dimlist.size()) {
		const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
		const std::string errormsg = WRONGDIMENSIONCOUNT;
		throw PaloServerException(errormsg, errormsg, errorcode);
	}

	CellArea(res, CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, coordinates), showRule, showLockState);
	return res;
}

void Cube::CellAreaProps(std::vector<CELL_VALUE_PATH_PROPS> &result, const std::vector<std::vector<std::string> > &coordinates, const std::vector<std::string> &properties)
{
	CellAreaPropsAggr(result, coordinates, properties, AGGREGATION_SUM, std::vector<EXPANDAGGR_TYPE>());
}

void Cube::CellAreaPropsAggr(std::vector<CELL_VALUE_PATH_PROPS> &result, const std::vector<std::vector<std::string> > &coordinates, const std::vector<std::string> &properties, AGGREGATION_TYPE aggr, const std::vector<EXPANDAGGR_TYPE> &expand)
{
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&area=";
	const CubeCache &cube = (*m_Cache)[m_Cube];
	jedox::util::ListeTListe(query, CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, cube.dimensions, coordinates), ':', ',', true);
	query << "&show_rule=1&show_lock_info=1";
	if (aggr) {
		query << "&function=" << aggr << "&expand=";
		for (std::vector<EXPANDAGGR_TYPE>::const_iterator it = expand.begin(); it != expand.end(); ++it) {
			if (it != expand.begin()) {
				query << ",";
			}
			query << *it;
		}
	}
	if (m_ServerImpl->existDim(m_PaloClient, m_Dat, CELL_PROPERTIES_DIM_NAME)) {
		query << "&properties=";
		jedox::util::TListe(query, CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, m_ServerImpl->getDim(m_PaloClient, m_Dat, CELL_PROPERTIES_DIM_NAME).dimension, properties), ',', false);
	}

	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/area", query.str(), token, sequencenumber, dummy);
		while ((*stream).eof() == false) {
			CELL_VALUE_PATH_PROPS tmp;
			(*stream) >> csv >> tmp;
			if (tmp.prop_vals.size() != properties.size()) {
				tmp.prop_vals.resize(properties.size());
			}
			result.push_back(tmp);
		}
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

/*
 Do not throw exceptions if some coordinates are wrong, but pass the coordinates on.
 This is done to make loading of data possible even if a few inputs are wrong.
 */
std::vector<CELL_VALUE> Cube::CellValues(const std::vector<std::vector<std::string> > & coordinates, unsigned short showRule, unsigned short showLockState)
{
	std::vector<CELL_VALUE> res;
	CellValues(res, coordinates, showRule, showLockState);
	return res;
}

void Cube::CellValues(std::vector<CELL_VALUE> & res, const std::vector<ELEMENT_LIST> & coord, unsigned short showRule, unsigned short showLockState)
{
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&paths=";
	jedox::util::ListeTListe(query, coord, ',', ':', false);
	query << "&show_rule=" << ((showRule + showLockState > 0) ? 1 : 0) << "&show_lock_info=" << ((showLockState > 0) ? 1 : 0);

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/values", query.str(), token, sequencenumber, dummy);
		size_t i = 0, max = coord.size();

		res.reserve(max);
		CELL_VALUE tmp;
		while ((i < max) && (*stream).eof() == false) {
			(*stream) >> csv >> tmp;
			res.push_back(tmp);
			++i;
		}
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

void Cube::CellValues(std::vector<CELL_VALUE> & res, const std::vector<std::vector<std::string> > & coordinates, unsigned short showRule, unsigned short showLockState)
{
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;
	std::vector<boost::shared_ptr<const SIElements> > elemlists;
	elemlists.reserve(dimlist.size());
	for (DIMENSION_LIST::const_iterator it = dimlist.begin(); it != dimlist.end(); ++it) {
		elemlists.push_back(m_ServerImpl->getElements(m_PaloClient, m_Dat, *it));
	}
	size_t vsize = coordinates.size(), vsize2, vsize3, dsize = dimlist.size();
	ELEMENT_LIST elemids;
	std::vector<ELEMENT_LIST> coord(vsize);
	for (size_t i = 0; i < vsize; i++) {
		vsize2 = coordinates[i].size();
		elemids.resize(vsize2);
		vsize3 = (vsize2 < dsize) ? vsize2 : dsize;
		for (size_t j = 0; j < vsize3; j++) {
			try {
				elemids[j] = (*elemlists[j])[coordinates[i][j]].element;
			} catch (ElementNotFoundException ) {
				elemids[j] = -1;
			}
		}
		for (size_t j = vsize3; j < vsize2; j++) {
			elemids[j] = -1;
		}
		coord[i] = elemids;
	}
	CellValues(res, coord, showRule, showLockState);
}

/*
 Do not throw exceptions if some coordinates are wrong, but pass the coordinates on.
 This is done to make loading of data possible even if a few inputs are wrong.
 */
void Cube::CellValues(C_2DARRAY_LONG& coord, Cell_Values_C* cvrc)
{
	if (coord.m_a == NULL) {
		cvrc->a = NULL;
		cvrc->len = 0;
		return;
	}

	cvrc->len = coord.m_rows;
	cvrc->a = (Cell_Value_C*)calloc(cvrc->len, sizeof(Cell_Value_C));

	if (cvrc->a == NULL) {
		cvrc->len = 0;
		return;
	}

	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&paths=" << coord.ListeListe(',', ':', false) << "&show_rule=0";
	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/values", query.str(), token, sequencenumber, dummy);

		size_t i = 0, maxsize = cvrc->len;
		while (((*stream).eof() == false) && (i < maxsize)) {
			(*stream) >> csv >> *(cvrc->a + i++);
		}
		cvrc->len = i;
		return;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

/*
 Do not throw exceptions if some coordinates are wrong, but pass the coordinates on.
 This is done to make loading of data possible even if a few inputs are wrong.
 */
void Cube::CellValues(const Cell_Values_Coordinates& coordinates, Cell_Values_C* cvrc)
{
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;
	size_t i, j, k, vsize = coordinates.rows, vsize2 = coordinates.cols, dsize = dimlist.size(), vsize3 = (vsize2 < dsize) ? vsize2 : dsize;
	C_2DARRAY_LONG coord(vsize, vsize2);

	std::vector<boost::shared_ptr<const SIElements> > elemlists;
	elemlists.reserve(dimlist.size());
	for (DIMENSION_LIST::const_iterator it = dimlist.begin(); it != dimlist.end(); ++it) {
		elemlists.push_back(m_ServerImpl->getElements(m_PaloClient, m_Dat, *it));
	}
	for (i = 0; i < vsize; i++) {
		k = i * vsize2;
		for (j = 0; j < vsize3; j++) {
			try {
				*(coord.m_a + k + j) = (*elemlists[j])[*(coordinates.a + (i * vsize2 + j))].element;
			} catch (ElementNotFoundException ) {
				*(coord.m_a + k + j) = -1;
			}

		}

		for (j = vsize3; j < vsize2; j++) {
			*(coord.m_a + k + j) = -1;
		}
	}

	CellValues(coord, cvrc);
}

void Cube::CellValuesProps(std::vector<CELL_VALUE_PROPS> &res, const std::vector<std::vector<std::string> > &coordinates, const std::vector<std::string> &properties)
{
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&paths=";
	const CubeCache &cube = (*m_Cache)[m_Cube];
	jedox::util::ListeTListe(query, CubeHelper::getCoordIDs(m_ServerImpl, m_PaloClient, m_Dat, cube.dimensions, coordinates), ',', ':', false);
	query << "&show_rule=1&show_lock_info=1";
	if (m_ServerImpl->existDim(m_PaloClient, m_Dat, CELL_PROPERTIES_DIM_NAME)) {
		query << "&properties=";
		jedox::util::TListe(query, CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, m_ServerImpl->getDim(m_PaloClient, m_Dat, CELL_PROPERTIES_DIM_NAME).dimension, properties), ',', false);
	}

	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/values", query.str(), token, sequencenumber, dummy);
		size_t i = 0, max = coordinates.size();

		res.reserve(max);
		while ((i < max) && (*stream).eof() == false) {
			CELL_VALUE_PROPS tmp;
			(*stream) >> csv >> tmp;
			if (tmp.prop_vals.size() != properties.size()) {
				tmp.prop_vals.resize(properties.size());
			}
			res.push_back(tmp);
			++i;
		}
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}
void Cube::CellExport(std::vector<CELL_VALUE_EXPORTED> &result, const std::vector<ELEMENT_LIST> &coordinates, unsigned long blocksize, const ELEMENT_LIST &start, const std::string condition, unsigned short BaseOnly, unsigned short SkipEmpty, unsigned short use_rules)
{
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;
	size_t vsize = coordinates.size(), dsize = dimlist.size();

	if (vsize != dsize) {
		const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
		const std::string errormsg = WRONGDIMENSIONCOUNT;
		throw PaloServerException(errormsg, errormsg, errorcode);
	}

	vsize = start.size();
	if (vsize > 0) {
		if (vsize != dsize) {
			const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
			const std::string errormsg = WRONGDIMENSIONCOUNT;
			throw PaloServerException(errormsg, errormsg, errorcode);
		}
	}

	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&area=";
	jedox::util::ListeTListe(query, coordinates, ':', ',', true);
	query << "&blocksize=" << blocksize << "&base_only=" << BaseOnly << "&skip_empty=" << SkipEmpty << "&use_rules=" << use_rules;

	if (vsize > 0) {
		query << "&path=";
		jedox::util::TListe(query, start, ',', false);
	}

	if (!condition.empty()) {
		query << "&condition=" << condition;
	}

	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/export", query.str(), token, sequencenumber, dummy);
		size_t i = 0, max = blocksize + 1;
		result.reserve(max);

		CELL_VALUE_EXPORTED tmp;
		while ((i < max) && ((*stream).eof() == false)) {
			(*stream) >> csv >> tmp;
			result.push_back(tmp);
			++i;
		}

		result.resize(i);
		//ensure that result only uses the memory it really needs.
		std::vector<CELL_VALUE_EXPORTED>(result).swap(result);

	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

void Cube::CellExport(std::vector<CELL_VALUE_EXPORTED> &result, const std::vector<std::vector<std::string> > &coordinates, unsigned long blocksize, const std::vector<std::string> &start, const std::string condition, unsigned short BaseOnly, unsigned short SkipEmpty, unsigned short use_rules)
{
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST &dimlist = cube.dimensions;
	ELEMENT_LIST elemids;

	std::vector<ELEMENT_LIST> coord(CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, coordinates));
	size_t vsize = start.size();
	elemids.resize(vsize);

	if (vsize > 0) {
		elemids = CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, start);
	}
	CellExport(result, coord, blocksize, elemids, condition, BaseOnly, SkipEmpty, use_rules);
}

void Cube::DataFeedExport( std::vector<CELL_VALUE_EXPORTED> &result, unsigned long blocksize, const ELEMENT_LIST& start, unsigned short SkipEmpty )
{
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;
	size_t vsize = dimlist.size();

	if ( !start.empty() && vsize != start.size() ){
		const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
		const std::string errormsg = WRONGDIMENSIONCOUNT;
		throw PaloServerException(errormsg, errormsg, errorcode);
	}

	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube;
	query << "&blocksize=" << blocksize << "&skip_empty=" << SkipEmpty << "&use_rules=" << 1 <<"&type=1";

	if ( !start.empty() ){
		query << "&path=";
		jedox::util::TListe(query, start, ',', false);
	}

	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/export", query.str(), token, sequencenumber, dummy);
		size_t i = 0, max = blocksize + 1;
		result.reserve(max);

		CELL_VALUE_EXPORTED tmp;
		while ((i < max) && ((*stream).eof() == false)) {
			(*stream) >> csv >> tmp;
			result.push_back(tmp);
			++i;
		}

		result.resize(i);
		//ensure that result only uses the memory it really needs.
		std::vector<CELL_VALUE_EXPORTED>(result).swap(result);

	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

std::vector<CELL_VALUE_EXPORTED> Cube::CellExport(const std::vector<std::vector<std::string> > & coordinates, unsigned long blocksize, const std::vector<std::string>& start, const std::string condition, unsigned short BaseOnly, unsigned short SkipEmpty, unsigned short use_rules)
{
	std::vector<CELL_VALUE_EXPORTED> res;
	CellExport(res, coordinates, blocksize, start, condition, BaseOnly, SkipEmpty, use_rules);
	return res;
}

void Cube::CellExportProps(std::vector<CELL_VALUE_EXPORTED_PROPS> &result, const std::vector<std::vector<std::string> > &coordinates, const std::vector<std::string> &properties, unsigned long blocksize, const std::vector<std::string>& start, const std::string condition, unsigned short BaseOnly, unsigned short SkipEmpty, unsigned short use_rules)
{
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;
	size_t vsize = coordinates.size(), dsize = dimlist.size();
	ELEMENT_LIST elemids;

	if (vsize != dsize) {
		const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
		const std::string errormsg = WRONGDIMENSIONCOUNT;
		throw PaloServerException(errormsg, errormsg, errorcode);
	}

	std::vector<ELEMENT_LIST> coord(CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, coordinates));

	vsize = start.size();
	elemids.resize(vsize);

	if (vsize > 0) {
		if (vsize != dsize) {
			const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
			const std::string errormsg = WRONGDIMENSIONCOUNT;
			throw PaloServerException(errormsg, errormsg, errorcode);
		}

		elemids = CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, start);
	}

	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&area=";
	jedox::util::ListeTListe(query, coord, ':', ',', true);
	query << "&blocksize=" << blocksize << "&base_only=" << BaseOnly << "&skip_empty=" << SkipEmpty << "&use_rules=" << use_rules;

	if (vsize > 0) {
		query << "&path=";
		jedox::util::TListe(query, elemids, ',', false);
	}
	if (!condition.empty()) {
		query << "&condition=" << condition;
	}
	if (m_ServerImpl->existDim(m_PaloClient, m_Dat, CELL_PROPERTIES_DIM_NAME)) {
		query << "&properties=";
		jedox::util::TListe(query, CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, m_ServerImpl->getDim(m_PaloClient, m_Dat, CELL_PROPERTIES_DIM_NAME).dimension, properties), ',', false);
	}

	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/export", query.str(), token, sequencenumber, dummy);
		size_t i = 0, max = blocksize + 1;
		result.reserve(max);

		while ((i < max) && ((*stream).eof() == false)) {
			CELL_VALUE_EXPORTED_PROPS tmp;
			(*stream) >> csv >> tmp;
			if (tmp.prop_vals.size() != properties.size()) {
				tmp.prop_vals.resize(properties.size());
			}
			result.push_back(tmp);
			++i;
		}

		result.resize(i);
		//ensure that result only uses the memory it really needs.
		std::vector<CELL_VALUE_EXPORTED_PROPS>(result).swap(result);

	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

bool Cube::CellGoalSeek(const std::vector<std::string> & path, double value)
{
	return CellGoalSeek(path, value, GOALSEEK_COMPLETE, std::vector<std::vector<std::string> >());
}

bool Cube::CellGoalSeek(const std::vector<std::string> & path, double value, GOALSEEK_TYPE type, const std::vector<std::vector<std::string> >& area)
{
	char result = 0;
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;
	size_t vsize = path.size(), dsize = dimlist.size();

	if (vsize != dsize) {
		const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
		const std::string errormsg = WRONGDIMENSIONCOUNT;
		throw PaloServerException(errormsg, errormsg, errorcode);
	}

	std::stringstream query;
	query.setf(std::ios_base::fixed, std::ios_base::floatfield);
	query.precision(PRECISION);

	query << "database=" << m_Dat << "&cube=" << m_Cube << "&path=";
	jedox::util::TListe(query, CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, path), ',', false);
	query << "&value=" << value;
	if (type != GOALSEEK_COMPLETE) {
		query << "&type=" << type << "&area=";
		jedox::util::ListeTListe(query, CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, area), ':', ',', true);
	}
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/goalseek", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
		return result == '1';
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

bool Cube::CellCopy(const std::vector<std::string> & path, const std::vector<std::string> & path_to, double value)
{
	std::vector<std::vector<std::string> > lockedCoordinates;
	return CellCopy(COPY, &path, 0, path_to, &value, false, lockedCoordinates);
}

bool Cube::CellCopy(const std::vector<std::string> & path, const std::vector<std::string> & path_to, double value, bool userule)
{
	std::vector<std::vector<std::string> > lockedCoordinates;
	return CellCopy(COPY, &path, 0, path_to, &value, userule, lockedCoordinates);
}

bool Cube::CellCopy(const std::vector<std::string> & path, const std::vector<std::string> & path_to)
{
	std::vector<std::vector<std::string> > lockedCoordinates;
	return CellCopy(COPY, &path, 0, path_to, (double *)0, false, lockedCoordinates);
}

bool Cube::CellCopy(const std::vector<std::string> & path, const std::vector<std::string> & path_to, bool userule)
{
	std::vector<std::vector<std::string> > lockedCoordinates;
	return CellCopy(COPY, &path, 0, path_to, (double *)0, userule, lockedCoordinates);
}

bool Cube::CellCopy(const std::vector<std::string>& path, const std::vector<std::string>& path_to, double value, bool userule, const std::vector<std::vector<std::string> > &lockedCoordinates)
{
	return CellCopy(COPY, &path, 0, path_to, &value, userule, lockedCoordinates);
}

bool Cube::CellCopy(const std::vector<std::string>& path, const std::vector<std::string>& path_to, bool userule, const std::vector<std::vector<std::string> > &lockedCoordinates)
{
	return CellCopy(COPY, &path, 0, path_to, (double *)0, userule, lockedCoordinates);
}

bool Cube::CellCopy(const std::vector<std::string>& path, const std::vector<std::string>& path_to, double value, const std::vector<std::vector<std::string> > &lockedCoordinates)
{
	return CellCopy(COPY, &path, 0, path_to, &value, false, lockedCoordinates);
}

bool Cube::CellCopy(const std::vector<std::string>& path, const std::vector<std::string>& path_to, const std::vector<std::vector<std::string> > &lockedCoordinates)
{
	return CellCopy(COPY, &path, 0, path_to, (double *)0, false, lockedCoordinates);
}

bool Cube::CellPredictLinearRegression(const std::vector<std::vector<std::string> > &coordinates, const std::vector<std::string>& path_to, bool userule, const std::vector<std::vector<std::string> > &lockedCoordinates)
{
	return CellCopy(PREDICT_LINEAR_REGRESSION, 0, &coordinates, path_to, (double *)0, userule, lockedCoordinates);
}

bool Cube::CellCopy(COPY_FUNCTION func, const std::vector<std::string> *path, const std::vector<std::vector<std::string> > *area, const std::vector<std::string> & path_to, double *value, bool userule, const std::vector<std::vector<std::string> > &lockedCoordinates)
{
	char result = 0;
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;

	std::stringstream query;
	query.setf(std::ios_base::fixed, std::ios_base::floatfield);
	query.precision(PRECISION);

	query << "database=" << m_Dat << "&cube=" << m_Cube << "&function=" << func;

	switch (func) {
	case COPY:
	{
		if (path) {
			if (path->size() != dimlist.size()) {
				throw PaloServerException(WRONGDIMENSIONCOUNT, WRONGDIMENSIONCOUNT, PaloExceptionFactory::ERROR_INVALID_COORDINATES);
			}
			query << "&path=";
			jedox::util::TListe(query, CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, *path), ',', false);
		}
		break;
	}
	case PREDICT_LINEAR_REGRESSION:
	{
		if (area) {
			if (area->size() != dimlist.size()) {
				throw PaloServerException(WRONGDIMENSIONCOUNT, WRONGDIMENSIONCOUNT, PaloExceptionFactory::ERROR_INVALID_COORDINATES);
			}
			query << "&area=";
			jedox::util::ListeTListe(query, CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, *area), ':', ',', true);
		}
		break;
	}
	default:
		throw PaloServerException("Invalid predict function.", "Invalid predict function.", PaloExceptionFactory::ERROR_INVALID_PREDICT_FUNCTION);
		break;
	}

	if (path_to.size() != dimlist.size()) {
		throw PaloServerException(WRONGDIMENSIONCOUNT, WRONGDIMENSIONCOUNT, PaloExceptionFactory::ERROR_INVALID_COORDINATES);
	}

	query << "&path_to=";
	jedox::util::TListe(query, CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, path_to), ',', false);
	if (value) {
		query << "&value=" << *value;
	}
	if (userule) {
		query << "&use_rules=1";
	}
	if (!lockedCoordinates.empty()) {
		query << "&locked_paths=";
		CubeHelper::coordinates2query(lockedCoordinates, query, m_ServerImpl, m_PaloClient, m_Dat, dimlist);
	}
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/copy", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
		return result == '1';
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

void Cube::CellDrillThrough(std::vector<DRILLTHROUGH_INFO> &result, const std::vector<std::string>& elements, DRILLTHROUGH_TYPE mode)
{
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;
	size_t vsize = elements.size();

	if (vsize != dimlist.size()) {
		const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
		const std::string errormsg = WRONGDIMENSIONCOUNT;
		throw PaloServerException(errormsg, errormsg, errorcode);
	}

	ELEMENT_LIST elemids(CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, elements));

	unsigned int sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&path=";
	jedox::util::TListe(query, elemids, ',', false);
	query << "&mode=" << mode;
	try {
		CUBE_TOKEN cubetoken(cube.getSequenceNumber());
		unsigned int dummy, sequencenumber = cube.getSequenceNumber();

		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/drillthrough", query.str(), token, sequencenumber, dummy);
		DRILLTHROUGH_INFO di;

		while ((*stream).eof() == false) {
			(*stream) >> csv >> di;
			result.push_back(di);
		}
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

void Cube::MDXCellDrillThrough(std::vector<DRILLTHROUGH_INFO> &result, ELEMENT_LIST elemids, DRILLTHROUGH_TYPE mode)
{
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;
	size_t vsize = elemids.size();

	if (vsize != dimlist.size()) {
		const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
		const std::string errormsg = WRONGDIMENSIONCOUNT;
		throw PaloServerException(errormsg, errormsg, errorcode);
	}

	unsigned int sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&path=";
	jedox::util::TListe(query, elemids, ',', false);
	query << "&mode=" << mode;
	try {
		CUBE_TOKEN cubetoken(cube.getSequenceNumber());
		unsigned int dummy, sequencenumber = cube.getSequenceNumber();

		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cell/drillthrough", query.str(), token, sequencenumber, dummy);
		DRILLTHROUGH_INFO di;

		while ((*stream).eof() == false) {
			(*stream) >> csv >> di;
			result.push_back(di);
		}
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

bool Cube::destroy()
{
	char result = 0;
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube;
	unsigned int dummy, sequencenumber = m_Cache->getSequenceNumber();
	DATABASE_TOKEN token(sequencenumber);
	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cube/destroy", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateDatabases(sequencenumber);
		(*stream) >> result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateDatabases(sequencenumber);
		throw e;
	}
	return result == '1';
}

bool Cube::save()
{
	char result = 0;
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube;
	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);
	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cube/save", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
	return result == '1';
}

bool Cube::unload()
{
	char result = 0;
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube;
	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);
	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cube/unload", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
	return result == '1';
}

bool Cube::load()
{
	char result = 0;
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube;
	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);
	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cube/load", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
	return result == '1';
}

const CUBE_INFO& Cube::getCacheData() const
{
	return (*m_Cache)[m_Cube];
}

const CUBE_INFO Cube::getCacheDataCopy() const
{
	return getCacheData();
}

RULE_INFO Cube::RuleCreate(const std::string& definition, unsigned short use_identifier, std::string extern_id, std::string comment, unsigned short activate, double position)
{
	RULE_INFO result;
	std::stringstream query;

	query << "database=" << m_Dat << "&cube=" << m_Cube << "&definition=";
	jedox::util::URLencoder(query, definition);
	query << "&use_identifier=" << use_identifier << "&external_identifier=";
	jedox::util::URLencoder(query, extern_id);
	query << "&comment=";
	jedox::util::URLencoder(query, comment);
	query << "&activate=" << activate;
	query << "&position=" << position;

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/rule/create", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
		return result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

RULE_INFO Cube::RuleModify(long id, const std::string& definition, unsigned short use_identifier, std::string extern_id, std::string comment, unsigned short activate, double position)
{
	RULE_INFO result;
	std::stringstream query;

	query << "database=" << m_Dat << "&cube=" << m_Cube << "&rule=" << id << "&use_identifier=" << use_identifier << "&external_identifier=";
	jedox::util::URLencoder(query, extern_id);
	query << "&comment=";
	jedox::util::URLencoder(query, comment);
	query << "&activate=" << activate;
	if (position) {
		query << "&position=" << position;
	}

	if (!definition.empty()) {
		query << "&definition=";
		jedox::util::URLencoder(query, definition);
	}

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/rule/modify", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
		return result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

std::vector<RULE_INFO> Cube::RulesMove(const ELEMENT_LIST &ruleIds, double startPosition, double belowPosition)
{
	RULE_INFO result;
	std::vector<RULE_INFO> res;
	std::stringstream query;

	query << "database=" << m_Dat << "&cube=" << m_Cube << "&rule=";
	jedox::util::TListe(query, ruleIds, ',', false);
	query << "&position=" << startPosition;
	if (belowPosition) {
		query << "," << belowPosition;
	}

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/rule/modify", query.str(), token, sequencenumber, dummy);
		while ((*stream).eof() == false) {
			(*stream) >> csv >> result;
			res.push_back(result);
		}
		return res;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

std::vector<RULE_INFO> Cube::RulesActivate(const ELEMENT_LIST &ruleIds, unsigned short activate)
{
	RULE_INFO result;
	std::vector<RULE_INFO> res;
	std::stringstream query;

	query << "database=" << m_Dat << "&cube=" << m_Cube << "&rule=";
	jedox::util::TListe(query, ruleIds, ',', false);
	query << "&activate=" << activate;

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/rule/modify", query.str(), token, sequencenumber, dummy);
		while ((*stream).eof() == false) {
			(*stream) >> csv >> result;
			res.push_back(result);
		}
		return res;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

RULE_INFO Cube::RuleInfo(IdentifierType id, unsigned short use_identifier)
{
	RULE_INFO result;
	std::stringstream query;

	query << "database=" << m_Dat << "&cube=" << m_Cube << "&rule=" << id << "&use_identifier=" << use_identifier;
	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/rule/info", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
		return result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

std::vector<RULE_INFO> Cube::Rules(unsigned short use_identifier)
{
	RULE_INFO result;
	std::vector<RULE_INFO> res;
	std::stringstream query;

	query << "database=" << m_Dat << "&cube=" << m_Cube << "&use_identifier=" << use_identifier;
	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cube/rules", query.str(), token, sequencenumber, dummy);
		while ((*stream).eof() == false) {
			(*stream) >> csv >> result;
			res.push_back(result);
		}
		return res;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

bool Cube::RuleDestroy(IdentifierType identifier)
{
	char result = 0;
	std::stringstream query;

	query << "database=" << m_Dat << "&cube=" << m_Cube << "&rule=" << identifier;
	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/rule/destroy", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
		return result == '1';
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

bool Cube::RulesDestroy(const ELEMENT_LIST &ruleIds)
{
	char result = 0;
	std::stringstream query;

	query << "database=" << m_Dat << "&cube=" << m_Cube << "&rule=";
	jedox::util::TListe(query, ruleIds, ',', false);

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/rule/destroy", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
		return result == '1';
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

std::string Cube::RuleParse(const std::string& definition)
{
	std::stringstream query;

	query << "database=" << m_Dat << "&cube=" << m_Cube << "&definition=";
	jedox::util::URLencoder(query, definition);
	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/rule/parse", query.str(), token, sequencenumber, dummy);
		return (*stream).str();
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

LOCK_INFO Cube::Lock(const std::vector<std::vector<std::string> > & coordinates)
{
	LOCK_INFO result;
	const CubeCache &cube = (*m_Cache)[m_Cube];
	const DIMENSION_LIST& dimlist = cube.dimensions;
	const size_t vsize = coordinates.size();

	if (vsize != dimlist.size()) {
		const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
		const std::string errormsg = WRONGDIMENSIONCOUNT;
		throw PaloServerException(errormsg, errormsg, errorcode);
	}

	std::vector<ELEMENT_LIST> coord(CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Dat, dimlist, coordinates));

	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&area=";
	jedox::util::ListeTListe(query, coord, ':', ',', true);

	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cube/lock", query.str(), token, sequencenumber, dummy);
		(*stream) >> csv >> result;
		return result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

LOCK_INFO Cube::Lock()
{
	LOCK_INFO result;
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&complete=1";

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cube/lock", query.str(), token, sequencenumber, dummy);
		(*stream) >> csv >> result;
		return result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

std::vector<LOCK_INFO> Cube::Locks()
{
	LOCK_INFO result;
	std::vector<LOCK_INFO> res;
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube;

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cube/locks", query.str(), token, sequencenumber, dummy);
		while ((*stream).eof() == false) {
			(*stream) >> csv >> result;
			res.push_back(result);
		}
		return res;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

bool Cube::Commit(IdentifierType LockID)
{
	char result = 0;
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&lock=" << LockID;

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cube/commit", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
	return result == '1';
}

bool Cube::Rollback(IdentifierType LockID)
{
	return Rollback(LockID, -1);
}

bool Cube::Rollback(IdentifierType LockID, long steps)
{
	char result = 0;
	std::stringstream query;

	query << "database=" << m_Dat << "&cube=" << m_Cube << "&lock=" << LockID;

	if (steps > -1) {
		query << "&steps=" << steps;
	}

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cube/rollback", query.str(), token, sequencenumber, dummy);
		(*stream) >> result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
	return result == '1';
}

void Cube::convert(const CUBE_INFO::TYPE cubetype)
{
	std::stringstream query;
	query << "database=" << m_Dat << "&cube=" << m_Cube << "&type=" << cubetype;

	const CubeCache &cube = (*m_Cache)[m_Cube];
	unsigned int dummy, sequencenumber = cube.getSequenceNumber();
	CUBE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/cube/convert", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateCubes(sequencenumber, m_Dat);
		throw e;
	}
}

unsigned int Cube::getSequenceNumberFromCache() const
{
	return (*m_Cache)[m_Cube].getSequenceNumber();
}

} /* palo */
} /* jedox */
