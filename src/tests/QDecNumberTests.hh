#ifndef QDECNUMBERTESTS_HH
#define QDECNUMBERTESTS_HH

#if defined(_MSC_VER) || defined(__GNUC__)
# pragma once
#endif

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QSet>


// FORWARDS
class QDecContext;
class QDecNumber;

class QDecNumberTests: public QObject
{
  Q_OBJECT
  Q_ENUMS(TestCodes_e)

  public:

  // CREATORS
  QDecNumberTests(const QStringList& arguments);
  
  enum TestCodes_e {
    TC_ignore = 0,
    TC_comment,
    TC_directive ,
    TC_test,
    TC_unknown
  };

 private slots:
  void compound_interest();
  void compressed_formats();
  void packed_decimals();
  void quad_tests();
  void quad_with_number();
  void QDecContext_tests();
  void QDecNumber_abs();
  void QDecNumber_add();
  void QDecimal_size();
  void conversion();
  void regression();
  void test_cases();

 private:  
  void procTestFile(const QString& filename);
  int procTestLine(const QString& line, QStringList& tokens);
  int applyTestDirective(const QStringList& tokens, QDecContext& ctx);
  int getDirectivesContext(QDecContext& ctx, bool precision=true);
  void displayDirectivesContext();
  void clearDirectivesContext();
  int opTest(const QStringList& tokens);
  int runTestCase(const QStringList& tokens, const QDecContext& ctx);


  bool token2QDecNumber(const QString& token, const QDecContext& ctx, QDecNumber& num);
  bool QDecNumber2token(QString& token, const QDecNumber& num);


  // MEMBERS
  // Current test file
  QString m_testFile; 
  // Test lines (cases + directives)
  QStringList m_testLines; 
  // Currently in force directives
  QMap<QString, QString> m_curDirectives;

  // Map of arguments
  QMap<QString, QString> m_argsMap;

  // Set of test cases to be skipped
  QSet<QString> m_skipSet;

};

#endif
