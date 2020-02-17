#include <float.h>

#include <QString>
#include <QStringList>
#include <QtTest/QtTest>

#include "QDecContext.hh"
#include "QDecNumber.hh"
#include "QDecSingle.hh"
#include "QDecDouble.hh"
#include "QDecQuad.hh"
#include "QDecPacked.hh"

extern "C" {
#include "decimal64.h"
#include "decimal128.h"
#include "decPacked.h"
#include "decQuad.h"
}

#include <iostream>
using namespace std;

#include "QDecNumberTests.hh"


#if defined(__GNUC__)
# ident "$Id$"
#elif defined(__sun)
# pragma ident "$Id$"
#elif defined(_WIN32)
# pragma comment( user, __FILE__ " " __DATE__ " " __TIME__ "$Id$" )
#endif


QDebug operator<<(QDebug dbg, const QDecContext& c)
{
  QString cstr;
  {
    QTextStream ts(&cstr);
    ts << c;
  }
  dbg.nospace() << cstr;
  return dbg.space();
}


QDecNumberTests::QDecNumberTests(const QStringList& args)
{
  // These are the flags we recognize
  QStringList flags;
  flags << "testdir" << "testfile" << "testcase"
        << "testfilefilter";

  // Generic flag format
  QRegExp flagre("--(\\w+)=(.*)");

  // Store flags and values in m_argsMap
  for(int i=0; i<args.size(); i++) {
    QString flag = args.at(i);
    if(flagre.exactMatch(flag)) {
      QString fkey = flagre.cap(1);
      QString fval = flagre.cap(2);
      m_argsMap.insert(fkey.toLower(), fval);
    }
  }

}


void QDecNumberTests::compound_interest()
{
  decNumber one, mtwo, hundred; // constants
  decNumber start, rate, years; // parameters
  decNumber total; // result
  decContext set; // working context
  char string[DECNUMDIGITS+14]; // conversion buffer
 
  decContextDefault(&set, DEC_INIT_BASE); // initialize
  set.traps=0; // no traps
  set.digits=25; // precision 25
  decNumberFromString(&one, "1", &set); // set constants
  decNumberFromString(&mtwo, "-2", &set);
  decNumberFromString(&hundred, "100", &set);

  QVERIFY(0 == set.status);
  
  decNumberFromString(&start, "100000", &set); // parameter words
  decNumberFromString(&rate, "6.5", &set);
  decNumberFromString(&years, "20", &set);
  
  decNumberDivide(&rate, &rate, &hundred, &set); // rate=rate/100
  decNumberAdd(&rate, &rate, &one, &set); // rate=rate+1
  decNumberPower(&rate, &rate, &years, &set); // rate=rate**years
  decNumberMultiply(&total, &rate, &start, &set); // total=rate*start
  decNumberRescale(&total, &total, &mtwo, &set); // two digits please

  QVERIFY(0 == (set.status & DEC_Errors));

  decNumberToString(&total, string);

  QCOMPARE(string, "352364.51");
}


void QDecNumberTests::compressed_formats()
{
  decimal64 a; // working decimal64 number
  decNumber d; // working number
  decContext set; // working context
  char string[DECIMAL64_String]; // number>string buffer
  char hexes[25]; // decimal64>hex buffer
  int i; // counter
   
  decContextDefault(&set, DEC_INIT_DECIMAL64); // initialize
  
  decimal64FromString(&a, "79", &set);
  // lay out the decimal64 as eight hexadecimal pairs
  for (i=0; i<8; i++) {
    sprintf(&hexes[i*3], "%02x ", a.bytes[i]);
  }
  decimal64ToNumber(&a, &d);
  decNumberToString(&d, string);

  QCOMPARE(string, "79");
  // Little endian:
  QCOMPARE((const char*)hexes, "79 00 00 00 00 00 38 22 ");

  // Big endian:
  //QCOMPARE(hexes, "22 38 00 00 00 00 00 79 ");
 
  //printf("%s => %s=> %s\n", argv[1], hexes, string);
}


void QDecNumberTests::packed_decimals()
{
  uint8_t startpack[]={0x01, 0x00, 0x00, 0x0C}; // investment=100000
  int32_t startscale=0;
  uint8_t ratepack[]={0x06, 0x5C}; // rate=6.5%
  int32_t ratescale=1;
  uint8_t yearspack[]={0x02, 0x0C}; // years=20
  int32_t yearsscale=0;
  uint8_t respack[16]; // result, packed
  int32_t resscale; // ..
  char hexes[49]; // for packed>hex
  int i; // counter

  decNumber one, mtwo, hundred; // constants
  decNumber start, rate, years; // parameters
  decNumber total; // result
  decContext set; // working context
 
  decContextDefault(&set, DEC_INIT_BASE); // initialize
  set.traps=0; // no traps
  set.digits=25; // precision 25
  decNumberFromString(&one, "1", &set); // set constants
  decNumberFromString(&mtwo, "-2", &set);
  decNumberFromString(&hundred, "100", &set);

  QVERIFY(0 == set.status);

  decPackedToNumber(startpack, sizeof(startpack), &startscale, &start);
  decPackedToNumber(ratepack, sizeof(ratepack), &ratescale, &rate);
  decPackedToNumber(yearspack, sizeof(yearspack), &yearsscale, &years);
  
  decNumberDivide(&rate, &rate, &hundred, &set); // rate=rate/100
  decNumberAdd(&rate, &rate, &one, &set); // rate=rate+1
  decNumberPower(&rate, &rate, &years, &set); // rate=rate**years
  decNumberMultiply(&total, &rate, &start, &set); // total=rate*start
  decNumberRescale(&total, &total, &mtwo, &set); // two digits please
  
  decPackedFromNumber(respack, sizeof(respack), &resscale, &total);
  
  // lay out the total as sixteen hexadecimal pairs
  for (i=0; i<16; i++) {
    sprintf(&hexes[i*3], "%02x ", respack[i]);
  }


  QVERIFY(resscale == 2);
  QCOMPARE((const char*)hexes, "00 00 00 00 00 00 00 00 00 00 00 03 52 36 45 1c ");
  //printf("Result: %s (scale=%ld)\n", hexes, (long int)resscale);

}


