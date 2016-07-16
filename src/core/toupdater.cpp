
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

#include "core/toupdater.h"
#include "core/utils.h"
#include "core/toconfiguration.h"
#include "core/toglobalconfiguration.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QtCore/QUrl>
#include <QtCore/QMetaEnum>
#include <QtCore/QPointer>
#include <QtCore/QUrl>
#include <QtCore/QDebug>
#include <QtCore/QDate>

toUpdater::toUpdater()
	: QObject()
    , m_updated(false)
	, m_mode(true)
{
	m_qnam = createQNAM();
	m_originalUrl = "http://sourceforge.net/projects/tora/files/tora/LastVersion.txt/download";
}

toUpdater::~toUpdater()
{
	if(m_qnam)
		m_qnam->deleteLater();
	m_qnam = NULL;
}

QNetworkAccessManager* toUpdater::createQNAM()
{
	QNetworkAccessManager* qnam = new QNetworkAccessManager(this);

	/* We'll handle the finished reply in replyFinished */
	connect(qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
	return qnam;
}

void toUpdater::check(bool force)
{
	bool useUpdates = toConfigurationNewSingle::Instance().option(ToConfiguration::Global::UpdatesCheckBool).toBool();
	QDate lastChecked = toConfigurationNewSingle::Instance().option(ToConfiguration::Global::UpdateLastDate).toDate();
	int updateState = toConfigurationNewSingle::Instance().option(ToConfiguration::Global::UpdateStateInt).toInt();

	if ( (!force && !useUpdates) // check during startup and check for updates is off
			|| (!force && updateState == 1) // already running
			|| (!force && updateState == 2) // update crashed (wrong ver. of OpenSSL loaded)
			|| (!force && lastChecked.isValid() && lastChecked.daysTo(QDate::currentDate()) < 7)
	)
		return;

	m_mode = force;
	m_version = QString("toUpdater::doRequest doing request to ").arg(m_originalUrl.toString());

	/* Let's just create network request for this predefined URL... */
	QNetworkRequest request;
	request.setUrl(m_originalUrl);
	request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
	request.setRawHeader("User-Agent", "TOraUpdater");

	/* ...and ask the manager to do the request. */
	toConfigurationNewSingle::Instance().setOption(ToConfiguration::Global::UpdateStateInt, 1);
	toConfigurationNewSingle::Instance().saveAll();
	this->m_qnam->get(request);
}

void toUpdater::replyFinished(QNetworkReply* reply)
{
	reply->ignoreSslErrors();
	if (reply->error() != QNetworkReply::NoError)
	{
		QString errorMsg = QString("Error: %1").arg(ENUM_NAME(QNetworkReply, NetworkError, reply->error()));
		m_version.append(errorMsg);
		reply->deleteLater();
		reply = NULL;
		m_updated = true;
		emit updatingFinished(m_version);
		return;
	}

	int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	switch (statusCode) {
	case 301:
	case 302:
	case 307:
	{
		qDebug() << "redirected: " << reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

		/*
		 * Reply is finished!
		 * We'll ask for the reply about the Redirection attribute
		 * http://doc.trolltech.com/qnetworkrequest.html#Attribute-enum
		 */
		QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

		/* We'll deduct if the redirection is valid in the redirectUrl function */
		m_urlRedirectedTo = this->redirectUrl(possibleRedirectUrl.toUrl());
		break;
	}
	case 200:
	{
		QByteArray body = reply->readAll();
		qDebug() <<  body;
		/*
		 * We weren't redirected anymore
		 * so we arrived to the final destination...
		 */
		QString text = QString("QNAMRedirect::replyFinished: Arrived to %1\n%2\n").arg(reply->url().toString()).arg(QString(body));
		m_version.append(text);
		/* ...so this can be cleared. */
		m_urlRedirectedTo.clear();
		m_updated = true;
		emit updatingFinished(m_version);
		toConfigurationNewSingle::Instance().setOption(ToConfiguration::Global::UpdateStateInt, 0);
		toConfigurationNewSingle::Instance().setOption(ToConfiguration::Global::UpdateLastDate, QDate::currentDate());
		toConfigurationNewSingle::Instance().saveAll();
		break;
	}
	default:
		QString errorMsg = QString("Error: %1").arg(ENUM_NAME(QNetworkReply, NetworkError, reply->error()));
		m_version.append(errorMsg);
		reply->deleteLater();
		reply = NULL;
		m_updated = true;
		emit updatingFinished(m_version);
		return;
	}

	/* If the URL is not empty, we're being redirected. */
	if(!m_urlRedirectedTo.isEmpty())
	{
		QString text = QString("QNAMRedirect::replyFinished: Redirected to %1\n").arg(m_urlRedirectedTo.toString());
		m_version.append(text);

		/* We'll do another request to the redirection url. */
		QNetworkRequest request;
		request.setUrl(m_urlRedirectedTo);
		request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
		request.setRawHeader("User-Agent", "TOraUpdater");
		this->m_qnam->get(request);
		emit updatingChanged(m_version);
	} else {
		emit updatingFinished(m_version);
	}
	/* Clean up. */
	reply->deleteLater();
	reply = NULL;
}

QUrl toUpdater::redirectUrl(const QUrl& possibleRedirectUrl)
{
	QUrl redirectUrl;
	/*
	 * Check if the URL is non-empty and that we aren't being fooled into a infinite redirect loop.
	 */
	if(!possibleRedirectUrl.isEmpty() && !m_oldUrls.contains(possibleRedirectUrl) && m_oldUrls.size() < 5)
	{
		m_oldUrls.insert(possibleRedirectUrl);
		redirectUrl = possibleRedirectUrl;
	}
	return redirectUrl;
}


