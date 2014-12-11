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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "PaloDispatcher/DirectPaloJob.h"

#include "Collections/StringUtils.h"
#include "Exceptions/ErrorException.h"
#include "Exceptions/WorkerException.h"
#include "HttpServer/HttpResponse.h"
#include "InputOutput/Statistics.h"
#include "Olap/Context.h"
#include "Olap/Server.h"
#include "Olap/PaloSession.h"
#ifdef ENABLE_GPU_SERVER
#include "Exceptions/GpuException.h"
#include "Exceptions/CudaRTException.h"
#endif
#include <boost/date_time.hpp>

using namespace boost::posix_time;

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

DirectPaloJob::DirectPaloJob(PaloJobRequest* jobRequest) :
		PaloJob(jobRequest)
{
}

// /////////////////////////////////////////////////////////////////////////////
// Job methods
// /////////////////////////////////////////////////////////////////////////////

void DirectPaloJob::work()
{
	for (int i = 0; i < 2; i++) {
		ptime startTime = microsec_clock::universal_time();
		try {
			Statistics::Timer timer(getName(), "direct");

			if (session) {
				session->onJobStart(this);
			}
			compute();
#ifdef ENABLE_GPU_SERVER
		} catch (const GpuException& e) {
			if (!i && e.getErrorType() == GpuException::ERROR_GPU_CANCELED_READ_REQUEST) {
				clear();
				Context::getContext()->turnOffGPU();
				Logger::warning << "error code: " << (int32_t)e.getErrorType() << " description: " << ErrorException::getDescriptionErrorType(e.getErrorType()) << " message: " << e.getMessage() << endl;
				Logger::info << "trying again on CPU ... " << endl;
				continue;
			} else {
				handleException(ErrorException::ERROR_INTERNAL, "GPU Error", "GPU Internal Error.");
			}
		} catch (const CudaRTException& e) {
            clear();
			Context *context = Context::getContext(0, false);
			server = context->getServerCopy();
			server->ShutdownGpuEngine(PUser());
            server->commit();
            clear();
            Logger::error << "error code: " << (int32_t)e.getErrorType() << " description: " << ErrorException::getDescriptionErrorType(e.getErrorType()) << " message: " << e.getMessage() << endl;
            Logger::warning << "GPU Engine deactivated after GPU-internal exception." << endl;
            continue;
#endif
		} catch (const WorkerException& e) {
			handleException(e.getErrorType(), e.getMessage(), e.getDetails());
		} catch (const ErrorException& e) {
			handleException(e.getErrorType(), e.getMessage(), e.getDetails());
		} catch (const bad_alloc&) {
			handleException(ErrorException::ERROR_OUT_OF_MEMORY, "Not enough memory", "");
		}
		if (session) {
			ptime currentTime(microsec_clock::universal_time());
			uint64_t liveTime = (uint64_t)(currentTime-startTime).total_microseconds();
			session->increaseTime(liveTime, this);
		}

		return;
	}
}

void DirectPaloJob::handleException(ErrorException::ErrorType type, const string& message, const string& details)
{

	if (ErrorException::ERROR_OUT_OF_MEMORY == type) {
		Context::getContext()->freeEngineCube();
	}

	response = new HttpResponse(HttpResponse::BAD);

	StringBuffer& body = response->getBody();

	body.appendCsvInteger((int32_t)type);

	if (type != ErrorException::ERROR_WORKER_MESSAGE) { // thrown from SVS, use only message (instead of type) and details
		body.appendCsvString(StringUtils::escapeString(ErrorException::getDescriptionErrorType(type)));
	}
	
	body.appendCsvString(StringUtils::escapeString(message));
	if (details != "") {
		body.appendCsvString(StringUtils::escapeString(details));
	}
	body.appendEol();

	Logger::warning << "error code: " << (int32_t)type << " description: " << ErrorException::getDescriptionErrorType(type) << ", message: " << message << " details: " << details << endl;

}

}
