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
 * \author Frank Celler, triagens GmbH, Cologne, Germany
 * \author Achim Brandt, triagens GmbH, Cologne, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef HTTP_SERVER_HTTP_RESPONSE_H
#define HTTP_SERVER_HTTP_RESPONSE_H 1

#include "palo.h"

#include "Collections/StringBuffer.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief http response
///
/// A http request handler is called to handle a http request. It returns its
/// answer as http response.
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS HttpResponse {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief http response codes
	////////////////////////////////////////////////////////////////////////////////

	enum HttpResponseCode {
		OK = 200, CREATED = 201, ACCEPTED = 202, PARTIAL = 203, NO_RESPONSE = 204, MOVED = 301, FOUND = 302, METHOD = 303, NOT_MODIFIED = 304, BAD = 400, UNAUTHORIZED = 401, PAYMENT = 402, FORBIDDEN = 403, NOT_FOUND = 404, ERROR_CODE = 500, NOT_IMPLEMENTED = 501,
	};

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief http response string
	///
	/// Converts the response code to a string suitable for delivering to a http
	/// client.
	////////////////////////////////////////////////////////////////////////////////

	static string getResponseString(HttpResponseCode);

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a new http response
	///
	/// Constructs a new http response. An empty response is returned with the
	/// given response code is returned. You can fill the response body be
	/// calling getBody and adding data the string buffer returned.
	////////////////////////////////////////////////////////////////////////////////

	HttpResponse(HttpResponseCode);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes a http response
	///
	/// The descrutor will free the string buffers used. After the http response
	/// is deleted, the string buffers returned by getHeader and getBody become
	/// invalid.
	////////////////////////////////////////////////////////////////////////////////

	~HttpResponse();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the header
	///
	/// Returns a constant reference to the header. This reference is only valid
	/// as long as http response exists. You should call getHeader only after
	/// the body has been created.
	////////////////////////////////////////////////////////////////////////////////

	const StringBuffer& getHeader();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the body
	///
	/// Returns a reference to the body. This reference is only valid as long as
	/// http response exists. You can add data to the body by appending
	/// information to the string buffer. Note that adding data to the body
	/// invalidates any previously returned header. You must call getHeader
	/// again.
	////////////////////////////////////////////////////////////////////////////////

	StringBuffer& getBody();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets the content type
	///
	/// Sets the content type of the information of the body.
	////////////////////////////////////////////////////////////////////////////////

	void setContentType(const string& contentType) {
		this->contentType = contentType;
	}
	;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets one header line
	////////////////////////////////////////////////////////////////////////////////

	void setToken(const string& tokenName, uint32_t tokenValue) {
		this->tokenName = tokenName;
		this->tokenValue = tokenValue;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets one header line
	////////////////////////////////////////////////////////////////////////////////

	void setSecondToken(const string& tokenName, uint32_t tokenValue) {
		this->secondTokenName = tokenName;
		this->secondTokenValue = tokenValue;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the client data
	////////////////////////////////////////////////////////////////////////////////

	IdentifierType getClientData() const {
		return clientData;
	}

    ////////////////////////////////////////////////////////////////////////////////
    /// @brief sets additional header fields and values
    ////////////////////////////////////////////////////////////////////////////////

    void setHeaderField(pair<string, string> field) {
        fields.push_back(field);
    }

protected:
	HttpResponse(const HttpResponse&);
	HttpResponse& operator=(const HttpResponse&);

private:
	const HttpResponseCode code;

	StringBuffer header;
	StringBuffer body;

	string contentType;

	string tokenName;
	uint32_t tokenValue;

	string secondTokenName;
	uint32_t secondTokenValue;

	IdentifierType clientData;

    list<pair<string, string> > fields;
};

}

#endif
