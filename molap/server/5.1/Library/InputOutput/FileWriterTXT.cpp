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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "InputOutput/FileWriterTXT.h"

#include "Exceptions/FileOpenException.h"

#include "Collections/StringBuffer.h"

#include "InputOutput/FileUtils.h"
#include "Logger/Logger.h"

#include <stdio.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifndef _MSC_VER
#include <sys/file.h>
#endif

extern "C" {
#include <sys/stat.h>
}

#include <iostream>

#include "Olap/Context.h"
#include "Engine/Area.h"

namespace palo {

FileWriterTXT::FileWriterTXT(const FileName& fileName) :
	FileWriter(fileName), outputFile(0)
{
	setFirstValue(true);
}

FileWriterTXT::~FileWriterTXT()
{
	closeFile();
}

void FileWriterTXT::openFile(bool append)
{
	if (append) {
		outputFile = FileUtils::newOfstreamAppend(fileName.fullPath());
	} else {
		outputFile = FileUtils::newOfstream(fileName.fullPath());
	}

#if defined(_MSC_VER)
	if (outputFile == 0 && errno == EACCES) {
		for (int ms = 0; ms < 5; ms++) {
			Logger::warning << "encountered MS file-system change-log bug during open, sleeping to recover" << endl;
			usleep(1000);

			if (append) {
				outputFile = FileUtils::newOfstreamAppend(fileName.fullPath());
			} else {
				outputFile = FileUtils::newOfstream(fileName.fullPath());
			}

			if (outputFile != 0 || errno != EACCES) {
				break;
			}
		}
	}
#endif

	if (outputFile == 0) {
		Logger::warning << "could not write to file '" << fileName.fullPath() << "'" << " (" << strerror(errno) << ")" << endl;
		throw FileOpenException("could not open file for writing", fileName.fullPath());
	}
}

void FileWriterTXT::closeFile()
{
	if (outputFile != 0) {
		nextLine();
		writeBuffer();
		FileUtils::paloofstream *o = dynamic_cast<FileUtils::paloofstream *>(outputFile);
		if (o) {
			o->close();
		}
		delete outputFile;
		outputFile = 0;
	}
}

void FileWriterTXT::appendComment(const string& value)
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	if (!isFirstValue()) {
		nextLine();
	}

	// delete line feeds
	size_t pos = value.find_first_of("\r\n");

	if (pos != string::npos) {
		string copy = value;
		size_t pos = copy.find_first_of("\r\n");

		while (pos != string::npos) {
			copy[pos] = ' ';
			pos = copy.find_first_of("\r\n", pos);
		}

		*outputFile << "# " << copy;
	} else {
		*outputFile << "# " << value;
	}

	setFirstValue(false);

	nextLine();
}

void FileWriterTXT::appendSection(const string& value)
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	*outputFile << "[" << value << "]";
	setFirstValue(false);
	nextLine();
}

void FileWriterTXT::appendString(const string& value, char terminator)
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	*outputFile << value;
	if (terminator) {
		*outputFile << terminator;
	}
	setFirstValue(false);
}

void FileWriterTXT::appendEscapeString(const string& value, char terminator)
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	*outputFile << escapeString(value);
	if (terminator) {
		*outputFile << terminator;
	}
	setFirstValue(false);
}

void FileWriterTXT::appendEscapeStrings(const vector<string>* value)
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	bool first = true;

	for (vector<string>::const_iterator i = value->begin(); i != value->end(); ++i) {
		if (first) {
			first = false;
		} else {
			*outputFile << ",";
		}
		*outputFile << escapeString(*i);
	}

	*outputFile << ";";

	setFirstValue(false);
}

void FileWriterTXT::appendInteger(const int32_t value)
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	*outputFile << value << ";";
	setFirstValue(false);
}

void FileWriterTXT::appendIntegers(const vector<int32_t>* value)
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	bool first = true;

	for (vector<int32_t>::const_iterator i = value->begin(); i != value->end(); ++i) {
		if (first) {
			first = false;
		} else {
			*outputFile << ",";
		}
		*outputFile << *i;
	}

	*outputFile << ";";

	setFirstValue(false);
}

