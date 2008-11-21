/*
For general Sqliteman copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Sqliteman
for which a new license (GPL+exception) is in place.
*/
#ifndef SHORTCUTMODEL_H
#define SHORTCUTMODEL_H

#include <QAbstractTableModel>
#include <QPair>


/*! \brief A model for sQL editor shortcuts.
See ShortcutEditorDialog for basic idea of shortcuts.
Shortcuts are handled as a QList of QPairs in the model.
It's due the need of changing "key" values what is impossible
in QMap/QHash structures (well, it could be done but it is
expensive).
Shortcuts are saved by Preferences as a QMap(QString,QVariant).
QVariant due the QSettings limitations.
\author Petr Vanek <petr@scribus.info>
*/
class ShortcutModel : public QAbstractTableModel
{
	Q_OBJECT

	public:
		ShortcutModel(QObject * parent = 0);
		~ShortcutModel();

		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;

		QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
		bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

		Qt::ItemFlags flags(const QModelIndex & index) const;

		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

		bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
		bool insertRows( int row, int count, const QModelIndex & parent = QModelIndex());
		void insertRow();
		void insertRow(QString key, QString value);

		QList<QPair<QString,QString> > values() { return m_values; };
		//! \brief Check emptiness of keys and save it by Preferences.
		void saveValues();

	signals:
		/*! \brief Signal emmitted when the user finish the editation of one value.
		*/
		void valueChanged();
		/*! \brief Emitted when user enters duplicate key value.
		QString is the duplicate value. It's handled in the GUI with message box */
		void keysNotUnique(QString);

	private:
		QList<QPair<QString,QString> > m_values;
};

#endif
