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

TO_NAMESPACE;

#include <stdio.h>

#include <qtextview.h>
#include <qpushbutton.h>
#include <qsizepolicy.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qprogressbar.h>

#include "toabout.h"

#include "toabout.moc"

#include "LICENSE.h"

#include "icons/largelogo.xpm"

static const char *AboutText=
"<IMG SRC=largelogo.xpm ALIGN=right><BR>
<H1>TOra</H1>Version %s
<P>
&copy; 2000 copyright of GlobeCom AB
(<A HREF=http://www.globecom.se>http://www.globecom.se/</a>).<P>
Written by Henrik Johnson.<P>
This program is available
under the GNU Public License.<P>
Uses the Qt library version 2.2 by TrollTech
(<A HREF=http://www.troll.no>http://www.troll.no/</A>).<P>

<HR BREAK=ALL>
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
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
<H3>Stay alert! Trust no one! Keep your laser handy!</H3>
<DIV ALIGN=RIGHT>Roleplaying game Paranoia
<DIV ALIGN=LEFT>
<H3>The best definition of a gentleman is a man who can play the accordion -- but doesn't.</H3>
<DIV ALIGN=RIGHT>Tom Crichton
<DIV ALIGN=LEFT>
<H3>The trouble with being punctual is that nobody's there to appreciate it.</H3>
<DIV ALIGN=RIGHT>Franklin P. Jones
<DIV ALIGN=LEFT>
<H3>I might disagree with what you have to say,
but I'll defend your right to say it to the death.</H3>
<DIV ALIGN=RIGHT>Voltaire
<DIV ALIGN=LEFT>
<H3>Life is a sexually transmittable desease with terminal outcome.</H3>
<DIV ALIGN=RIGHT>Unknown
<DIV ALIGN=LEFT>
<H3>If you don't know where you are going you are never lost.</H3>
<DIV ALIGN=RIGHT>Unknown
<DIV ALIGN=LEFT>
<H3>If you're right 90% of the time, why quibble about the remaining 3%?</H3>
<DIV ALIGN=RIGHT>Unknown
<DIV ALIGN=LEFT>
<H3>Hiroshima '45 Chernobyl '86 Windows '95</H3>
<DIV ALIGN=RIGHT>Unknown
<DIV ALIGN=LEFT>
<H3>Accepptance testing: An unsuccessfull attempt to find bugs.</H3>
<DIV ALIGN=RIGHT>Unknown
<DIV ALIGN=LEFT>
<H3>Jag spenderade mina pengar på sprit kvinnor och sång.<BR>
Resten slösade jag bort.</H3>
<DIV ALIGN=RIGHT>Unknown Swede
<DIV ALIGN=LEFT>
<H3>
<UL>
<LI>Make it idiot proof, and someone will make a better idiot.<P>
<LI>I don't suffer from insanity. I enjoy every minute of it.<P>
<LI>Press any key to continue or any other key to quit<P>
<LI>The box said \"Requires Windows 95 or better.\" So I installed LINUX<P>
<LI>&lt;-- Your information went that way --&gt;<P>
<LI>Go away, or I will replace you with a very small shell script.<P>
<LI>Do not meddle in the affairs of sysadmins, for they are suddle and quick to anger.
</UL>
</H3>
<DIV ALIGN=RIGHT>Think Geek (http://www.thinkgeek.com/)
<DIV ALIGN=LEFT>
<H3>
<UL>
<LI>Luck can't last a lifetime unless you die young!<P>
<LI>Misfortune: While good fortune often eludes you, this kind never misses.<P>
<LI>For every winner, there are dozens of losers. Odds are you are one of them.<P>
<LI>Failure: When your best just isn't good enough.<P>
<LI>Quitters never win, winners never quit, but those who never win and never quit are idiots.<P>
<LI>If you can't learn to do something well, learn to enjoy doing it poorly.<P>
<LI>If at first you don't succeed, failure may be your style.<P>
<LI>There are no stupid questions, but there are a lot of inquisitive idiots.<P>
<LI>There is no joy greater than soaring high on the wings of your dreams,
	except mayby the joy of watching a dreamer who has nowhere to land but in the ocean of reality.<P>
<LI>The only consistent feature of all of your dissatisfying relationships is you.<P>
<LI>When you earnestly believe you can compensate for a lack of skill by doubling your efforts,
	there's no end to what you can't do.<P>
<LI>Never underestimate the power of stupid people in large groups.<P>
<LI>The downside of being better than everyone else is that people tend to assume you're pretentious.<P>
<LI>In the battle between you and the world, bet on the world.<P>
<LI>You'll always miss 100% of the shots you don't take, and, statistically speaking, 99% of the shots you do.<P>
<LI>The secret to success is knowing who to blame for your failures.<P>
<LI>When birds fly in the right formation, they need only exert half the effort.
	Even in nature, teamwork results in collective laziness.<P>
<LI>It's amazing how much easier it is for a team to work together when no one has any idea where they're going.<P>
<LI>Attitudes are contagious. Mine might kill you.<P>
<LI>When people are free to do as they please, they usually imitate each other.<P>
<LI>If we don't take care of the customer, mayby they'll stop bugging us.<P>
<LI>It takes months to find a customer, but only seconds to lose one.
	The good news is we should run out of them in no time.<P>
<LI>Mediocrity: It takes a lot less time, and most people won't notice the difference until it's too late.<P>
<LI>Laziness: Success is a jurney, not a destination. So stop running.<P>
<LI>Hard work often pays off after time, but laziness always pays off now.<P>
<LI>The tallest blade of grass is the first to be cut by the lawnmower.
</UL>
</H3>
<DIV ALIGN=RIGHT>Despair INC (http://www.despair.com/)
<DIV ALIGN=LEFT>";


#define ABOUT_CAPTION "TOra %s"

static QPixmap *toLogoPixmap=NULL;

void toAllocLogo(void)
{
  if (!toLogoPixmap) {
    toLogoPixmap=new QPixmap((const char **)largelogo_xpm);
    QMimeSourceFactory::defaultFactory()->setPixmap("largelogo.xpm",*toLogoPixmap);
  }
}

toSplash::toSplash(QWidget *parent=0,const char *name=0,WFlags f=0,bool allowLines=TRUE)
  : QVBox(parent,name,f,allowLines)
{
  toAllocLogo();

  setBackgroundColor(white);
  QLabel *logo=new QLabel(this,"Logo");
  logo->setBackgroundColor(white);
  logo->setPixmap(*toLogoPixmap);
  Label=new QLabel("Loading plugins",this);
  Label->setBackgroundColor(white);
  Progress=new QProgressBar(this,"Progress");

  QWidget *d=QApplication::desktop();
  move((d->width()-width())/2,(d->height()-height())/2);
}

toAbout::toAbout(QWidget* parent=0,const char* name=0,bool modal=false,WFlags fl=0)
  : QDialog(parent,name,modal,fl)
{
  toAllocLogo();

  TextView=new QTextView(this);
  TextView->setTextFormat(RichText);
  TextView->setPaper(QBrush(QColor(0xFA,0xFA,0xFE)));
  resize(640,340);
  setMinimumSize(640,340);
  setMaximumSize(640,340);
  TextView->setGeometry(QRect(10,10,620,280));
  TextView->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
  
  ChangeButton=new QPushButton(this,"ChangeButton");
  ChangeButton->setGeometry(QRect(144,300,104,32)); 

  QPushButton *OkButton=new QPushButton(this,"OkButton");
  OkButton->setGeometry(QRect(392,300,104,32)); 
  OkButton->setText(tr("&Ok"));
  OkButton->setDefault(true);

  if (!parent) {
    Page=1;
    changeView();
    QPushButton *CancelButton=new QPushButton(this,"CanceButton");
    CancelButton->setGeometry(QRect(506,300,104,32)); 
    CancelButton->setText(tr("&Cancel"));
    connect(CancelButton,SIGNAL(clicked()),SLOT(reject()));
  } else {
    char buffer[10000];
    sprintf(buffer,ABOUT_CAPTION,TOVERSION);
    setCaption(buffer);
    sprintf(buffer,AboutText,TOVERSION);
    TextView->setText(buffer);
    ChangeButton->setText(tr("&Quotes"));
    Page=0;
  }

  connect(ChangeButton,SIGNAL(clicked()),SLOT(changeView()));
  connect(OkButton,SIGNAL(clicked()),SLOT(accept()));
}

void toAbout::changeView(void)
{
  switch (Page) {
  case 0:
    ChangeButton->setText(tr("&License"));
    TextView->setText(QuoteText);
    setCaption("Favourite Quotes");
    Page++;
    break;
  case 1:
    ChangeButton->setText(tr("&About"));
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
    ChangeButton->setText(tr("&Quotes"));
    Page=0;
    break;
  }
}
