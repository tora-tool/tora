//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
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
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include <stdio.h>

#include <map>

#include <qtextview.h>
#include <qpushbutton.h>
#include <qsizepolicy.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qprogressbar.h>

#include "toabout.h"
#include "toconf.h"
#include "totool.h"

#include "toabout.moc"
#include "toaboutui.moc"

#include "LICENSE.h"

#include "icons/largelogo.xpm"

#ifdef OAS
#include "oasabout.h"
#else
#  ifdef TO_LICENSE
static const char *AboutText="<IMG SRC=largelogo.xpm><BR>\n"
"Version %1 (<A HREF=http://www.globecom.se/tora>http://www.globecom.se/tora</A>)\n"
"<P>\n"
"&copy; 2000-2001,2001 copyright of Underscore AB\n"
"(<A HREF=http://www.underscore.se>http://www.underscore.se/</a>).<P>\n"
"Written by Henrik Johnson.<P>\n"
"Webhosting by GlobeCom AB (<A HREF=http://www.globecom.se>www.globecom.se</A>).<P>\n"
"This program is <B>NOT FREE SOFTWARE</B> although you are allowed to use it free of "
"charge outside work. You are also allowed to evaluate the software at your work for "
"<B>no more than 30 days</B> from the initial installation. See the license for more information.<P>\n"
"You can purchase a commercial license at<BR>\n"
"<A HREF=http://www.globecom.se/tora/register>http://www.globecom.se/tora/register</A><P>\n"
"Uses the Qt library by TrollTech\n"
"(<A HREF=http://www.troll.no>http://www.troll.no/</A>).<P>\n"
"\n"
"<HR BREAK=ALL>\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
#  else
static const char *AboutText="<IMG SRC=largelogo.xpm><BR>\n"
"Version %1 (<A HREF=http://www.globecom.se/tora>http://www.globecom.se/tora</A>)\n"
"<P>\n"
"&copy; 2000-2001,2001 copyright of Underscore AB\n"
"(<A HREF=http://www.underscore.se>http://www.underscore.se/</a>).<P>\n"
"Written by Henrik Johnson.<P>\n"
"Webhosting by GlobeCom AB (<A HREF=http://www.globecom.se>www.globecom.se</A>).<P>\n"
"<b>If you find this program usefull please have a look at my Amazon wish list at\n"
"<A HREF=http://www.globecom.se/tora/wish>"
"http://www.globecom.se/tora/wish</A> if you feel like showing your appreciation for this\n"
"program.</b><P>"
"This program is available\n"
"under the GNU General Public License.\n"
"Uses the Qt library by TrollTech\n"
"(<A HREF=http://www.troll.no>http://www.troll.no/</A>).<P>\n"
"\n"
"<HR BREAK=ALL>\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details."
#  endif
"<HR BREAK=ALL>\n"
"Contributors (In alphabetical order):<P>\n"
"<UL>\n"
"<LI><B>Alexander (Leo) Bergolth</B> - Access to Oracle 7 database.\n"
"<LI><B>Volker Götz</B> - Improved PostgreSQL support.\n"
"<LI><B>Oliver Jehle</B> - Object caching to disc and much more.\n"
"<LI><B>Sergei Kuchin</B> - Oracle Template Library.\n"
"<LI><B>Michael Meskes</B> - Debian maintainer (<A HREF=http://www.debian.org/>http://www.debian.org/</A>).\n"
"<LI><B>Selcek Ozturk</B> - Improved content editor.\n"
"<LI><B>Jeff Price</B> - Endless feature requests and more.\n"
"<LI><B>Stefan Rindeskär</B> - Improved PostgreSQL support.\n"
"<LI><B>Adam vonNieda</B> - Improvements to RMAN monitoring from OracleTool (<A HREF=http://www.oracletool.com>http://www.oracletool.com</A>)\n"
"<LI><B>Guillaume Moulard</B> - Log4PL/SQL documentation.\n"
"</UL>\n"
"Other contributors:<P>\n"
"<B>Robert Cheramy</B>,\n"
"<B>Ferdinand Gassauer</B>,\n"
"<B>Frits Hoogland</B>,\n"
"<B>Mihai Ibanescu</B>,\n"
"<B>Bo Johansson</B>,\n"
"<B>Olof Jönsson</B>,\n"
"<B>Norbert Kiesel</B>,\n"
"<B>Laramie Leavitt</B>,\n"
"<B>Peter Nyborg</B>,\n"
"<B>Frank Pavageau</B>,\n"
"<B>Jan Pruner</B>,\n"
"<B>Jonas Rathert</B>,\n"
"<B>Christopher R. Redinger</B>,\n"
"<B>Erik Selberg</B>,\n"
"<B>Dennis Selsky</B>,\n"
"<B>Roman Stepanov</B>,\n"
"<B>Joachim Zetzsche</B>,\n"
"<B>Massimiliano Ziccardi</B>...\n";
#endif