void QDecNumberTests::quad_tests()
{
  decQuad a, b; // working decQuads
  decContext set; // working context
  char string[DECQUAD_String]; // number>string buffer

  decContextDefault(&set, DEC_INIT_DECQUAD); // initialize

  decQuadFromString(&a, "1.123456", &set);
  decQuadFromString(&b, "2.111111", &set);
  decQuadAdd(&a, &a, &b, &set); // a=a+b
  decQuadToString(&a, string);

  QCOMPARE(string, "3.234567");

  //printf("%s + %s => %s\n", argv[1], argv[2], string);
}


void QDecNumberTests::quad_with_number()
{
 decQuad a; // working decQuad
 decNumber numa, numb; // working decNumbers
 decContext set; // working context
 char string[DECQUAD_String]; // number>string buffer

 decContextDefault(&set, DEC_INIT_DECQUAD); // initialize

 decQuadFromString(&a, "1.0", &set); // get a
 decQuadAdd(&a, &a, &a, &set); // double a
 decQuadToNumber(&a, &numa); // convert to decNumber
 decNumberFromString(&numb, "2.0", &set);
 decNumberPower(&numa, &numa, &numb, &set); // numa=numa**numb
 decQuadFromNumber(&a, &numa, &set); // back via a Quad
 decQuadToString(&a, string); // ..

 QCOMPARE(string, "4.00");
 //printf("power(2*%s, %s) => %s\n", argv[1], argv[2], string);
}


void QDecNumberTests::QDecContext_tests()
{
   QDecContext dc;
   dc.setDigits(15);
   dc.setRound(DEC_ROUND_HALF_UP);
   dc.setEmin(-999999999);
   dc.setEmax(999999999);

   
   QCOMPARE(0, (int)dc.status());
   QVERIFY(0==dc.status());
   QCOMPARE(DEC_ROUND_HALF_UP, dc.round());

}




void QDecNumberTests::QDecNumber_abs()
{
  QDecContext cxt;
  QDecNumber  dcn;
  
  cxt.setDigits(15);
  cxt.setRound(DEC_ROUND_HALF_UP);
  cxt.setEmax(384);
  cxt.setEmin(-383);

  QVERIFY(dcn.fromString("1").abs(&cxt).toString() == "1");
  QVERIFY(dcn.fromString("-1").abs(&cxt).toString() == "1");
  //qDebug() << "abs:" << dcn.fromString("0.00").abs(&cxt).toString();
  QVERIFY(dcn.fromString("0.00").abs(&cxt).isZero());
  QVERIFY(dcn.fromString("-101.5").abs(&cxt).toString() == "101.5");
  
  cxt.setDigits(9);
  QVERIFY(dcn.fromString("-56267E-5").abs(&cxt).toString() == "0.56267");
  
}


void QDecNumberTests::QDecNumber_add()
{
  QDecContext cxt;
  QDecNumber  op1,op2;

  cxt.setDigits(9);
  cxt.setRound(DEC_ROUND_HALF_UP);
  cxt.setEmax(384);
  cxt.setEmin(-383);

  op1.fromString("2");
  op2.fromString("3");
  QVERIFY(op1.add(op2,&cxt).toString() == "5");

  op1.fromString("-7");
  op2.fromString("2.5");
  QVERIFY(op1.add(op2,&cxt).toString() == "-4.5");

  op1.fromString("7000");
  op2.fromString("10000e+9");
  QVERIFY(op1.add(op2,&cxt).toString() == "1.00000000E+13");

}

void QDecNumberTests::QDecimal_size()
{
  qDebug() << "sizeof(QDecContext)" << sizeof(QDecContext);
  QVERIFY(sizeof(decContext) == sizeof(QDecContext));

  qDebug() << "sizeof(QDecPacked)" << sizeof(QDecPacked);

  qDebug() << "QDecNumDigits=" << QDecNumDigits;
  qDebug() << "sizeof(QDecNumber)" << sizeof(QDecNumber);

  qDebug() << "sizeof(QDecSingle)" << sizeof(QDecSingle);
  QVERIFY(sizeof(QDecSingle)==4);
  qDebug() << "sizeof(QDecDouble)" << sizeof(QDecDouble);
  QVERIFY(sizeof(QDecDouble)==8);
  qDebug() << "sizeof(QDecQuad)" << sizeof(QDecQuad);
  QVERIFY(sizeof(QDecQuad)==16);
  
}


