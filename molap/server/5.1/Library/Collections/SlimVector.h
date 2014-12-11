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

#ifndef SLIMVECTOR_H_
#define SLIMVECTOR_H_


#include "InputOutput/FileReader.h"
#include "InputOutput/FileWriter.h"
#include "SlimPage.h"

namespace palo {

template <typename TType> class SlimVector : public Commitable {
public:
	SlimVector(uint32_t pageMemorySize);
	virtual ~SlimVector();

	TType & push_back(TType value);
	void push_back(SlimVector<TType> &v);
	uint32_t push_back(vector<TType> &v, TType empty, bool &pageAdded, bool &wasCheckout);
	const TType & operator[](size_t index) const;
	TType & operator[](size_t index);

	TType & insertBefore(uint32_t page, uint32_t offset, TType value);
	uint32_t pageCount() const {
		return (uint32_t)pages.size();
	}
	uint32_t pageSize(uint32_t index) const {
		return pages[index]->count();
	}
	uint32_t endSpace() const;
	size_t size() const;
	bool empty() const {
		return pages.size() == 0;
	}
	void clear() {
		pages.clear();
	}
	void erase(uint32_t page, uint32_t offset);
	TType & at(uint32_t pageIndex, uint32_t offset) const {
		return pages[pageIndex]->blocks[offset];
	}

	uint32_t maxPageSize() const {return blocksInPage;}
	vector<typename Slim<TType>::PSlimPage> &getPages() {return pages;}
	const vector<typename Slim<TType>::PSlimPage> &getPages() const {return pages;}

	void load(FileReader *file);
	void save(FileWriter *file) const;

	virtual bool merge(const CPCommitable &o, const PCommitable &p);
	virtual PCommitable copy() const;

	void setOld(CPCommitable o) {old = o;}
	CPCommitable getOld() const {return old;}

	typename Slim<TType>::PSlimPage addPage();
protected:
	class base_iterator {
	protected:
		const SlimVector<TType> *parentVector;
		uint32_t page;
		uint32_t offset;

		base_iterator(const SlimVector<TType> *parentVector, uint32_t page, uint32_t offset) :
			parentVector(parentVector), page(page), offset(offset) {}

	public:
		base_iterator(const base_iterator &it) :
			parentVector(it.parentVector), page(it.page), offset(it.offset) {}

		base_iterator & operator++();
		/*base_iterator & operator+=(ptrdiff_t n);
		base_iterator operator+(ptrdiff_t n);*/
		base_iterator & operator=(const base_iterator &iter);
		bool operator!=(const base_iterator &iter);
		/*void increment(size_t n);
		void decrement(size_t n);*/
	};

public:
	class iterator : public base_iterator {
		friend class SlimVector<TType>;
		friend class StringVector;
	protected:
		iterator(const SlimVector<TType> *parentVector, uint32_t page, uint32_t offset) :
			base_iterator(parentVector, page, offset) {}

	public:
		iterator(const iterator &it) : base_iterator(it) {}

		TType & operator*();
		iterator & operator++() {
			base_iterator::operator++();
			return *this;
		}
	};

	class const_iterator : public base_iterator {
		friend class SlimVector<TType>;
		friend class StringVector;
	protected:
		const_iterator(const SlimVector<TType> *parentVector, uint32_t page, uint32_t offset) :
			base_iterator(parentVector, page, offset) {}

	public:
		const_iterator(const const_iterator &it) : base_iterator(it) {}
		const_iterator(const iterator &it) : base_iterator(it) {}

		const TType & operator*() const;
	};

	typename SlimVector<TType>::iterator begin();
	typename SlimVector<TType>::iterator end();
	typename SlimVector<TType>::const_iterator begin() const;
	typename SlimVector<TType>::const_iterator end() const;


protected:
	uint32_t pageMemorySize;
	uint32_t blocksInPage;
	vector<typename Slim<TType>::PSlimPage > pages;

	SlimVector(const SlimVector<TType> &p);

	void joinPages(uint32_t page1, uint32_t page2);
};


template <typename TType> SlimVector<TType>::SlimVector(uint32_t pageMemorySize) :
	Commitable(""), pageMemorySize(pageMemorySize)
{
	blocksInPage = pageMemorySize / sizeof(TType);
}


template <typename TType> SlimVector<TType>::SlimVector(const SlimVector &p) :
	Commitable(p), pageMemorySize(p.pageMemorySize), blocksInPage(p.blocksInPage), pages(p.pages)
{
}


template <typename TType> SlimVector<TType>::~SlimVector()
{
}


template <typename TType> const TType & SlimVector<TType>::operator[](size_t index) const
{
	typename Slim<TType>::CPSlimPage page = pages[index / blocksInPage];
	return (*page)[index % blocksInPage];
}


template <typename TType> TType & SlimVector<TType>::operator[](size_t index)
{
	checkCheckedOut();
	uint32_t pageIndex = (uint32_t)(index / blocksInPage);
	if (!pages[pageIndex]->isCheckedOut()) {
		pages[pageIndex] = typename Slim<TType>::PSlimPage(new SlimPage<TType>(*pages[pageIndex]));
	}
	return (*pages[pageIndex])[index % blocksInPage];
}


template <typename TType> uint32_t SlimVector<TType>::endSpace() const
{
	if (pages.size() == 0) {
		return 0;
	} else {
		return pages[pages.size() - 1]->endSpace();
	}
}


template <typename TType> size_t SlimVector<TType>::size() const
{
	if (pages.size() > 1) {
		return (pages.size() - 1) * blocksInPage + pages[(uint32_t)pages.size() - 1]->count();
	} else if (pages.size() == 1) {
		return pages[0]->count();
	} else {
		return 0;
	}
}


template <typename TType> void SlimVector<TType>::erase(uint32_t page, uint32_t offset)
{
	pages[page] = pages[page]->getCopy();
	if (!pages[page]->almostEmpty()) {
		// enough items in page
		pages[page]->erase(offset);
	} else {
		if (pages[page]->count() == 1) {
			// last item, remove page
			pages.erase(pages.begin() + page);
		} else {
			pages[page]->erase(offset);
			// minimum size of page reached
			if (page > 0 && (float)(pages[page - 1]->count() + pages[page]->count()) / pages[page]->maxCount() < SlimPage<TType>::maxFill) {
				// join with previous page
				pages[page - 1] = pages[page - 1]->getCopy();
				joinPages(page - 1, page);
			} else if (page < pages.size() - 1 && (float)(pages[page + 1]->count() + pages[page]->count()) / pages[page]->maxCount() < SlimPage<TType>::maxFill) {
				// join with next page
				pages[page + 1] = pages[page + 1]->getCopy();
				joinPages(page, page + 1);
			} else {
				// not possible to join with neighbours, possible complete shake algorithm here
			}
		}
	}
}


template <typename TType> TType & SlimVector<TType>::push_back(TType value)
{
	typename Slim<TType>::PSlimPage lastPage;
	if (!pages.empty()) {
		lastPage = pages[pages.size() - 1];
		if (lastPage->fullEnough()) {
			lastPage = addPage();
		} else {
			if (!lastPage->isCheckedOut()) {
				pages[pages.size() - 1] = lastPage = lastPage->getCopy();
			}
		}
	} else {
		lastPage = addPage();
	}

	return lastPage->push_back(value);
}


template <typename TType> void SlimVector<TType>::push_back(SlimVector<TType> &v)
{
	if (v.pageMemorySize != pageMemorySize) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "cannot push vector with different page size");
	}
	// TODO - whole pages added, empty space in current last page skipped
	for (typename vector<typename Slim<TType>::PSlimPage >::iterator it = v.pages.begin(); it != v.pages.end(); ++it) {
		it->checkOut();
		pages.push_back(*it);
	}
}


