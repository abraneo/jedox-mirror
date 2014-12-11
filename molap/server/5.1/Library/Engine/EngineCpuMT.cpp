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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include <stack>
#include "Engine/EngineCpuMT.h"
#include "Olap/Context.h"
#include "Engine/Cache.h"
#include "Engine/AggregationProcessor.h"
#include "Thread/ThreadPool.h"
#include "Olap/Server.h"
#include "Engine/StorageCpu.h"

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp> //include all types plus i/o

using namespace boost::posix_time;

namespace palo {

class TestBase
{
public:
	static const size_t RUN_LIMIT_US = 100000;
	TestBase(string name = "TestBase") : name(name) {}
	void run() {
		for (iterations = 0, tStart = microsec_clock::local_time();;iterations++) {
			tEnd = microsec_clock::local_time();
			duration = size_t((tEnd-tStart).total_microseconds());
			if (duration > RUN_LIMIT_US) {
				break;
			}
			step();
		}
		end();
	}
	double iterationsPerSecond() const {return duration ? (double)iterations/duration*RUN_LIMIT_US : 0;}
	double uSecondsPerIteration() const {return iterations ? (double)duration/iterations : 0;}
protected:
	virtual void step() {}
	virtual void end() {}
private:
	string name;
	ptime tStart;
	ptime tEnd;
	size_t iterations;
	size_t duration;

	friend ostream& operator<<(ostream& ostr, const TestBase &test);
};

ostream& operator<<(ostream& ostr, const TestBase &test)
{
	ostr << "test: " << test.name << " finished in " << test.duration << " ms. Iterations " << test.iterations;
	ostr << " iterationsPerSecond: " << test.iterationsPerSecond() << " uSecondsPerIteration: " << test.uSecondsPerIteration();
	return ostr;
}

class AggregationProcessorMT : public AggregationProcessor, public StorageCpu::ProcessorCallback {
	friend class AggregationProcessorWorker;
public:
	AggregationProcessorMT(PEngineBase engine, CPPlanNode node) : AggregationProcessor(engine, node), reader(0), minJump(0), hashStorage(0) {}

	virtual void readCallback();

protected:
	virtual void aggregate();

private:
	void createStorage(double resultSize) {
		if (resultSize < 1000) {
			hashStorage = new HashValueStorage(aggregationPlan->getArea());
			storage.reset(hashStorage);
		} else {
			storage = CreateDoubleCellMap(aggregationPlan->getArea()->dimCount());
			storage->setLimit(IdentifiersType(), (aggregationPlan->getMaxCount() == 0 ? 0 : aggregationPlan->getMaxCount() + 1));
		}
	}

	boost::mutex dataMutex;
	StorageCpu::Processor *reader;
	PThreadPool tp;
	size_t minJump;
	ThreadPool::ThreadGroup tg;
	PDoubleCellMap threadStorage;
	HashValueStorage *hashStorage;
};

class AggregationProcessorWorker : public AggregationProcessor, public ThreadPoolJob, public StorageCpu::ProcessorCallback {
public:
	AggregationProcessorWorker(PEngineBase engine, CPPlanNode node, ThreadPool::ThreadGroup &tg, const StorageCpu::Processor &p, size_t s, size_t e, PThreadPool tp, AggregationProcessorMT &parent);
	virtual void readCallback();

private:
	virtual void operator()();