static void qDebugDouble(const char* label, double d)
{
  char bfr[1024];
  sprintf(bfr,"%.*g",QDecNumDigits, d);
  qDebug() << label << bfr;
}

static bool qRealFuzzyCompare(double d1, double d2)
{
  double delta = d1-d2;
  
  if(delta==0.0) return true;
  
  // We want absolute values
  if(delta < 0) delta *= -1.0;

  double max = (d1 > d2) ? d1 : d2;
  // 1e-6 is the highest level of error margin
  if(delta/max > 0.000001)  {
    qDebug() << "max=" << max
             << "delta=" << delta
             << "d/m=" << delta/max
             << endl;
    return false; // not equal
  }

  return true; // equal
}

void QDecNumber_conv(const char* dblstr)
{
  QDecNumber n;
  QDecContext c(DEC_INIT_DECIMAL128);
  double d;
  const char* ns = dblstr; 
  char bfr[1024];

  qDebug() << endl << "QDecNumber conversion tests using string" << ns;
  d = strtod(ns,0);
  qDebugDouble("d=", d);
  sprintf(bfr,"%.*g",QDecNumDigits, d);
  qDebug() << "d=" << d << bfr;
  QCOMPARE(d, atof(ns));

  n.fromString(ns /*,&c*/);
  qDebug() << "n=" << n.toString() << n.toEngString();
  if(n.isNaN())
    return; // Skip rest of the tests
  qDebug() << "n.toDouble()=" << n.toDouble();
  qDebug() << "QString(n.toDouble())="
           << QString::number(n.toDouble(),'g',QDecNumDigits);
  sprintf(bfr, "%.*g", QDecNumDigits, n.toDouble());
  qDebug() << "sprintf(n.toDouble())=" << bfr;
  QCOMPARE(n.toDouble(), d);

  QDecNumber n2;
  n2.fromDouble(d); 
  qDebug() << "n2=" << n2.toString();
  QCOMPARE(d, n2.toDouble());
}


void QDecX_conv(const char* dblstr)
{
  QDecSingle qs;
  QDecDouble qd;
  QDecQuad   qq;
  double d;
  const char* ns = dblstr; 

  qDebug() << endl << "QDecX conversion tests using string" << ns;
  d = strtod(ns,0);
  qDebugDouble("d=",d);
  QCOMPARE(d, atof(ns));

  qs.fromString(ns);
  qDebug() << "qs.fromString()=" << qs.toString();
  qs.fromDouble(d);
  qDebug() << "qs.fromDouble()=" << qs.toString();
  qDebugDouble("qs.toDouble()=",qs.toDouble());
  QVERIFY(qRealFuzzyCompare(d, qs.toDouble()));

  qd.fromString(ns);
  qDebug() << "qd.fromString()=" << qd.toString();
  qd.fromDouble(d);
  qDebug() << "qd.fromDouble()=" << qd.toString();
  qDebugDouble("qd.toDouble()=",qd.toDouble());
  QVERIFY(qRealFuzzyCompare(d, qd.toDouble()));

  qq.fromString(ns);
  qDebug() << "qq.fromString()=" << qq.toString();
  qq.fromDouble(d);
  qDebug() << "qq.fromDouble()=" << qq.toString();
  qDebugDouble("qq.toDouble()=",qq.toDouble());
  QVERIFY(qRealFuzzyCompare(d, qq.toDouble()));

}

void QDecPacked_conv(const char* dblstr)
{
  QDecPacked qp;
  QDecNumber n;
  double d;
  const char* ns = dblstr; 

  qDebug() << endl << "QDecPacked conversion tests using string" << ns;
  d = strtod(ns,0);
  qDebugDouble("d=",d);
  QCOMPARE(d, atof(ns));

  n.fromString(ns);
  if(n.isNaN())
    return; // Skip rest of the tests

  qp.fromString(ns);
  qDebug() << "qp.fromString()=" << qp.toString();
  qDebugDouble("qp...toDouble()=",qp.toQDecNumber().toDouble());
  QVERIFY(qRealFuzzyCompare(d, qp.toQDecNumber().toDouble()));
  
  qp.fromDouble(d);
  qDebug() << "qp.fromDouble()=" << qp.toString();
  // fromDouble() is not precise, use the string again
  qp.fromString(ns);
  qDebug() << "qp.scale()=" << qp.scale()
           << "qp.length()=" << qp.length()
           << "qp.bytes()=" << qp.bytes().toHex();
  QVERIFY(n == qp.toQDecNumber());
}

