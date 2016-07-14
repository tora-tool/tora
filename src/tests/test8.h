/*
 * Copyright (c) 2009 Nokia Corporation
 */
#pragma once

#include <QMainWindow>
#include <QFrame>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QtCore/QPointer>
#include <QtCore/QUrl>

class QNAMRedirect : public QMainWindow
{
    Q_OBJECT

public:
	QNAMRedirect(QWidget *parent = 0);
	~QNAMRedirect();

private slots:
	void doRequest();
	void replyFinished(QNetworkReply* reply);

private:
	QPointer<QNetworkAccessManager> _qnam;
	QUrl _originalUrl;
	QUrl _urlRedirectedTo;
	QSet<QUrl> _oldUrls;

	QPointer<QLabel> _textContainer;

	QNetworkAccessManager* createQNAM();
	QUrl redirectUrl(const QUrl& possibleRedirectUrl);
};
