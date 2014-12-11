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

#include "Options/Options.h"

#include <iomanip>
#include <sstream>
#include <iostream>

#include "Options/OptionSpecification.h"
#include "Exceptions/ErrorException.h"

namespace palo {
size_t Options::MAX_COLUMN_WIDTH = 79;

enum PartialMatch_t {
	PARTIAL_MATCH_NO, PARTIAL_MATCH_PARTIAL, PARTIAL_MATCH_EXACT
};

static PartialMatch_t PartialMatch(const string& src, const string& attempt)
{
	if (src == attempt) {
		return PARTIAL_MATCH_EXACT;
	}

	if (src.empty() || attempt.empty()) {
		return PARTIAL_MATCH_NO;
	}

	size_t i;

	for (i = 0; i < attempt.length(); i++) {
		if (attempt[i] == ':' || attempt[i] == '=') {
			break;
		}

		if (::tolower(src[i]) != ::tolower(attempt[i])) {
			return PARTIAL_MATCH_NO;
		}
	}

	return i < src.length() ? PARTIAL_MATCH_PARTIAL : PARTIAL_MATCH_EXACT;
}

Options::Options(const string& name, const char * const optv[]) :
	programName(name), optionControls(OptionSpecification::DEFAULT), state(STATE_LOAD_ARGUMENT), errorState(OPTIONS_END_OPTIONS), currentArgument(0), withinGuessing(false), currentOption(0), listOption(0)
{
	size_t pos = programName.find_last_of("/\\");

	if (pos != string::npos) {
		programName = programName.substr(pos + 1);
	}

	if (optv != 0) {
		for (int i = 0; optv[i] != 0; i++) {
			OptionSpecification * optspec = new OptionSpecification(string(optv[i]));

			if (optspec->hasSyntaxError(programName)) {
				throw ErrorException(ErrorException::ERROR_INVALID_STRING, "cannot parse '" + string(optv[i]));
			}

			specifications.push_back(optspec);
		}
	}
}

Options::~Options()
{
	for (vector<OptionSpecification*>::iterator iter = specifications.begin(); iter != specifications.end(); ++iter) {
		delete *iter;
	}
}

void Options::clear()
{
	state = STATE_LOAD_ARGUMENT;
	errorState = OPTIONS_END_OPTIONS;
	currentArgument = 0;
	withinGuessing = false;
	currentOption = 0;
	listOption = 0;
}

//void Options::format(ostream& stream, const string& part, size_t margin, size_t& len) const
//{
//	if ((len + part.length() + 1) > MAX_COLUMN_WIDTH) {
//		stream << endl << setw((int)margin) << "";
//		len = margin;
//	} else {
//		stream << ' ';
//		len++;
//	}
//
//	len += part.length();
//
//	stream << part;
//}

string Options::usage(const string& positionals) const
{
	stringstream stream;

	if (specifications.empty()) {
		return "";
	}

	// print first portion "usage: progname"
	stream << "usage: " << programName << " ";

	// save the current length so we know how much space to skip for subsequent lines
	size_t margin = stream.str().size();

	// print the options and the positional arguments
	for (vector<OptionSpecification*>::const_iterator iter = specifications.begin(); iter != specifications.end(); ++iter) {
		const OptionSpecification * optspec = *iter;

		if (optspec->isHiddenOption())
			continue;

		stream << optspec->format(optionControls) << endl << setw((int)margin) << "";
	}

	stream << endl;

	return stream.str();
}

const OptionSpecification * Options::matchOption(char opt, bool ignoreCase) const
{
	for (vector<OptionSpecification*>::const_iterator iter = specifications.begin(); iter != specifications.end(); ++iter) {
		const OptionSpecification * optspec = *iter;

		if (optspec->isNullOption())
			continue;

		char optchar = optspec->getShortOption();

		if (opt == optchar) {
			return optspec;
		} else if (ignoreCase && (::tolower(opt) == ::tolower(optchar))) {
			return optspec;
		}
	}

	return 0;
}

const OptionSpecification * Options::matchLongOption(const string& option, bool & isAmbiguous) const
{
	const OptionSpecification * matched = 0;

	isAmbiguous = false;

	for (vector<OptionSpecification*>::const_iterator iter = specifications.begin(); iter != specifications.end(); ++iter) {
		const OptionSpecification * optspec = *iter;
		const string& longOption = optspec->getLongOption();

		if (longOption.empty()) {
			continue;
		}

		PartialMatch_t result = PartialMatch(longOption, option);

		if (result == PARTIAL_MATCH_EXACT) {
			return optspec;
		} else if (result == PARTIAL_MATCH_PARTIAL) {
			if (matched) {
				isAmbiguous = true;

				return 0;
			} else {
				matched = optspec;
			}
		}
	}

	return matched;
}

void Options::loadArgument(OptionsIterator & iter)
{
	if (unprocessedOptions != "") {
		currentArgument = &unprocessedOptions;
		state = STATE_PROCESS_SHORT_OPTION_NO_GUESS;
	} else {
		currentArgument = iter.getCurrent();
		state = (currentArgument == 0) ? STATE_ARGUMENTS_EXHAUSTED : STATE_PROCESS_ARGUMENT;
	}
}

void Options::processArgument(OptionsIterator & iter)
{
	if (*currentArgument == "--") {
		endOptions = true;
		state = STATE_RETURN_POSITIONAL;
		iter.advance();
	} else if (isLongOption(*currentArgument)) {
		state = STATE_PROCESS_LONG_OPTION;
		iter.advance();
	} else if (isShortOption(*currentArgument)) {
		state = STATE_PROCESS_SHORT_OPTION;
		iter.advance();
	} else if (listOption != 0) {
		state = STATE_RETURN_LIST_OPTION;
	} else {
		state = STATE_RETURN_POSITIONAL;
	}
}

int Options::returnPositional(OptionsIterator & iter, string & optarg)
{
	if (isOptionsOnly()) {
		return OPTIONS_END_OPTIONS;
	} else {
		currentArgument = iter();

		if (currentArgument == 0) {
			return OPTIONS_END_OPTIONS;
		} else {
			optarg = *currentArgument;

			return OPTIONS_POSITIONAL;
		}
	}
}

int Options::returnListOption(OptionsIterator & iter, string & optarg)
{
	optarg = *currentArgument;
	state = STATE_LOAD_ARGUMENT;
	iter.advance();

	return listOption->getShortOption();
}

void Options::processShortOption(OptionsIterator & iter, bool allowGuessing)
{
	unprocessedOptions = stripOption(*currentArgument);
	currentOptionName = unprocessedOptions[0];
	currentOption = matchOption(unprocessedOptions[0], isAnycase());

	if (currentOption == 0) {
		if (!withinGuessing && allowGuessing && isGuessing()) {
			withinGuessing = true;
			state = STATE_PROCESS_LONG_OPTION;
		} else {
			errorState = OPTIONS_BAD_CHAR;
			errorMessage = "unknown option '" + currentOptionName + "'";
			state = STATE_RETURN_ERROR;
		}
	} else {
		unprocessedOptions.erase(0, 1);
		withinGuessing = false;

		state = STATE_PROCESS_CURRENT_OPTION;
	}
}

void Options::processLongOption(OptionsIterator & iter)
{
	currentOptionName = stripOption(*currentArgument);
	bool isAmbiguous;
	currentOption = matchLongOption(currentOptionName, isAmbiguous);

	if (isAmbiguous) {
		errorState = OPTIONS_AMBIGUOUS;
		errorMessage = "ambiguous option '" + currentOptionName + "'";
		state = STATE_RETURN_ERROR;
	} else if (currentOption == 0) {
		if (!withinGuessing && isGuessing() && isShortOption(*currentArgument)) {
			withinGuessing = true;
			state = STATE_PROCESS_SHORT_OPTION;
		} else {
			errorState = OPTIONS_BAD_KEYWORD;
			errorMessage = "unknown option '" + currentOptionName + "'";
			state = STATE_RETURN_ERROR;
		}
	} else {
		size_t pos = currentOptionName.find_first_of(":=");

		if (pos != string::npos) {
			if (!currentOption->isValueTaken()) {
				errorState = OPTIONS_EXTRA_VALUE;
				errorMessage = "option '" + currentOptionName + "' does not take a value";
				state = STATE_RETURN_ERROR;
			} else {
				unprocessedOptions = currentOptionName.substr(pos + 1);
			}
		} else if (withinGuessing) {
			unprocessedOptions = "";
		}

		withinGuessing = false;
		state = STATE_PROCESS_CURRENT_OPTION;
	}
}

void Options::processCurrentOption()
{
	if (currentOption->isList()) {
		listOption = currentOption;
	} else {
		listOption = 0;
	}

	if (currentOption->isValueTaken()) {
		if (unprocessedOptions != "") {
			currentValue = unprocessedOptions;
			unprocessedOptions = "";
			state = STATE_RETURN_VALUE;
		} else {
			state = STATE_LOAD_VALUE;
		}
	} else {
		currentValue = "";
		state = STATE_RETURN_VALUE;
	}
}

void Options::loadValue(OptionsIterator & iter)
{
	currentArgument = iter.getCurrent();

	if (currentArgument != 0 && isOption(*currentArgument)) {
		currentArgument = 0;
	}

	if (currentOption->isValueRequired() && currentArgument == 0) {
		errorState = OPTIONS_MISSING_VALUE;
		errorMessage = "option '" + currentOptionName + "' requries an argument";
		state = STATE_RETURN_ERROR;
	} else if (currentArgument == 0) {
		currentValue = "";
		state = STATE_RETURN_VALUE;
	} else {
		iter.advance();

		currentValue = *currentArgument;
		state = STATE_RETURN_VALUE;
	}
}

int Options::returnValue(string & optarg)
{
	optarg = currentValue;
	state = STATE_LOAD_ARGUMENT;

	return currentOption->getShortOption();
}

int Options::returnError(string & optarg)
{
//	if (!isQuite()) {
//		cerr << programName << ": " << errorMessage << endl;
//	}

	optarg = errorMessage;

	return errorState;
}

int Options::operator()(OptionsIterator & iter, string & optarg)
{
	while (true) {
		switch (state) {
		case STATE_ARGUMENTS_EXHAUSTED:
			return OPTIONS_END_OPTIONS;
		case STATE_LOAD_ARGUMENT:
			loadArgument(iter);
			break;
		case STATE_LOAD_VALUE:
			loadValue(iter);
			break;
		case STATE_PROCESS_ARGUMENT:
			processArgument(iter);
			break;
		case STATE_PROCESS_CURRENT_OPTION:
			processCurrentOption();
			break;
		case STATE_PROCESS_LONG_OPTION:
			processLongOption(iter);
			break;
		case STATE_PROCESS_SHORT_OPTION:
			processShortOption(iter, true);
			break;
		case STATE_PROCESS_SHORT_OPTION_NO_GUESS:
			processShortOption(iter, false);
			break;
		case STATE_RETURN_ERROR:
			return returnError(optarg);
			break;
		case STATE_RETURN_LIST_OPTION:
			return returnListOption(iter, optarg);
		case STATE_RETURN_POSITIONAL:
			return returnPositional(iter, optarg);
		case STATE_RETURN_VALUE:
			return returnValue(optarg);
		}
	}
}

}
