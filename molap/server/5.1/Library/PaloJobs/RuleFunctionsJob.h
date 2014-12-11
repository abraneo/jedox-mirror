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
 * 
 *
 */

#ifndef PALO_JOBS_RULE_FUNCTIONS_JOB_H
#define PALO_JOBS_RULE_FUNCTIONS_JOB_H 1

#include "palo.h"

#include "InputOutput/FileReader.h"
#include "Olap/Rule.h"
#include "PaloDispatcher/PaloBrowserJob.h"
#include "Parser/RuleParserDriver.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief rule functions
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS RuleFunctionsJob : public PaloBrowserJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new RuleFunctionsJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	RuleFunctionsJob(PaloJobRequest* jobRequest) :
		PaloBrowserJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start working
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		response = new HttpResponse(HttpResponse::OK);
		StringBuffer& sb = response->getBody();

		languages.clear();
		functionNames.clear();
		function2Category.clear();
		function2Minimal.clear();
		function2Maximal.clear();
		function2ShortDescription.clear();
		function2LongDescription.clear();
		function2ArgumentType.clear();
		function2ArgumentDesc.clear();

		loadCategories();
		loadShortDescription();
		loadLongDescription();

		sb.appendText("<functions>");
		sb.appendEol();

		for (set<string>::const_iterator iter = functionNames.begin(); iter != functionNames.end(); iter++) {
			string name = *iter;

			sb.appendText("  <function>");
			sb.appendEol();

			sb.appendText("    <name>" + StringUtils::escapeXml(name) + "</name>");
			sb.appendEol();

			sb.appendText("    <category>" + StringUtils::escapeXml(function2Category[name]) + "</category>");
			sb.appendEol();

			map<string, string>& shortDescription = function2ShortDescription[name];

			sb.appendEol();

			for (set<string>::const_iterator langIter = languages.begin(); langIter != languages.end(); langIter++) {
				string lang = *langIter;
				string text = shortDescription[lang];

				if (!text.empty()) {
					sb.appendText("    <short-description language=\"" + StringUtils::escapeXml(lang) + "\">");
					sb.appendEol();

					sb.appendText("      " + StringUtils::escapeXml(text));
					sb.appendEol();

					sb.appendText("    </short-description>");
					sb.appendEol();
					sb.appendEol();
				}
			}

			map<string, string>& longDescription = function2LongDescription[name];

			for (set<string>::const_iterator langIter = languages.begin(); langIter != languages.end(); langIter++) {
				string lang = *langIter;
				string text = longDescription[lang];

				if (!text.empty()) {
					sb.appendText("    <long-description language=\"" + StringUtils::escapeXml(lang) + "\">");
					sb.appendEol();

					sb.appendText("      " + StringUtils::escapeXml(text));
					sb.appendEol();

					sb.appendText("    </long-description>");
					sb.appendEol();
					sb.appendEol();
				}
			}

			sb.appendText("    <minimal-arguments>" + StringUtils::convertToString((uint32_t)function2Minimal[name]) + "</minimal-arguments>");
			sb.appendEol();

			sb.appendText("    <maximal-arguments>" + StringUtils::convertToString((uint32_t)function2Maximal[name]) + "</maximal-arguments>");
			sb.appendEol();

			vector<string>& argumentTypes = function2ArgumentType[name];
			vector<map<string, string> >& argumentDescs = function2ArgumentDesc[name];

			sb.appendEol();

			for (size_t pos = 0; pos < argumentTypes.size() && pos < argumentDescs.size(); pos++) {
				if (0 < pos) {
					sb.appendEol();
				}

				sb.appendText("    <argument number=\"" + StringUtils::convertToString((int32_t)pos) + "\">");
				sb.appendEol();

				sb.appendText("      <type>" + StringUtils::escapeXml(argumentTypes[pos]) + "</type>");
				sb.appendEol();

				for (set<string>::const_iterator langIter = languages.begin(); langIter != languages.end(); langIter++) {
					string lang = *langIter;
					string desc = argumentDescs[pos][lang];

					if (!desc.empty()) {
						sb.appendText("      <description language=\"" + StringUtils::escapeXml(lang) + "\">");
						sb.appendText(StringUtils::escapeXml(desc));
						sb.appendText("</description>");
						sb.appendEol();
					}
				}

				sb.appendText("    </argument>");
				sb.appendEol();
			}

			sb.appendText("  </function>");
			sb.appendEol();
		}

		sb.appendText("</functions>");
		sb.appendEol();
	}

private:
	void loadCategories() {
		boost::shared_ptr<FileReader> fr(FileReader::getFileReader(FileName(templatePath, "categories", "csv")));
		fr->openFile(true, false);

		while (fr->isDataLine()) {
			string name = fr->getDataString(0);
			string category = fr->getDataString(1);
			int min = fr->getDataInteger(2);
			int max = fr->getDataInteger(3);

			functionNames.insert(name);
			function2Category[name] = category;
			function2Minimal[name] = min;
			function2Maximal[name] = max;

			fr->nextLine();
		}
	}

	void loadShortDescription() {
		boost::shared_ptr<FileReader> fr(FileReader::getFileReader(FileName(templatePath, "short-description", "csv")));
		fr->openFile(true, false);

		while (fr->isDataLine()) {
			string name = fr->getDataString(0);
			string lang = fr->getDataString(1);
			string desc = fr->getDataString(2);

			languages.insert(lang);
			function2ShortDescription[name][lang] = desc;

			for (size_t i = 3; i < fr->getDataSize(); i += 2) {
				string type = fr->getDataString(i);
				string tdsc = fr->getDataString(i + 1);
				size_t pos = (i - 3) / 2;

				if (type.empty() && tdsc.empty()) {
					break;
				}

				if (function2ArgumentType[name].size() <= pos) {
					function2ArgumentType[name].resize(pos + 1);
					function2ArgumentDesc[name].resize(pos + 1);
				}

				function2ArgumentType[name][pos] = type;
				function2ArgumentDesc[name][pos][lang] = tdsc;
			}

			fr->nextLine();
		}
	}

	void loadLongDescription() {
		boost::shared_ptr<FileReader> fr(FileReader::getFileReader(FileName(templatePath, "long-description", "csv")));
		fr->openFile(true, false);

		while (fr->isDataLine()) {
			string name = fr->getDataString(0);
			string lang = fr->getDataString(1);
			string desc = fr->getDataString(2);

			languages.insert(lang);
			function2LongDescription[name][lang] = desc;

			fr->nextLine(true);
		}
	}

private:
	set<string> languages;
	set<string> functionNames;

	map<string, string> function2Category;
	map<string, int> function2Minimal;
	map<string, int> function2Maximal;

	map<string, map<string, string> > function2ShortDescription;
	map<string, map<string, string> > function2LongDescription;

	map<string, vector<string> > function2ArgumentType;
	map<string, vector<map<string, string> > > function2ArgumentDesc;
};

}

#endif