void FileWriterTXT::appendIdentifier(const IdentifierType value, char terminator)
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	*outputFile << value;
	if (terminator) {
		*outputFile << terminator;
	}

	setFirstValue(false);
}

void FileWriterTXT::appendSet(const Set &s)
{
	bool firstRange = true;
	appendString("(",0);
	for (Set::range_iterator rit = s.rangeBegin(); rit != s.rangeEnd(); ++rit) {
		IdentifierType rangeSize = rit.high() - rit.low();
		if (!firstRange) {
			appendString(" ",0);
		} else {
			firstRange = false;
		}
		appendIdentifier(rit.low(), rangeSize ? '+' : 0);
		if (rangeSize) {
			appendIdentifier(rangeSize,0);
		}
	}
	appendString(")",0);
}

void FileWriterTXT::appendAreaCompact(CPArea value, CPArea parent)
{
	if (!value || value == parent || (parent && *value == *parent)) {
		appendString("{}",0);
		return;
	}
	appendString("{",0);
	size_t counter = 0;
	for (size_t i = 0; i != value->dimCount(); i++) {
		CPSet s = value->getDim(i);
		if (parent) {
			CPSet sp = parent->getDim(i);
			if (sp == s || (sp && s && *sp == *s)) {
				// identical set as parent
				continue;
			}
		}
		if (!s) {
			// all elements = identical set as parent
			continue;
		}
		if (i != counter++) {
			appendIdentifier(IdentifierType(i),' ');
		}
		appendSet(*s);
	}
	appendString("}",0);
}

void FileWriterTXT::appendPlan(CPPlanNode plan, CPArea parentArea)
{
	appendString("(",0);
	if (plan) {
		plan->write(*this, parentArea);
	}
	appendString(")",0);
}

void FileWriterTXT::appendAggregationMaps(CPAggregationMaps aggrMaps)
{
	appendString("{",0);
	if (aggrMaps) {
		size_t counter = 0;
		for (size_t i = 0; i != aggrMaps->size(); i++) {
			const AggregationMap &am = aggrMaps->at(i);

			if (am.getSingleTarget() != NO_IDENTIFIER && !am.hasWeights()) {
				// sum to one parent no weights
				continue;
			}
			if (i != counter++) {
				appendIdentifier(IdentifierType(i),0);
			}
			appendString("{",0);
			bool firstSource = true;
			for (AggregationMap::Iterator srcIt = am.beginSource(); srcIt != am.endSource(); ++srcIt) {
				if (!firstSource) {
					appendString(" ",0);
				} else {
					firstSource = false;
				}
				appendIdentifier(*srcIt,'>');
				AggregationMap::TargetReader targets = srcIt.targetReader();
				bool firstTarget = true;
				while (!targets.end()) {
					if (!firstTarget) {
						appendString(",",0);
					} else {
						firstTarget = false;
					}
					if (*targets == NO_IDENTIFIER) {
						appendString("X",0);
					} else {
						appendIdentifier(*targets,0);
					}
					if (targets.getWeight() != 1.0) {
						appendString("*",0);
						appendDouble(targets.getWeight(),0);
					}
					++targets;
				}

			}
			appendString("}",0);
		}
	}
	appendString("}",0);
}

void FileWriterTXT::appendArea(CPArea value)
{
	if (!value) {
		appendString("");
		return;
	}
	size_t innerSeparators = value->dimCount();
	if (innerSeparators) {
		innerSeparators--;
	}

	for (size_t i = 0; i != value->dimCount(); i++, innerSeparators--) {
		CPSet s = value->getDim(i);
		if (s) {
			appendIdentifiers(value->elemBegin(i), value->elemEnd(i), ':', innerSeparators ? ',' : ';');
		} else {
			appendString("*", innerSeparators ? ',' : ';');
		}
	}
}