static const char *QuoteText="<H3>People who think they know everything tend to irritate those of us who do.</H3>\n"
"<DIV ALIGN=RIGHT>Oscar Wilde<DIV ALIGN=LEFT>\n"
"<H3>My taste is simple, the best.</H3>\n"
"<DIV ALIGN=RIGHT>Oscar Wilde<DIV ALIGN=LEFT>\n"
"<H3>Working weeks come to its end, party time is here again.</H3>\n"
"<DIV ALIGN=RIGHT>Come with me, Depeche Mode<DIV ALIGN=LEFT>\n"
"<H3>I want to run<BR>I want to hide<BR>I want to tear down the walls<BR>\n"
"That hold me inside</H3>\n"
"<DIV ALIGN=RIGHT>Where the streets have no name, U2<DIV ALIGN=LEFT>\n"
"<H3>Got a letter from the government the other day.<BR>\n"
"Opened it and read it. It said they were suckers.</H3>\n"
"<DIV ALIGN=RIGHT>Black Steel In The Hour Of Chaos, Public Enemy<DIV ALIGN=LEFT>\n"
"<H3>It's a small world and it smells funny<BR>\n"
"I'd buy another if it wasn't for the money</H3>\n"
"<DIV ALIGN=RIGHT>Vision Thing, Sisters of Mercy<DIV ALIGN=LEFT>\n"
"<H3>I'm a 21:st century digital boy<BR>\n"
"I don't know how to live, but I got a lot of toys</H3>\n"
"<DIV ALIGN=RIGHT>21:st century digital boy, Bad Religion\n"
"<DIV ALIGN=left>\n"
"<H3>When will the world listen to reason?<BR>\n"
"When will the truth come into season?<BR>\n"
"I have a feeling it will be a long time.</H3>\n"
"<DIV ALIGN=RIGHT>It'll be a long time, Offspring\n"
"<DIV ALIGN=left>\n"
"<H3>Happiness isn't something you experience, it's something you remember.</H3>\n"
"<DIV ALIGN=RIGHT>Oscar Levant\n"
"<DIV ALIGN=LEFT>\n"
"<H3>I didn't believe in reincarnation in any of my other lives.<BR>\n"
"I don't see why I should have to believe in it in this one.</H3>\n"
"<DIV ALIGN=RIGHT>Strange de Jim\n"
"<DIV ALIGN=LEFT>\n"
"<H3>If a man has a strong faith he can indulge in the luxury of skepticism.</H3>\n"
"<DIV ALIGN=RIGHT>Friedrich Nietzsche\n"
"<DIV ALIGN=LEFT>\n"
"<H3>I'm a firm believer in the concept of a ruling class,<BR>\n"
"especially since I rule.</H3>\n"
"<DIV ALIGN=RIGHT>The movie \"Clerks\"\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Stay alert! Trust no one! Keep your laser handy!</H3>\n"
"<DIV ALIGN=RIGHT>Roleplaying game Paranoia\n"
"<DIV ALIGN=LEFT>\n"
"<H3>The best definition of a gentleman is a man who can play the accordion -- but doesn't.</H3>\n"
"<DIV ALIGN=RIGHT>Tom Crichton\n"
"<DIV ALIGN=LEFT>\n"
"<H3>The trouble with being punctual is that nobody's there to appreciate it.</H3>\n"
"<DIV ALIGN=RIGHT>Franklin P. Jones\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Only two things are infinite, the universe and human stupidity, and I'm not sure about the former.</H3>\n"
"<DIV ALIGN=RIGHT>Albert Einstein\n"
"<DIV ALIGN=LEFT>\n"
"<H3>If there's no light at the end of the tunnel, get down there and light the darn thing yourself!</H3>\n"
"<DIV ALIGN=RIGHT>Lauri Watts\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Unix is the most user friendly system I know, the point is the it is really selective about who is indeed its friend.</H3>\n"
"<DIV ALIGN=RIGHT>Luigi Genoni\n"
"<DIV ALIGN=LEFT>\n"
"<H3>The difference between theory and practice, is that in theory, there is no difference between theory and practice.</H3>\n"
"<DIV ALIGN=RIGHT>Richard Moore\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Your mind is like a parachute. It works best when open.</H3>\n"
"<DIV ALIGN=RIGHT>Matthias Elter\n"
"<DIV ALIGN=LEFT>\n"
"<H3>I might disagree with what you have to say,\n"
"but I'll defend your right to say it to the death.</H3>\n"
"<DIV ALIGN=RIGHT>Voltaire\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Software is like sex, it is better when it is free.</H3>\n"
"<DIV ALIGN=RIGHT>Linus Torvalds\n"
"<DIV ALIGN=LEFT>\n"
"<H3>The 3 great virtues of a programmer:<BR>\n"
"Laziness, Impatience, and Hubris.</H3>\n"
"<DIV ALIGN=RIGHT>Larry Wall\n"
"<DIV ALIGN=LEFT>\n"
"<H3>It's the mature and adult thing to do!<BR>\n"
"How does that affect me?</H3>\n"
"<DIV ALIGN=RIGHT>Seinfeldt\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Men don't want children!<BR>\n"
"Any man who thinks or says he wants children is no longer a man<BR>\n"
"but a pussywhipped freak of nature.</H3>\n"
"<DIV ALIGN=RIGHT>Bill Hicks, Rant in E-Minor\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Dates are for having fun and people should use them to get to know each other.<BR>\n"
"Even boys have something to say if you listen long enough.</H3>\n"
"<DIV ALIGN=RIGHT>Lynette, age 8\n"
"<DIV ALIGN=LEFT>\n"
"<H3>CNNESPNABCBCVTNT, but mostly BS</H3>\n"
"<DIV ALIGN=RIGHT>Television, Disposable Heroes of Hiphoprasy\n"
"<DIV ALIGN=LEFT>\n"
"<H3>What did you do yesterday?<BR>\n"
"I did absolutely nothing and it was everything I thought it would be.</H3>\n"
"<DIV ALIGN=RIGHT>The movie \"Officespace\"\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Think about how stupid the average person is.<BR>Then realise that half of them are more stupid than that.</H3>\n"
"<DIV ALIGN=RIGHT>George Carlin\n"
"<DIV ALIGN=LEFT>\n"
"<H3>I drink to make other people interesting.</H3>\n"
"<DIV ALIGN=RIGHT>George Jean Nathan\n"
"<DIV ALIGN=LEFT>\n"
"<H3>When everybody is out to get you, paranoid just seems like a good idea.</H3>\n"
"<DIV ALIGN=RIGHT>Woody Allen\n"
"<DIV ALIGN=LEFT>\n"
"<H3>How to relate to women:<BR>\n"
"Treat them like compilers that take simple statements and turn them into\n"
"big productions.</H3>\n"
"<DIV ALIGN=RIGHT>Userfriendly\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Communism doesn't work because people like to own stuff.</H3>\n"
"<DIV ALIGN=RIGHT>Frank Zappa\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Sure there have been injuries and deaths in boxing,<BR>\n"
"but none of them serious.</H3>\n"
"<DIV ALIGN=RIGHT>Boxer Alan Minter\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Plans are useless, but planning is indispensible.</H3>\n"
"<DIV ALIGN=RIGHT>Dwight Eisenhower\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Life is a sexually transmittable desease with terminal outcome.</H3>\n"
"<DIV ALIGN=RIGHT>Unknown\n"
"<DIV ALIGN=LEFT>\n"
"<H3>I'm on a seafood diet.<P>\n"
"So am I. When I see food I eat it.</H3>\n"
"<DIV ALIGN=RIGHT>The Drew Carey Show\n"
"<DIV ALIGN=LEFT>\n"
"<H3>I'm bi-sexual. Whenever I want sex I have to buy it.</H3>\n"
"<DIV ALIGN=RIGHT>Splitting Heirs\n"
"<DIV ALIGN=LEFT>\n"
"<H3>What do you think of western civilisation?<BR>\n"
"I think it would be a good idea.</H3>\n"
"<DIV ALIGN=RIGHT>Muhatma Gandhi\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Failure is not an option, it comes bundled with the software.</H3>\n"
"<DIV ALIGN=RIGHT>Unknown\n"
"<DIV ALIGN=LEFT>\n"
"<H3>If you don't know where you are going you are never lost.</H3>\n"
"<DIV ALIGN=RIGHT>Unknown\n"
"<DIV ALIGN=LEFT>\n"
"<H3>If you're right 90% of the time, why quibble about the remaining 3%?</H3>\n"
"<DIV ALIGN=RIGHT>Unknown\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Hiroshima '45 Chernobyl '86 Windows '95</H3>\n"
"<DIV ALIGN=RIGHT>Unknown\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Acceptance testing: An unsuccessfull attempt to find bugs.</H3>\n"
"<DIV ALIGN=RIGHT>Unknown\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Great minds discuss ideas,<BR>\n"
"Average minds discuss events,<BR>\n"
"Small minds discuss people.</H3>\n"
"<DIV ALIGN=RIGHT>Unknown\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Light travels faster than sound.<BR>\n"
"That is why some people appear bright<BR>\n"
"until you hear them speak.</H3>\n"
"<DIV ALIGN=RIGHT>Unknown\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Jag spenderade mina pengar på sprit kvinnor och sång.<BR>\n"
"Resten slösade jag bort.</H3>\n"
"<H3>(Roughly: I spent my money on women, drinking and singing..<BR>\n"
"The rest I squandered).</H3>\n"
"<DIV ALIGN=RIGHT>Unknown Swede\n"
"<DIV ALIGN=LEFT>\n"
"<H3>A hen is an egg's way of making another egg.</H3>\n"
"<DIV ALIGN=RIGHT>Unknown\n"
"<DIV ALIGN=LEFT>\n"
"<H3>Those who live by the sword, get shot by those who don't.</H3>\n"
"<DIV ALIGN=RIGHT>Unknown\n"
"<DIV ALIGN=LEFT>\n"
"<H3>\n"
"<UL>\n"
"<LI>Make it idiot proof, and someone will make a better idiot.<P>\n"
"<LI>I don't suffer from insanity. I enjoy every minute of it.<P>\n"
"<LI>Press any key to continue or any other key to quit<P>\n"
"<LI>The box said \"Requires Windows 95 or better.\" So I installed LINUX<P>\n"
"<LI>&lt;-- Your information went that way --&gt;<P>\n"
"<LI>Go away, or I will replace you with a very small shell script.<P>\n"
"<LI>Do not meddle in the affairs of sysadmins, for they are suddle and quick to anger.\n"
"<LI>SELECT * FROM users WHERE clue > 0<BR>\n"
"0 rows returned\n"
"</UL>\n"
"</H3>\n"
"<DIV ALIGN=RIGHT>Think Geek (http://www.thinkgeek.com/)\n"
"<DIV ALIGN=LEFT>\n"
"<H3>\n"
"<UL>\n"
"<LI>Luck can't last a lifetime unless you die young!<P>\n"
"<LI>Misfortune: While good fortune often eludes you, this kind never misses.<P>\n"
"<LI>For every winner, there are dozens of losers. Odds are you are one of them.<P>\n"
"<LI>Failure: When your best just isn't good enough.<P>\n"
"<LI>Quitters never win, winners never quit, but those who never win and never quit are idiots.<P>\n"
"<LI>If you can't learn to do something well, learn to enjoy doing it poorly.<P>\n"
"<LI>If at first you don't succeed, failure may be your style.<P>\n"
"<LI>There are no stupid questions, but there are a lot of inquisitive idiots.<P>\n"
"<LI>There is no joy greater than soaring high on the wings of your dreams,\n"
"	except mayby the joy of watching a dreamer who has nowhere to land but in the ocean of reality.<P>\n"
"<LI>The only consistent feature of all of your dissatisfying relationships is you.<P>\n"
"<LI>When you earnestly believe you can compensate for a lack of skill by doubling your efforts,\n"
"	there's no end to what you can't do.<P>\n"
"<LI>Never underestimate the power of stupid people in large groups.<P>\n"
"<LI>The downside of being better than everyone else is that people tend to assume you're pretentious.<P>\n"
"<LI>In the battle between you and the world, bet on the world.<P>\n"
"<LI>You'll always miss 100% of the shots you don't take, and, statistically speaking, 99% of the shots you do.<P>\n"
"<LI>The secret to success is knowing who to blame for your failures.<P>\n"
"<LI>When birds fly in the right formation, they need only exert half the effort.\n"
"	Even in nature, teamwork results in collective laziness.<P>\n"
"<LI>It's amazing how much easier it is for a team to work together when no one has any idea where they're going.<P>\n"
"<LI>Attitudes are contagious. Mine might kill you.<P>\n"
"<LI>When people are free to do as they please, they usually imitate each other.<P>\n"
"<LI>If we don't take care of the customer, mayby they'll stop bugging us.<P>\n"
"<LI>It takes months to find a customer, but only seconds to lose one.\n"
"	The good news is we should run out of them in no time.<P>\n"
"<LI>Mediocrity: It takes a lot less time, and most people won't notice the difference until it's too late.<P>\n"
"<LI>Laziness: Success is a jurney, not a destination. So stop running.<P>\n"
"<LI>Hard work often pays off after time, but laziness always pays off now.<P>\n"
"<LI>The tallest blade of grass is the first to be cut by the lawnmower.\n"
"</UL>\n"
"</H3>\n"
"<DIV ALIGN=RIGHT>Despair INC (http://www.despair.com/)\n"
"<DIV ALIGN=LEFT>";

