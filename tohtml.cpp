#include <string.h>
#include <ctype.h>

#include "tohtml.h"

toHtml::toHtml(const QCString &data)
{
  Data=qstrdup(data);
  Length=data.length();
  Position=0;
  LastChar=0;
}

toHtml::~toHtml()
{
  delete Data;
}

void toHtml::skipSpace(void)
{
  if (Position>=Length)
    return;
  char c=LastChar;
  if (!c)
    c=Data[Position];
  if (isspace(c)) {
    Position++;
    LastChar=0;
    while(Position<Length&&isspace(Data[Position]))
      Position++;
  }
}

bool toHtml::eof(void)
{
  if (Position>Length)
    throw QString("Invalidly went beyond end of file");
  return Position==Length;
}

void toHtml::nextTag(void)
{
  if (eof())
    throw QString("Reading HTML after eof");
  QualifierNum=0;
  char c=LastChar;
  if (!c)
    c=Data[Position];
  if (c=='<') {
    IsTag=true;
    Position++;
    LastChar=0;
    skipSpace();
    if (Position>=Length)
      throw QString("Lone < at end");
    if (Data[Position]!='/') {
      Open=true;
    } else {
      Open=false;
      Position++;
    }
    skipSpace();
    {
      size_t start=Position;
      while(Position<Length&&!isspace(Data[Position])&&Data[Position]!='>') {
	Data[Position]=tolower(Data[Position]);
	Position++;
      }
      Tag=mid(start,Position-start);
    }
    for(;;) {
      skipSpace();
      if (Position>=Length)
	throw QString("Unended tag at end");

      c=LastChar;
      if (!c)
	c=Data[Position];
      if (c=='>') {
	LastChar=0;
	Position++;
	break;
      }

      // Must always be an empty char here, so LastChar not needed to be checked.

      {
	size_t start=Position;
	
	while(Position<Length&&!isspace(Data[Position])&&Data[Position]!='=') {
	  Data[Position]=tolower(Data[Position]);
	  Position++;
	}
	Qualifiers[QualifierNum].Name=mid(start,Position-start);
      }
      skipSpace();
      if (Position>=Length)
	throw QString("Unended tag qualifier at end");
      c=LastChar;
      if (!c)
	c=Data[Position];
      if (c=='=') {
	LastChar=0;
	Position++;
	skipSpace();
	if (Position>=Length)
	  throw QString("Unended tag qualifier data at end");
	c=Data[Position];
	if (c=='\''||c=='\"') {
	  Position++;
	  size_t start=Position;
	  while(Data[Position]!=c) {
	    Position++;
	    if (Position>=Length)
	      throw QString("Unended quoted string at end");
	  }
	  Qualifiers[QualifierNum].Value=mid(start,Position-start);
	  Position++;
	  LastChar=0;
	} else {
	  size_t start=Position;
	  while(!isspace(Data[Position])&&Data[Position]!='>') {
	    Position++;
	    if (Position>=Length)
	      throw QString("Unended qualifier data at end");
	  }
	  Qualifiers[QualifierNum].Value=mid(start,Position-start);
	}
      }
      QualifierNum++;
      if (QualifierNum>=TO_HTML_MAX_QUAL)
	throw QString("Exceded qualifier max in toHtml");
    }
  } else {
    IsTag=false;
    size_t start=Position;
    Position++;
    LastChar=0;
    while(Position<Length) {
      if (Data[Position]=='<')
	break;
      Position++;
    }
    Text=mid(start,Position-start);
  }
}

const char *toHtml::value(const QCString &q)
{
  for (int i=0;i<QualifierNum;i++) {
    if (q==Qualifiers[i].Name)
      return Qualifiers[i].Value;
  }
  return NULL;
}

QCString toHtml::text()
{
  QCString ret;
  for (const char *cur=Text;*cur;cur++) {
    char c=*cur;
    if (c=='&') {
      const char *start=cur+1;
      while(*cur&&*cur!=';')
	cur++;
      QCString tmp(start,cur-start);
      if (tmp[0]=='#') {
	tmp=tmp.right(tmp.length()-1);
	ret+=QChar(char(tmp.toInt()));
      } else if (tmp=="auml")
	ret+="å";
      // The rest of the & codes...
    } else
      ret+=QChar(c);
  }
  return ret;
}

const char *toHtml::mid(size_t start,size_t size)
{
  if (size==0)
    return "";
  if (start>=Length)
    throw QString("Tried to access string out of bounds in mid (start=%1)").arg(start);
  if (size>Length)
    throw QString("Tried to access string out of bounds in mid (size=%1)").arg(size);
  if (start+size>Length)
    throw QString("Tried to access string out of bounds in mid (total=%1+%2>%3)").
      arg(start).
      arg(size).
      arg(Length);

  LastChar=Data[start+size];
  Data[start+size]=0;
  return Data+start;
}

bool toHtml::search(const QString &str)
{
  QCString data(str.lower().latin1());
  enum {
    beginning,
    inTag,
    inString,
    inWord
  } lastState,state=beginning;
  unsigned int pos=0;
  char endString;
  for (size_t i=0;i<Length;i++) {
    char c=tolower(Data[i]);
    if (c=='\''||c=='\"') {
      endString=c;
      state=inString;
    } else if (c=='<') {
      state=inTag;
    } else {
      switch (state) {
      case inString:
	if (c==endString)
	  state=lastState;
	break;
      case beginning:
	if (data[pos]!=c) {
	  pos=0;
	  state=inWord;
	} else {
	  pos++;
	  if (pos>=data.length()) {
	    if (i+1>=Length||!isalnum(Data[i+1]))
	      return true;
	    pos=0;
	  }
	  break;
	}
	// Intentionally no break here
      case inWord:
	if (!isalnum(c))
	  state=beginning;
	break;
      case inTag:
	if (c=='>')
	  state=beginning;
	break;
      }
    }
  }
  return false;
}
