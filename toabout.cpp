/****************************************************************************
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      As a special exception, you have permission to link this program
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 ****************************************************************************/



#include <stdio.h>

#include <qtextview.h>
#include <qpushbutton.h>
#include <qsizepolicy.h>
#include <qpixmap.h>

#include "toabout.h"

#include "toabout.moc"

#include "LICENSE.h"

#include "icons/largelogo.xpm"

static const char *AboutText=
"<IMG SRC=largelogo.xpm ALIGN=right>
<H1>TOra</H1>
Toolkit For Oracle %s
<P>
&copy; 2000 copyright of GlobeCom AB
(<A HREF=http://www.globecom.se>http://www.globecom.se/</a>).<P>
Written by Henrik Johnson.<P>
This program is available
under the GNU Public License.<P>
Uses the Qt library version 2.2 by TrollTech
(<A HREF=http://www.troll.no>http://www.troll.no/</A>).
";

static const char *QuoteText=
"<H3>People who think they know everything tend to irritate those of us who do.</H3>
<DIV ALIGN=RIGHT>Oscar Wilde<DIV ALIGN=LEFT>
<H3>My taste is simple, the best.</H3>
<DIV ALIGN=RIGHT>Oscar Wilde<DIV ALIGN=LEFT>

<H3>Working weeks come to its end, party time is here again.</H3>
<DIV ALIGN=RIGHT>Come with me, Depeche Mode<DIV ALIGN=LEFT>

<H3>I want to run<BR>I want to hide<BR>I want to tear down the walls<BR>
That hold me inside</H3>
<DIV ALIGN=RIGHT>Where the streets have no name, U2<DIV ALIGN=LEFT>

<H3>Got a letter from the government the other day.<BR>
Opened it and read it. It said they were suckers.</H3>
<DIV ALIGN=RIGHT>Black steel, Tricky<DIV ALIGN=LEFT>

<H3>It's a small world and it smells funny<BR>
I'd buy another if it wasn't for the money</H3>
<DIV ALIGN=RIGHT>Vision Thing, Sisters of Mercy<DIV ALIGN=LEFT>
<H3>I'm a 21:st century digital boy<BR>
I don't know how to live, but I got a lot of toys</H3>
<DIV ALIGN=RIGHT>21:st century digital boy, Bad Religion
<DIV ALIGN=left>
<H3>When will the world listen to reason?<BR>
When will the truth come into season?<BR>
I have a feeling it will be a long time.</H3>
<DIV ALIGN=RIGHT>It'll be a long time, Offspring
<DIV ALIGN=left>
<H3>Happiness isn't something you experience, it's something you remember.</H3>
<DIV ALIGN=RIGHT>Oscar Levant
<DIV ALIGN=LEFT>
<H3>I didn't believe in reincarnation in any of my other lives.<BR>
I don't see why I should have to believe in it in this one.</H3>
<DIV ALIGN=RIGHT>Strange de Jim
<DIV ALIGN=LEFT>
<H3>If a man has a strong faith he can indulge in the luxury of skepticism.</H3>
<DIV ALIGN=RIGHT>Friedrich Nietzsche
<DIV ALIGN=LEFT>
<H3>I'm a firm believer in the concept of a ruling class,<BR>
especially since I rule.</H3>
<DIV ALIGN=RIGHT>The movie \"Clerks\"
<DIV ALIGN=LEFT>
<H3>Life is a sexually transmittable desease with terminal outcome.</H3>
<DIV ALIGN=RIGHT>Unknown
<DIV ALIGN=LEFT>
<H3>If you don't know where you are going you are never lost.</H3>
<DIV ALIGN=RIGHT>Unknown
<DIV ALIGN=LEFT>
<H3>Jag spenderade mina pengar på sprit kvinnor och sång.<BR>
Resten slösade jag bort.</H3>
<DIV ALIGN=RIGHT>Unknown Swede
<DIV ALIGN=LEFT>";


#define ABOUT_CAPTION "TOra %s"

toAbout::toAbout(QWidget* parent=0,const char* name=0,bool modal=FALSE,WFlags fl=0)
  : QDialog(parent,name,modal,fl)
{
  QMimeSourceFactory::defaultFactory()->setPixmap("largelogo.xpm",
						  QPixmap((const char **)largelogo_xpm));

  TextView=new QTextView(this);
  TextView->setTextFormat(RichText);
  TextView->setPaper(QBrush(QColor(0xFA,0xFA,0xFE)));
  resize(640,340);
  setMinimumSize(640,340);
  setMaximumSize(640,340);
  TextView->setGeometry(QRect(10,10,620,280));
  TextView->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
  
  ChangeButton = new QPushButton( this, "ChangeButton" );
  ChangeButton->setGeometry( QRect( 144, 300, 104, 32 ) ); 

  QPushButton *OkButton = new QPushButton( this, "OkButton" );
  OkButton->setGeometry( QRect( 392, 300, 104, 32 ) ); 
  OkButton->setText( tr( "&Ok"  ) );
  OkButton->setDefault( TRUE );

  if (!parent) {
    Page=1;
    changeView();
    QPushButton *CancelButton = new QPushButton(this,"CanceButton");
    CancelButton->setGeometry( QRect( 506, 300, 104, 32 ) ); 
    CancelButton->setText( tr( "&Cancel"  ) );
    connect(CancelButton,SIGNAL(clicked()),SLOT(reject()));
  } else {
    char buffer[10000];
    sprintf(buffer,ABOUT_CAPTION,TOVERSION);
    setCaption(buffer);
    sprintf(buffer,AboutText,TOVERSION);
    TextView->setText(buffer);
    ChangeButton->setText( tr( "&Quotes"  ) );
    Page=0;
  }

  connect(ChangeButton,SIGNAL(clicked()),SLOT(changeView()));
  connect(OkButton,SIGNAL(clicked()),SLOT(accept()));
}

void toAbout::changeView(void)
{
  switch (Page) {
  case 0:
    ChangeButton->setText( tr( "&License" ) );
    TextView->setText(QuoteText);
    setCaption("Favourite Quotes");
    Page++;
    break;
  case 1:
    ChangeButton->setText( tr( "&About" ) );
    TextView->setText(LicenseText);
    setCaption("License");
    Page++;
    break;
  default:
    char buffer[10000];
    sprintf(buffer,ABOUT_CAPTION,TOVERSION);
    setCaption(buffer);
    sprintf(buffer,AboutText,TOVERSION);
    TextView->setText(buffer);
    ChangeButton->setText( tr( "&Quotes" ) );
    Page=0;
    break;
  }
}
