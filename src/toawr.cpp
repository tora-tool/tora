
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include <iostream>

#include <list>
#include <sstream>
#include "tologger.h"


#include <QToolBar>
#include <QLabel>
//#include <qtoolbutton.h>
#include <QLineEdit>
#include <QDateEdit>
#include <QMessageBox> //TODO see utis.h define TOMessageBox QMessageBox(QMessageBox should be included there)
#include <toresultcombo.h>

#include "utils.h"
#include "totool.h"
#include "toawr.h"
#include "toresultview.h"
#include "toparamget.h"
#include "tochangeconnection.h"
 
//#include "tosimplequery.moc"
 
#include "icons/execute.xpm"
#include "icons/awrtool.xpm"

// -- Instances in this Workload Repository schema
// => instnum
// => dbid
static toSQL SQLDBInstances("toAWR:DBInstances",
		 "select distinct \n"
		 " (case when cd.dbid = wr.dbid and                              \n"
		 "                   cd.name = wr.db_name and                    \n"
		 "                   ci.instance_number = wr.instance_number and \n"
		 "                   ci.instance_name   = wr.instance_name   and \n"
		 "                   ci.host_name       = wr.host_name           \n"
		 "              then '* '||wr.dbid||':'||wr.instance_name        \n"
		 " else '  '||wr.dbid||':'||wr.instance_name \n"
		 "	end) disp_dbdbid    \n"
		 "   , wr.dbid   dbbid  \n"
		 "   , wr.instance_number instt_num    \n"
		 "   , wr.db_name         dbb_name     \n"
		 "   , wr.instance_name   instt_name   \n"
		 "   , wr.host_name       host         \n"
		 "  from dba_hist_database_instance wr, v$database cd, v$instance ci"
		 "  order by 1 desc",
//		 "  order by wr.startup_time desc",
		 "Instances in this Workload Repository schema");


// --
// --  Error reporting
// --  Plus query max_snap_time

// whenever sqlerror exit;
// variable max_snap_time char(10);
// declare
//   cursor cidnum is
//      select 'X'
//        from dba_hist_database_instance
//       where instance_number = :instnum
//         and dbid            = :dbid;
//   cursor csnapid is
//      select to_char(max(end_interval_time),'dd/mm/yyyy')
//        from dba_hist_snapshot
//       where instance_number = :instnum
//         and dbid            = :dbid;
//   vx     char(1);
// begin
//   -- Check Database Id/Instance Number is a valid pair
//   open cidnum;
//   fetch cidnum into vx;
//   if cidnum%notfound then
//     raise_application_error(-20200,
//       'Database/Instance ' || :dbid || '/' || :instnum ||
//       ' does not exist in DBA_HIST_DATABASE_INSTANCE');
//   end if;
//   close cidnum;
//   -- Check Snapshots exist for Database Id/Instance Number
//   open csnapid;
//   fetch csnapid into :max_snap_time;
//   if csnapid%notfound then
//     raise_application_error(-20200,
//       'No snapshots exist for Database/Instance '||:dbid||'/'||:instnum);
//   end if;
//   close csnapid;
// end;
// /
// whenever sqlerror continue;

// tohle asi muzu preskocit
// - validni kombinace <instnum,dbid> je uz znama
// - <snapid,end_interval_time> nactu do komboboxu (anebo tam bude "No snaps")


// -- List available snapshots
static toSQL SQLSnaps("toAWR:Snaps",
		" select to_char(s.end_interval_time,'HH24:mi') snapdat                  \n"
		" , di.instance_name                                  inst_name          \n"
		" , di.db_name                                        db_name            \n"
		" , s.snap_id                                         snap_id            \n"
		" , s.snap_level                                      lvl                \n"
		" , to_char(s.startup_time,'dd Mon \"at\" HH24:mi:ss')  instart_fmt      \n"
		"from dba_hist_snapshot s                      \n"
		" , dba_hist_database_instance di              \n"
		"where s.dbid              = :db_id<char[40],in>    \n"
		"  and di.dbid             = s.dbid                    \n"
		"  and s.instance_number   = :instnum<char[40],in>  \n"
		"  and di.instance_number  = s.instance_number         \n"
		"  and di.dbid             = s.dbid            \n"
		"  and di.instance_number  = s.instance_number \n"
		"  and di.startup_time     = s.startup_time    \n"
		"  and s.end_interval_time between trunc(to_date(:sdate<char[11],in>, 'YYYY:MM:DD'))    \n"
		      "	and trunc(to_date(:edate<char[11],in>, 'YYYY:MM:DD')+1)                         \n"
//		"  and s.end_interval_time >= decode( &num_days "
//		"                                  , 0   , to_date('31-JAN-9999','DD-MON-YYYY') "
//		"                                  , 3.14, s.end_interval_time "
//		"                                  , to_date(:max_snap_time,'dd/mm/yyyy') - (&num_days-1)) "
		" order by db_name, instance_name, snap_id     \n"
		,
		"List availanble snapshots");


