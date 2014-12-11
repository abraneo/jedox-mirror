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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef SLIMPAGE_H_
#define SLIMPAGE_H_

#include "palo.h"
#include "Exceptions/ErrorException.h"
#include "Olap/Commitable.h"
#include <iostream>

namespace palo {

template <typename TType> class SlimVector;
template <typename TType> class SlimPage;
template <typename TKey, typename TValue, typename Compare> class SlimMap;


template <typename T1> struct Slim {
	typedef boost::shared_ptr<SlimVector<T1> > PSlimVector;
	typedef boost::shared_ptr<const SlimVector<T1> > CPSlimVector;

	typedef boost::shared_ptr<SlimPage<T1> > PSlimPage;
	typedef boost::shared_ptr<const SlimPage<T1> > CPSlimPage;
};

template < typename T1, typename T2 = T1, typename Compare = less<T1> > struct Slim2 {
	typedef boost::shared_ptr<SlimMap<T1, T2, Compare> > PSlimMap;
	typedef boost::shared_ptr<const SlimMap<T1, T2, Compare> > CPSlimMap;
};


template <typename TType> class SlimVector;


template <typename TType> class SlimPage {
	friend class SlimVector<TType>;
public:
	SlimPage(uint32_t size);
	virtual ~SlimPage();

	TType & push_back(TType value);
	void push_back(vector<TType> &values);
	TType & insertBefore(uint32_t offset, TType value);
	uint32_t count() const {
		return blocksFilled;
	}
	uint32_t getEmptyCount() const {
		return emptyCount;
	}
	TType & operator[](uint32_t index) {
		checkCheckedOut();
		return blocks[index];
	}
	const TType & operator[](uint32_t index) const {
		return blocks[index];
	}
	TType & at(uint32_t index) const {
		return blocks[index];
	}
	void checkIn() {
		checkedOut = false;
	}
	void checkOut() {
		checkedOut = true;
	}
	void checkCheckedOut() const {
		if (!checkedOut) throw CommitException(ErrorException::ERROR_COMMIT_OBJECTNOTCHECKEDOUT, "Object not checked out");
	}
	bool isCheckedOut() const {
		return checkedOut;
	}
	typename Slim<TType>::PSlimPage getCopy();
	void moveValues(typename Slim<TType>::PSlimPage &page, uint32_t fromOffset);
	bool full() const {
		return blocksFilled == maxBlockCount;
	}
	bool fullEnough() const {
		return (float)blocksFilled / maxBlockCount >= maxFill;
	}
	bool empty() const {
		return blocksFilled == 0;
	}
	bool almostEmpty() const {
		return (float)blocksFilled / maxBlockCount <= minFill;
	}
	uint32_t maxCount() const {
		return maxBlockCount;
	}
	uint32_t endSpace() const {
		return maxBlockCount - blocksFilled;
	}
	void erase(uint32_t offset);
	uint32_t fill(TType value, uint32_t count=0);

	void setSize(uint32_t s) {blocksFilled = s;}
	void setValue(uint32_t start, uint32_t end, TType val) {for (uint32_t i = start; i < end; i++) blocks[i] = val;}
	void setEmptyCount(uint32_t emptyCount) {this->emptyCount = emptyCount;}

	void load(FileReader *file);
	void save(FileWriter *file) const;

	static const float maxFill;
	static const float minFill;

private:
	SlimPage(const SlimPage & page);

	uint32_t maxBlockCount;	// objects
	uint32_t blocksFilled;
	uint32_t emptyCount; // null objects in the page
	TType *blocks;
	bool checkedOut;
};

template <typename TType> const float SlimPage<TType>::maxFill = 1;
template <typename TType> const float SlimPage<TType>::minFill = 0.3f;


template <typename TType> SlimPage<TType>::SlimPage(uint32_t size) :
	maxBlockCount(size / sizeof(TType)), blocksFilled(0), emptyCount(0), checkedOut(true)
{
	blocks = new TType[maxBlockCount];
	//memset(blocks, 0, size);
}


template <typename TType> SlimPage<TType>::SlimPage(const SlimPage & page) :
	maxBlockCount(page.maxBlockCount), blocksFilled(page.blocksFilled), emptyCount(page.emptyCount), checkedOut(true)
{
	//blocks = (TType *)new char[size];
	//memcpy(blocks, page.blocks, size);
	blocks = new TType[maxBlockCount];
	for (uint32_t i = 0; i < maxBlockCount; i++) {
		blocks[i] = page.blocks[i];
	}
}


template <typename TType> SlimPage<TType>::~SlimPage()
{
	delete[] blocks;
}


template <typename TType> TType & SlimPage<TType>::push_back(TType value)
{
//	cout << "Page size: " << size << " maxBlockCount: " << maxBlockCount << " blocksFilled: " << blocksFilled << " blocks: " << blocks << endl;
	checkCheckedOut();
	return blocks[blocksFilled++] = value;
}


template <typename TType> void SlimPage<TType>::push_back(vector<TType> &values)
{
	checkCheckedOut();
	for (size_t i = 0; i < values.size(); i++) {
		blocks[blocksFilled++] = values[i];
	}
}


template <typename TType> TType & SlimPage<TType>::insertBefore(uint32_t offset, TType value)
{
	checkCheckedOut();
	//memmove(blocks + offset + 1, blocks + offset, (blocksFilled - offset) * sizeof(TType));
	for (int i = blocksFilled - 1; i >= (int)offset; i--) {
		blocks[i + 1] = blocks[i];
	}
	blocksFilled++;
	return blocks[offset] = value;
}


template <typename TType> typename Slim<TType>::PSlimPage SlimPage<TType>::getCopy()
{
	typename Slim<TType>::PSlimPage page(new SlimPage<TType>(*this));
	return page;
}


template <typename TType> void SlimPage<TType>::moveValues(typename Slim<TType>::PSlimPage &page, uint32_t fromOffset)
{
	checkCheckedOut();
	page->checkCheckedOut();

	uint32_t blocksToCopy = page->blocksFilled - fromOffset;
	//memcpy(blocks + blocksFilled, page->blocks + fromOffset, blocksToCopy * sizeof(TType));
	for (uint32_t i = 0; i < blocksToCopy; i++) {
		blocks[blocksFilled + i] = page->blocks[fromOffset + i];
	}
	blocksFilled += blocksToCopy;

	//memset(page->blocks + fromOffset, 0, blocksToCopy * sizeof(TType));
	page->blocksFilled -= blocksToCopy;
}


template <typename TType> void SlimPage<TType>::erase(uint32_t offset)
{
	checkCheckedOut();
	//memmove(blocks + offset, blocks + offset + 1, (blocksFilled - offset - 1) * sizeof(TType));
	for (uint32_t i = offset; i < (blocksFilled - 1); i++) {
		blocks[i] = blocks[i + 1];
	}

	blocksFilled--;
	//memset(&blocks[--blocksFilled], 0, sizeof(TType));
}


template <typename TType> uint32_t SlimPage<TType>::fill(TType value, uint32_t count)
{
	checkCheckedOut();

	uint32_t filled = maxBlockCount - blocksFilled;
	if (count && count < filled) {
		filled = count;
	}
	memset(&blocks[blocksFilled], value, sizeof(TType) * filled);
	blocksFilled += filled;
	return filled;
}

template <typename TType> void SlimPage<TType>::load(FileReader *file)
{
	file->getRaw((char *)&blocksFilled, sizeof(uint32_t));
	uint32_t size = blocksFilled * sizeof(TType);
	file->getRaw((char *)blocks, size);
}

template <typename TType> void SlimPage<TType>::save(FileWriter *file) const
{
	file->appendRaw((const char *)&blocksFilled, sizeof(uint32_t));
	uint32_t size = blocksFilled * sizeof(TType);
	file->appendRaw((const char *)blocks, size);
}

}

#endif /* SLIMPAGE_H_ */
