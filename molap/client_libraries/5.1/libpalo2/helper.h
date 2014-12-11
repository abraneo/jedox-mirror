////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// Copyright (C) 2006-2014 Jedox AG
///
/// This program is free software; you can redistribute it and/or modify it
/// under the terms of the GNU General Public License (Version 2) as published
/// by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
/// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
/// more details.
///
/// You should have received a copy of the GNU General Public License along with
/// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
/// Place, Suite 330, Boston, MA 02111-1307 USA
///
/// If you are developing and distributing open source applications under the
/// GPL License, then you are free to use Palo under the GPL License.  For OEMs,
/// ISVs, and VARs who distribute Palo with their products, and do not license
/// and distribute their source code under the GPL, Jedox provides a flexible
/// OEM Commercial License.
///
/// @author
////////////////////////////////////////////////////////////////////////////////

#ifndef HAS_HELPER_H
#define HAS_HELPER_H

#include <vector>

#include <libpalo_ng/libpalo_ng_version.h>

using namespace std;

palo_err number2types(long ntype, de_type *type);
palo_err types2number(de_type type, ELEMENT_INFO::TYPE *ntype);
palo_err number2valtypes(long ntype, arg_palo_value_type *type);
palo_err splasmode2number(splash_mode mode, SPLASH_MODE *nmode);
unsigned short palo_bool2number(palo_bool value);
palo_bool FindDimElementIndex(vector<ELEMENT_INFO> &elems, char *utf8_elem, size_t *index);
palo_err getDimElementSiblingIndex(wchar_t **errmsg, size_t *siblingindex, vector<ELEMENT_INFO> &elems, size_t elemindex, int offset);
void getConditionString(struct arg_getdata_export_options_filter_m *Filter, std::string& conditionstring);
void initso(struct sock_obj *so);
void updateso(struct sock_obj *so, std::string &key);
void api_version(struct VERSION_INFO &api_version_info);
void checkedInitSSL(std::string &TrustFile);
const std::string& getPermissionArtString(permission_art pa);

class CheckForSpecialCubes {
public:
	inline static bool isPropertyCube(const string& cube) {
		return cube.compare(0, LengthPrefixPropertyCube, PrefixPropertyCube) == 0;
	}

private:
	static const string PrefixPropertyCube;
	static const size_t LengthPrefixPropertyCube;
};



#endif
