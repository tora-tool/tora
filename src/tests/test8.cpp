
#include "tests/test8.h"
#include "core/utils.h"

#include <QApplication>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkProxyFactory>

QNAMRedirect::QNAMRedirect(QWidget *parent)
	: QMainWindow(parent)
{
	_qnam = createQNAM();
	_originalUrl = "http://sourceforge.net/projects/tora/files/tora/LastVersion.txt/download";

	QVBoxLayout* layout = new QVBoxLayout;

	_textContainer = new QLabel("Click the button to test URL redirect!");
	_textContainer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	_textContainer->setAlignment(Qt::AlignCenter);
	_textContainer->setWordWrap(true);
	layout->addWidget(_textContainer);

	QPushButton* testButton = new QPushButton("Click here!");
	/* If the button is clicked, we'll do a request. */
	connect(testButton, SIGNAL(clicked()), this, SLOT(doRequest()));
	layout->addWidget(testButton);

	QFrame* frame = new QFrame;
	frame->setLayout(layout);
	setCentralWidget(frame);
	show();
}

QNAMRedirect::~QNAMRedirect() {
	if(_qnam) {
		_qnam->deleteLater();
	}
	if(_textContainer) {
		_textContainer->deleteLater();
	}
}

QNetworkAccessManager* QNAMRedirect::createQNAM()
{
	QNetworkAccessManager* qnam = new QNetworkAccessManager(this);

	/* We'll handle the finished reply in replyFinished */
	connect(qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
	return qnam;
}

void QNAMRedirect::doRequest()
{
	QString text = "QNAMRedirect::doRequest doing request to ";
	text.append(this->_originalUrl.toString());
	this->_textContainer->setText(text);

	/* Let's just create network request for this predifned URL... */
	QNetworkRequest request;
	request.setUrl(this->_originalUrl);
	request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
	request.setRawHeader("User-Agent", "QtNetworkRequestSimpleTest");

	/* ...and ask the manager to do the request. */
	this->_qnam->get(request);
}

void QNAMRedirect::replyFinished(QNetworkReply* reply)
{
	if (reply->error() != QNetworkReply::NoError)
	{
		QString errorMsg = QString("Error: %1").arg(ENUM_NAME(QNetworkReply, NetworkError, reply->error()));
		this->_textContainer->setText(errorMsg);
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
		_urlRedirectedTo = this->redirectUrl(possibleRedirectUrl.toUrl());
		break;
	}
	case 200:
	{
		qDebug() <<  reply->readAll();
		/*
		 * We weren't redirected anymore
		 * so we arrived to the final destination...
		 */
		QString text = QString("QNAMRedirect::replyFinished: Arrived to %1").arg(reply->url().toString());
		this->_textContainer->setText(text);
		/* ...so this can be cleared. */
		_urlRedirectedTo.clear();
		break;
	}
	}

	/* If the URL is not empty, we're being redirected. */
	if(!_urlRedirectedTo.isEmpty())
	{
		QString text = QString("QNAMRedirect::replyFinished: Redirected to %1").arg(_urlRedirectedTo.toString());
		this->_textContainer->setText(text);

		/* We'll do another request to the redirection url. */
		QNetworkRequest request;
		request.setUrl(_urlRedirectedTo);
		request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
		request.setRawHeader("User-Agent", "QtNetworkRequestSimpleTest");
		this->_qnam->get(request);
	}
	/* Clean up. */
	reply->deleteLater();
}

QUrl QNAMRedirect::redirectUrl(const QUrl& possibleRedirectUrl)
{
	QUrl redirectUrl;
	/*
	 * Check if the URL is non-empty and that we aren't being fooled into a infinite redirect loop.
	 */
	if(!possibleRedirectUrl.isEmpty() && !_oldUrls.contains(possibleRedirectUrl) && _oldUrls.size() < 5)
	{
		_oldUrls.insert(possibleRedirectUrl);
		redirectUrl = possibleRedirectUrl;
	}
	return redirectUrl;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    new QNAMRedirect;
    int ret = qApp->exec();
}