// --
// --  Error reporting
// whenever sqlerror exit;
static toSQL SQLCheckSnaps("toAWR:CheckSnaps",
		" declare \n"
		"   cursor cspid(vspid dba_hist_snapshot.snap_id%type) is \n"
		"      select end_interval_time                           \n"
		"           , startup_time                                \n"
		"        from dba_hist_snapshot                           \n"
		"       where snap_id         = vspid                     \n"
		"         and dbid            = :dbid<char[40],in>     \n"
		"         and instance_number = :inst<char[40],in>;    \n"
		"   bsnapt  dba_hist_snapshot.end_interval_time%type;     \n"
		"   bstart  dba_hist_snapshot.startup_time%type;          \n"
		"   esnapt  dba_hist_snapshot.end_interval_time%type;     \n"
		"   estart  dba_hist_snapshot.startup_time%type;          \n"
		" begin                                                   \n"
		"   -- Check Begin Snapshot id is valid, get corresponding instance startup time \n"
		"   open cspid(:fsnap<char[40],in>);                   \n"
		"   fetch cspid into bsnapt, bstart;                      \n"
		"   if cspid%notfound then                                \n"
		"     raise_application_error(-20200,                     \n"
		"       'Begin Snapshot Id '||:fsnap<char[40],in>||' does not exist for this database/instance'); \n"
		"   end if;                                               \n"
		"   close cspid;                                          \n"
		"   -- Check End Snapshot id is valid and get corresponding instance startup time \n"
		"   open cspid(:tsnap<char[40],in>);                   \n"
		"   fetch cspid into esnapt, estart;                      \n"
		"   if cspid%notfound then                                \n"
		"     raise_application_error(-20200,                     \n"
		"       'End Snapshot Id '||:tsnap<char[40],in>||' does not exist for this database/instance'); \n"
		"   end if;                                               \n"
		"   if esnapt <= bsnapt then                              \n"
		"     raise_application_error(-20200,                     \n"
		"       'End Snapshot Id '||:tsnap<char[40],in>||' must be greater than Begin Snapshot Id '||:fsnap<char[40],in>); \n"
		"   end if;                                               \n"
		"   close cspid;                                          \n"
		"   -- Check startup time is same for begin and end snapshot ids \n"
		"   if ( bstart != estart) then                           \n"
		"     raise_application_error(-20200,                     \n"
		"       'The instance was shutdown between snapshots '||:fsnap<char[40],in>||' and '||:tsnap<char[40],in>); \n"
		"   end if;                                               \n"
		" end; \n",
		"Check if the pair of snap ids is valid" );


class toAWRTool : public toTool
{
protected:
	std::map<toConnection *, QWidget *> Windows;
	
	virtual const char **pictureXPM(void);
public:
	toAWRTool()
		: toTool(10003,"AWR")
	{ }
	virtual const char *menuItem()
	{
		return "AWR Report";
	}

	virtual bool canHandle(toConnection &conn)
	{
		return toIsOracle(conn);
	}

	virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
	{
		std::map<toConnection *, QWidget *>::iterator i = Windows.find(&connection);
		if (i != Windows.end())
		{
			(*i).second->raise();
			(*i).second->setFocus();
			return NULL;
		}
		else
		{
			QWidget *window = new toAWR(parent, connection);
			Windows[&connection] = window;
			return window;
		}		
	}
	virtual void closeWindow(toConnection &connection)
	{
		std::map<toConnection *, QWidget *>::iterator i = Windows.find(&connection);
		if (i != Windows.end())
		{
			std::cerr << "Window deleted:" << std::endl;
			Windows.erase(i);
		}
	}
};

const char **toAWRTool::pictureXPM(void) { return const_cast<const char**>(awrtool_xpm); }


static toAWRTool AWRTool;
 
