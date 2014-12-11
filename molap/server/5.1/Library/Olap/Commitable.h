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

#ifndef OLAP_COMMITABLE_H
#define OLAP_COMMITABLE_H 1

#include "palo.h"
#include "Exceptions/CommitException.h"
#include "boost/enable_shared_from_this.hpp"

#if defined(_MSC_VER)
	#if _MSC_VER < 1700 /*  MSVC 11.0 has std::atomic */
		#define USE_BOOST_ATOMIC 1
	#else /* _MSC_VER < 1700 */
		#include <atomic>
	#endif /* _MSC_VER < 1700 */
#endif /* _MSC_VER */

#if defined(__GNUC__) && !defined(__clang__)
	#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 4)
		#define USE_BOOST_ATOMIC 1
	#else /* GCC ver < 4.3  */
		#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 5)
			#include <cstdatomic>
		#else /* GCC ver < 4.5  */
			#include <atomic> /* GCC ver > 4.5 has std::atomic */
		#endif /* GCC ver < 4.5  */
	#endif /* GCC ver < 4.3  */
#endif /* __GNUC__ */

#ifdef __clang__
#include <atomic>
#endif

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief Base class for all commitable objects
/// Whole idea behind commitable object is that all readers are sharing same
/// version of object tree and writers will create copies of objects they
/// need to change. Afterwards they will merge their changes with latest version.
/// Only shared pointers are used for working with commitable object thats why
/// it's guaranteed that job is working all the time with same version it got
/// at the beginning.
/// Usual writing procedure is something like this (example for changing the cube):
///
///     PServer server = server::getInstance(true); // true means, that we want a copy
///	    PDatabaseList dbs = server->getDatabaseList(true);
///     server->setDatabaseList(dbs); // we have to set our new copy of list to our copy of server
///     PDatabase db = server->lookupDatabase(dbid, true);
///     dbs->set(db); // we have to replace database in the copied list with new copy
///     PCubeList cubes = db->getCubeList(true);
///     db->setCubeList(cubes);
///     PCube cube = db->lookupCube(cubeid, true);
///     cubes->set(cube);
///     cube->makesomechanges();
///     bool ret = server->commit();
///
/// The last call goes through whole tree and merges our changes with changes made by other users.
/// It returns true if it succeeds and false if changes are unmergable. In that case
/// we can either return error to user that server is busy and he should try it again later
/// or throw everything away and start again with latest version. There are also scenarios
/// where we are doing changes itself in the commit/merge. Than the change is guaranteed
/// to succeed, but it blocks other writers (readers are never blocked and they always have
/// consistent version).
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS Commitable : public boost::enable_shared_from_this<Commitable> {
public:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief Number of repeats before exception.
	/// In most cases we are trying to make our changes 5 times before throwing exception,
	/// that server->commit failed.
	////////////////////////////////////////////////////////////////////////////////
	static const int COMMIT_REPEATS = 5;
	////////////////////////////////////////////////////////////////////////////////
	/// @brief Number of repeats before exception in cell replace.
	/// In cell replace we are having different approach. We try to merge changes once
	/// and if it fails, we'll do whole cell replace in merge, where it's guaranteed
	/// to succeed, but blocks other writers.
	////////////////////////////////////////////////////////////////////////////////
	static const int COMMIT_REPEATS_CELL_REPLACE = 2;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Constructor
	/// New objects are automatically checked out and have old version set to NULL.
	////////////////////////////////////////////////////////////////////////////////
	Commitable(const string &n);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Copy constructor
	/// All commitable objects should have working copy constructor.
	/// Copied objects are marked as checked out and have old version set to version
	/// from which copy was made. Only shallow copies are made. We don't want
	/// to copy any object we won't change.
	////////////////////////////////////////////////////////////////////////////////
	Commitable(const Commitable &c);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Destructor
	/// Be aware that there can be also other copies of commitable object.
	/// So destructors shouldn't do any destruction of stuff that is shared among
	/// copies.
	////////////////////////////////////////////////////////////////////////////////
	virtual ~Commitable();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Internal function
	/// CommitableLists are setting new object id during add function.
	////////////////////////////////////////////////////////////////////////////////
	void setID(IdentifierType i);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Merge all changes with latest version
	/// @param o The latest version of object
	/// @param p Parent of the object.
	///
	/// This is the place where everything interesting happens. There all three version
	/// of an object available. My changed (this), original (this->old) and latest (o).
	/// General idea is to do for each member, that can be changed, comparsion with old
	/// to realize if I changed it or not. If I didn't change it than latest version
	/// is taken. If I changed it then my version is used for not commitable members or
	/// merge is called for commitable members.
	////////////////////////////////////////////////////////////////////////////////
	virtual bool merge(const CPCommitable &o, const PCommitable &p) = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Returns new copy of object.
	/// This is the only possibility, how to get new copy of an object. Usually it
	/// just internally uses copy constructor, but copy constructors shouldn't be used
	/// directly.
	////////////////////////////////////////////////////////////////////////////////
	virtual PCommitable copy() const = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Returns true if object is check out.
	////////////////////////////////////////////////////////////////////////////////
	bool isCheckedOut() const
	{
		return checkedOut;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Returns id.
	////////////////////////////////////////////////////////////////////////////////
	IdentifierType getId() const
	{
		return comm_id;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Returns name.
	////////////////////////////////////////////////////////////////////////////////
	const string &getName() const
	{
		return comm_name;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Internal function.
	/// It's only used by CommitableList when rename is called.
	////////////////////////////////////////////////////////////////////////////////
	void setName(const string &name)
	{
		comm_name = name;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Helper function.
	/// Throws an exception when object is not checked out. It's very helpful to
	/// place this call in the functions that are changing the object. We can find
	/// bugs where we forgot to make the copy and we should make one.
	////////////////////////////////////////////////////////////////////////////////
	void checkCheckedOut() const
	{
		if (!checkedOut)
			throw CommitException(ErrorException::ERROR_COMMIT_OBJECTNOTCHECKEDOUT, "Object not checked out");
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Helper function.
	/// Used in copy functions. It's not good idea to copy something that was
	/// already copied
	////////////////////////////////////////////////////////////////////////////////
	void checkNotCheckedOut() const
	{
		if (checkedOut)
			throw CommitException(ErrorException::ERROR_COMMIT_OBJECTCHECKEDOUT, "Object already checked out");
	}

#ifdef USE_BOOST_ATOMIC
	int getCopiesCount() const
	{
		if(old)
			return (int) (const_cast<Commitable *>(old.get())->copiesCount);
		else
			return 0;
	}
#else
	int getCopiesCount() const
	{
		if(old)
			return const_cast<Commitable *>(old.get())->copiesCount.load(std::memory_order_acquire);
		else
			return 0;
	}
#endif
	uint64_t getObjectRevision() const {return objectRevision;}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Voluntary check of consistency before merging lists.
	/// Used for check if the dimension used in newly created cube wasn't deleted.
	////////////////////////////////////////////////////////////////////////////////
	virtual bool checkBeforeInsertToMergedList(const CPCommitable &parent);

protected:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief Every commitable object has to call this at the end of merge!!!
	////////////////////////////////////////////////////////////////////////////////
	void commitintern();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Every commitable object has to call this at the beginning of merge!!!
	////////////////////////////////////////////////////////////////////////////////
	void mergeint(const CPCommitable &o, const PCommitable &p);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Internal function
	////////////////////////////////////////////////////////////////////////////////
	void checkOut()
	{
		checkedOut = true;
	}
	CPCommitable old;

#ifdef USE_BOOST_ATOMIC
	volatile long copiesCount;
#else
	std::atomic<int> copiesCount;
#endif
	uint64_t objectRevision;

private:
	IdentifierType comm_id;
	bool checkedOut;
	string comm_name;
};

}

#endif
