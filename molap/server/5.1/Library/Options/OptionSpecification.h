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

#ifndef OPTIONS_OPTION_SPECIFICATION_H
#define OPTIONS_OPTION_SPECIFICATION_H 1

#include "palo.h"

#include <ctype.h>

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief class that represents an option specification
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS OptionSpecification {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief or-ed value of the constant below defines the controls
	////////////////////////////////////////////////////////////////////////////////

	typedef uint32_t OptionControls;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief default setting
	////////////////////////////////////////////////////////////////////////////////

	static const uint32_t DEFAULT = 0x00;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief ignore case when matching short-options
	////////////////////////////////////////////////////////////////////////////////

	static const uint32_t ANYCASE = 0x01;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief do not print error messages
	////////////////////////////////////////////////////////////////////////////////

	static const uint32_t QUIET = 0x02;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief allow "+" as a long-option prefix
	////////////////////////////////////////////////////////////////////////////////

	static const uint32_t PLUS = 0x04;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief do not accept long-options
	////////////////////////////////////////////////////////////////////////////////

	static const uint32_t SHORT_ONLY = 0x08;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief do not accept short-options
	///
	/// This also allows "-" as a long-option prefix
	////////////////////////////////////////////////////////////////////////////////

	static const uint32_t LONG_ONLY = 0x10;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief no guessing
	///
	/// Normally, when we see a short (long) option on the command line that
	/// doesnt match any known short (long) options, then we try to "guess" by
	/// seeing if it will match any known long (short) option. Setting this mask
	/// prevents this "guessing" from occurring.
	////////////////////////////////////////////////////////////////////////////////

	static const uint32_t NOGUESSING = 0x20;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief parse position arguments
	///
	/// By default, Options will not present positional command-line arguments
	/// to the user and will instead stop parsing when the first positonal
	/// argument has been encountered. If this flag is given, Options will
	/// present positional arguments to the user with a return code of
	/// POSITIONAL; ENDOPTS will be returned only when the end of the argument
	/// list is reached.
	////////////////////////////////////////////////////////////////////////////////

	static const uint32_t PARSE_POS = 0x40;

public:
	OptionSpecification(const string& decl) :
		specification(decl) {
		checkSpecification();
	}

	OptionSpecification(const OptionSpecification & copy) :
		hidden(copy.hidden), optionChar(copy.optionChar), longOption(copy.longOption), valueSpecification(copy.valueSpecification), valueDescription(copy.valueDescription), specification(copy.specification) {
	}

	OptionSpecification & operator=(const OptionSpecification & copy) {
		if (this != &copy) {
			hidden = copy.hidden;
			optionChar = copy.optionChar;
			longOption = copy.longOption;
			valueSpecification = copy.valueSpecification;
			valueDescription = copy.valueDescription;
			specification = copy.specification;
		}

		return *this;
	}

	OptionSpecification & operator=(const string& decl) {
		if (specification != decl) {
			specification = decl;
			checkSpecification();
		}

		return *this;
	}

	operator const string&() {
		return specification;
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if an syntax error exists, print error message
	////////////////////////////////////////////////////////////////////////////////

	bool hasSyntaxError(const string& programName) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the short option character
	////////////////////////////////////////////////////////////////////////////////

	char getShortOption() const {
		return optionChar;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the long option string
	////////////////////////////////////////////////////////////////////////////////

	const string& getLongOption() const {
		return longOption;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the value descriptor
	////////////////////////////////////////////////////////////////////////////////

	const string& getValue() const {
		return valueDescription;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if option is a hidden option
	////////////////////////////////////////////////////////////////////////////////

	bool isHiddenOption() const {
		return hidden;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if the option requires at least one arguments
	////////////////////////////////////////////////////////////////////////////////

	bool isValueRequired() const {
		return valueSpecification == ':' || valueSpecification == '+';
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if the option can have optional arguments
	////////////////////////////////////////////////////////////////////////////////

	bool isValueOptional() const {
		return valueSpecification == '?' || valueSpecification == '*';
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true the option can take arguments
	////////////////////////////////////////////////////////////////////////////////

	bool isValueTaken() const {
		return isValueRequired() || isValueOptional();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if the option can take more than one argument
	////////////////////////////////////////////////////////////////////////////////

	bool isList() const {
		return (valueSpecification == '+') || (valueSpecification == '*');
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if the short option is not given
	////////////////////////////////////////////////////////////////////////////////

	bool isNullOption() const {
		return optionChar == 0 || ::isspace(optionChar) || !::isprint(optionChar);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief formats an options specification for a usage message
	////////////////////////////////////////////////////////////////////////////////

	string format(OptionControls) const;

private:
	void checkSpecification();

private:
	bool hidden;
	char optionChar;
	string longOption;
	char valueSpecification;
	string valueDescription;
	string specification;
};

}

#endif