void QDecNumberTests::conversion() 
{
  const char* darr[] = {
    "1", "0.123", "10.0123", "210.01234567",
    "9876543210.01234567890123456789",
    "1.01234567890123456789012345678901234567890123456789012345678901234567890123456789",
    "x.y?",
    0 
  };

  /*
  QDecNumber n;
  n.fromString("1.125");
  qDebug() << n.toDouble();
  return;
  */

  int i;
  for(i=0; darr[i] != 0; ++i) 
    QDecNumber_conv(darr[i]);

  for(i=0; darr[i] != 0; ++i) 
    QDecX_conv(darr[i]);

  for(i=0; darr[i] != 0; ++i) 
    QDecPacked_conv(darr[i]);

}


void QDecNumberTests::regression()
{
  { // Issue #1
    double dmax = DBL_MAX;
    double dmin = DBL_MIN;

    QDecDouble ddmax(dmax);
    qDebug() << "dmax=" << dmax << "ddmax=" << ddmax.toString();
    //QCOMPARE(ddmax.toDouble(), dmax);
    QVERIFY(1);
    
    QDecDouble ddmin(dmin);
    qDebug() << "dmin=" << dmin << "ddmin=" << ddmin.toString();
    //QCOMPARE(ddmin.toDouble(), dmin);
    QVERIFY(1);
  }
    
}


//
//--------------------------------------
//



void QDecNumberTests::procTestFile(const QString& filename)
{
  QFile file(filename);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Cannot open " << filename;
    return;
  }
  else {
    m_testFile = filename;
    m_testLines.clear();
  }

  while(!file.atEnd()) {
    QByteArray line = file.readLine();
    m_testLines.append(line);
  }

  // Remove directives belonging to previous files
  clearDirectivesContext();
  
  int rv ;
  QByteArray line;
  QStringList tokens;
  QStringListIterator si(m_testLines);
  QDecContext context;
  
  while(si.hasNext()) {
    QString line = si.next();
    rv = procTestLine(line , tokens);
    switch(rv) {
      case TC_unknown:
      case TC_ignore:
      case TC_comment: break;

      case TC_directive:
        applyTestDirective(tokens, context);
        break;

      case TC_test:
        qDebug() << "TESTCASE: " << line.trimmed();
        if(m_skipSet.contains(tokens.at(0))) {
          qDebug() << "SKIP(skipSet):" << line.trimmed();
        }
        else if(context.digits() > QDecNumDigits) {
          // Skip testcase if precision required is higher than
          // QDecNumber can accommodate.
          qDebug() << "SKIP(precision):" << line.trimmed();
        }
        else {
          // No precision issue, run the test case
          runTestCase(tokens, context);
        }
        break;
    }
  } // end while

  qDebug() << "Number of test lines " << m_testLines.size();
}


int QDecNumberTests::procTestLine(const QString& line,
                                  QStringList& tokens)
{
  QRegExp re_space("^\\s*");
  QRegExp re_comment("^(\\s*)--(.*)");
  QRegExp re_directive("^([^:]+):(.+)");
  QRegExp re_testop("^(.+)->(.+)");

  tokens.clear();

  if(re_space.exactMatch(line)) {
    // Ignore empty lines
    return TC_ignore;
  }
  if(re_comment.exactMatch(line)) {
    // Ignore full comment lines
    return TC_comment;
  }

  // Find inline comments if any
  QString ln = line;
  int cpos = line.indexOf("--");
  if(cpos > 0) {
    // If comment start is not in quotes
    int qpos = line.indexOf("'");
    if(!(qpos > 0 && qpos < cpos)) {
      // Comment is not in quotes...
      ln = line.mid(0,cpos-1); // Extract non-comment part      
    }
  }
  
  if(re_directive.exactMatch(ln)) {
    QString keyword = re_directive.cap(1).simplified();
    QString value = re_directive.cap(2).simplified();
    //qDebug() << '[' << keyword << ':' << value << ']';
    tokens << keyword << value;
    return TC_directive;
  }
  else if(re_testop.exactMatch(ln)) {
    // Unary/Binary test operation tokens
    QRegExp tot("^\\s*(\\S+)\\s+(\\S+)\\s+('[^']+'|\\S+)\\s*(\\S*)\\s*(\\S*)\\s*->\\s*(\\S+)\\s*(.*)");
    if(tot.exactMatch(ln)) {
      QString id = tot.cap(1).simplified();
      QString op = tot.cap(2).simplified();
      // Don't trim whitespace from tokens
      QString opd1 = tot.cap(3); //.simplified();
      QString opd2 = tot.cap(4); //.simplified();
      QString opd3 = tot.cap(5); //.simplified();
      QString res = tot.cap(6).simplified();
      QString cond = tot.cap(7).simplified();
      tokens << id << op << opd1 << opd2 << opd3 << res << cond;
      //qDebug() << "Parsed tokens: " << tokens.join("|");
      return TC_test;
    }
    else {
      qWarning() << "Cannot parse test op: " << ln;
    }
  }
  else {
    // Unidentified line
    qWarning() << "Unidentified line: " << line;
  }
  
  return TC_unknown;  
}

