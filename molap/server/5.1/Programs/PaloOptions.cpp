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
 * \author Zurab Khadikov, Jedox AG, Freiburg, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "PaloOptions.h"

#include <iostream>

#include "Logger/Logger.h"
#include "Olap/PaloSession.h"
#include "Olap/RollbackStorage.h"
#include "Olap/Server.h"
#include "Options/OptionsFileIterator.h"
#include "Worker/LoginWorker.h"
#include "Worker/DimensionWorker.h"
#include "PaloJobs/AreaJob.h"
#include "InputOutput/FileReaderBF.h"

namespace palo {
using namespace std;

// /////////////////////////////////////////////////////////////////////////////
// list of all options
// /////////////////////////////////////////////////////////////////////////////

// build options parser
static const char * AllowedOptions[] = {"?|help",
        "a+admin                 <address> <port>",
        "A|auto-load",
        "b:cache-barrier         <maximum of number_of_cells to store in each Cube cache>",
        "B|auto-commit",
        "c|crypt",
        "C|chdir",
        "d:data-directory        <directory>",
        "D|add-new-databases",
        "e|windows-sso",
        "E:extensions            <directory>",
        "-f|fake-session-id",
#if defined(_MSC_VER)
        "F:friendly-service-name <service-name>",
#endif
        "g:cross-origin          <domain_name>",
        "h+http                  <address> <port>",
#if defined(ENABLE_HTTPS)
        "H+https                 <port>",
#endif
        "i:init-file             <init-file>",
        "I|ignore-journal",
        "j*device                <device_ids>",
        "J|no-csv-save",
        "k:crypt-key             <passphrase for crypting csv files>",
        "K+key-files             <ca> <private> <dh>",
        "l:maximum-return-cells  <maximum_return_cells>",
        "L+splash-limit          <error> <warning> <info>",
        "M:session-timeout       <seconds>",
        "m:undo-memory-size      <undo_memory_size_in_bytes_per_lock>",
        "n|load-init-file",
        "o:log                   <logfile>",
        "O|amazon-id",
        "p:password              <private-password>",

        "P|enable-gpu",
#if defined(_MSC_VER)
        "q+service-description   <service-description>",
#endif
        "Q+autosave              <mode> <hour>:<minute>",
        "r|short-sid",
        "R:default-db-right      <N, R, W, or D>",
#if defined(_MSC_VER)
        "s|start-service",
        "S:service-name          <service-name>",
#endif
        "t:template-directory    <directory>",
#ifdef ENABLE_TRACE_OPTION
        "-T:trace                <trace-file>",
#endif
        "u:undo-file-size        <undo_file_size_in_bytes_per_lock>",
        "-U|ignore-cell-data",
        "v:verbose               <level>",
        "V|version",
        "w+worker                <worker-executable> <argument1> <argument2> <argumentX>",
		"W|use-dimension-worker",
        "x:workerlogin           <worker-login-type>",
        "X:encryption            <encryption-type>",
        "y|enable-drillthrough",
        "Y|use-cube-worker",
        "z:goalseek-timeout      <miliseconds>",
        "Z:goalseek-limit        <number_of_cells>",
        0
                                       };

/****************************************************************************
 **

 *defines rule error behaviour. affects results for division by zero
 */

// /////////////////////////////////////////////////////////////////////////////
// constructors
// /////////////////////////////////////////////////////////////////////////////

PaloOptions::PaloOptions(int argc, char** argv) :
	arguments(argc - 1, argv + 1), options(*argv, AllowedOptions)
{
	options.setControls(OptionSpecification::NOGUESSING);

	autoAddDb = true;
	autoCommit = true;
	ignoreJournal = false;
	autoLoadDb = true;
	cacheBarrier = 1000000.0;
	changeDirectory = true;
	dataDirectory = "./Data";
	defaultTtl = -1;
	drillThroughEnabled = false;
	amazonEnabled = false;
	encryptionType = ENC_NONE;
#if defined(_MSC_VER)
	extensionsDirectory = "../Modules";
#else
	std::string suffix = std::string(OUT_LIB_SUFFIX);
	extensionsDirectory = "usr/lib"+suffix;
#endif
	friendlyServiceName = "PALO Server Service";
	goalseekCellLimit = 1000;
	goalseekTimeout = 10000;
	ignoreCellData = false;
	initFile = "palo.ini";
	logFile = "-";
	outputToStdout = false;
	logLevel = "error";
	loginType = WORKER_NONE;
	winSSOenabled = false;
	serviceName = "PALOServerService";
	shortSid = false;
	showUsage = false;
	showVersion = false;
	startAsService = false;
	templateDirectory = "../Api";
	undoFileSize = 50 * 1024 * 1024;
	undoMemorySize = 10 * 1024 * 1024;
	useCubeWorkers = false;
	useDimensionWorker = false;
	useFakeSession = false;
	useInitFile = true;

	externalHttpInterface = 0;
	externalHttpsInterface = 0;
	externalJobAnalyser = 0;

	service_description = "Jedox Suite MOLAP Server Service";
	maximumReturnCells = 20000;

	enableGpu = false;

	crypt = false;
	saveCSV = true;
}

// /////////////////////////////////////////////////////////////////////////////
// public methods
// /////////////////////////////////////////////////////////////////////////////

void PaloOptions::parseOptions()
{
	parseOptions(arguments, true);
}

void PaloOptions::parseOptions(const string &filename)
{
	OptionsFileIterator file(filename);

	parseOptions(file, false);
}

void PaloOptions::checkOptions()
{

	// check key files
	if (!keyFiles.empty()) {
		if (keyFiles.size() % 3 != 0) {
			cout << "expecting three key files, not " << keyFiles.size() << "\n" << endl;
			usage();
		}

		if (keyFiles.size() > 3) {
			keyFiles.erase(keyFiles.begin(), keyFiles.end() - 3);
		}
	}

	// splash limits
	if (!splashLimits.empty()) {
		if (splashLimits.size() % 3 != 0) {
			cout << "expecting three splash limits, not " << splashLimits.size() << "\n" << endl;
			usage();
		}

		if (splashLimits.size() > 3) {
			splashLimits.erase(splashLimits.begin(), splashLimits.end() - 3);
		}
	}

	// check http host/port pairs
	if (httpPorts.size() % 2 == 1) {
		cout << "not enough arguments to --http\n" << endl;
		usage();
	}

	// check https ports
	if (!httpsPorts.empty() && httpsPorts.size() != httpPorts.size() / 2) {
		cout << "got " << httpsPorts.size() << " https ports, but " << (httpPorts.size() / 2) << " http addresses\n" << endl;
		usage();
	}

	if (encryptionType == ENC_REQUIRED) {
		if (!httpPorts.empty() && httpsPorts.empty()) {
			cout << "encryption type 'REQUIRED', but no https ports given\n" << endl;
			usage();
		}
	}

	// check admin host/port pairs
	if (adminPorts.size() % 2 == 1) {
		cout << "not enough arguments to --admin\n" << endl;
		usage();
	}

	if (httpPorts.empty() && adminPorts.empty()) {
		cout << "need at least one --http or --admin option\n" << endl;
		usage();
	}

	// check undo size
	if (undoMemorySize < 1024 * 1024) {
		undoMemorySize = 1024 * 1024;
		Logger::error << "undo memory size too small, using a size of 1MB" << endl;
	}

	string okStrings = "NRWD";
	if (defaultDbRight.length() > 1 || defaultDbRight.find_first_not_of(okStrings) != string::npos) {
		cout << "invalid value for default-db-right\n" << endl;
		usage();
	}
}

void PaloOptions::updateGlobals()
{
	PServer server = Context::getContext()->getServerCopy();
	if (defaultTtl != -1) {
		server->setDefaultTtl(defaultTtl);
		Logger::debug << "setting default session timeout to " << defaultTtl << endl;
	}

	if (splashLimits.size() == 3) {
		Cube::splashLimit1 = splashLimits[0];
		Cube::splashLimit2 = splashLimits[1];
		Cube::splashLimit3 = splashLimits[2];
	}

	Logger::debug << "using splash limits " << Cube::splashLimit1 << ", " << Cube::splashLimit2 << ", " << Cube::splashLimit3 << endl;

	server->setLoginType(loginType);
	server->setWinAuth(winSSOenabled);
	server->setDrillThroughEnabled(drillThroughEnabled);
	server->setEncryptionType(encryptionType);
	server->setShortSid(shortSid);
	server->setEnableGpu(enableGpu);

	Server::setCrossOrigin(crossOrigin);
	Cube::setCacheBarrier(cacheBarrier);
	Cube::setGoalseekCellLimit(goalseekCellLimit);
	Cube::setGoalseekTimeout(goalseekTimeout);
	Cube::setIgnoreCellData(ignoreCellData);
	Cube::setSaveCSV(saveCSV);
	if (defaultDbRight.length()) {
		Server::setDefaultDbRight(defaultDbRight);
	}

	CubeWorker::setUseCubeWorker(useCubeWorkers);

	RollbackStorage::setMaximumMemoryRollbackSize(undoMemorySize);
	RollbackStorage::setMaximumFileRollbackSize(undoFileSize);

	//AreaJob::setMaximalAreaGrowSize(maximalAreaGrowSize);
	AreaJob::setMaxCellCount(maximumReturnCells);

	if (loginType != WORKER_NONE || useCubeWorkers || useDimensionWorker || winSSOenabled) {
		Worker::setExecutable(workerProgram);
		Worker::setArguments(workerProgramArguments);

		string arg;

		for (vector<string>::iterator i = workerProgramArguments.begin(); i != workerProgramArguments.end(); ++i) {
			arg += " \'" + *i + "'";
		}

		Logger::info << "worker executable: '" << workerProgram << "'" << endl;
		Logger::info << "worker arguments: " << arg << endl;

		if (loginType != WORKER_NONE || winSSOenabled) {
			boost::shared_ptr<PaloSession> session = PaloSession::createSession(PUser(), true, 0, shortSid, false, "worker", 0, 0, 0, "worker", "");
			PLoginWorker worker(new LoginWorker(session->getSid()));
			server->setLoginWorker(worker);
		}
		if (useDimensionWorker) {
			boost::shared_ptr<PaloSession> session = PaloSession::createSession(PUser(), true, 0, shortSid, false, "worker", 0, 0, 0, "worker", "");
			PDimensionWorker worker(new DimensionWorker(session->getSid()));
			server->setDimensionWorker(worker);
		}
	}

	bffilebuf::setOptions(cryptPassphrase, 0x1122334455667788LL, crypt);

	if (ignoreJournal) {
		autoCommit = true;
		Server::ignoreJournal = true;
	}

	server->commit();
	Context::reset();
}

void PaloOptions::version()
{
	cout << "Palo Server Version " << Server::getVersionRevision() << endl;
	exit(0);
}

void PaloOptions::usage()
{
	if (showUsage) {
		cout << "Palo Server Version " << Server::getVersionRevision() << endl;
	}

	cout << options.usage("") << "\n"
#if defined(ENABLE_HTTPS)
	     << "<encryption-type> can be 'none', 'optional', or 'required'\n"
#endif
	     << "<worker-login-type> can be 'information', 'authentication',\n" << "or 'authorization'. Setting <address> to \"\" binds the port to any\n" << "address. By default palo will change into the data directory before\n" << "opening the log file.\n" << endl;

	if (showUsage) {
		cout << "data-directory:        " << dataDirectory << "\n"
		     << "chdir on startup:      " << (changeDirectory ? "true" : "false") << "\n"
		     << "init-file:             " << initFile << "\n"
		     << "use init-file:         " << (useInitFile ? "true" : "false") << "\n"
		     << "auto-load databases:   " << (autoLoadDb ? "true" : "false") << "\n"
		     << "auto-add databases:    " << (autoAddDb ? "true" : "false") << "\n"
		     << "auto-commit on exit:   " << (autoCommit ? "true" : "false") << "\n"
		     << "use cube workers:      " << (useCubeWorkers ? "true" : "false") << "\n"
		     << "use dimension worker:  " << (useDimensionWorker ? "true" : "false") << "\n"
		     << "drillthrough enabled:  " << (drillThroughEnabled ? "true" : "false") << "\n"
		     << "cache-barrier:         " << cacheBarrier << "\n"
		     << "gpu server enabled:    " << (enableGpu ? "true" : "false") << "\n"
		     << "gpu device ids:              list of gpu device ids\n"
		     ;
		cout << "\n"
		     << "The server side cache is configured to 1000000 cells per cube\n"
		     << "by default. In order to deactivate the cache completely set\n"
		     << "<cache-barrier> to 0.\n";

		cout << "\n"
		     << "In a locked cube area it is possible to undo changes. Each lock can use\n"
		     << "<undo_memory_size_in_bytes_per_lock> bytes in memory and\n"
		     << "<undo_file_size_in_bytes_per_lock> bytes in files for storing changes:\n"
		     << "\n"
		     << "undo memory size in byte per lock: " << undoMemorySize << " (" << int(undoMemorySize / 1024 / 1024) << "MB)\n"
		     << "undo file size in byte per lock:   " << undoFileSize << " (" << int(undoFileSize / 1024 / 1024) << "MB)\n"
		     << "\n"
		     << "goalseek cell limit:   " << goalseekCellLimit << "\n"
		     << "goalseek timeout:   " << goalseekTimeout << "ms\n"

#if defined(_MSC_VER)
		     << "\n"
		     << "Or: " << options.getProgramName() << " [service-related-options] install_srv <data-directory>\n"
		     << "    " << options.getProgramName() << " [service-related-options] delete_srv\n\n"
		     << "to install or delete a Windows XP service. You can change the service\n"
		     << "name with --service-name. The default is '" << serviceName << "'\n"
#endif
		     << endl;
	}

	exit(showUsage ? 0 : 1);
}

// /////////////////////////////////////////////////////////////////////////////
// private methods
// /////////////////////////////////////////////////////////////////////////////

void PaloOptions::parseOptions(OptionsIterator& iter, bool commandLine)
{
	options.clear();

	int optchar;
	string optarg;
	bool seenWorker = false;

	while ((optchar = options(iter, optarg)) != Options::OPTIONS_END_OPTIONS) {
		if (optchar == 'w') {
			if (!seenWorker) {
				workerProgram = "";
				workerProgramArguments.clear();
			}

			seenWorker = true;
		} else {
			seenWorker = false;
		}

		try {
			int i;
			double d;
			switch (optchar) {
			case 'a':
				if (adminPorts.size() % 2 == 1) {
					i = StringUtils::stringToInteger(optarg);
				}
				adminPorts.push_back(optarg);
				break;

			case 'A':
				autoLoadDb = !autoLoadDb;
				break;

			case 'b':
				d = StringUtils::stringToDouble(optarg);
				cacheBarrier = d;
				break;

			case 'B':
				autoCommit = !autoCommit;
				break;

			case 'c':
				crypt = !crypt;
				break;

			case 'C':
				if (commandLine) {
					changeDirectory = !changeDirectory;
				} else {
					printError(ErrUnknownArgument, "chdir");
				}
				break;

			case 'd':
				if (commandLine) {
					dataDirectory = optarg;
				} else {
					printError(ErrUnknownArgument, "data-directory");
				}
				break;

			case 'D':
				autoAddDb = !autoAddDb;
				break;

			case 'e':
				winSSOenabled = !winSSOenabled;
				break;

			case 'E':
				extensionsDirectory = optarg;
				break;

			case 'f':
				useFakeSession = !useFakeSession;
				break;

#if defined(_MSC_VER)
			case 'F':
				friendlyServiceName = optarg;
				break;
#endif

			case 'g':
				crossOrigin = optarg;
				break;

			case 'h':
				if (httpPorts.size() % 2 == 1) {
					i = StringUtils::stringToInteger(optarg);
				}
				httpPorts.push_back(optarg);
				break;

#if defined(ENABLE_HTTPS)
			case 'H':
				i = StringUtils::stringToInteger(optarg);
				httpsPorts.push_back(optarg);
				break;
#endif

			case 'i':
				if (commandLine) {
					initFile = optarg;
				} else {
					printError(ErrUnknownArgument, "init-file");
				}
				break;

			case 'I':
				ignoreJournal = true;
				break;

			case 'j':
				device_ids.push_back(optarg);
				break;

			case 'J':
				saveCSV = false;
				break;

			case 'k':
				cryptPassphrase = optarg;
				break;

			case 'K':
				keyFiles.push_back(optarg);
				break;

			case 'l':
				i = StringUtils::stringToInteger(optarg);
				maximumReturnCells = i;
				break;

			case 'L':
				d = StringUtils::stringToDouble(optarg);
				splashLimits.push_back(d);
				break;

			case 'm':
				i = StringUtils::stringToInteger(optarg);
				undoMemorySize = i;
				break;

			case 'M':
				i = StringUtils::stringToInteger(optarg);
				defaultTtl = i;
				break;

			case 'n':
				if (commandLine) {
					useInitFile = !useInitFile;
				} else {
					printError(ErrUnknownArgument, "load-init-file");
				}
				break;

			case 'o':
				if (optarg == "-") {
					outputToStdout = true;
				} else {
					logFile = optarg;
				}
				break;

			case 'O':
				amazonEnabled = !amazonEnabled;
				break;

			case 'p':
				keyFilePassword = optarg;
				break;

			case 'P':
				enableGpu = !enableGpu;
				break;

#if defined(_MSC_VER)
			case 'q':
				service_description = optarg;
				break;
#endif
			case 'Q':
				autosave_definition.push(optarg);
				break;

			case 'r':
				shortSid = !shortSid;
				break;

			case 'R':
				defaultDbRight = optarg;
				break;

#if defined(_MSC_VER)
			case 's':
				startAsService = !startAsService;
				break;

			case 'S':
				serviceName = optarg;
				break;
#endif

			case 't':
				templateDirectory = optarg;
				break;

			case 'T':
				traceFile = optarg;
				break;

			case 'u':
				i = StringUtils::stringToInteger(optarg);
				undoFileSize = i;
				break;

			case 'U':
				ignoreCellData = !ignoreCellData;
				break;

			case 'v':
				logLevel = optarg;
				break;

			case 'V':
				if (commandLine) {
					showVersion = !showVersion;
				} else {
					printError(ErrUnknownArgument, "version");
				}
				break;

			case 'w':
				if (workerProgram == "") {
					workerProgram = optarg;
				} else {
					workerProgramArguments.push_back(optarg);
				}
				break;

			case 'W':
				useDimensionWorker = !useDimensionWorker;
				break;

			case 'x':
				loginType = convertWorkerLoginType(optarg);
				break;

			case 'X':
				encryptionType = convertEncryptionType(optarg);
				break;

			case 'y':
				drillThroughEnabled = !drillThroughEnabled;
				break;

			case 'Y':
				useCubeWorkers = !useCubeWorkers;
				break;

			case 'z':
				i = StringUtils::stringToInteger(optarg);
				goalseekTimeout = i;
				break;

			case 'Z':
				i = StringUtils::stringToInteger(optarg);
				goalseekCellLimit = i;
				break;

			case '?':
				if (commandLine) {
					showUsage = !showUsage;
				} else {
					printError(ErrUnknownArgument, "help");
				}
				break;

			case Options::OPTIONS_POSITIONAL:
			case Options::OPTIONS_MISSING_VALUE:
			case Options::OPTIONS_EXTRA_VALUE:
			case Options::OPTIONS_AMBIGUOUS:
			case Options::OPTIONS_BAD_KEYWORD:
			case Options::OPTIONS_BAD_CHAR:
				printError(ErrMessage, optarg);
				break;

			default:
				if (commandLine) {
					printError((char)optchar);
				} else {
					printError(ErrUnknownArgument, optarg);
				}
				break;
			}
		} catch (const ErrorException &e) {
			switch (optchar) {
			case 'a':
			case 'b':
			case 'h':
			case 'H':
			case 'I':
			case 'j':
			case 'l':
			case 'L':
			case 'm':
			case 'M':
			case 'u':
			case 'z':
			case 'Z':
				PaloOptions::printError(ErrNumericConversion, optarg);
				break;
			case 'x':
			case 'X':
				PaloOptions::printError(optchar == 'x' ? ErrWorkerLogin : ErrEncryption, optarg);
				break;
			default:
				if (e.getDetails().empty()) {
					PaloOptions::printError(ErrMessage, "exception '" + e.getMessage() + "'");
				} else {
					PaloOptions::printError(ErrMessage, "exception '" + e.getMessage() + "' (" + e.getDetails() + ")");
				}
				break;
			}
		}
	}
}

WorkerLoginType PaloOptions::convertWorkerLoginType(const string &type)
{
	if (UTF8Comparer::compare(type, "information") == 0) {
		return WORKER_INFORMATION;
	} else if (UTF8Comparer::compare(type, "authentication") == 0) {
		return WORKER_AUTHENTICATION;
	} else if (UTF8Comparer::compare(type, "authorization") == 0) {
		return WORKER_AUTHORIZATION;
	} else {
		throw ErrorException(ErrorException::ERROR_INVALID_STRING, type);
	}
}

Encryption_e PaloOptions::convertEncryptionType(const string &type)
{
	if (UTF8Comparer::compare(type, "none") == 0) {
		return ENC_NONE;
	} else if (UTF8Comparer::compare(type, "optional") == 0) {
		return ENC_OPTIONAL;
	} else if (UTF8Comparer::compare(type, "required") == 0) {
		return ENC_REQUIRED;
	} else {
		throw ErrorException(ErrorException::ERROR_INVALID_STRING, type);
	}
}

void PaloOptions::printError(char optchar)
{
	Logger::warning << "unknown option used: " << (char)optchar << endl;
	usage();
}

void PaloOptions::printError(ErrorType type, string s)
{
	switch (type) {
	case ErrUnknownArgument:
		Logger::error << "unknown option '" << s << "'" << endl;
		break;
	case ErrNumericConversion:
		Logger::error << "numeric conversion failed, '" << s << "' is not a number" << endl;
		break;
	case ErrWorkerLogin:
		Logger::error << "unknown worker login type '" << s << "'" << endl;
		break;
	case ErrEncryption:
		Logger::error << "unknown encryption type '" << s << "'" << endl;
		break;
	case ErrMessage:
		Logger::error << s << endl;
		break;
	}
	usage();
}

}