void FileWriterTXT::appendArea(vector<IdentifiersType> &value, bool emptyAll)
{
	if (value.empty()) {
		appendString("");
		return;
	}
	size_t innerSeparators = value.size();
	if (innerSeparators) {
		innerSeparators--;
	}

	for (size_t i = 0; i < value.size(); ++i, innerSeparators--) {
		if (value[i].size()) {
			appendIdentifiers(value[i].begin(), value[i].end(), ':', innerSeparators ? ',' : ';');
		} else {
			if (emptyAll) {
				appendString("*", innerSeparators ? ',' : ';');
			} else {
				appendString("", innerSeparators ? ',' : ';');
			}
		}
	}
}

void FileWriterTXT::appendPaths(CPPaths value)
{
	if (!value) {
		appendString("");
		return;
	}
	size_t innerSeparators = value->size();
	if (innerSeparators) {
		innerSeparators--;
	}

	for (vector<IdentifiersType>::const_iterator it = value->begin(); it != value->end(); ++it, innerSeparators--) {
		appendIdentifiers(it->begin(), it->end(), ',', innerSeparators ? ':' : ';');
	}
}

void FileWriterTXT::appendDouble(const double value, char terminator)
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	outputFile->precision(15);

	*outputFile << value;
	if (terminator) {
		*outputFile << terminator;
	}
	setFirstValue(false);
}

void FileWriterTXT::appendDoubles(const vector<double>* value)
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	bool first = true;

	for (vector<double>::const_iterator i = value->begin(); i != value->end(); ++i) {
		if (first) {
			first = false;
		} else {
			*outputFile << ",";
		}
		*outputFile << *i;
	}

	*outputFile << ";";

	setFirstValue(false);
}

void FileWriterTXT::appendBool(const bool value)
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	if (value) {
		*outputFile << "1;";
	} else {
		*outputFile << "0;";
	}

	setFirstValue(false);
}

void FileWriterTXT::appendTimeStamp()
{
	timeval tv;
	gettimeofday(&tv, 0);
	appendTimeStamp(tv);
}

void FileWriterTXT::appendTimeStamp(timeval &tv)
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	*outputFile << tv.tv_sec << "." << tv.tv_usec << ";";
	setFirstValue(false);
}

void FileWriterTXT::nextLine()
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	if (!isFirstValue()) {
		*outputFile << endl;

		outputFile->flush();
	}

	setFirstValue(true);
}

void FileWriterTXT::appendRaw(const string& value)
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	*outputFile << value;
	outputFile->flush();
	setFirstValue(true);
}

void FileWriterTXT::appendRaw(const char *p, streamsize size)
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	outputFile->write(p, size);
}

void FileWriterTXT::writeBuffer()
{
	if (outputFile == 0) {
		Logger::error << "file writer is closed" << endl;
		return;
	}

	outputFile->flush();
}

string FileWriterTXT::escapeString(const string& text)
{
	StringBuffer sb;
	size_t begin = 0;
	size_t end = text.find("\"");

	sb.appendText("\"");

	while (end != string::npos) {

		sb.appendText(text.substr(begin, end - begin));
		sb.appendText("\"\"");

		begin = end + 1;
		end = text.find("\"", begin);
	}

	sb.appendText(text.substr(begin, end - begin));
	sb.appendText("\"");
	string result = sb.c_str();
	return result;
}

bool FileWriterTXT::isFirstValue()
{
	return isFirst;
}

void FileWriterTXT::setFirstValue(bool v)
{
	isFirst = v;
}

void FileWriterTXT::appendArray(bool bFirst, IdentifierType value, char separator)
{
	if (outputFile == 0) {
		return;
	}
	if (!bFirst) {
		*outputFile << separator;
	}
	*outputFile << value;
}

void FileWriterTXT::appendArrayFinish(char terminator)
{
	if (outputFile == 0) {
		return;
	}
	*outputFile << terminator;
	setFirstValue(false);
}

}