int QDecNumberTests::applyTestDirective(const QStringList& tokens, QDecContext& ctx)
{
  if(tokens.size() != 2 ) {
    qWarning() << "Invalid tokens:" << tokens.join(" ");
    return -1;
  }
  QString key = tokens.at(0).toLower(); // keyword
  QString val = tokens.at(1).toLower(); // value
  int rv = -1; // Return value error by default

  // Flags required to construct context object
  bool ok;

  // Clear any status value left before
  ctx.zeroStatus();
  
  //
  // Mandatory directives
  //
  if(!key.compare("precision")) {
    // No operation to be done on context
    unsigned pval = val.toUInt(&ok);
    // Check if conversion is ok
    if(ok) {
      rv = 0;
      if(pval < (unsigned)QDecNumDigits)
        ctx.setDigits(pval);
    }
    else {
      qWarning() << "Precison value conversion failed: " << val;
    }    
  }
  else if(!key.compare("rounding")) {
    rv = 0; // Assume success
    if(val == "ceiling") {
      ctx.setRound(DEC_ROUND_CEILING);
    }
    else if(val == "down") {
      ctx.setRound(DEC_ROUND_DOWN);
    }
    else if(val == "floor") {
      ctx.setRound(DEC_ROUND_FLOOR);
    }
    else if(val == "half_down") {
      ctx.setRound(DEC_ROUND_HALF_DOWN);
    }
    else if(val == "half_even") {
      ctx.setRound(DEC_ROUND_HALF_EVEN);
    }
    else if(val == "half_up") {
      ctx.setRound(DEC_ROUND_HALF_UP);
    }
    else if(val == "up") {
      ctx.setRound(DEC_ROUND_UP);
    }
    else if(val == "05up") {
      ctx.setRound(DEC_ROUND_05UP);
    }
    else {
      rv = -1;
      qWarning() << "Unknown value for rounding: " << val;
    }
  }
  else if(!key.compare("maxexponent")) {
    int32_t emax = (int32_t) val.toInt(&ok, 10);
    if(ok) {
      rv = 0;
      ctx.setEmax(emax);
    }
    else {
      qWarning() << "Unrecognized maxexponent: " << val;
    }
  }
  else if(!key.compare("minexponent")) {
    int32_t emin = (int32_t) val.toInt(&ok, 10);
    if(ok) {
      rv = 0;
      ctx.setEmin(emin);
    }
    else {
      qWarning() << "Unrecognized minexponent: " << val;
    }
  }

  //
  // Optional directives
  //
  if(!key.compare("version")) {
    // No operation for version, just store it
    rv = 0;
  }
  else if(!key.compare("extended")) {
    uint8_t ext = static_cast<uint8_t>(val.toInt(&ok));
    if(ok) {
      rv = 0;
      ctx.setExtended(ext); 
    }
    else
      qWarning() << "Unrecognized extended: " << val;
  }
  else if(!key.compare("clamp")) {
    uint8_t clp = static_cast<uint8_t>(val.toInt(&ok));
    if(ok) {
      rv = 0;
      ctx.setClamp(clp); 
    }
    else
      qWarning() << "Unrecognized clamp: " << val;

  }
  else if(!key.compare("dectest")) {
    // Process the test cases in a file
    rv = 0;
    //TODO: Include the file
  }


  // Check if keyword/value pair is recognized
  if(rv != 0)
    qWarning() << "Unknown keyword " << key;
  else {
    m_curDirectives.insert(key, val);
    //qDebug() << "directive=" << key << " val=" << val;
  }

  //qDebug() << "ctx=" << ctx;
  return rv;
}



int QDecNumberTests::getDirectivesContext(QDecContext& ctx, bool precision)
{
  QMapIterator<QString, QString> i(m_curDirectives);
  QStringList tokens;
  
  while(i.hasNext()) {
    i.next();
    if(!precision) {
      if(i.key() == "precision")
        continue; // Ignore precison directives if not wanted
    }
    tokens.clear();
    tokens << i.key() << i.value();
    applyTestDirective(tokens, ctx);
  }

  // If precision is not wanted, pick largest exponent values
  // to avoid rounding
  if(!precision) {
    ctx.setEmax(QDecMaxExponent); 
    ctx.setEmin(QDecMinExponent); 
  }

  if(ctx.status())
    qWarning() << "getDirectivesContext ctx=" << ctx.statusToString();
  //qDebug() << "getDirectivesContext ctx=" << ctx;
  
  return 0;
}

void QDecNumberTests::displayDirectivesContext()
{
  QMapIterator<QString, QString> i(m_curDirectives);

  qDebug() << "Currently valid context settings:";
  while(i.hasNext()) {
    i.next();
    qDebug() << i.key() << '=' << i.value();
  }
}

void QDecNumberTests::clearDirectivesContext()
{
  m_curDirectives.clear();
}


inline bool is_unary_op(QString op)
{
  QStringList op_list;
  op_list 
      << "abs"
      << "apply"
      << "class"
      << "canonical"
      << "copy"
      << "copyabs"
      << "copynegate"
      << "exp"
      << "invert"
      << "ln"
      << "log10"
      << "logb"
      << "minus"
      << "nextminus"
      << "nextplus"
      << "plus"
      << "reduce"
      << "squareroot"
      << "toSci"
      << "toEng"
      << "minus"
      << "tointegral"
      << "tointegralx"
      << "trim"
      ;

  return op_list.contains(op, Qt::CaseInsensitive);
}

