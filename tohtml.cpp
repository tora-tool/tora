#include <string.h>
#include <ctype.h>

#include "tohtml.h"

toHtml::toHtml(const char *file,size_t siz)
{
  Data=QCString(file,siz);
  Position=0;
}

toHtml::~toHtml()
{
  delete Data;
}

void toHtml::skipSpace(void)
{
  while(Position<Data.length()&&isspace(Data[Position]))
    Position++;
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
      while(Position<Data.length()&&!isspace(Data[Position])&&Data[Position]!='>')
	Position++;
      ret.Tag=QString::fromLocal8Bit(Data.mid(start,Position-start)).lower();
    }
    for(;;) {
      skipSpace();
      if (Position>=Data.length()||Data[Position]=='>') {
	Position++;
	break;
      }

      QString nam;
      QString val;
      {
	size_t start=Position;
	while(Position<Data.length()&&!isspace(Data[Position])&&Data[Position]!='=')
	  Position++;
	nam=QString::fromLocal8Bit(Data.mid(start,Position-start)).lower();
      }
      skipSpace();
      if (Position<Data.length()&&Data[Position]=='=') {
	Position++;
	skipSpace();
	if (Position<Data.length()&&(Data[Position]=='\''||Data[Position]=='\"')) {
	  char c=Data[Position];
	  Position++;
	  size_t start=Position;
	  while(Position<Data.length()&&Data[Position]!=c)
	    Position++;
	  val=QString::fromLocal8Bit(Data.mid(start,Position-start));
	  Position++;
	} else {
	  size_t start=Position;
	  while(Position<Data.length()&&!isspace(Data[Position])&&Data[Position]!='>')
	    Position++;
	  val=QString::fromLocal8Bit(Data.mid(start,Position-start));
	}
      }
      ret.Qualifiers[nam]=val;
    }
  } else {
    size_t start=Position;
    while(Position<Data.length()) {
      if (Data[Position]=='<')
	break;
      Position++;
    }
    ret.Text=QString::fromLocal8Bit(Data.mid(start,Position-start));
  }
  return ret;
}

bool toHtml::search(const QString &str)
{
  QCString data(str.lower().local8Bit());
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
