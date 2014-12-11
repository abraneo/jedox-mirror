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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef HTTP_SERVER_HTTP_REQUEST_H
#define HTTP_SERVER_HTTP_REQUEST_H 1

#include "palo.h"

#include "Logger/Logger.h"

namespace palo {
class HttpRequestHandler;

////////////////////////////////////////////////////////////////////////////////
/// @brief http request
///
/// The http server reads the request string from the client and converts it
/// into an instance of this class. An http request object provides methods to
/// inspect the header and parameter fields.
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS HttpRequest {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief http request type
	////////////////////////////////////////////////////////////////////////////////

	enum HttpRequestType {
		HTTP_REQUEST_GET, HTTP_REQUEST_POST, HTTP_REQUEST_ILLEGAL
	};

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief http request
	///
	/// Constructs a http request given the header string. A client request
	/// consists of two parts: the header and the body. For a GET request the
	/// body is always empty and all information about the request is delivered
	/// in the header. For a POST request some information is also delivered in
	/// the body. However, it is necessary to parse the header information,
	/// before the body can be read.
	////////////////////////////////////////////////////////////////////////////////

	HttpRequest(const string& url, HttpRequestHandler*);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief destructor
	////////////////////////////////////////////////////////////////////////////////

	virtual ~HttpRequest() {
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief extracts the header fields of the request
	///
	/// @warning this might alter the contents of the string buffer
	////////////////////////////////////////////////////////////////////////////////

	virtual void extractHeader(char* begin, char* end);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief extracts the body of the request
	///
	/// @warning this might alter the contents of the string buffer
	////////////////////////////////////////////////////////////////////////////////

	virtual void extractBody(char* begin, char* end);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief releases the buffers
	///
	/// @warning the buffers passed to extractHeader and extractBody will become
	/// invalid after a call to releaseBuffers
	////////////////////////////////////////////////////////////////////////////////

	virtual void releaseBuffers() {
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the http request type
	////////////////////////////////////////////////////////////////////////////////

	HttpRequestHandler* getHttpRequestHandler() const {
		return httpRequestHandler;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the http request type
	///
	/// A request is either a GET or a POST request.
	////////////////////////////////////////////////////////////////////////////////

	HttpRequestType getRequestType() const {
		return type;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the path of the request
	///
	/// The path consists of the URL without the host and without any parameters.
	////////////////////////////////////////////////////////////////////////////////

	const string& getRequestPath() const {
		return requestPath;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns a header field
	///
	/// Returns the value of a header field with given name. If no header field
	/// with the given name was specified by the client, the empty string is
	/// returned.
	////////////////////////////////////////////////////////////////////////////////

	const string& getHeader(const string& field) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns all header fields
	///
	/// Returns all header fields
	////////////////////////////////////////////////////////////////////////////////

	const map<string, string>& getHeaders() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the value of a key
	///
	/// Returns the value of a key. The empty string is returned if key was not
	/// specified by the client.
	////////////////////////////////////////////////////////////////////////////////

	const string& getValue(const string& key) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the value of a key
	///
	/// Returns the value of a key. The empty string is returned if key was not
	/// specified by the client. found is true if the client specified the key.
	////////////////////////////////////////////////////////////////////////////////

	const string& getValue(const string& key, bool& found) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns all values
	///
	/// Returns all key/value pairs of the request.
	////////////////////////////////////////////////////////////////////////////////

	const map<string, string>& getValues() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the content length
	////////////////////////////////////////////////////////////////////////////////

	size_t getContentLength() const {
		return contentLength;
	}

#ifdef ENABLE_TRACE_OPTION
	const string& getHeaderString() const {
		return headerString;
	}

	const string& getBodyString() const {
		return bodyString;
	}
#endif

protected:
	static inline int hex2int(char ch) {
		if ('0' <= ch && ch <= '9') {
			return ch - '0';
		} else if ('A' <= ch && ch <= 'F') {
			return ch - 'A' + 10;
		} else if ('a' <= ch && ch <= 'f') {
			return ch - 'a' + 10;
		}

		Logger::warning << "'" << ch << "' is not a hex number" << endl;

		return 0;
	}

protected:
	// use default copy constructor HttpRequest (const HttpRequest&);
	// use default copy constructor HttpRequest& operator= (const HttpRequest&);

protected:
	HttpRequestHandler* httpRequestHandler;
	HttpRequestType type;

	string requestPath;

	map<string, string> headerFields;

	map<string, string> requestFields;
	vector<string> requestKeys;

	size_t contentLength;

#ifdef ENABLE_TRACE_OPTION
	string headerString;
	string bodyString;
#endif

private:
	void setKeyValues(char* begin, char* end);
};

}

#endif
