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

/*!
 * \brief
 * TODO
 * \author Florian Schaper <florian.schaper@jedox.com>
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * \author Frieder Hofmann <frieder.hofmann@jedox.com>
 */

#ifndef JEDOXXLHELPER_H
#define JEDOXXLHELPER_H

#include <boost/smart_ptr/scoped_ptr.hpp>

#include <libpalo_ng/Palo/ServerPool.h>


#if defined(WIN32) || defined(WIN64)
#   define JEDOXXLHELPER_CLASS_EXPORT __declspec(dllexport)
#else
#   define JEDOXXLHELPER_CLASS_EXPORT
#endif


namespace jedox {
	namespace palo{

		class JEDOXXLHELPER_CLASS_EXPORT JedoxXLHelper {
		public:
			~JedoxXLHelper();
			ServerSPtr getServer(const std::string& host, const unsigned int port, const std::string& user, const std::string& password, std::string& key, jedox::palo::ServerProtocol protocol = jedox::palo::Http, bool diretclyfromxll = false);
			void removeServer(const std::string& key, bool disconnect);
			void SetVersionInfo(const std::string& version);
			void TestConnection(const std::string& host, const unsigned int port, const std::string& user, const std::string& password);

			void Cleanup();
			std::string getVersionInfo() const;

			static JedoxXLHelper& getInstance();

		private:
			struct JedoxXLHelperImpl;
			boost::scoped_ptr<JedoxXLHelperImpl> m_JedoxXLHelperImpl;

			JedoxXLHelper();
		};

	} /* palo */
} /* jedox */


#endif							 // JEDOXXLHELPER_H
