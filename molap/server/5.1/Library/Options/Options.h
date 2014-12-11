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

#ifndef OPTIONS_OPTIONS_H
#define OPTIONS_OPTIONS_H 1

#include "palo.h"

#include <string>
#include <vector>

#include "OptionsIterator.h"
#include "OptionsArgvIterator.h"
#include "OptionsFileIterator.h"
#include "OptionSpecification.h"

namespace palo {
class OptionSpecification;

////////////////////////////////////////////////////////////////////////////////
/// @brief class to parse command line options
///
/// The Options constructor expects a command-name (usually argv[0]) and
/// a pointer to an array of strings.  The last element in this array MUST
/// be null. Each non-null string in the array must have the following format:
///
///  The 1st character must be the option-name ('c' for a -c option).
///
///  The 2nd character must be one of '|', '?', ':', '*', or '+'.
///  - '|' indicates that the option takes NO argument;
///  - '?' indicates that the option takes an OPTIONAL argument;
///  - ':' indicates that the option takes a REQUIRED argument;
///  - '*' indicates that the option takes 0 or more arguments;
///  - '+' indicates that the option takes 1 or more arguments;
///
/// The remainder of the string must be the long-option name.
///
/// If desired, the long-option name may be followed by one or more
/// spaces and then by the name of the option value. This name will
/// be used when printing usage messages. If the option-value-name
/// is not given then the string "<value>" will be used in usage
/// messages.
///
/// One may use a space to indicate that a particular option does not
/// have a corresponding long-option.  For example, "c: " (or "c:")
/// means the -c option takes a value & has NO corresponding long-option.
///
/// To specify a long-option that has no corresponding single-character
/// option is a bit trickier: Options::operator() still needs an "option-
/// character" to return when that option is matched. One may use a whitespace
/// character or a non-printable character as the single-character option
/// in such a case. (hence " |hello" would only match "--hello").
///
/// @par EXCEPTIONS TO THE ABOVE
///
/// If the 1st character of the string is '-', then the rest of the
/// string must correspond to the above format, and the option is
/// considered to be a hidden-option. This means it will be parsed
/// when actually matching options from the command-line, but will
/// NOT show-up if a usage message is printed using the usage() member
/// function. Such an example might be "-h|hidden". If you want to
/// use any "dummy" options (options that are not parsed, but that
/// to show up in the usage message), you can specify them along with
/// any positional parameters to the usage() member function.
///
/// If the 2nd character of the string is 0x00 then it is assumed
/// that there is no corresponding long-option and that the option
/// takes no argument (hence "f", and "f| " are equivalent).
///
/// Examples:
///
/// @verbatim
///      const char * optv[] = {
///          "c:count   <number>",
///          "s?str     <string>",
///          "x",
///          " |hello",
///          "g+groups  <newsgroup>",
///          0
///      };
///
///      optv[] now corresponds to the following:
///
///      usage: cmdname [-c|--count <number>] [-s|--str [<string>]]
///                     [-x] [--hello] [-g|--groups <newsgroup> ...]
/// @endverbatim
///
/// Long-option names are matched case-insensitive and only a unique prefix
/// of the name needs to be specified.
///
/// Option-name characters are case-sensitive!
///
/// @par CAVEAT
///
/// Because of the way in which multi-valued options and options with optional
/// values are handled, it is NOT possible to supply a value to an option in
/// a separate argument (different argv[] element) if the value is OPTIONAL
/// and begins with a '-'. What this means is that if an option "-s" takes an
/// optional value value and you wish to supply a value of "-foo" then you must
/// specify this on the command-line as "-s-foo" instead of "-s -foo" because
/// "-s -foo" will be considered to be two separate sets of options.
///
/// A multi-valued option is terminated by another option or by the end-of
/// options. The following are all equivalent (if "-l" is a multi-valued
/// option and "-x" is an option that takes no value):
///
/// - cmdname -x -l item1 item2 item3 -- arg1 arg2 arg3
/// - cmdname -x -litem1 -litem2 -litem3 -- arg1 arg2 arg3
/// - cmdname -l item1 item2 item3 -x arg1 arg2 arg3
///
///
/// @par EXAMPLE
///
/// @verbatim
/// #include "Options/Options.h"
///
/// #include <iostream>
///
/// static const char * optv[] = {
///   "H|help",
///   "c:count   <number>",
///   "s?str     <string>",
///   "x",
///   " |hello",
///   "g+groups  <newsgroup>",
///   "u*qwertz   <users>",
///   "-h|hidden",
///   0
/// };
///
/// using namespace std;
/// using namespace triagens;
///
/// int main (int argc, char * argv []) {
///   Options opts(*argv, optv);
///   OptionsArgvIterator iter(--argc, ++argv);
///
///   // opts.setControls(OptionSpecification::SHORT_ONLY);
///   // opts.setControls(OptionSpecification::LONG_ONLY);
///   // opts.setControls(OptionSpecification::NOGUESSING);
///   // opts.setControls(OptionSpecification::PLUS);
///
///   cout << opts.usage("files") << endl
///        << "where" << endl
///        << "  -H|--help        takes no argument" << endl
///        << "  -c|--count       takes one required argument" << endl
///        << "  -s|--str         takes one optional argument" << endl
///        << "  -x               has no long option and takes no argument" << endl
///        << "  --hello          has no short option and takes no argument" << endl
///        << "  -g|--groups      takes one or more arguments" << endl
///        << "  -u|--qwertz      takes no or more arguments" << endl
///        << "  -h|--hidden      is hidden and takes no argument" << endl
///        << endl;
///
///   int optchar;
///   string optarg;
///
///   while ((optchar = opts(iter, optarg)) != Options::OPTIONS_END_OPTIONS) {
///     switch (optchar) {
///     case ' ':
///     case 'H':
///     case 'c':
///     case 'g':
///     case 'h':
///     case 's':
///     case 'u':
///     case 'x':
///       cout << "option '" << (char) optchar << "' with argument '" + optarg + "'" << endl;
///       break;
///
///     case Options::OPTIONS_POSITIONAL:
///       cout << "positional '" << optarg << "'" << endl;
///       break;
///
///     case Options::OPTIONS_MISSING_VALUE:
///     case Options::OPTIONS_EXTRA_VALUE:
///     case Options::OPTIONS_AMBIGUOUS:
///     case Options::OPTIONS_BAD_KEYWORD:
///     case Options::OPTIONS_BAD_CHAR:
///       cout << "error '" << optarg << "'" << endl;
///       return 127;
///
///     default:
///       cout << "Oops! Got '" << optchar << "'" << endl;
///       return 127;
///     }
///   }
///
///   const string * args;
///
///   while ((args = iter()) != 0) {
///     cout << "remaining '" << *args << "'" << endl;
///   }
///
///   return 0;
/// }
/// @endverbatim
///
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Options {
public:
	static const int OPTIONS_POSITIONAL = 0;
	static const int OPTIONS_END_OPTIONS = -1;
	static const int OPTIONS_BAD_CHAR = -2;
	static const int OPTIONS_BAD_KEYWORD = -3;
	static const int OPTIONS_AMBIGUOUS = -4;
	static const int OPTIONS_MISSING_VALUE = -5;
	static const int OPTIONS_EXTRA_VALUE = -6;

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs class from options description
	////////////////////////////////////////////////////////////////////////////////

	Options(const string& programName, const char * const optv[]);

	virtual ~Options();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the program name
	////////////////////////////////////////////////////////////////////////////////

	const string& getProgramName() const {
		return programName;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the control set
	////////////////////////////////////////////////////////////////////////////////

	OptionSpecification::OptionControls getControls() const {
		return optionControls;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets the control set
	////////////////////////////////////////////////////////////////////////////////

	void setControls(OptionSpecification::OptionControls newctrls) {
		optionControls = newctrls;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true, if "--" has been given
	///
	/// Call this member function after operator() has returned 0 if
	/// you want to know whether or not options were explicitly
	/// terminated because "--" appeared on the command-line.
	////////////////////////////////////////////////////////////////////////////////

	bool hasEndOptions() const {
		return endOptions;
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns an usage message
	////////////////////////////////////////////////////////////////////////////////

	string usage(const string& positionals) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief clears state and start again
	////////////////////////////////////////////////////////////////////////////////

	void clear();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief iterates through the arguments.
	///
	/// Iterates through the arguments as necessary (using the
	/// given iterator) and returns the character value of the option
	/// (or long-option) that it matched. If the option has a value then the
	/// value given may be found in optarg (otherwise optarg will be empty).
	///
	/// OPTIONS_END_OPTIONS is returned upon end-of-options. At this point,
	/// "iter" may be used to process any remaining positional parameters. If
	/// the PARSE_POS control-flag is set then 0 is returned only when all
	/// arguments in "iter" have been exhausted.
	///
	/// If an invalid option is found then OPTIONS_BAD_CHAR is returned and
	/// optarg is the unrecognized option character.
	///
	/// If an invalid long-option is found then OPTIONS_BAD_KEYWORD is returned
	/// and optarg points to the bad long-option.
	///
	/// If an ambiguous long-option is found then OPTIONS_AMBIGUOUS is returned
	/// and optarg points to the ambiguous long-option.
	///
	/// If the PARSE_POS control-flag is set then OPTIONS_POSITIONAL is returned
	/// when a positional argument is encountered and optarg points to the
	/// positonal argument (and "iter" is advanced to the next argument in the
	/// iterator).
	///
	/// Unless Options::QUIET is used, missing option-arguments and invalid
	/// options (and the like) will automatically cause error messages to be
	/// issued to cerr.
	////////////////////////////////////////////////////////////////////////////////

	int operator()(OptionsIterator & iter, string & optarg);

private:
	static size_t MAX_COLUMN_WIDTH;

	enum State_t {
		STATE_ARGUMENTS_EXHAUSTED, STATE_LOAD_ARGUMENT, STATE_LOAD_VALUE, STATE_PROCESS_ARGUMENT, STATE_PROCESS_CURRENT_OPTION, STATE_PROCESS_LONG_OPTION, STATE_PROCESS_SHORT_OPTION, STATE_PROCESS_SHORT_OPTION_NO_GUESS, STATE_RETURN_ERROR, STATE_RETURN_LIST_OPTION, STATE_RETURN_POSITIONAL, STATE_RETURN_VALUE
	};

private:
	Options(const Options&);
	Options& operator=(const Options&);

private:
	bool isAnycase() {
		return (optionControls & OptionSpecification::ANYCASE) != 0;
	}

	bool isGuessing() {
		return (optionControls & OptionSpecification::NOGUESSING) == 0;
	}

	bool isQuite() {
		return (optionControls & OptionSpecification::QUIET) != 0;
	}

	bool isOptionsOnly() {
		return (optionControls & OptionSpecification::PARSE_POS) == 0;
	}

	bool isOption(const string & option) {
		if ((optionControls & OptionSpecification::PLUS) && option[0] == '+') {
			return true;
		}

		if (option.size() < 2) {
			return false;
		}

		return option[0] == '-' ? true : false;
	}

	string stripOption(const string & option) {
		string result = option;

		if ((optionControls & OptionSpecification::PLUS) && result[0] == '+') {
			result.erase(0, 1);
		} else if (result[0] == '-') {
			if (result[1] == '-') {
				result.erase(0, 2);
			} else {
				result.erase(0, 1);
			}
		}

		return result;
	}

	bool isShortOption(const string & option) {
		if (optionControls & OptionSpecification::LONG_ONLY) {
			return false;
		}

		if (option.size() < 2) {
			return false;
		}

		return option[0] == '-' ? (option[1] == '-' ? false : true) : false;
	}

	bool isLongOption(const string & option) {
		if (optionControls & OptionSpecification::SHORT_ONLY) {
			return false;
		}

		if ((optionControls & OptionSpecification::PLUS) && option[0] == '+') {
			return true;
		}

		if (option.size() < 2) {
			return false;
		}

		if (optionControls & OptionSpecification::LONG_ONLY) {
			return option[0] == '-' ? true : false;
		}

		return (2 <= option.length() && option[0] == '-' && option[1] == '-') ? true : false;
	}

private:
	//void format(ostream&, const string& part, size_t margin, size_t& len) const;
	const OptionSpecification * matchOption(char opt, bool ignore_case = false) const;
	const OptionSpecification * matchLongOption(const string& option, bool & isAmbiguous) const;
	void loadArgument(OptionsIterator &);
	void processArgument(OptionsIterator &);
	int returnPositional(OptionsIterator &, string & optarg);
	void processShortOption(OptionsIterator &, bool allowGuessing);
	int returnListOption(OptionsIterator &, string & optarg);
	void processLongOption(OptionsIterator &);
	void processCurrentOption();
	void loadValue(OptionsIterator &);
	int returnValue(string & optarg);
	int returnError(string & optarg);

private:
	string programName;
	OptionSpecification::OptionControls optionControls;
	vector<OptionSpecification*> specifications;

	State_t state;
	int errorState;

	const string * currentArgument;

	string unprocessedOptions;
	string currentOptionName;
	string errorMessage;
	string currentValue;

	bool endOptions;
	bool withinGuessing;

	const OptionSpecification * currentOption;
	const OptionSpecification * listOption;
};

}

#endif
