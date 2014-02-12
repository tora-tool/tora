
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#undef  TORA_MAKE_DLL
#ifndef TORA_DLL
#define TORA_DLL
#endif
#ifndef TROTL_DLL
#define TROTL_DLL
#endif

#include "connection/tooracleconnection.h"
#include "connection/tooraclequery.h"

#include "core/toconf.h"       // TOAPPNAME
#include "core/toraversion.h"
#include "core/tologger.h"
#include "core/toconfiguration.h"

#include "trotl.h"
#include "trotl_convertor.h"
#include "trotl_describe.h"

#include <QtCore/QVariant>
#include <QtGui/QInputDialog>
#include <QtGui/QLineEdit>
#include <QtGui/QApplication>
#include <iomanip>

_Noreturn void ReThrowException(const ::trotl::OciException &exc)
{
    TLOG(5, toDecorator, __HERE__)
            << "Exception thrown, what:" << exc.what() << std::endl
            << exc.get_sql() << std::endl
            << "--------------------------------------------------------------------------------"
            << std::endl;

    toConnection::exception ret(QString::fromUtf8(exc.get_mesg()), exc.parse_offset(), exc.line(), exc.column());
    throw ret;
}

toOracleConnectionImpl::toOracleConnectionImpl(toConnection &conn, ::trotl::OciEnv &env)
    : toConnection::connectionImpl(conn)
    , _env(env)
{
}