template <typename TType> uint32_t SlimVector<TType>::push_back(vector<TType> &v, TType empty, bool &pageAdded, bool &pageCheckout)
{
	checkCheckedOut();
	uint32_t emptyAdded = 0;
	pageAdded = false;
	pageCheckout = false;

	typename Slim<TType>::PSlimPage lastPage;
	if (!pages.empty()) {
		lastPage = pages[pages.size() - 1];
		if (lastPage->endSpace() < v.size()) {
			if (!lastPage->isCheckedOut()) {
				pages[pages.size() - 1] = lastPage = lastPage->getCopy();
				pageCheckout = true;
			}
			emptyAdded = lastPage->fill(empty); // fill with "empty" TType to the rest of the page (not enough space here)
			lastPage->setEmptyCount(lastPage->getEmptyCount()+emptyAdded);
			lastPage = addPage(); // add new page
			pageAdded = true;
		} else {
			if (!lastPage->isCheckedOut()) {
				pages[pages.size() - 1] = lastPage = lastPage->getCopy();
				pageCheckout = true;
			}
		}
	} else {
		lastPage = addPage();
		pageAdded = true;
	}

	lastPage->push_back(v);
	return emptyAdded;
}

template <typename TType> TType & SlimVector<TType>::insertBefore(uint32_t page, uint32_t offset, TType value)
{
	if (!pages[page]->isCheckedOut()) {
		pages[page] = pages[page]->getCopy();
	}
	if (pages[page]->full()) {
//		cout << "dividing pages" << endl;
		pages.insert(pages.begin() + page + 1, typename Slim<TType>::PSlimPage(new SlimPage<TType>(pageMemorySize)));
		uint32_t cutOff = pages[page]->count() / 2;
		pages[page + 1]->moveValues(pages[page], cutOff);
		if (offset < cutOff) {
			return pages[page]->insertBefore(offset, value);
		} else {
			return pages[page + 1]->insertBefore(offset - cutOff, value);
		}
	} else {
		return pages[page]->insertBefore(offset, value);
	}
}


template <typename TType> typename Slim<TType>::PSlimPage SlimVector<TType>::addPage()
{
	pages.push_back(typename Slim<TType>::PSlimPage(new SlimPage<TType>(pageMemorySize)));
	return pages[pages.size() - 1];
}


