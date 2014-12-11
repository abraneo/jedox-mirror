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

#ifndef ENGINE_ROUNDCORRECTPROCESSOR_H_
#define ENGINE_ROUNDCORRECTPROCESSOR_H_

#include "palo.h"
#include "Engine/Area.h"
#include "Engine/Streams.h"
#include "Engine/EngineBase.h"

namespace palo {

class SERVER_CLASS RoundCorrectProcessor : public ProcessorBase {
public:
	RoundCorrectProcessor(PEngineBase engine, CPRoundCorrrectPlanNode node);
	virtual ~RoundCorrectProcessor() {}

	virtual bool next();
	virtual const CellValue &getValue();
	virtual double getDouble();
	virtual void setValue(const CellValue &value);
	virtual const IdentifiersType &getKey() const;
	virtual const GpuBinPath &getBinKey() const;
	virtual void reset();
	//virtual bool move(const IdentifiersType &key, bool *found); // TODO: -jj implement for better performance if needed

private:
	IdentifiersType lastKey;
	CellValue lastValue;
	PEngineBase engine;
	CPPlanNode child;
	PCellStream pStream;
	bool moveToNext;
	double targetValue;
	double unitValue;
	double error;
	double referenceValue;
	CPPathTranslator pathTranslator;
	GpuBinPath binPath;
};

}
#endif /* ENGINE_ROUNDCORRECTPROCESSOR_H_ */
