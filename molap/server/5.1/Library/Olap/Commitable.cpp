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

#include "Commitable.h"

namespace palo {

Commitable::Commitable(const string &n) :
		copiesCount(0), objectRevision(0), comm_id(-1), checkedOut(true), comm_name(n)
{
}

Commitable::Commitable(const Commitable &c) :
		old(c.shared_from_this()), copiesCount(0), objectRevision(c.objectRevision), comm_id(c.comm_id), checkedOut(true), comm_name(c.comm_name)
{
#ifdef USE_BOOST_ATOMIC
	BOOST_INTERLOCKED_INCREMENT( &(const_cast<Commitable *>(&c)->copiesCount) );
#else
	const_cast<Commitable *>(&c)->copiesCount.fetch_add(1, std::memory_order_relaxed);
#endif
}

Commitable::~Commitable()
{
	if (old) {
#ifdef USE_BOOST_ATOMIC
		BOOST_INTERLOCKED_DECREMENT( &(const_cast<Commitable *>(old.get())->copiesCount) );
#else
		const_cast<Commitable *>(old.get())->copiesCount.fetch_sub(1, std::memory_order_relaxed);
#endif
	}
}

void Commitable::commitintern()
{
	if (checkedOut) {
		checkedOut = false;
		old.reset();
	} else {
		throw CommitException(ErrorException::ERROR_COMMIT_OBJECTNOTCHECKEDOUT, "");
	}
}

void Commitable::setID(IdentifierType i)
{
	comm_id = i;
}

void Commitable::mergeint(const CPCommitable &o, const PCommitable &p)
{
	if (old != 0) {
		if (comm_name == old->comm_name && o != 0) {
			comm_name = o->comm_name;
		}
	}
	if (p) {
		objectRevision = p->getObjectRevision();
	} else {
		objectRevision = o ? o->getObjectRevision() + 1 : 1;
	}
}

bool Commitable::checkBeforeInsertToMergedList(const CPCommitable &parent)
{
	return true;
}

}
