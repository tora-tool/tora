
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

#include "connection/toqmysqlsetting.h"
#include "connection/toqmysqlprovider.h"
#include "core/toconfiguration.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>

toQMySqlSetting::toQMySqlSetting(QWidget *parent) 
	: QWidget(parent)
	, toSettingTab("database.html#qsql")
	, OnlyForward(NULL)
	, CreateLabel(NULL)
	, BeforeCreateAction(NULL)
{		
	QVBoxLayout *vbox = new QVBoxLayout;
	this->setLayout(vbox);
	vbox->setSpacing(0);
	vbox->setContentsMargins(0, 0, 0, 0);

	QGroupBox *box = new QGroupBox(this);
	box->setSizePolicy(QSizePolicy::Expanding,
			   QSizePolicy::Expanding);
	vbox->addWidget(box);

	vbox = new QVBoxLayout;
	vbox->setSpacing(6);
	vbox->setContentsMargins(11, 11, 11, 11);

	OnlyForward = new QCheckBox(
		qApp->translate(
			"qSqlSetting",
			"Posibility to break MySQL queries (Can require more connections)"),
		box);
	OnlyForward->setChecked(toConfigurationSingle::Instance().onlyForward());
	vbox->addWidget(OnlyForward);

	CreateLabel = new QLabel(
		qApp->translate("qSqlSetting", "When calling create routine statement in worksheet"),
		box);
	vbox->addWidget(CreateLabel);

	BeforeCreateAction = new QComboBox(box);
	BeforeCreateAction->clear();
	BeforeCreateAction->insertItems(0, QStringList()
					<< qApp->translate("qSqlSetting", "Do nothing")
					<< qApp->translate("qSqlSetting", "Drop before creating")
					<< qApp->translate("qSqlSetting", "Drop before creating (if exists)")
					<< qApp->translate("qSqlSetting", "Ask")
					<< qApp->translate("qSqlSetting", "Ask (if exists)"));
	BeforeCreateAction->setCurrentIndex(toConfigurationSingle::Instance().createAction());
	vbox->addWidget(BeforeCreateAction);

	QSpacerItem *spacer = new QSpacerItem(
		20,
		20,
		QSizePolicy::Minimum,
		QSizePolicy::Expanding);
	vbox->addItem(spacer);

	box->setLayout(vbox);
}

void toQMySqlSetting::saveSetting(void)
{
	toConfigurationSingle::Instance().setOnlyForward(OnlyForward->isChecked());
	toQMySqlProvider::OnlyForward = OnlyForward->isChecked();
	toConfigurationSingle::Instance().setCreateAction(BeforeCreateAction->currentIndex());
}