inline bool is_binary_op(QString op)
{
  QStringList op_list;
  op_list 
      << "add"
      << "and"
      << "compare"
      << "comparesig"
      << "comparetotal"
      << "comparetotalmag"
      << "comparetotmag"
      << "copysign"
      << "divide"
      << "divideint"
      << "max"
      << "min"
      << "maxmag"
      << "minmag"
      << "multiply"
      << "nexttoward"
      << "or"
      << "power"
      << "quantize"
      << "remainder"
      << "remaindernear"
      << "rescale"
      << "rotate"
      << "samequantum"
      << "scaleb"
      << "shift"
      << "subtract"
      << "xor"
      ;

  return op_list.contains(op, Qt::CaseInsensitive);
}

inline bool is_ternary_op(QString op)
{
  QStringList op_list;
  op_list 
      << "fma"
      ;
  return op_list.contains(op, Qt::CaseInsensitive);
}


// Binary operations
inline QDecNumber op_do(QString op,
                        QDecNumber& n1, QDecNumber& n2, QDecNumber& n3,
                        QDecContext& c, QString& rs)
{
  //
  // Unary operations
  //
  if("abs" == op)
    return n1.abs(&c);
  if("apply" == op)
    return n1.plus(&c);
  // canonical is similar to apply
  if("canonical" == op)
    return n1.plus(&c);
  if("class" == op) {
    enum decClass dc = n1.toClass(&c);
    rs = n1.ClassToString(dc);
    return n1;
  }
  // Copy operation modifies the callee, thus operation
  // is done on unused operand
  if("copy" == op) 
    return n2.copy(n1);
  if("copyabs" == op)
    return n2.copyAbs(n1);
  if("copynegate" == op)
    return n2.copyNegate(n1);
  if("copysign" == op)
    return n3.copySign(n1,n2);
  if("exp" == op)
    return n1.exp(&c);
  if("invert" == op)
    return n1.invert(&c);
  if("ln" == op)
    return n1.ln(&c);
  if("log10" == op)
    return n1.log10(&c);
  if("logb" == op)
    return n1.logB(&c);
  if("minus" == op)
    return n1.minus(&c);
  if("nextminus" == op)
    return n1.nextMinus(&c);
  if("nextplus" == op)
    return n1.nextPlus(&c);
  if("plus" == op)
    return n1.plus(&c);
  if("reduce" == op)
    return n1.reduce(&c);
  if("squareroot" == op)
    return n1.squareRoot(&c);
  if("tosci" == op) {
    rs = n1.toString().data();
    return n1;
  }
  if("toeng" == op) {
    rs = n1.toEngString().data();
    return n1;
  }
  if("tointegral" == op)
    return n1.toIntegralValue(&c);
  if("tointegralx" == op)
    return n1.toIntegralExact(&c);
  if("trim" == op)
    return n1.trim();
  
  //
  // Binary operations
  //
  if("add" == op)
    return n1.add(n2, &c);
  if("and" == op)
    return n1.digitAnd(n2, &c);
  if("compare" == op)
    return n1.compare(n2, &c);
  if("comparesig" == op)
    return n1.compareSignal(n2, &c);  
  if("comparetotal" == op)
    return n1.compareTotal(n2, &c);
  if("comparetotalmag" == op ||
     "comparetotmag" == op)
    return n1.compareTotalMag(n2, &c);
  if("divide" == op)
    return n1.divide(n2, &c);
  if("divideint" == op)
    return n1.divideInteger(n2, &c);
  if("max" == op)
    return n1.max(n2, &c);
  if("min" == op)
    return n1.min(n2, &c);
  if("maxmag" == op)
    return n1.maxMag(n2, &c);
  if("minmag" == op)
    return n1.minMag(n2, &c);
  if("multiply" == op)
    return n1.multiply(n2, &c);
  if("nexttoward" == op)
    return n1.nextToward(n2, &c);
  if("or" == op)
    return n1.digitOr(n2, &c);
  if("power" == op)
    return n1.power(n2, &c);
  if("quantize" == op)
    return n1.quantize(n2, &c);
  if("remainder" == op)
    return n1.remainder(n2, &c);
  if("remaindernear" == op)
    return n1.remainderNear(n2, &c);
  if("rescale" == op)
    return n1.rescale(n2, &c);
  if("rotate" == op)
    return n1.rotate(n2, &c);
  if("samequantum" == op) {
    if(n1.sameQuantum(n2)) return "1";
    else return "0";
  }
  if("scaleb" == op)
    return n1.scaleB(n2, &c);
  if("shift" == op)
    return n1.shift(n2, &c);
  if("subtract" == op)
    return n1.subtract(n2, &c);
  if("xor" == op)
    return n1.digitXor(n2, &c);

  //
  // Ternary operations
  //
  if("fma" == op)
    return n1.fma(n2, n3, &c);

  
  qWarning() << "Unrecognized operation: " << op << endl;
  return QDecNumber();
}


