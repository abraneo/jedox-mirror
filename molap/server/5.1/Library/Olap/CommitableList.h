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

#ifndef OLAP_COMMITABLELIST_H
#define OLAP_COMMITABLELIST_H 1

#include "palo.h"
#include "Commitable.h"
#include "Thread/Mutex.h"
#include "Collections/StringUtils.h"

namespace palo {

class SERVER_CLASS IdHolder {
public:
	IdHolder() : id(0) {}
	IdentifierType getNewId();
	void setStart(IdentifierType newid);
	IdentifierType getLastId() {
		return id;
	}

private:
	IdentifierType id;
	Mutex idlock;
};

typedef boost::shared_ptr<IdHolder> PIdHolder;

class SERVER_CLASS CommitableList : public Commitable {
public:
	class SERVER_CLASS Iterator {
		friend class CommitableList;
	public:
		Iterator operator=(const Iterator &iter);
		bool operator==(const Iterator &iter) const;
		bool operator!=(const Iterator &iter) const;
		Iterator operator++(int i);
		Iterator operator++();
		const PCommitable &operator*() const;
		PCommitable getCopy();
	private:
		Iterator(const map<IdentifierType, PCommitable>::iterator &i);
		map<IdentifierType, PCommitable>::iterator it;
	};

	class SERVER_CLASS ConstIterator {
		friend class CommitableList;
	public:
		ConstIterator operator=(const ConstIterator &iter);
		bool operator==(const ConstIterator &iter) const;
		bool operator!=(const ConstIterator &iter) const;
		ConstIterator operator++(int i);
		ConstIterator operator++();
		CPCommitable operator*() const;
	private:
		ConstIterator(const map<IdentifierType, PCommitable>::const_iterator &i);
		map<IdentifierType, PCommitable>::const_iterator it;
	};

	CommitableList(const PIdHolder &newidh, bool ignoreNames = false);
	CommitableList(bool ignoreNames = false);
	CommitableList(const CommitableList &l);
	void add(const PCommitable &item, bool newid);
	void clear();
	IdentifierType size() const;
	PCommitable get(IdentifierType id, bool write) const;
	PCommitable get(const string &name, bool write) const;
	void set(const PCommitable &item);
	virtual PCommitable copy() const;
	void remove(IdentifierType id);
	void erase(Iterator it);
	virtual bool merge(const CPCommitable &o, const PCommitable &p);
	void setNewIDStart(IdentifierType newstart) {
		idh->setStart(newstart);
	}
	IdentifierType getLastId() const {
		return idh->getLastId();
	}
	void rename(IdentifierType id, const string &newname);

	Iterator begin();
	Iterator end();
	ConstIterator const_begin() const;
	ConstIterator const_end() const;

protected:
	virtual PCommitableList createnew(const CommitableList& l) const = 0;

private:
	PIdHolder idh;
	map<IdentifierType, PCommitable> ids;
	map<string, IdentifierType, UTF8Comparer> names;
	bool ignoreNames;
};

}

#endif
