#include <string.h>
#include <ctype.h>

#include "tohtml.h"

toHtml::toHtml(const QCString &data)
  : Data(data)
{
  Position=0;
}

toHtml::~toHtml()
{
}

void toHtml::skipSpace(void)
{
  while(isspace(data()))
    Position++;
}

bool toHtml::eof(void)
{
  if (Position>Data.length())
    throw QString("Invalidly went beyond end of file");
  return Position==Data.length();
}

char toHtml::data(size_t pos)
{
  if (pos>=Data.size())
    throw QString("Tried to access string out of bounds in data (%d)").arg(pos);
  return Data[pos];
}

toHtml::tag toHtml::nextTag(void)
{
  tag ret;
  ret.Open=false;
  if (eof())
    throw QString("Reading HTML after eof");
  if (Data[Position]=='<') {
    Position++;
    skipSpace();
    if (Data[Position]!='/')
      ret.Open=true;
    else
      Position++;
    skipSpace();
    {
      size_t start=Position;
      while(!isspace(data())&&data()!='>')
	Position++;
      ret.Tag=mid(start,Position-start).lower();
    }
    for(;;) {
      skipSpace();
      if (data()=='>') {
	Position++;
	break;
      }

      QString nam;
      QString val;
      {
	size_t start=Position;
	while(!isspace(data())&&data()!='=')
	  Position++;
	nam=mid(start,Position-start).lower();
      }
      skipSpace();
      if (data()=='=') {
	Position++;
	skipSpace();
	char c=data();
	if (c=='\''||c=='\"') {
	  Position++;
	  size_t start=Position;
	  while(data()!=c)
	    Position++;
	  val=mid(start,Position-start);
	  Position++;
	} else {
	  size_t start=Position;
	  while(!isspace(data())&&data()!='>')
	    Position++;
	  val=mid(start,Position-start);
	}
      }
      ret.Qualifiers[nam]=val;
    }
  } else {
    size_t start=Position;
    while(Position<Data.length()) {
      if (data()=='<')
	break;
      Position++;
    }
    QCString str=Data.mid(start,Position-start);
    for (size_t i=0;i<str.length();i++) {
      char c=str[i];
      if (c=='&') {
	size_t start=i+1;
	while(i<str.length()&&str[i]!=';')
	  i++;
	QCString tmp=str.mid(start,i-start);
	if (tmp[0]=='#') {
	  tmp=tmp.right(tmp.length()-1);
	  ret.Text+=QChar(char(tmp.toInt()));
	} else if (tmp=="auml")
	  ret.Text+="å";
	// The rest of the & codes...
      } else
	ret.Text+=QChar(c);
    }
  }
  return ret;
}

QCString toHtml::mid(size_t start,size_t size)
{
  if (size==0)
    return "";
  if (start>=Data.length())
    throw QString("Tried to access string out of bounds in mid (start=%1)").arg(start);
  if (size>Data.length())
    throw QString("Tried to access string out of bounds in mid (size=%1)").arg(size);
  if (start+size>Data.length())
    throw QString("Tried to access string out of bounds in mid (total=%1+%2>%3)").
      arg(start).
      arg(size).
      arg(Data.length());
  return Data.mid(start,size);
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
  for (size_t i=0;i<Data.length();i++) {
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
	    if (i+1>=Data.length()||!isalnum(Data[i+1]))
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