int QDecNumberTests::opTest(const QStringList& tokens)
{
  QString id = tokens.at(0);
  QString op = tokens.at(1).toLower();
  QString opd1 = tokens.at(2);
  QString opd2 = tokens.at(3);
  QString opd3 = tokens.at(4);
  QString res = tokens.at(5);
  QString cond = tokens.at(6);
  bool ret = false;
  
  QDecNumber n1,n2,n3,e;
  // Conversion Context - needs high precision
  QDecContext cc(DEC_INIT_DECIMAL128);
  // Operation Context
  QDecContext oc(DEC_INIT_DECIMAL128);
  QString rs; // Result String
  bool op_precision_needed = false;
  bool is_rs_used = false; // Is result string used?


  // Skip a testcase with # as any of the operands
  for(int i=2; i<=4; i++)
    if(QString('#')==tokens.at(i)) {
      qDebug() << "SKIP(operand#): " << tokens.join(",");
      return 0;
    }
  
  // Expected result will get maximum allowable precision
  cc.setEmax(QDecMaxExponent); 
  cc.setEmin(QDecMinExponent); 
  // Expected result should not be affected by current context
  if(res != "?") {
    ret = token2QDecNumber(res, cc, e); // Expected result
    qDebug() << "cc: " << cc;
  }
  cc.zeroStatus(); // Clear status flag for next operation
  
  // Apply current context to operands now
  if(op=="tosci" ||
     op=="toeng" ||
     op=="apply") {
    op_precision_needed = true;
    is_rs_used = true;
    res.remove(QChar('\''));
  }
  getDirectivesContext(cc, op_precision_needed);

  ret = token2QDecNumber(opd1, cc, n1);
  cc.zeroStatus(); // Clear status flag for next operation

  if(is_binary_op(op) ||
     is_ternary_op(op)) {
    ret = token2QDecNumber(opd2, cc, n2);
    cc.zeroStatus();
  }
  if(is_ternary_op(op)) {
    ret = token2QDecNumber(opd3, cc, n3);
    cc.zeroStatus();
  }

  // Get context directives including precision
  getDirectivesContext(oc, true);
  // Perform the operation, obtain the result
  QDecNumber r = op_do(op,n1,n2,n3,oc,rs);
  
  if(res=="?") { 
    ret = true;
    if(oc.status()) {
      qDebug() << "runTestCase ctx=" << oc.statusToString()
               << "flg=" << oc.statusFlags();
    }
  }
  else {
    if(op == "tosci" ||
       op == "toeng" ||
       op == "class" ) {
      ret = (0==res.trimmed().compare(rs));
      if(!ret)
        // If false check the result is identical
        // This is also acceptable as there might be more than
        // one representation of same number
        ret = r.compare(e, &oc).isZero();
    }
    else {
      ret = r.compare(e, &oc).isZero();
      if(r.isNaN() && e.isNaN()) ret = true;
    }
  }
  qDebug() << "oc: " << oc;
  if(ret) {
    qDebug() << "PASS: " << tokens.join(",");
    /* Uncomment to receive more information about passing test cases: */
    qDebug() << "n1=" << n1.toString().data()
             << "n2=" << n2.toString().data()
             << "r="
             << (is_rs_used ? rs.toLocal8Bit().data() : r.toString().data())
             << "e=" << e.toString().data()
             << "prc=" << oc.digits()
             << "ctx=" << (oc.status() ? oc.statusToString() : 0)
             << (is_rs_used ?  res + "|" + rs : (const char*)0);

    return 0; // Success
  }
  else {
    qDebug() << "FAIL: " << tokens.join(",");
    qDebug() << "n1=" << n1.toString().data()
             << "n2=" << n2.toString().data()
             << "n3=" << n3.toString().data()
             << "r="
             << (is_rs_used ? rs.toLocal8Bit().data() : r.toString().data())
             << "e=" << e.toString().data()
             << "prc=" << oc.digits()
             << "ctx=" << (oc.status() ? oc.statusToString() : 0)
             << (is_rs_used ?  res + "|" + rs : (const char*)0);

    // Print out operation context
    qDebug() << "oc: " << oc;
    // Print out prevailing context settings
    displayDirectivesContext();
    // Uncomment this if you want to stop the test cases after failure
    //qFatal("End");                    
    return 1; // Failure
  }
  
  return 0;
}

int QDecNumberTests::runTestCase(const QStringList& tokens, const QDecContext& /* ctx */)
{
  if(tokens.size() != 7) {
    qWarning() << "Invalid number of tokens: " << tokens.join(",");
    return 1; // Failure
  }

  QString op = tokens.at(1);
  
  if(is_unary_op(op) ||
     is_binary_op(op) ||
     is_ternary_op(op))
    return opTest(tokens);
  else
    qDebug() << "SKIP(unknown op): " << tokens.join(",");
  
  return 0;
}


