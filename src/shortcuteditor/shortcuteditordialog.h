/*
For general Sqliteman copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Sqliteman
for which a new license (GPL+exception) is in place.
*/

#ifndef SHORTCUTEDITORDIALOG_H
#define SHORTCUTEDITORDIALOG_H

#include "ui_shortcuteditordialog.h"

class ShortcutModel;


/*! \brief A GUI for settings sQL editor shortcuts.
The basic idea of "shortcuts" is: user can define some string
sequences like "sf" what will be expanded into e.g. "select * from"
after pressing Tabelator in the editor.
\author Petr Vanek <petr@scribus.info>
*/
class ShortcutEditorDialog : public QDialog, public Ui::ShortcutEditorDialog
{
	Q_OBJECT

	public:
		ShortcutEditorDialog(QWidget * parent = 0);
		~ShortcutEditorDialog();

	private:
		ShortcutModel * model;

	private slots:
		void removeAllButton_clicked();
		void removeButton_clicked();
		void addButton_clicked();

		void acceptDialog();

		void keysNotUnique(QString value);

		void exportButton_clicked();
		void importButton_clicked();
};

#endif