template <typename TType> void SlimVector<TType>::joinPages(uint32_t page1, uint32_t page2)
{
	//cout << "joining pages " << page1 << " & " << page2 << endl;
	if (page1 + 1 != page2) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "pages join not possible");
	}

	pages[page1]->checkCheckedOut();
	pages[page2]->checkCheckedOut();
	pages[page1]->moveValues(pages[page2], 0);
	pages.erase(pages.begin() + page2);
}

template <typename TType> void SlimVector<TType>::load(FileReader *file)
{
	//number of pages
	uint32_t size;
	file->getRaw((char *)&size, sizeof(uint32_t));
	clear();

	if (size) {
		vector<typename Slim<TType>::PSlimPage > pagesLocal;
		pagesLocal.reserve(size);

		//page size
		file->getRaw((char *)&pageMemorySize, sizeof(uint32_t));
		blocksInPage = pageMemorySize / sizeof(TType);

		for (uint32_t i = 0; i < size; i++) {
			typename Slim<TType>::PSlimPage page(new SlimPage<TType>(pageMemorySize));
			page->load(file);
			pagesLocal.push_back(page);
		}

		// complete load was successful
		this->pages.swap(pagesLocal);
	}
}

template <typename TType> void SlimVector<TType>::save(FileWriter *file) const
{
	uint32_t size = (uint32_t)pages.size();
	//number of pages
	file->appendRaw((const char *)&size, sizeof(uint32_t));

	if (size) {
		//page size
		file->appendRaw((const char *)&pageMemorySize, sizeof(uint32_t));

		for (uint32_t i = 0; i < size; i++) {
			pages[i]->save(file);
		}
	}
}

template <typename TType> bool SlimVector<TType>::merge(const CPCommitable &o, const PCommitable &p)
{
	bool ret = true;
	checkCheckedOut();
	if (old != 0 && o != 0) {
		if (old != o) {
			ret = false;
		}
	}
	if (ret) {
		mergeint(o, p);
		for (uint32_t i = 0; i < pages.size(); i++) {
			pages[i]->checkIn();
		}
		commitintern();
	}
	return ret;
}


template <typename TType> PCommitable SlimVector<TType>::copy() const
{
	checkNotCheckedOut();
	typename Slim<TType>::PSlimVector newVector(new SlimVector<TType>(*this));
	return newVector;
}


template <typename TType> typename SlimVector<TType>::iterator SlimVector<TType>::begin()
{
	return typename SlimVector<TType>::iterator(this, 0, 0);
}


template <typename TType> typename SlimVector<TType>::iterator SlimVector<TType>::end()
{
	return typename SlimVector<TType>::iterator(this, (uint32_t)pages.size(), 0);
}


template <typename TType> typename SlimVector<TType>::const_iterator SlimVector<TType>::begin() const
{
	return typename SlimVector<TType>::const_iterator(this, 0, 0);
}


template <typename TType> typename SlimVector<TType>::const_iterator SlimVector<TType>::end() const
{
	return typename SlimVector<TType>::const_iterator(this, (uint32_t)pages.size(), 0);
}


template <typename TType> typename SlimVector<TType>::base_iterator & SlimVector<TType>::base_iterator::operator++()
{
	offset++;
	if (offset >= parentVector->pageSize(page)) {
		offset = 0;
		page++;
	}
	return *this;
}


template <typename TType> typename SlimVector<TType>::base_iterator & SlimVector<TType>::base_iterator::operator=(const base_iterator &iter)
{
	parentVector = iter.parentVector;
	page = iter.page;
	offset = iter.offset;
	return *this;
}


template <typename TType> bool SlimVector<TType>::base_iterator::operator!=(const base_iterator &iter)
{
	return /*parentVector != iter.parentVector || */page != iter.page || offset != iter.offset;
}


template <typename TType> TType & SlimVector<TType>::iterator::operator*()
{
	if (!this->parentVector->pages[this->page]->isCheckedOut()) {
		const_cast<SlimVector<TType> *>(this->parentVector)->pages[this->page] = typename Slim<TType>::PSlimPage(new SlimPage<TType>(*this->parentVector->pages[this->page]));
	}
	return (*this->parentVector->pages[this->page])[this->offset];

}


template <typename TType> const TType & SlimVector<TType>::const_iterator::operator*() const
{
	const SlimPage<TType> &page = (*this->parentVector->pages[this->page]);
	return page[this->offset];
}


/*template <typename TType> typename SlimVector<TType>::base_iterator & SlimVector<TType>::base_iterator::operator+=(ptrdiff_t n)
{
	if (n > 0) {
		increment(n);
	} else if (n < 0) {
		decrement(-n);
	}
	return *this;
}


template <typename TType> typename SlimVector<TType>::base_iterator SlimVector<TType>::base_iterator::operator+(ptrdiff_t n)
{
	typename SlimVector<TType>::iterator ret = *this;
	return ret += n;
}


template <typename TType> void SlimVector<TType>::base_iterator::increment(size_t n)
{
	page += (offset + n) / parentVector->pageSize();
	offset = (offset + n) % pageSize;
}

template <typename TType> void SlimVector<TType>::base_iterator::decrement(size_t n)
{
	TODOMD
}*/

}

#endif /* SLIMVECTOR_H_ */
