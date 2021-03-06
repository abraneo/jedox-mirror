/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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
 * 
 *
 */

#ifndef PROGRAMS_PALO_OPTIONS_H
#define PROGRAMS_PALO_OPTIONS_H 1

#include "palo.h"

#include "Collections/StringUtils.h"
#include "Options/Options.h"
#include "Options/OptionsArgvIterator.h"
#include "AutosaveType.h"
#include "Programs/PaloHttpInterface.h"

namespace palo {
class Server;

struct ServerInfo_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief palo options
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS PaloOptions {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	PaloOptions(int argc, char** argv);

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief parses options
	////////////////////////////////////////////////////////////////////////////////

	void parseOptions();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief parses options from file
	////////////////////////////////////////////////////////////////////////////////

	void parseOptions(const string& filename);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks options
	////////////////////////////////////////////////////////////////////////////////

	void checkOptions();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief updates globals
	////////////////////////////////////////////////////////////////////////////////

	void updateGlobals();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief shows usage and exits
	////////////////////////////////////////////////////////////////////////////////

	void usage();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief shows version and exits
	////////////////////////////////////////////////////////////////////////////////

	void version();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief admin http address and port list (-a)
	////////////////////////////////////////////////////////////////////////////////

	vector<string> adminPorts;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief auto add databases found (-D)
	////////////////////////////////////////////////////////////////////////////////

	bool autoAddDb;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief auto commit databases and cubes on shutdown (-B)
	////////////////////////////////////////////////////////////////////////////////

	bool autoCommit;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Don't write to journal
	////////////////////////////////////////////////////////////////////////////////

	bool ignoreJournal;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief auto load databases and cubes on startup (-A)
	////////////////////////////////////////////////////////////////////////////////

	bool autoLoadDb;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief ignores arithmetic error generated by rules (-G)
	////////////////////////////////////////////////////////////////////////////////
	bool ignoreRuleError;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief palo barrier for caching element (-b)
	////////////////////////////////////////////////////////////////////////////////

	double cacheBarrier;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief number of changed cells for clearing cache (-g)
	////////////////////////////////////////////////////////////////////////////////

	double cacheClearBarrierCells;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief change into data directory (-C)
	////////////////////////////////////////////////////////////////////////////////

	bool changeDirectory;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief data directory (-d)
	////////////////////////////////////////////////////////////////////////////////

	string dataDirectory;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief default time to live for session (-M)
	////////////////////////////////////////////////////////////////////////////////

	int defaultTtl;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief enable drillthrough (-y)
	////////////////////////////////////////////////////////////////////////////////

	bool drillThroughEnabled;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief encryption type (-X)
	////////////////////////////////////////////////////////////////////////////////

	Encryption_e encryptionType;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief extensions directory (-E)
	////////////////////////////////////////////////////////////////////////////////

	string extensionsDirectory;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief user friendly service name (-F)
	////////////////////////////////////////////////////////////////////////////////

	string friendlyServiceName;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief goalseek cell limit (-J)
	////////////////////////////////////////////////////////////////////////////////

	int goalseekCellLimit;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief goalseek timeout (-j)
	////////////////////////////////////////////////////////////////////////////////

	int goalseekTimeout;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief http address and port list (-h)
	////////////////////////////////////////////////////////////////////////////////

	vector<string> httpPorts;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief http port list (-H)
	////////////////////////////////////////////////////////////////////////////////

	vector<string> httpsPorts; // address is identical to http

	////////////////////////////////////////////////////////////////////////////////
	/// @brief ignore cell data (-U)
	////////////////////////////////////////////////////////////////////////////////

	bool ignoreCellData;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief init file (-i)
	////////////////////////////////////////////////////////////////////////////////

	string initFile;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief password for private certificate (-p)
	////////////////////////////////////////////////////////////////////////////////

	string keyFilePassword;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief key files (-K)
	////////////////////////////////////////////////////////////////////////////////

	vector<string> keyFiles;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief logfile (-o)
	////////////////////////////////////////////////////////////////////////////////

	string logFile;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief log level (-v)
	////////////////////////////////////////////////////////////////////////////////

	string logLevel;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief use worker login (-x)
	////////////////////////////////////////////////////////////////////////////////

	WorkerLoginType loginType;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief service name (-e)
	////////////////////////////////////////////////////////////////////////////////

	bool winSSOenabled;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief service name (-S)
	////////////////////////////////////////////////////////////////////////////////

	string serviceName;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief splash limits (-K)
	////////////////////////////////////////////////////////////////////////////////

	vector<double> splashLimits;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start as Windows service (-s)
	////////////////////////////////////////////////////////////////////////////////

	bool startAsService;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief enables old style 4-letter session id (-r)
	////////////////////////////////////////////////////////////////////////////////

	bool shortSid;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief shows version info and exit (-V)
	////////////////////////////////////////////////////////////////////////////////

	bool showVersion;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief shows usage and exit (-?)
	////////////////////////////////////////////////////////////////////////////////

	bool showUsage;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief template directory (-t)
	////////////////////////////////////////////////////////////////////////////////

	string templateDirectory;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief trace file, no trace if empty (-T)
	////////////////////////////////////////////////////////////////////////////////

	string traceFile;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief palo maximum undo file size (-u)
	////////////////////////////////////////////////////////////////////////////////

	size_t undoFileSize;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief palo maximum undo memory size (-m)
	////////////////////////////////////////////////////////////////////////////////

	size_t undoMemorySize;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start worker for cubes (-Y)
	////////////////////////////////////////////////////////////////////////////////

	bool useCubeWorkers;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start worker for dimensions (-W)
	////////////////////////////////////////////////////////////////////////////////

	bool useDimensionWorker;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief use a fake session id for debugging (-f)
	////////////////////////////////////////////////////////////////////////////////

	bool useFakeSession;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief use init file (-n)
	////////////////////////////////////////////////////////////////////////////////

	bool useInitFile;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief worker program (-w)
	////////////////////////////////////////////////////////////////////////////////

	string workerProgram;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief worker program arguments (-w)
	////////////////////////////////////////////////////////////////////////////////

	vector<string> workerProgramArguments;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief governs the autosave behaviour (-Q)
	////////////////////////////////////////////////////////////////////////////////
	autosave_type autosave_definition;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief governs the autosave behaviour (-Q)
	////////////////////////////////////////////////////////////////////////////////
	std::string service_description;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief the maximum number of cells returned by an area (-l)
	////////////////////////////////////////////////////////////////////////////////
	size_t maximumReturnCells;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief enable gpu server (-P)
	////////////////////////////////////////////////////////////////////////////////

	bool enableGpu;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief device ids (-j)
	////////////////////////////////////////////////////////////////////////////////

	vector<string> device_ids;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief governs crypting of save files
	////////////////////////////////////////////////////////////////////////////////

	bool crypt;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Passphrase for crypting save files
	////////////////////////////////////////////////////////////////////////////////

	string cryptPassphrase;

	bool saveCSV;

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief http interface
	////////////////////////////////////////////////////////////////////////////////

	InitHttpInterface_fptr externalHttpInterface;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief https interface
	////////////////////////////////////////////////////////////////////////////////

	InitHttpsInterface_fptr externalHttpsInterface;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief job analyser
	////////////////////////////////////////////////////////////////////////////////

	InitJobAnalyser_fptr externalJobAnalyser;

private:
	void parseOptions(OptionsIterator &iter, bool commandLine);

	Encryption_e convertEncryptionType(const string &type);

	WorkerLoginType convertWorkerLoginType(const string &type);

	enum ErrorType {
		ErrUnknownArgument, ErrNumericConversion, ErrWorkerLogin, ErrEncryption, ErrMessage,
	};

	void printError(char optchar);
	void printError(ErrorType type, string s);

private:
	OptionsArgvIterator arguments;
	Options options;

};

}

#endif
