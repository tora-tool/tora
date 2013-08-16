/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries.
 *
 *      You may link this product with any GPL'd Qt library.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef __QSQL_CONNECTION__
#define __QSQL_CONNECTION__

#include "core/toconnection.h"
#include "core/toconnectionsub.h"

class toQSQLProvider;

// Utility class to lock QSqlDriver - TODO move it into Utils or completely remove
// much thanks to:
// http://www.ddj.com/cpp/184403766
// i added lock and unlock
template <typename T> class LockingPtr
{
public:
  // Constructors/destructors
  LockingPtr(volatile T& obj, QMutex& mtx) : pObj_(const_cast<T*>(&obj)),
					     pMtx_(&mtx)
  {
    mtx.lock();
    locked = true;
  }

  /**
   * Overloaded contructor that attempts lock rather than blocking.
   *
   * Will throw an instance of QString if class failed to obtain
   * lock and tryLock is true.
   *
   */
  LockingPtr(volatile T& obj,
	     QMutex& mtx,
	     bool tryLock) : pObj_(const_cast<T*>(&obj)),
			     pMtx_(&mtx)
  {
    if(tryLock)
      {
	locked = mtx.tryLock();
	if(!locked)
	  throw QString::fromLatin1("Busy");
      }
    else
      {
	mtx.lock();
	locked = true;
      }
  }

  ~LockingPtr()
  {
    if(locked)
      pMtx_->unlock();
  }

  void lock()
  {
    pMtx_->lock();
    locked = true;
  }

  void unlock()
  {
    locked = false;
    pMtx_->unlock();
  }

  // Pointer behavior
  T& operator*()
  {
    return *pObj_;
  }

  T* operator->()
  {
    return pObj_;
  }

private:
  bool locked;
  T* pObj_;
  QMutex* pMtx_;
  LockingPtr(const LockingPtr&);
  LockingPtr& operator=(const LockingPtr&);
};

class toQSqlConnectionImpl: public toConnection::connectionImpl
{
    friend class toQSqlProvider;
    friend class toQMySqlProvider;
    friend class toQPSqlProvider;
protected:
    toQSqlConnectionImpl(toConnection &conn) : toConnection::connectionImpl(conn) {};
public:
    /** Create a new connection to the database. */
    virtual toConnectionSub *createConnection(void)
    {
    	throw QString("Not implemented yet toQSqlConnectionImpl::createConnection");
    }

    /** Close a connection to the database. */
    virtual void closeConnection(toConnectionSub *)
    {
    	throw QString("Not implemented yet toQSqlConnectionImpl::closeConnection");
    }
};

class toQSqlConnectionSub: public toConnectionSub
{
    //friend class oracleQuery;
public:
    toQSqlConnectionSub(void*);

    virtual ~toQSqlConnectionSub();
    virtual void cancel();
    virtual void close();
    virtual void commit();
    virtual void rollback();
    virtual queryImpl* createQuery(toQuery *query);

    virtual toQAdditionalDescriptions* decribe(toCache::ObjectRef const&);

private:
};


#endif
