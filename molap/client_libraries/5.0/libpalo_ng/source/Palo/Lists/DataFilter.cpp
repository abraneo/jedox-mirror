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
 * \author Frieder Hofmann <frieder.hofmann@jedox.com>
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * \author Jiri Junek <jiri.junek@qbicon.cz>
 * 
 *
 */

#include <string>
#include <vector>
#include <functional>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/limits.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <libpalo_ng/Palo/Database.h>
#include <libpalo_ng/Util/StringUtils.h>

#include "ListBasicException.h"
#include "DataFilter.h"
#include "SubSet.h"
#include "PickList.h"
#include "Filter.h"

namespace jedox {
namespace palo {


DataFilter::DataFilter(SubSet& s, unsigned long int flags) :
	Filter(s, flags, DATA_FILTER_NUMFLAGS, Filter::DATA), m_percentage1(0), m_percentage2(0), m_top_num(0)
{
	if (queryFlag(ONLY_LEAVES) && queryFlag(ONLY_CONSOLIDATED)) {
		throw ListBasicException("Wrong number of dimensions passed to set_coordinates", "Invalid Data filter usage");
	} else if (queryFlag(ONLY_LEAVES)) {
		m_subset_ref.setGlobalFlag(SubSet::DATA_ONLY_LEAVES);
	} else if (queryFlag(ONLY_CONSOLIDATED)) {
		m_subset_ref.setGlobalFlag(SubSet::DATA_ONLY_CONSOLIDATED);
	}
	m_subset_ref.setGlobalFlag(SubSet::DATA_FILTER_ACTIVE);
	if (queryFlag(UPPER_PERCENTAGE) || queryFlag(LOWER_PERCENTAGE) || queryFlag(MID_PERCENTAGE) || queryFlag(TOP)) {
		m_subset_ref.setGlobalFlag(SubSet::DONT_SHOW_DUPLICATES);
	}
	if (queryFlag(DATA_STRING)) {
		m_subset_ref.setGlobalFlag(SubSet::DATA_STRING);
	}
}

/**@brief algorithm:
 -- for each dimension element of our subsets basis-dimension that has not been filtered until now
 -- export the data if "*" has been used, otherwise use get-value
 -- compute the value
 -- if the value does not meet the conditions, filter the element
 -- otherwise save the data-value inside the data-map
 -- throw away the exported data */
void DataFilter::apply()
{
	if (m_source_cube.empty()) {
		throw ListBasicException("No source cube defined", "Wrong data filter usage");
	}
	bool got_empty = m_subset_ref.getConcreteSubset().empty();
	if (!got_empty || m_subset_ref.getDimension().getCacheData().type == DIMENSION_INFO::SYSTEM_ID) {
		m_subset_ref.getDimension().dfilter(m_subset_ref.getConcreteSubset(), m_source_cube, m_coords, filter_flags, m_server_condition, m_top_num, m_percentage1, m_percentage2);
	}
}

void DataFilter::setSourceCube(const std::string& s)
{
	m_source_cube = s;
}

void DataFilter::setOperands(const double& par1, const std::string& op1, const double& par2, const std::string& op2)
{
	set_server_condition(par1, op1, par2, op2);
}

void DataFilter::setOperands(const std::string& par1, const std::string op1, const string& par2, const std::string& op2)
{
	set_server_condition(par1, op1, par2, op2);
}

void DataFilter::setCoordinates(const std::vector<std::vector<std::string> > & coords)
{
	if (m_subset_ref.getDatabase().cube[m_source_cube].getCacheData().dimensions.size() != (coords.size())) {
		throw ListBasicException("Wrong number of dimensions passed to set_coordinates", "Invalid Data filter usage");
	}

	m_coords = coords;
}

void DataFilter::setPercentage(double p1, double p2)
{
	if (!queryFlag(UPPER_PERCENTAGE | LOWER_PERCENTAGE | MID_PERCENTAGE)) {
		if (p1 != 0 && p2 != 0) {
			setFlag(MID_PERCENTAGE);
		} else if (p1 != 0) {
			setFlag(UPPER_PERCENTAGE);
		} else if (p2 != 0) {
			setFlag(LOWER_PERCENTAGE);
		}
	}

	if (p1 < 0 || p1 > 100) {
		p1 = 0;
		resetFlag(MID_PERCENTAGE | UPPER_PERCENTAGE);
	}
	if (p2 < 0 || p2 > 100) {
		p2 = 0;
		resetFlag(MID_PERCENTAGE | LOWER_PERCENTAGE);
	}

	m_percentage1 = p1;
	m_percentage2 = p2;
}

void DataFilter::setTop(unsigned int num)
{
	if (!queryFlag(TOP))
		setFlag(TOP);
	m_top_num = num;
}

void DataFilter::set_server_condition(const std::string& par1, const std::string op1, const std::string& par2, const std::string& op2)
{
	try {
		if ("" == op1) {
			m_server_condition = "";
		} else {
			std::string str_par1, str_par2;
			str_par1 = jedox::util::StringUtils::URLencode(jedox::util::StringUtils::CSVencode(par1));
			if ("" != op2) {
				str_par2 = jedox::util::StringUtils::URLencode(jedox::util::StringUtils::CSVencode(par2));
				m_server_condition = url_encode_operator(op1) + str_par1 + "and" + url_encode_operator(op2) + str_par2;
			} else {
				m_server_condition = url_encode_operator(op1) + str_par1;
			}
		}
	} catch (PaloException &) {
		throw ListBasicException("Wrong usage - can't convert the arguments passed to Data filter!", "Invalid Data filter usage");
	}
}

void DataFilter::set_server_condition(const double& par1, const std::string& op1, const double& par2 /*= 0*/, const std::string& op2 /*= "" */)
{
	if ("" == op1) {
		m_server_condition = "";
	} else {
		std::string str_par1 = util::lexicalConversion(std::string, double, par1);

		if ("" != op2) {
			std::string str_par2 = util::lexicalConversion(std::string, double, par2);
			m_server_condition = url_encode_operator(op1) + str_par1 + "and" + url_encode_operator(op2) + str_par2;

		} else {
			m_server_condition = url_encode_operator(op1) + str_par1;

		}
	}
}

const inline std::string DataFilter::url_encode_operator(const std::string& op) const
{
	std::string url_op;
	if (op == "<") {
		url_op = "%3C";
	} else if (op == ">") {
		url_op = "%3E";
	} else if (op == "<=") {
		url_op = "%3C%3D";
	} else if (op == ">=") {
		url_op = "%3E%3D";
	} else if (op == "=") {
		url_op = "%3D";
	} else if (op == "<>") {
		url_op = "!%3D";
	} else {
		throw ListBasicException("Invalid operator string: " + op, "Invalid Data filter usage");
	}
	return url_op;
}

} //palo

} //jedox
