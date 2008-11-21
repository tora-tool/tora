/*
For general Sqliteman copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Sqliteman
for which a new license (GPL+exception) is in place.
*/
#include "shortcutmodel.h"
#include "toconfiguration.h"


ShortcutModel::ShortcutModel(QObject * /*parent*/)
	: QAbstractTableModel()
{
	EditorShortcutsMap map(toConfigurationSingle::Instance().editorShortcuts());
	EditorShortcutsMapIterator i(map);
	while (i.hasNext())
	{
		i.next();
		m_values.append(qMakePair(i.key(), i.value().toString()));
	}
}


ShortcutModel::~ShortcutModel()
{
}

int ShortcutModel::rowCount(const QModelIndex & /*parent*/) const
{
	return m_values.count();
}

int ShortcutModel::columnCount(const QModelIndex & /*parent*/) const
{
	return 2;
}

QVariant ShortcutModel::data(const QModelIndex & index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		QString s;
		return QVariant(index.column() == 0 ? m_values.at(index.row()).first : m_values.at(index.row()).second);
	}
	if (role == Qt::BackgroundRole && index.column() == 0 && m_values.at(index.row()).first.isEmpty())
		return QVariant(Qt::red);
	return QVariant();
}

bool ShortcutModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
	if (role != Qt::EditRole || !index.isValid())
		return false;
	QString newVal(value.toString());
	if (index.column() == 0)
	{
		// ensure it's unique. Message is handled in the ShortcutEditorDialog
		QPair<QString,QString> p;
		foreach (p, m_values)
		{
			if (p.first == newVal)
			{
				emit keysNotUnique(newVal);
				return false;
			}
		}
		m_values[index.row()].first = newVal;
	}
	else
		m_values[index.row()].second = newVal;
	emit dataChanged(index, index);
	return true;
}

Qt::ItemFlags ShortcutModel::flags(const QModelIndex & /*index*/) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}

QVariant ShortcutModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
         return QVariant();

	if (orientation == Qt::Horizontal)
		return section == 0 ? tr("Shortcut") : tr("Expanded Text");
	else
		return QString("%1").arg(section + 1);
}

bool ShortcutModel::removeRows(int row, int count, const QModelIndex & parent)
{
	if(count <= 0 || row < 0 || (row + count) > rowCount(parent))
		return false;

	beginRemoveRows(parent, row, row + count - 1);
	for (int i = 0; i < count; ++i)
		m_values.removeAt(row);
	endRemoveRows();
	return true;
}

bool ShortcutModel::insertRows( int row, int count, const QModelIndex & parent)
{
	beginInsertRows(parent, row, row + count - 1);
	for (int i = 0; i < count; ++i)
		m_values.insert(row + count, qMakePair(QString(), QString()));
	endInsertRows();
	return true;
}

void ShortcutModel::insertRow()
{
	insertRows(rowCount(), 1);
}

void ShortcutModel::insertRow(QString key, QString value)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_values.insert(rowCount(), qMakePair(key, value));
	endInsertRows();
}

void ShortcutModel::saveValues()
{
	EditorShortcutsMap ret;
	QPair<QString,QString> p;

	foreach (p, m_values)
		ret[p.first] = p.second;

	toConfigurationSingle::Instance().setEditorShortcuts(ret);
}
