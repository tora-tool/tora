
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "core/tosettingtab.h"
#include "core/toconfiguration.h"

#include <QtCore/QDebug>
#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QSlider>
#include <QComboBox>

void toSettingTab::loadSettings(QWidget *widget)
{
    static QRegExp any(".*");
    QList<QWidget*> lst = widget->findChildren<QWidget*>(any);
    Q_FOREACH(QWidget *w, lst)
    {
        qDebug() << w->objectName();
        if (w->objectName() == "qt_spinbox_lineedit") // internal widget inside QSpinBox
            continue;
        if (QComboBox *combo = qobject_cast<QComboBox*>(w))
        {
            try
            {
                QVariant v = toConfigurationNewSingle::Instance().option(combo->objectName());
                if (combo->objectName().endsWith("Int"))
                    combo->setCurrentIndex(v.toInt());
                else
                    combo->setCurrentIndex(combo->findText(v.toString()));
            }
            catch (...)
            {
                qDebug() << w->objectName() << '*';
                combo->setDisabled(true);
            }
        }
        else if (QSpinBox *spin = qobject_cast<QSpinBox*>(w))
        {
            try
            {
                QVariant v = toConfigurationNewSingle::Instance().option(spin->objectName());
                if (v.type() == QVariant::Int)
                {
                    spin->setValue(v.toInt());
                }
                else
                {
                    spin->setDisabled(true);
                }
            }
            catch (...)
            {
                qDebug() << w->objectName() << '#';
                spin->setDisabled(true);
            }
        }
        else if (QSlider *slider = qobject_cast<QSlider*>(w))
        {
            try
            {
                QVariant v = toConfigurationNewSingle::Instance().option(slider->objectName());
                if (v.type() == QVariant::Int)
                {
                    slider->setValue(v.toInt());
                }
                else
                {
                    slider->setDisabled(true);
                }
            }
            catch (...)
            {
                qDebug() << w->objectName() << '#';
                slider->setDisabled(true);
            }
        }
        else if (QLineEdit *edit = qobject_cast<QLineEdit*>(w))
        {
            try
            {
                QVariant v = toConfigurationNewSingle::Instance().option(edit->objectName());
                edit->setText(v.toString());
            }
            catch (...)
            {
                qDebug() << w->objectName() << '&';
                edit->setDisabled(true);
            }
        }
        else if (QCheckBox *checkbox = qobject_cast<QCheckBox*>(w))
        {
            try
            {
                QVariant v = toConfigurationNewSingle::Instance().option(checkbox->objectName());
                if (v.type() == QVariant::Bool)
                {
                    checkbox->setChecked(v.toBool());
                }
                else
                {
                    checkbox->setDisabled(true);
                }
            }
            catch (...)
            {
                qDebug() << w->objectName() << '%';
                checkbox->setDisabled(true);
            }
        }

    }
}

void toSettingTab::saveSettings(QWidget *widget)
{
    static QRegExp any(".*");
    QList<QWidget*> lst = widget->findChildren<QWidget*>(any);
    Q_FOREACH(QWidget *w, lst)
    {
        qDebug() << w->objectName();
        if (w->objectName() == "qt_spinbox_lineedit") // internal widget inside QSpinBox
            continue;
        if (QComboBox *combo = qobject_cast<QComboBox*>(w))
        {
            if (combo->objectName().endsWith("Int"))
                toConfigurationNewSingle::Instance().setOption(combo->objectName(), combo->currentIndex());
            else
                toConfigurationNewSingle::Instance().setOption(combo->objectName(), combo->currentText());
        }
        else if (QSpinBox *spin = qobject_cast<QSpinBox*>(w))
        {
            toConfigurationNewSingle::Instance().setOption(spin->objectName(), spin->value());
        }
        else if (QSlider *slider = qobject_cast<QSlider*>(w))
        {
            toConfigurationNewSingle::Instance().setOption(slider->objectName(), slider->value());
        }
        else if (QLineEdit *edit = qobject_cast<QLineEdit*>(w))
        {
            if (edit->objectName().endsWith("Int"))
                toConfigurationNewSingle::Instance().setOption(edit->objectName(), edit->text().toInt());
            else
                toConfigurationNewSingle::Instance().setOption(edit->objectName(), edit->text());
        }
        else if (QCheckBox *checkbox = qobject_cast<QCheckBox*>(w))
        {
            toConfigurationNewSingle::Instance().setOption(checkbox->objectName(), /*checkbox->isEnabled() &&*/ checkbox->isChecked());
        }

    }
}