toConnectionSub* toOracleConnectionImpl::createConnection(void)
{
    ::trotl::OciConnection *conn = NULL;
    ::trotl::OciLogin *login = NULL;
    QString oldSid;

    QSet<QString> options = parentConnection().options();

    bool sqlNet = (options.find("SQL*Net") != options.end());
    if (!sqlNet)
    {
        oldSid = qgetenv("ORACLE_SID");
		qputenv("ORACLE_SID", parentConnection().database().toLatin1());
    }
    try
    {
        int session_mode = OCI_DEFAULT;
        if (options.find("SYS_OPER") != options.end())
            session_mode = OCI_SYSOPER;
        else if (options.find("SYS_DBA") != options.end())
            session_mode = OCI_SYSDBA;
#ifdef OCI_SYSASM
        else if (options.find("SYS_ASM") != options.end())
            session_mode = OCI_SYSASM;
#endif

        do
        {
            /* TODO
               if (!sqlNet)
               conn->server_attach();
               else
            */
            QString user = parentConnection().user();
            QString pass = parentConnection().password();

            try
            {
                // TODO what does _login destructor? and where is it?
                /*::trotl::OciLogin */ login = new ::trotl::OciLogin(_env,
                        ::trotl::LoginPara(
                            user.isEmpty() ? "" : user.toUtf8().constData(),
                            pass.isEmpty() ? "" : pass.toUtf8().constData(),
                            parentConnection().database().toUtf8().constData()
                        ),
                        (ub4) session_mode);
                conn = new ::trotl::OciConnection(_env, *login);

                TLOG(5, toDecorator, __HERE__) << "Oracle database version: "
                                               << ::std::hex << ::std::showbase << ::std::setw(10)
                                               << ::std::setfill('0') << ::std::internal
                                               << login->_server._version << ::std::endl
                                               << login->_server._version_string << ::std::endl
                                               << login->_server.versionNumber() << "."
                                               << login->_server.releaseNumber() << "."
                                               << login->_server.updateNumber() << "."
                                               << login->_server.portReleaseNumber() << "."
                                               << login->_server.portUpdateNumber()
                                               << ::std::dec << ::std::endl;
            }
            catch (const ::trotl::OciException &exc)
            {
                TLOG(5, toDecorator, __HERE__) << "TODO: catch" << std::endl << __HERE__ << std::endl;

                if (/*toThread::mainThread() &&*/ exc.get_code() == 28001)
                {
                    bool ok = false;
                    QString newpass = QInputDialog::getText(
                                          Utils::toQMainWindow(),
                                          qApp->translate("toOracleConnection", "Password expired"),
                                          qApp->translate("toOracleConnection", "Enter new password"),
                                          QLineEdit::Password,
                                          QString::null,
                                          &ok);
                    if (!ok)
                        throw exc;

                    QString newpass2 = QInputDialog::getText(
                                           Utils::toQMainWindow(),
                                           qApp->translate("toOracleConnection", "Password expired"),
                                           qApp->translate("toOracleConnection", "Enter password again for confirmation"),
                                           QLineEdit::Password,
                                           QString::null,
                                           &ok);

                    if (!ok)
                        throw exc;

                    if (newpass2 != newpass)
                        throw qApp->translate("toOracleConnection", "The two passwords doesn't match");
                    QString nputf = newpass;
                    if( login ) delete login;
                    login = new ::trotl::OciLogin(_env,
                                                  ::trotl::LoginAndPChangePara(
                                                      user.isEmpty() ? "" : user.toUtf8().constData(),
                                                      pass.isEmpty() ? "" : pass.toUtf8().constData(),
                                                      newpass.isEmpty() ? "" : newpass.toUtf8().constData(),
                                                      parentConnection().database().toUtf8().constData()
                                                  ),
                                                  (ub4) session_mode);
                    conn = new ::trotl::OciConnection(_env, *login);

                    parentConnection().setPassword(newpass);
                }
                else
                {
                    throw exc;
                } //  (toThread::mainThread() && exc.get_code() == 28001)
            } // catch (const ::trotl::OciException &exc)
        }
        while (!conn);
    }
    catch (const ::trotl::OciException &exc)
    {
        if (!sqlNet)
        {
            if (oldSid.isNull())
                qputenv("ORACLE_SID", "");
            else
                qputenv("ORACLE_SID", oldSid.toLatin1());
        }
        delete conn;
        ReThrowException(exc);
    }
    if (!sqlNet)
    {
        if (oldSid.isNull())
            qputenv("ORACLE_SID", "");
        else
        {
            qputenv("ORACLE_SID", oldSid.toLatin1());
        }
    }

    try
    {
        QString alterSessionSQL = QString::fromLatin1("ALTER SESSION SET NLS_DATE_FORMAT = '");
        alterSessionSQL += toConfigurationSingle::Instance().dateFormat();
        alterSessionSQL += QString::fromLatin1("'");
        oracleQuery::trotlQuery date(*conn, qPrintable(alterSessionSQL));
    }
    catch (...)
    {
        TLOG(5, toDecorator, __HERE__)
                << "Failed to set new default date format for session: "
                << toConfigurationSingle::Instance().dateFormat()
                << std::endl;
        Utils::toStatusMessage(QObject::tr("Failed to set new default date format for session: %1")
                               .arg(toConfigurationSingle::Instance().dateFormat()));
    }

    try
    {
        QString alterSessionSQL = QString::fromLatin1("ALTER SESSION SET NLS_TIMESTAMP_FORMAT = '");
        alterSessionSQL += toConfigurationSingle::Instance().timestampFormat();
        alterSessionSQL += QString::fromLatin1("'");
        oracleQuery::trotlQuery timestmp(*conn, qPrintable(alterSessionSQL));
    }
    catch (::trotl::OciException const& e)
    {
	    TLOG(5, toDecorator, __HERE__)
		    << "Failed to set new default timestmp format for session: "
		    << toConfigurationSingle::Instance().timestampFormat()
		    << std::endl
		    << e.what();
	    Utils::toStatusMessage(QObject::tr("Failed to set new default timestamp format for session: %1")
				   .arg(toConfigurationSingle::Instance().timestampFormat()));
    }    
    catch (...)
    {
	    TLOG(5, toDecorator, __HERE__)
		    << "Failed to set new default timestmp format for session: "
		    << toConfigurationSingle::Instance().timestampFormat()
		    << std::endl;
	    Utils::toStatusMessage(QObject::tr("Failed to set new default timestamp format for session: %1")
				   .arg(toConfigurationSingle::Instance().timestampFormat()));
    }

    try
    {
      oracleQuery::trotlQuery info(*conn,
				   "BEGIN\n"
				   "  SYS.DBMS_APPLICATION_INFO.SET_CLIENT_INFO('" TOAPPNAME " " TORAVERSION
				   " (http://tora.sf.net)"
				   "');\n"
				   "  SYS.DBMS_APPLICATION_INFO.SET_MODULE('" TOAPPNAME "','Access Database');\n"
				   "END;");
    }
    catch (::trotl::OciException const& e)
    {
        TLOG(5, toDecorator, __HERE__) << "Failed to set client info for session:\n" << e.what();
    }
    catch (...)
    {
        TLOG(5, toDecorator, __HERE__) << "Failed to set client info for session\n";
    }

    return new toOracleConnectionSub(conn, login);
}

void toOracleConnectionImpl::closeConnection(toConnectionSub *)
{
}

toOracleConnectionSub::toOracleConnectionSub(::trotl::OciConnection *conn, ::trotl::OciLogin *login)
    : _conn(conn)
    , _login(login)
	, _hasTransactionStat(new ::trotl::SqlStatement(*_conn, "select nvl2(dbms_transaction.local_transaction_id, 1, 0) from dual"))
	, _hasTransaction(NO_TRANSACTION)
{
}

toOracleConnectionSub::~toOracleConnectionSub()
{
	delete _hasTransactionStat;
}

void toOracleConnectionSub::cancel()
{
	try {
		_conn->cancel();
		_conn->reset();
	}   catch (const ::trotl::OciException &exc)  {
		ReThrowException(exc);
    }

    TLOG(0, toDecorator, __HERE__) << ":oracleConn::cancel(conn=" <<_conn << ", this=" << Query << ")" << std::endl;
}

void toOracleConnectionSub::close()
{
}

void toOracleConnectionSub::commit()
{
	_conn->commit();
	_hasTransaction = NO_TRANSACTION;
}

