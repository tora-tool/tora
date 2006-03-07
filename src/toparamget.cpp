/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "utils.h"

#include "toconf.h"
#include "tohelp.h"
#include "tomemoeditor.h"
#include "toparamget.h"
#include "totool.h"

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpalette.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qscrollview.h>
#include <qsizepolicy.h>

#include "toparamget.moc"

std::map<QString, std::list<QString> > toParamGet::DefaultCache;
std::map<QString, std::list<QString> > toParamGet::Cache;

toParamGet::toParamGet(QWidget *parent, const char *name)
        : QDialog(parent, name, true), toHelpContext(QString::fromLatin1("common.html#param"))
{
    toHelp::connectDialog(this);
    resize(500, 480);
    setCaption(tr("Define binding variables"));

    QGridLayout *layout = new QGridLayout(this, 3, 2);
    layout->setSpacing( 6 );
    layout->setMargin( 11 );

    View = new QScrollView(this);
    View->enableClipper(true);
    View->setGeometry(10, 10, 330, 480);
    View->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    layout->addMultiCellWidget(View, 0, 2, 0, 0);

    Container = new QGrid(4, View->viewport());
    View->addChild(Container, 5, 5);
    Container->setSpacing(10);
    Container->setFixedWidth(View->width() - 30);
    View->viewport()->setBackgroundColor(qApp->palette().active().background());

    QPushButton *OkButton = new QPushButton(this, "OkButton");
    OkButton->setText(tr("&Ok"));
    OkButton->setDefault(true);
    layout->addWidget(OkButton, 0, 1);

    QPushButton *CancelButton = new QPushButton(this, "CancelButton");
    CancelButton->setText(tr("Cancel"));
    CancelButton->setDefault(false);
    layout->addWidget(CancelButton, 1, 1);

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout->addItem(spacer, 2, 1);

    connect(OkButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

toQList toParamGet::getParam(toConnection &conn, QWidget *parent, QString &str, bool interactive)
{
    std::map<QString, bool> parameters;
    std::list<QString> names;
    toParamGet *widget = NULL;

    enum {
        afterName,
        inString,
        normal,
        comment,
        multiComment,
        name,
        specification,
        endInput
    } state;
    state = normal;

    QChar endString;
    QString fname;
    QString direction;
    QString res;

    QString def = QString::fromLatin1("<char[4000]>");

    int colon = 1;

    int num = 0;
    for (unsigned int i = 0;i < str.length() + 1;i++)
    {
        QChar c;
        QChar nc;
        QChar pc;

        c = nc = pc = '\n';

        if(i < str.length())
            c = str.at(i);

        if(i < str.length() - 1)
            nc = str.at(i + 1);

        if(i - 1 > 0)
            pc = str.at(i - 1);

        if (state == normal && c == '-' && nc == '-')
            state = comment;
        else if (state == normal && c == '/' && nc == '*')
            state = multiComment;
        else
        {
            switch (state)
            {
            case inString:
                if (c == endString)
                    state = normal;
                break;
            case comment:
                if (c == '\n')
                    state = normal;
                break;
            case multiComment:
                if (c == '*' && nc == '/')
                    state = normal;
                break;
            case normal:
                switch (c.latin1())
                {
                case '\'':
                case '\"':
                    endString = c;
                    state = inString;
                    break;
                case ':':
                    // ignore ::
                    // this is a type cast for postgres, not a parameter.
                    if(nc == ':' || pc == ':')
                        break;

                    if(nc != '=')
                        state = name;
                    direction = "";
                    fname = "";
                    break;
                case '?':
                    fname = QString::fromLatin1("f");
                    fname += QString::number(colon);
                    colon++;
                    res += QString::fromLatin1(":");
                    res += fname;
                    res += def.mid(0, def.length() - 1);
                    c = def.at(def.length() - 1);
                    break;
                }
                break;
            case name:
                if (c.isLetterOrNumber() || c == '_')
                {
                    fname += c;
                    break;
                }
                if (fname.isEmpty() && !toIsMySQL(conn))
                {
                    toStatusMessage(tr("Missing field name"));
                    throw tr("Missing field name");
                }
                state = afterName;
            case afterName:
                if (c == '<')
                    state = specification;
                else
                {
                    state = normal;
                    res += def;
                }
                break;
            case specification:
                if (c == ',')
                    state = endInput;
                else if (c == '>')
                    state = normal;
                break;
            case endInput:
                if (c == '>')
                    state = normal;
                else
                    direction += c;
                break;
            }
        }
        if (state == normal && !fname.isEmpty())
        {
#if 0
            fname.replace(QRegExp("_"), " ");
#endif

            if (direction.isEmpty() || direction == "in" || direction == "inout")
            {
                if (!parameters[fname])
                {
                    parameters[fname] = true;
                    if (!widget)
                        widget = new toParamGet(parent);
                    new QLabel(fname, widget->Container);
                    QComboBox *edit = new QComboBox(widget->Container, QString::number(num));
                    edit->setEditable(true);
                    QString defval;
                    std::map<QString, std::list<QString> >::iterator fnd = Cache.find(fname);
                    if (fnd != Cache.end())
                        for (std::list<QString>::iterator i = (*fnd).second.begin();i != (*fnd).second.end();i++)
                        {
                            if (edit->count() == 0)
                                defval = *i;
                            edit->insertItem(*i);
                        }

                    fnd = DefaultCache.find(fname);
                    if (fnd != DefaultCache.end())
                        for (std::list<QString>::iterator i = (*fnd).second.begin();i != (*fnd).second.end();i++)
                        {
                            if (edit->count() == 0)
                                defval = *i;
                            edit->insertItem(*i);
                        }

                    QCheckBox *box = new QCheckBox(tr("NULL"), widget->Container);
                    connect(box, SIGNAL(toggled(bool)), edit, SLOT(setDisabled(bool)));
                    if (edit->count() > 0)
                    {
                        if (defval.isNull())
                            box->setChecked(true);
                    }
                    toParamGetButton *btn = new toParamGetButton(num, widget->Container);
                    btn->setText(tr("Edit"));
                    btn->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
                    connect(btn, SIGNAL(clicked(int)), widget, SLOT(showMemo(int)));
                    connect(box, SIGNAL(toggled(bool)), btn, SLOT(setDisabled(bool)));
                    widget->Value.insert(widget->Value.end(), edit);
                    names.insert(names.end(), fname);
                    num++;
                }
            }
            fname = "";
        }
        if (i < str.length())
            res += c;
    }

    toQList ret;
    if (widget)
    {
        (*widget->Value.begin())->setFocus();
        if (!interactive || widget->exec())
        {
            std::list<QString>::iterator cn = names.begin();
            for (std::list<QComboBox *>::iterator i = widget->Value.begin();i != widget->Value.end();i++)
            {
                QComboBox *current = *i;
                QString val;
                if (current)
                {
                    if (current->isEnabled())
                        val = current->currentText();
                    else
                        val = QString::null;
                }
                if (cn != names.end())
                {
                    std::list<QString> &lst = Cache[*cn];
                    for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
                        if ((*i) == val)
                        {
                            lst.erase(i);
                            break;
                        }
                    lst.insert(lst.begin(), val);

                    std::map<QString, std::list<QString> >::iterator fnd = DefaultCache.find(*cn);
                    if (fnd != DefaultCache.find(*cn))
                        for (std::list<QString>::iterator i = (*fnd).second.begin();i != (*fnd).second.end();i++)
                            if ((*i) == val)
                            {
                                (*fnd).second.erase(i);
                                break;
                            }

                    cn++;
                }
                ret.insert(ret.end(), val);
            }
            delete widget;
        }
        else
        {
            delete widget;
            toStatusMessage(tr("Aborted execution"), false, false);
            throw tr("Aborted execution");
        }
    }
    str = res;
    return ret;
}

void toParamGet::setDefault(toConnection &, const QString &name, const QString &val)
{
    std::map<QString, std::list<QString> >::iterator fnd = Cache.find(name);
    if (fnd != Cache.end())
        for (std::list<QString>::iterator i = (*fnd).second.begin();i != (*fnd).second.end();i++)
            if (val == *i)
                return ;

    std::list<QString> &lst = DefaultCache[name];
    for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
        if ((*i) == val)
        {
            lst.erase(i);
            break;
        }
    lst.insert(lst.begin(), val);
}

void toParamGet::showMemo(int row)
{
    QObject *obj = child(QString::number(row));
    if (obj)
    {
        toMemoEditor *memo = new toMemoEditor(this,
                                              ((QComboBox *) obj)->currentText(),
                                              row,
                                              0,
                                              false,
                                              true);
        if (memo->exec())
            ((QComboBox *)obj)->setCurrentText(memo->text());
    }
}

void toParamGet::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent(e);
    Container->setFixedWidth(View->width() - 30);
}
