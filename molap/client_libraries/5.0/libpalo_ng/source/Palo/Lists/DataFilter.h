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
 * 
 *
 */

#ifndef __DATAFILTER_H_INCL__
#define __DATAFILTER_H_INCL__

#include <string>
#include <list>
#include <utility>
#include <boost/scoped_ptr.hpp>
#include <map>

#include <libpalo_ng/config_ng.h>
#include <libpalo_ng/Palo/Cube.h>

#include "Filter.h"
#include "SubSet.h"

namespace jedox {
namespace palo {

// DATA_FILTER_NUMFLAGS is defined int types.h

class SubSet;
class DataFilter;


// DataFilterBase is defined in types.h

class LIBPALO_NG_CLASS_EXPORT DataFilter : public DataFilterBase, public Filter {
private:
	typedef std::vector<std::vector<std::string> > coords_vec_type;
	double m_percentage1, m_percentage2;
	unsigned int m_top_num;
	coords_vec_type m_coords;
	std::string m_source_cube;
	std::string m_server_condition;

	// Constructor
	DataFilter(SubSet& s, unsigned long int flag);

	/**@brief apply this filter*/
	virtual void apply();
	void set_server_condition(const double& par1, const std::string& op1, const double& par2 = 0, const std::string& op2 = "");
	void set_server_condition(const std::string& par1, const std::string op1, const std::string& par2 = 0, const std::string& op2 = 0);
	const std::string url_encode_operator(const std::string& op) const;
public:
	friend class SubSet;
	friend class CellArrayGenerator;

	/**@brief Set the cube that serves as our data source */
	void setSourceCube(const std::string&);

	/**@brief set the first and (optionally second) operand and value
	 string parameters are operands like ">" and double parameters are the values
	 to compare with. The two operands are applied together using AND. An element
	 is inside the set if it meets both conditions. The second operand can be left empty. */
	void setOperands(const double& par1, const std::string& op1, const double& par2 = 0, const std::string& op2 = "");

	/**@brief set string arguments instead of doubles. Compare lexicographically */
	void setOperands(const std::string& par1, const std::string op1, const std::string& par2 = 0, const std::string& op2 = 0);

	/**@brief set the coordinates from which we extract the data. The meaning of parameters is exactly equal
	 to data-export. For the Basis-Dimension of this subset, an EMPTY vector must be passed */
	void setCoordinates(const std::vector<std::vector<std::string> > & coordinates);

	/**@brief set the size of the portion we want to show. p2 is used for mid-percentage and lower_percentage. It determines the lower
	 portion we remove when using mid-percentage. E.g. p1 = 10 p2 = 20 --> remove highest 10% and lowest 20%, show remaining 70% */
	void setPercentage(double p1 = 0, double p2 = 0);
	void setTop(unsigned int num);
};

} //palo
} //jedox
#endif