void toOracleConnectionSub::rollback()
{
	_conn->rollback();
	_hasTransaction = NO_TRANSACTION;
}

QString toOracleConnectionSub::version()
{
	QString ver("%1%2");
	return ver
			.arg((int)_login->_server.versionNumber(), 2, 10, QChar('0'))
			.arg((int)_login->_server.releaseNumber(), 2, 10, QChar('0'));
}

QString toOracleConnectionSub::sessionId()
{
	QString retval("(%1, %2)");
	return retval.arg(_login->sid()).arg(_login->serial());
}

bool toOracleConnectionSub::hasTransaction()
{
	// NOTE: do not use OCI_ATTR_TRANSACTION_IN_PROGRESS, it is Oracle 12c feature
	if (_hasTransaction != DIRTY_FLAG)
		return _hasTransaction;
	try
	{
		int i;
		*_hasTransactionStat >> i;
		_hasTransaction = i ? HAS_TRANSACTION : NO_TRANSACTION;
		return i;
	} catch (const ::trotl::OciException &exc) {
		ReThrowException(exc);
	}
}

queryImpl * toOracleConnectionSub::createQuery(toQuery *query)
{
	_hasTransaction = DIRTY_FLAG;
	return new oracleQuery(query, this);
}

toQAdditionalDescriptions* toOracleConnectionSub::decribe(toCache::ObjectRef const& objectName)
{
	toQAdditionalDescriptions *retval = new toQAdditionalDescriptions();
	::trotl::Describe *d = ::trotl::Describe::createDescription(*_conn, qPrintable(objectName.toString()));

	switch (d->whatIsThis()) {
	case ::trotl::Describe::O_SYNONYM:
		retval->insert("TOOLTIP", QString(d->toString().c_str()));
		break;
	case ::trotl::Describe::O_PUBLIC_SYNONYM:
		retval->insert("TOOLTIP", QString(d->toString().c_str()));
		break;
	case ::trotl::Describe::O_VIEW:
	case ::trotl::Describe::O_TABLE:
		{
			retval->insert("TOOLTIP", QString(d->toString().c_str()));
						
			toQColumnDescriptionList targetCols;
			std::vector<trotl::DescribeColumn*> &sourceCols = (d->whatIsThis() == ::trotl::Describe::O_VIEW) ?
					static_cast<trotl::DescribeView*>(d)->_columns :
					static_cast<trotl::DescribeTable*>(d)->_columns;
			foreach(::trotl::DescribeColumn *dc, sourceCols)
			{
				targetCols.append(toCache::ColumnDescription());
				toCache::ColumnDescription &lastColumn = targetCols.last();
				lastColumn.Name = dc->_name.c_str();
				lastColumn.Null = dc->_is_null;
				lastColumn.Datatype = dc->typeName().c_str();
				lastColumn.ToolTip = dc->toString().c_str();
			}
			retval->insert("COLUMNLIST", QVariant::fromValue(targetCols));
		}
		break;
	case ::trotl::Describe::O_COLUMN:
		retval->insert("TOOLTIP", QString(d->toString().c_str()));
		break;
	case ::trotl::Describe::O_TYPE:
		retval->insert("TOOLTIP", QString(d->toString().c_str()));
		break;
	case ::trotl::Describe::O_SEQUENCE:
		retval->insert("TOOLTIP", QString(d->toString().c_str()));
		break;
	case ::trotl::Describe::O_PROCEDURE:
		retval->insert("TOOLTIP", QString(d->toString().c_str()));
		break;
	case ::trotl::Describe::O_FUNCTION:
		retval->insert("TOOLTIP", QString(d->toString().c_str()));
		break;
	case ::trotl::Describe::O_PACKAGE:
		retval->insert("TOOLTIP", QString(d->toString().c_str()));
		break;
	default:
		break;
	}

	delete d;
	return retval;
}

toCache::ObjectRef toOracleConnectionSub::resolve(toCache::ObjectRef const& objectName)
{
	toCache::ObjectRef retval;
	retval.context = objectName.context;
	try
	{
		// TODO set context as current schema
		toQAdditionalDescriptions *descr = new toQAdditionalDescriptions();
		::trotl::Describe *d = ::trotl::Describe::createDescription(*_conn, qPrintable(objectName.toString()));
		switch (d->whatIsThis()) {
		case ::trotl::Describe::O_SYNONYM:
		case ::trotl::Describe::O_PUBLIC_SYNONYM:
		{
			trotl::DescribeSynonym *s = static_cast<trotl::DescribeSynonym*>(d);
			retval.first = s->_schema.c_str();
			retval.second= s->_name.c_str();
			if (!s->_link.empty())
			{
				retval.second += "@";
				retval.second += s->_link.c_str();
			}
		}
		break;
		default:
			break;
		}
		delete d;
	} catch (trotl::OciException const &exc)
	{
		// ignore OCI exception here (object to be described does not exist)
		if(exc.is_critical())
			Broken = true;
	}
	return retval;
}