toAWR::toAWR(/*toTool *tool,*/ QWidget *parent, toConnection &_connection)
	: toToolWidget(/* *tool*/AWRTool, "simplequery.html", parent, _connection, "toAWR")
{
	QToolBar *toolbar=toAllocBar(this, tr("Simple Query"));
	layout()->addWidget(toolbar);
	
	toolbar->addWidget(new QLabel("Inst:", toolbar));
	dbid = new toResultCombo(toolbar, "AWR toolbar");
	fsnap = new toResultCombo(toolbar, "AWR toolbar");	fsnap->setSelectionPolicy(toResultCombo::LastButOne);
	tsnap = new toResultCombo(toolbar, "AWR toolbar");	tsnap->setSelectionPolicy(toResultCombo::Last);

	startdate = new QDateTimeEdit(QDate::currentDate());	startdate->setCalendarPopup(true);
	enddate = new QDateTimeEdit(QDate::currentDate());	enddate->setCalendarPopup(true);

	connect(dbid, SIGNAL(activated(int)), this, SLOT(instanceChanged(int)));
	connect(dbid, SIGNAL(done()), this, SLOT(instanceRead()));
	connect(startdate, SIGNAL(dateChanged(QDate)), this, SLOT(startDateChanged(QDate)));
	connect(enddate, SIGNAL(dateChanged(QDate)), this, SLOT(endDateChanged(QDate)));
	
	toolbar->addWidget(dbid);
	toolbar->addWidget(startdate);
	toolbar->addWidget(fsnap);
	toolbar->addWidget(enddate);
	toolbar->addWidget(tsnap);

	try
	{
		dbid->query(toSQL::sql("toAWR:DBInstances", connection()));
	}
	TOCATCH;

	toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(execute_xpm))),
			   tr("Generate report"),
			   this,
			   SLOT(execute()));
	toolbar->addWidget(new toSpacer());
	
	QAction *executeAct = new QAction(QPixmap(execute_xpm), tr("Execute_ current statement"), this);
        executeAct->setShortcut(QKeySequence::Refresh);
	connect(executeAct, SIGNAL(triggered()), this, SLOT(refresh(void)));

	new toChangeConnection(toolbar);
 
	Tabs = new QTabWidget(this);
	layout()->addWidget(Tabs);	
}
 
void toAWR::execute(void)
{
	try {
		//QString sql=Statement->text();
		//toQList params=toParamGet::getParam(this,sql);
		//Result->query(sql,params);
		//Result->query(sql,toQList());

		QVariant vdbid = dbid->itemData(dbid->currentIndex());
		QString dbids( vdbid.toStringList().at(0) );
		QString insts( vdbid.toStringList().at(1) );

		QVariant vf = fsnap->itemData(fsnap->currentIndex());
		QVariant vt = tsnap->itemData(tsnap->currentIndex());
		QString fsnaps( vf.toStringList().at(2) );
		QString tsnaps( vt.toStringList().at(2) );
//		toQList params;
//		params.push_back(fsnaps);
//		params.push_back(tsnaps);
		TLOG(0,toDecorator,__HERE__)
				<< "Dbid:" << dbids.toAscii().constData() << ' '
				<< "Inst:" << insts.toAscii().constData() << ' '
				<< "fsnap:" << fsnaps.toAscii().constData() << ' '
				<< "tsnap:" << tsnaps.toAscii().constData() << std::endl;

		try {
			toQuery checkSnaps(connection(), SQLCheckSnaps, dbids, insts, fsnaps, tsnaps);

			toQuery report(connection(),
					"select NVL(output,' ') \n"
					//"from table(dbms_workload_repository.awr_report_text( \n"
					"from table(dbms_workload_repository.awr_report_html( \n"
					"               :dbid<char[40],in>, \n"
					"               :inst<char[40],in>, \n"
					"               :f<char[40],in>,    \n"
					"               :t<char[40],in>, 0))",
					dbids, insts, fsnaps, tsnaps);

			QString reports;
			while (!report.eof())
			{
				QString line = report.readValue();
				reports += line;
				reports += '\n';
			}
			std::cerr << reports.toAscii().constData() << std::endl;

			QWidget *box = new QWidget(Tabs);
			QVBoxLayout *vbox = new QVBoxLayout;
			vbox->setSpacing(0);
			vbox->setContentsMargins(0, 0, 0, 0);
			box->setLayout(vbox);

			vbox->addWidget(new QLabel(tr("AWR report"), box));
			QTextBrowser *tb = new QTextBrowser(box);
			tb->setFontFamily("monospace");
			tb->setReadOnly(true);
			tb->setText(reports);
			//tb->setFontFamily("Courier");

			vbox->addWidget(tb);
			Tabs->addTab(box, tr("AWR Report"));

		} catch (const toConnection::exception &t ) {
			TOMessageBox::information(this, t, t);
		}

	} TOCATCH
}
 
