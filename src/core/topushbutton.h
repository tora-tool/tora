
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

#ifndef TOPUSHBUTTON_H
#define TOPUSHBUTTON_H



#include <QPushButton>
#include <QToolButton>
#include <QtCore/QMetaEnum>

class QToolBar;

// a button that can be rotated.
class toRotatingButton : public QPushButton
{
        Q_OBJECT;

    public:
        enum Rotation
        {
            NoRotation       = 0,
            UpsideDown       = 180,
            Clockwise        = 90,
            CounterClockwise = 270
        };

        explicit toRotatingButton(QWidget *parent = 0);
        explicit toRotatingButton(const QString &text,
                                  QWidget *parent = 0);
        explicit toRotatingButton(const QIcon &icon,
                                  const QString &text,
                                  QWidget *parent = 0);

        /**
         * Return the rotation set for this button
         *
         */
        inline Rotation rotation() const
        {
            return rotate;
        }


        /**
         * Set rotation for this button
         *
         */
        void setRotation(Rotation rotation);


        virtual QSize sizeHint() const;
        virtual QSize minimumSizeHint() const;

    protected:
        virtual void paintEvent(QPaintEvent *);

    private:
        Rotation rotate;
        QStyleOptionButton getStyleOption();
};

//NOTE: moved from utils.h
/** Popup toolbutton that works like I think they should under Qt 3.0 as well.
 * this means they will pop up a tool menu when you click on the button as well.
 */
class toPopupButton : public QToolButton
{
        Q_OBJECT;
    public:
        /** Create button, same constructor as toolbutton except no slots
         */
        toPopupButton(const QIcon &iconSet, const QString &textLabel,
                      const QString &grouptext, QToolBar *parent, const char *name = 0);
        /** Create button somewhere else than in a toolbar.
         */
        toPopupButton(QWidget *parent, const char *name = 0);
};

/**
 * This toggle button changes it's label each time its clicked
 * It uses enum values as circular list of string labels
 * See toHighlighterTypeButton as an example
 */
class toToggleButton : public QPushButton
{
        Q_OBJECT;
    public:
        toToggleButton(QMetaEnum const& e, QWidget *parent, const char* name = 0);

        /**
         * Change buttons value(does not emit the toggled signal)
         */
        void setValue(int);
        void setValue(QString const&);

    private slots:
        void toggle();
    signals:
        void toggled(QString const&);
        void toggled(int);
    private:
        QMetaEnum m_enum;
        int m_idx;
};

#endif