bool QDecNumberTests::token2QDecNumber(const QString& token, const QDecContext& ctx, QDecNumber& num)
{
  QString tt = token;
  // Deal with quotes, double quotes and escaped quotes
  if(tt.contains("''")) {
    tt.replace("''","'");
    tt = tt.remove(0,1);
    tt.chop(1);
  }
  else
    tt.remove(QChar('\''));

  if(tt.contains("\"\"")) {
    tt.replace("\"\"","\"");
    tt = tt.remove(0,1);
    tt.chop(1);   
  }
  else
    tt.remove(QChar('\"'));

  if(token.contains('#')) {
    QRegExp expl("#([0-9a-fA-F]+)"); // explicit notation
    QRegExp altn("([0-9]+)#(.+)"); // alternative notation

    if(expl.exactMatch(token)) {
      QString hexval = expl.cap(1); // get hex value
      switch(hexval.size()) {
        case 8: {
          QDecSingle ds;
          ds.fromHexString(hexval.toLocal8Bit().data());
          num = ds.toQDecNumber();
          return true;
          }      
        case 16: {
          QDecDouble dd;
          dd.fromHexString(hexval.toLocal8Bit().data());
          num = dd.toQDecNumber();
          return true;
        }
        case 32: {
          QDecQuad dq;
          dq.fromHexString(hexval.toLocal8Bit().data());
          num = dq.toQDecNumber();
          return true;
        }
      } // end switch
    } // expl.

    if(altn.exactMatch(token)) {
      QString fmt = altn.cap(1); // get format size
      QString val = altn.cap(2); // get number value in string

      uint fmtsize = fmt.toUInt();
      switch(fmtsize) {
        case 32: {
          qDebug() << "fmt=" << fmt << "val=" << val;
          QDecSingle ds(val.toLocal8Bit().data());
          num = ds.toQDecNumber();
          return true;
        }

        case 64: {
          qDebug() << "fmt=" << fmt << "val=" << val;
          QDecDouble dd(val.toLocal8Bit().data());
          num = dd.toQDecNumber();
          return true;
        }

        case 128: {
          qDebug() << "fmt=" << fmt << "val=" << val;
          QDecQuad dq(val.toLocal8Bit().data());
          num = dq.toQDecNumber();
          return true;
        }

      } // end switch

    } // altn.

    
    // '#' in a token by itself
    num.fromString("NaN");
    return true;
      
  } // contains #
  
  //qDebug() << "ctx=" << ctx;
  QDecContext c(ctx);

  QDecNumber tnum;
  tnum.fromString(tt.toLocal8Bit().data(), &c);
  num = tnum;

  //TODO: Check if warning is necessary
  if(c.status()) {
    qDebug() << "token2QDecNumber "
             << "tkn=" << token
             << "ctx=" << c.statusToString()
             << c.statusFlags()
             << "val=" << tnum.toString();

    qDebug() << "c=" << c;

  }
  
  
  return true;
}

bool QDecNumberTests::QDecNumber2token(QString& token, const QDecNumber& num)
{
  QString numstr = num.toString();
  token = numstr;
  return true;
}

void QDecNumberTests::test_cases()
{

  // Initiase the set of test cases to be skipped
  m_skipSet << "pwsx805" << "powx4302" << "powx4303" << "powx4342"
            << "powx4343" << "lnx116" << "lnx732";
  // Invalid operations due to restrictions
  m_skipSet << "expx901" << "expx902" << "lnx901" << "lnx902"
            << "logx901" << "logx902" << "powx4001" << "powx4002";
  // Failures due to settings of clamp, could be ignored
  m_skipSet << "basx716" << "basx720" << "basx724" << "basx744";
  
  QString cwd = QDir::currentPath() ;
  // Assume test application is run from cwd
  QString prjdir = cwd + "/../../../../test/";
  QDir pdir(prjdir);
  // If not, assume it's called from project root directory
  if(!pdir.exists()) {
    prjdir = cwd + "/test/";
  }
      
  // Check if user specified a test case directory
  QString tdir = m_argsMap.value("testdir",
                                 //"tc_subset");
                                 "tc_full"); 
  tdir = prjdir + tdir;
  QString tfile = m_argsMap.value("testfile");
  QString tffilter = m_argsMap.value("testfilefilter");
  
  try {
    qDebug() << "Locating test data from directory " << tdir;
    QDir dir(tdir);
    if(!dir.exists()) {
      qWarning() << "Cannot find test directory" << tdir;
      return;
    }

    dir.setFilter(QDir::Files);
    qDebug() << "testfilefilter=" << tffilter.toLocal8Bit();      
    if(tffilter.size()) {
      QStringList filters;
      filters << tffilter;
      dir.setNameFilters(filters);
    }
    QStringList list = dir.entryList();
    qDebug() << "Found test files: " << list;

    QStringListIterator si(list);
    while(si.hasNext()) {
      QString f = si.next();
      qDebug() << f;
      procTestFile(tdir + "/" + f);
    }

    /*
    for(i = 0; i < list.size(); ++i) {
      // Skip test file is wanted and don't match with current file name
      if(tfile.size() && tfile!=list[i]) {
        qWarning() << "Skipping " << list[i].toLocal8Bit();
        continue;
      }
      qDebug() << list[i].toLocal8Bit();      
      procTestFile(tdir + "/" + list[i]);
    }
    */

    QVERIFY(1);
  }
  catch(const char* s) {
    qWarning() << "Ex " << s;
  }
  catch(const std::exception& e) {
    qWarning() << e.what();
  }
  catch(...) {
    qWarning() << "Unknown exception" ;
  }
}