toAWR::~toAWR()
{
	delete dbid;
	delete fsnap;
	delete tsnap;
	delete startdate;
	delete enddate;
}

void toAWR::closeEvent(QCloseEvent *event)
{
    try
    {
        AWRTool.closeWindow(connection());
    }
    TOCATCH;

    event->accept();
}

void toAWR::startDateChanged(QDate date)
{
	std::cerr << "startDateChanged:" << date.toString() << std::endl;

	int pos = dbid->currentIndex();
	QVariant d = dbid->itemData(pos);
	QStringList l = d.toStringList();
	std::stringstream s;
	for(QList<QString>::iterator i=l.begin(); i!=l.end(); ++i)
	{
		s << ":\'" << (*i).toAscii().constData() << '\'';
	}
	TLOG(0,toDecorator,__HERE__) << "start date changed:" << date.toString("YYYY:MM:DD") << std::endl;
	QString dbid = l.at(0);
	QString inst = l.at(1);
	try
	{
		toQList params;
		params.push_back(dbid);
		params.push_back(inst);
		params.push_back(date.toString("yyyy:MM:dd"));
		params.push_back(date.toString("yyyy:MM:dd"));
		fsnap->setSelectionPolicy(toResultCombo::None);
		fsnap->query(toSQL::sql("toAWR:Snaps", connection()), const_cast<const toQList&>(params ) );
		fsnap->refresh();
	}
	TOCATCH;
}

void toAWR::endDateChanged(QDate date)
{
	std::cerr << "endDateChanged:" << date.toString() << std::endl;

	int pos = dbid->currentIndex();
	QVariant d = dbid->itemData(pos);
	QStringList l = d.toStringList();
	std::stringstream s;
	for(QList<QString>::iterator i=l.begin(); i!=l.end(); ++i)
	{
		s << ":\'" << (*i).toAscii().constData() << '\'';
	}
	TLOG(0,toDecorator,__HERE__) << "end date changed:" << date.toString("YYYY:MM:DD") << std::endl;
	QString dbid = l.at(0);
	QString inst = l.at(1);
	try
	{
		toQList params;
		params.push_back(dbid);
		params.push_back(inst);
		params.push_back(date.toString("yyyy:MM:dd"));
		params.push_back(date.toString("yyyy:MM:dd"));
		fsnap->setSelectionPolicy(toResultCombo::None);
		tsnap->query(toSQL::sql("toAWR:Snaps", connection()), const_cast<const toQList&>(params ) );
		tsnap->refresh();
	}
	TOCATCH;

}

void toAWR::instanceChanged(int pos)
{
	QVariant d = dbid->itemData(pos);
	QStringList l = d.toStringList();
	std::stringstream s;
	for(QList<QString>::iterator i=l.begin(); i!=l.end(); ++i)
	{
		s << ":\'" << (*i).toAscii().constData() << '\'';
	}
	TLOG(0,toDecorator,__HERE__) << "Instance changed:" << pos << s.str() << std::endl;
	QString dbid = l.at(0);
	QString inst = l.at(1);
	try
	{
		toQList sparams, eparams;
		sparams.push_back(dbid);
		sparams.push_back(inst);
		sparams.push_back(startdate->date().toString("yyyy:MM:dd"));
		sparams.push_back(startdate->date().toString("yyyy:MM:dd"));
		TLOG(1,toDecorator,__HERE__) << "FSNAP: ("  << sparams.size() << ")" << std::endl;
		fsnap->query(toSQL::sql("toAWR:Snaps", connection()), const_cast<const toQList&>(sparams ) );
		fsnap->refresh();
		eparams.push_back(dbid);
		eparams.push_back(inst);
		eparams.push_back(enddate->date().toString("yyyy:MM:dd"));
		eparams.push_back(enddate->date().toString("yyyy:MM:dd"));
		TLOG(1,toDecorator,__HERE__) << "TSNAP: ("  << eparams.size() << ")" << std::endl;
		tsnap->query(toSQL::sql("toAWR:Snaps", connection()), const_cast<const toQList&>(eparams ) );
		tsnap->refresh();
	}
	TOCATCH;
};

void toAWR::instanceRead(void)
{
	dbid->setCurrentIndex(0);
};