#define ABOUT_CAPTION TOAPPNAME " %1"

void toAllocLogo(void)
{
  static bool Alloced=false;
  if (!Alloced) {
    Alloced=true;
    QMimeSourceFactory::defaultFactory()->setPixmap(QString::fromLatin1("largelogo.xpm"),
						    QPixmap((const char **)largelogo_xpm));
  }
}

toSplash::toSplash(QWidget *parent,const char *name,WFlags f)
  : QVBox(parent,name,f)
{
  toAllocLogo();

  setBackgroundColor(white);
  QLabel *logo=new QLabel(this,"Logo");
  logo->setBackgroundColor(white);
  logo->setPixmap(QPixmap((const char **)largelogo_xpm));
  Label=new QLabel(tr("Loading plugins"),this);
  Label->setBackgroundColor(white);
  Progress=new QProgressBar(this,"Progress");

  QWidget *d=QApplication::desktop();
  move((d->width()-width())/2,(d->height()-height())/2);
}

toAbout::toAbout(int page,QWidget* parent,const char* name,bool modal,WFlags fl)
  : toAboutUI(parent,name,modal,fl)
{
  toAllocLogo();

  switch (page) {
  case 2:
    setCaption(tr("Quotes"));
    TextView->setText(tr(QuoteText));
    TextView->setPaper(QColor(255,255,255));
    TextView->setTextFormat(RichText);
    CancelButton->hide();
    break;
  case 1:
#ifdef TO_LICENSE
    setCaption(tr("End User License"));
#else
    setCaption(tr("GNU General Public License"));
#endif
    TextView->setText(tr(LicenseText));
    TextView->setPaper(QColor(255,255,255));
    TextView->setTextFormat(PlainText);
    if (parent)
      CancelButton->hide();
    break;
  default:
    setCaption(tr(ABOUT_CAPTION).arg(QString::fromLatin1(TOVERSION)));
    QString buffer=tr(AboutText).arg(QString::fromLatin1(TOVERSION));
    TextView->setText(buffer);
#ifdef OAS
    TextView->setPaper(QColor(255,255,255));
#else
    TextView->setPaper(QColor(227,184,54));
#endif
    TextView->setTextFormat(RichText);
    CancelButton->hide();
    break;
  }
}

const char *toAbout::aboutText(void)
{
  toAllocLogo();

  return AboutText;
}