	StorageCpu::Processor reader;
	PThreadPool tp;
	AggregationProcessorMT &parent;
};

AggregationProcessorWorker::AggregationProcessorWorker(PEngineBase engine, CPPlanNode node, ThreadPool::ThreadGroup &tg, const StorageCpu::Processor &p, size_t s, size_t e, PThreadPool tp, AggregationProcessorMT &parent) :
	AggregationProcessor(engine, node), ThreadPoolJob(tg), reader(p), tp(tp), parent(parent)
{
	reader.mtCallback = this;
	reader.pos = s;
	reader.endp = e ? e : reader.pageList->size();
}

void AggregationProcessorWorker::operator()()
{
	double resultSize = aggregationPlan->getArea()->getSize();
	HashValueStorage *hashStorage = 0;
	if (resultSize < 1000) {
		hashStorage = new HashValueStorage(aggregationPlan->getArea());
		storage.reset(hashStorage);
	} else {
		storage = CreateDoubleCellMap(aggregationPlan->getArea()->dimCount());
		storage->setLimit(IdentifiersType(), (aggregationPlan->getMaxCount() == 0 ? 0 : aggregationPlan->getMaxCount() + 1));
	}

	initIntern();

	bool hasVals = false;
	if (hashStorage) {
		hasVals = true;
		reader.aggregate(hashStorage, &parentMaps[0]);
	} else {
		while (reader.next()) {
			aggregateCell(reader.getKey(), reader.getValue().getNumeric());
			hasVals = true;
		}
	}
	if (hasVals) {
		boost::unique_lock<boost::mutex> lock(parent.dataMutex);
		if (parent.threadStorage) {
			storageReader = storage->getValues();
			while (storageReader->next()) {
				parent.threadStorage->add(storageReader->getKey(), storageReader->getDouble());
			}
		} else {
			parent.threadStorage = storage;
		}
	}
}

void AggregationProcessorWorker::readCallback()
{
	size_t s = reader.jump[reader.depth];
	if (s < reader.endp && s - reader.pos > parent.minJump && reader.setSingle[reader.depth] == NO_IDENTIFIER && tp->hasFreeCore(true) && reader.nextValid(s)) {
		size_t e = reader.endp;
		reader.endp = s;
		PThreadPoolJob w(new AggregationProcessorWorker(engine, planNode, getThreadGroup(), reader, s, e, tp, parent));
		tp->addJob(w);
	}
}

void AggregationProcessorMT::readCallback()
{
	size_t s = reader->jump[reader->depth];
	if (s < reader->endp && s - reader->pos > minJump && reader->setSingle[reader->depth] == NO_IDENTIFIER && tp->hasFreeCore(false) && reader->nextValid(s)) {
		size_t e = reader->endp;
		reader->endp = s;
		PThreadPoolJob w(new AggregationProcessorWorker(engine, planNode, tg, *reader, s, e, tp, *this));
		tp->addJob(w);
	}
}

void AggregationProcessorMT::aggregate()
{
	Context *con = Context::getContext();
	tp = con->getServer()->getThreadPool();
	tg = tp->createThreadGroup();

	double resultSize = aggregationPlan->getArea()->getSize();

	initIntern();

	const vector<PPlanNode> &sources = planNode->getChildren();
	for (vector<PPlanNode>::const_iterator source = sources.begin(); source != sources.end(); ++source) {
		PCellStream sourceDataSP = createProcessor(*source, false);

		reader = dynamic_cast<StorageCpu::Processor *>(sourceDataSP.get());
		if (reader && (*source)->getType() == SOURCE) {
			minJump = (size_t)reader->pageList->maxPageSize() * 20;
			reader->mtCallback = this;
			if (!storage) {
				createStorage(resultSize);
			}
			if (hashStorage) {
				reader->aggregate(hashStorage, &parentMaps[0]);
			} else {
				while (reader->next()) {
					if (!storage) {
						createStorage(resultSize);
					}
					aggregateCell(reader->getKey(), reader->getValue().getNumeric());
				}
			}
		} else {
			CellValueStream *sourceData = sourceDataSP.get();
			size_t counter = 0;
			while (sourceData->next()) {
				if (!(++counter % 10000)) {
					con->check();
				}
				if (!storage) {
					createStorage(resultSize);
				}
				aggregateCell(sourceData->getKey(), sourceData->getValue().getNumeric());
			}
		}
	}

	tp->join(tg);
	if (threadStorage) {
		if (storage) {
			storageReader = threadStorage->getValues();
			while (storageReader->next()) {
				storage->add(storageReader->getKey(), storageReader->getDouble());
			}
		} else {
			storage = threadStorage;
		}
	}
	if (!storage) {
		storage = CreateDoubleCellMap(aggregationPlan->getArea()->dimCount());
	}
	// log entry
	if (resultSize > 1000 && (Logger::isDebug() || Logger::isTrace())) {
		Logger::debug << "Aggregated area of " << resultSize << " cells. " << storage->size() << " aggregations exists." << endl;
	} else if (Logger::isDebug() || Logger::isTrace()) {
		Logger::trace << "Aggregated area of " << resultSize << " cells. " << endl;
	}

	storageReader = storage->getValues();
}

PCommitable EngineCpuMT::copy() const
{
	checkNotCheckedOut();
	PEngineBase newd(new EngineCpuMT(*this));
	return newd;
}

class ThreadPoolTestWorker : public ThreadPoolJob {
public:
	ThreadPoolTestWorker(ThreadPool::ThreadGroup &tg) : ThreadPoolJob(tg) {}
	virtual void operator()() {
		//cout << "ThreadPoolTestWorker" << endl;
		++cnt;
	}
	size_t cnt;
};

class TestThreads : public TestBase
{
public:
	TestThreads() : TestBase("TestThreads") {
		tp = Context::getContext()->getServer()->getThreadPool();
		tg = tp->createThreadGroup();
		worker = new ThreadPoolTestWorker(tg);
		pw.reset(worker);
	}
	virtual void end() {
	}
	virtual void step() {
		size_t cnt = worker->cnt;
		tp->addJob(pw);
		tp->join(tg);
		if (++cnt != worker->cnt) {
			cout << "test failed!" << endl;
		}
	}
private:
	PThreadPool tp;
	ThreadPool::ThreadGroup tg;
	PThreadPoolJob pw;
	ThreadPoolTestWorker *worker;
};

//static void testThreadPool()
//{
//	TestBase test("void");
//	test.run();
//	cout << test << endl;
//	TestThreads tt;
//	tt.run();
//	cout << tt << endl;
//	cout << tt.uSecondsPerIteration()-test.uSecondsPerIteration() << endl;
//}

PProcessorBase EngineCpuMT::createProcessor(CPPlanNode node, bool sortedOutput, bool useCache)
{
//	static bool once = true;
//	if (once) {
//		testThreadPool();
//		once = false;
//	}
	if (node->getType() == AGGREGATION) {
		PProcessorBase ret;
		const AggregationPlanNode *apn = dynamic_cast<const AggregationPlanNode *>(node.get());

		if (apn->getCacheCube()) {
			Context *context = Context::getContext();
			for (Context::CacheDependences::const_iterator cacheIt = context->getCacheDependences().begin(); cacheIt != context->getCacheDependences().end(); ++cacheIt) {
				ValueCache::CacheWriteProcessor *cacheProc = (*cacheIt);
				if (cacheProc->getCubeArea()->getCube() == apn->getCacheCube() && *node->getArea() == *cacheProc->getCubeArea()) {
					PCellStream cacheInput = cacheProc->getInputProcessor();
					AggregationProcessor *cachedAggrProc = dynamic_cast<AggregationProcessor*>(cacheProc->getInputProcessor().get());
					if (cachedAggrProc) {
						PProcessorBase result = cachedAggrProc->getCalculatedValues();
						if (result) {
							return result;
						}
					}
				}
			}
		}

		if (apn->getAggregationType() == AggregationPlanNode::SUM && apn->getDimIndex() == NO_DFILTER) {
			ret.reset(new AggregationProcessorMT(COMMITABLE_CAST(EngineBase, shared_from_this()), node));
			if (useCache && node->getCache()) {
				ret = node->getCache()->getWriter(node->getCacheCube(), PCubeArea(new CubeArea(CPDatabase(), CPCube(), *node->getArea())), ret, NO_IDENTIFIER);
			}
			return ret;
		}
	}

	return EngineCpu::createProcessor(node, sortedOutput, useCache);
}

}
