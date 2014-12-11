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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef PALO_HTTP_SERVER_PALO_HTTP_REQUEST_H
#define PALO_HTTP_SERVER_PALO_HTTP_REQUEST_H 1

#include "palo.h"

#include "HttpServer/HttpRequest.h"

namespace palo {
class PaloJobRequest;
struct ViewSubset;
struct ViewAxis;
struct ViewArea;
struct BasicFilterSettings;
struct StructuralFilterSettings;
struct AliasFilterSettings;
struct FieldFilterSettings;
struct TextFilterSettings;
struct DataFilterSettings;
struct SortingFilterSettings;

////////////////////////////////////////////////////////////////////////////////
/// @brief http request
///
/// The http server reads the request string from the client and converts it
/// into an instance of this class. An http request object provides methods to
/// inspect the header and parameter fields.
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS PaloHttpRequest : public HttpRequest {

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief palo http request
	////////////////////////////////////////////////////////////////////////////////

	PaloHttpRequest(const string& url, HttpRequestHandler*);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief destructor
	////////////////////////////////////////////////////////////////////////////////

	~PaloHttpRequest();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief extracts the header fields of the request
	///
	/// @warning this might alter the contents of the string buffer
	////////////////////////////////////////////////////////////////////////////////

	void extractHeader(char* begin, char* end);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief extracts the body of the request
	///
	/// @warning this might alter the contents of the string buffer
	////////////////////////////////////////////////////////////////////////////////

	void extractBody(char* begin, char* end);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief releases the job request
	////////////////////////////////////////////////////////////////////////////////

	PaloJobRequest* releasePaloJobRequest() {
		PaloJobRequest* job = paloJobRequest;
		paloJobRequest = 0;
		return job;
	}

	PaloJobRequest * getPaloJobRequest() {
		return paloJobRequest;
	}

	const stringstream * getHttpParams() {
		return &httpParams;
	}

private:
	void setKeyValues(char* begin, char* end);
	void setKeyValue(char * keyStart, char * keyPtr, char * valueStart, char * valuePtr);

	void fillToken(uint32_t*& token, char* begin, char* end);

	void fillIdentifier(IdentifierType& identifier, char* begin, char* end);
	void fillBoolean(bool& flag, char* begin, char* end);
	void fillString(string*& text, char* begin, char* end);
	void fillSid(string& text, char* begin, char* end);
	void fillUint(uint32_t& identifier, char* begin, char* end);
	void fillVectorIdentifier(IdentifiersType*& identifiers, char* begin, char* end);
	void fillVectorUint(vector<uint32_t>*& ints, char* begin, char* end);
	void fillVectorDouble(vector<double>*& doubles, char* begin, char* end);
	void fillVectorString(vector<string>*& strings, char* begin, char* end, char separator);
	void fillVectorStringQuote(vector<string>*& strings, char* begin, char* end, char separator);
	void fillVectorStringQuote(vector<string>& strings, const char* begin, const char* end, char separator);
	void fillVectorVectorIdentifier(vector<IdentifiersType>*& identifiers, char* begin, char* end, char first, char second);
	void fillArea(PArea area, char* begin, char* end, char first, char second);
	void fillVectorVectorString(vector<vector<string> >*& strings, char* begin, char* end, char first, char second);
	void fillVectorVectorStringQuote(vector<vector<string> >*& strings, const char* valueStart, const char* valueEnd, char first, char second);
	void fillVectorVectorStringQuote(vector<vector<string> >& strings, const char* valueStart, const char* valueEnd, char first, char second);
	void fillVectorVectorDouble(vector<vector<double> >*& doubles, char* begin, char* end, char first, char second);
	void fillViewSubsets(map<string, ViewSubset>*& subsets, char* begin, char* end);
	void fillViewAxes(vector<ViewAxis>*& axes, char* begin, char* end);
	void fillViewArea(ViewArea*& area, char* begin, char* end);
	bool fillPicklistFilter(BasicFilterSettings &filter, vector<string>::iterator &sit, vector<string>::iterator send);
	bool fillStructuralFilter(StructuralFilterSettings &filter, vector<string>::iterator &sit, vector<string>::iterator send);
	bool fillAliasFilter(AliasFilterSettings &filterA, FieldFilterSettings &filterF, vector<string>::iterator &sit, vector<string>::iterator send);
	bool fillTextFilter(TextFilterSettings &filter, vector<string>::iterator &sit, vector<string>::iterator send);
	bool fillDataFilter(DataFilterSettings &filter, vector<string>::iterator &sit, vector<string>::iterator send);
	bool fillSortinglistFilter(SortingFilterSettings &filter, vector<string>::iterator &sit, vector<string>::iterator send);

protected:
	PaloJobRequest * paloJobRequest;
	stringstream httpParams;

private:
	bool loginRequest;
};

}

#endif
