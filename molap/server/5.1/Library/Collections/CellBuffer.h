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
 * 
 *
 */

#ifndef CELLBUFFER_H
#define CELLBUFFER_H

#include "palo.h"
#include "Engine/Streams.h"
#include "Engine/EngineBase.h"

using namespace palo;

namespace palo {

typedef boost::shared_ptr<class CellStreamBuffer> PCellStreamBuffer;
typedef boost::shared_ptr<const class CellStreamBuffer> CPCellStreamBuffer;

class CellStreamBuffer : public boost::enable_shared_from_this<CellStreamBuffer>
{
public:
	CellStreamBuffer(size_t dimensions, size_t expectedValues) : dimensions(dimensions) {rawKeys.reserve(expectedValues * dimensions);values.reserve(expectedValues);}
	virtual ~CellStreamBuffer() {}
	void push_back(const IdentifiersType &key, const double val) {
		values.push_back(val);
		copy(key.begin(),key.end(),back_inserter(rawKeys));
	}
	class Reader : public ProcessorBase {
	public:
		Reader(PCellStreamBuffer buf) : ProcessorBase(true, PEngineBase()), buf(buf), rawKeyStartIt(buf->rawKeys.begin()), valueIt(buf->values.begin()), started(false) {}
		virtual ~Reader() {}
		virtual bool next() {
			if (!started) {
				started = true;
				key.resize(buf->dimensions);
			} else if (rawKeyStartIt != buf->rawKeys.end()) {
				rawKeyStartIt += buf->dimensions;
				++valueIt;
			}
			if (rawKeyStartIt == buf->rawKeys.end()) {
				return false;
			}
			copy(rawKeyStartIt, rawKeyStartIt+buf->dimensions, key.begin());
			return true;
		}
		virtual const CellValue &getValue() {value = *valueIt; return value;}
		virtual double getDouble() {return *valueIt;}
		virtual const IdentifiersType &getKey() const {return key;}
		virtual const GpuBinPath &getBinKey() const {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "getBinKey not supported!");
		}
		virtual void reset() {
			rawKeyStartIt = buf->rawKeys.begin();
			valueIt = buf->values.begin();
			started = false;
			key.clear();
		}
	private:
		PCellStreamBuffer buf;
		IdentifiersType key;
		CellValue value;
		IdentifiersType::const_iterator rawKeyStartIt;
		vector<double>::const_iterator valueIt;
		bool started;
	};
	PProcessorBase getValues() {
		return PProcessorBase(new CellStreamBuffer::Reader(shared_from_this()));
	}
private:
	IdentifiersType rawKeys;
	vector<double> values;
	size_t dimensions;
	friend class Reader;
};

}
#endif

