#include <QtTest/QtTest>

#include "QDecNumberTests.hh"

#include "core/toqvalue.h"
#include "QDecNumber.hh"
#include "QDecPacked.hh"

#if defined(__GNUC__)
# ident "$Id$"
#elif defined(__sun)
# pragma ident "$Id$"
#elif defined(_WIN32)
# pragma comment( user, __FILE__ " " __DATE__ " " __TIME__ "$Id$" )
#endif



void MessageOutput(QtMsgType type, const QMessageLogContext &context,
                     const QString &msg)
{
  QByteArray lmsg = msg.toLocal8Bit();
  const char* cmsg = lmsg.constData();
  switch (type) {
    case QtDebugMsg:
      fprintf(stderr, "%s\n", cmsg);
      break;
    case QtWarningMsg:
      fprintf(stderr, "Warn: %s\n", cmsg);
      break;
    case QtCriticalMsg:
      fprintf(stderr, "Critical: %s\n", cmsg);
      break;
    case QtFatalMsg:
      fprintf(stderr, "Fatal: %s\n", cmsg);
      abort();
  }
}



//QTEST_MAIN(QDecNumberTests)

int main(int argc, char* argv[])
{
  int rv;
  qInstallMessageHandler(MessageOutput);
  QCoreApplication app(argc, argv);
#if 0
  QStringList args = QCoreApplication::arguments();

  QRegExp flagre("--(\\w+)=.*");
  QStringList tc_args;
  QStringList qt_args;

  // Separate QTest arguments out of test class arguments
  QStringListIterator ai(args);
  while(ai.hasNext()) {
    QString item = ai.next();
    if(flagre.exactMatch(item))
      tc_args << item;
    else
      qt_args << item;
  }

  QDecNumberTests tc(tc_args);

  // Increase limit for warnings count
  qt_args << "-maxwarnings" << "9999999";
  rv = QTest::qExec(&tc, qt_args);

#endif

  toQValue v1;
  QVariant v2;

  QDecNumber n1("0.12345");
  QDecNumber n2("1.12345");

  QString a = n2.toString();

  if (n2 > n1)
  {
	  qDebug() << n2.toString() << " " << a;
  }

  v2.setValue(n1);
  v2.setValue(n2);

  QDecPacked p1("0.12345");
  QDecPacked p2("1.12345");

  //v2.setValue(p1);
  //v2.setValue(p2);

  qDebug() << p1.toString() << " " << p2.toString();
  //if (p2 > p1)
  //{

  //}

  //return rv;
}

//include "moc_QDecNumberTests.cpp"
