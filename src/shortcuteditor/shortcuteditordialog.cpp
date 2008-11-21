/*
For general Sqliteman copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Sqliteman
for which a new license (GPL+exception) is in place.
*/
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#if QT_VERSION >= 0x040300
#include <QXmlStreamReader>
#else
#warning "QXmlStreamReader is disabled. Qt 4.3.x required."
#endif

#include "shortcuteditordialog.h"
#include "shortcutmodel.h"


ShortcutEditorDialog::ShortcutEditorDialog(QWidget * parent)
	: QDialog(parent)
{
	setupUi(this);

	model = new ShortcutModel();
	tableView->setModel(model);
	tableView->resizeColumnsToContents();

	connect(removeAllButton, SIGNAL(clicked()), this, SLOT(removeAllButton_clicked()));
	connect(removeButton, SIGNAL(clicked()), this, SLOT(removeButton_clicked()));
	connect(addButton, SIGNAL(clicked()), this, SLOT(addButton_clicked()));
	connect(model, SIGNAL(keysNotUnique(QString)), this, SLOT(keysNotUnique(QString)));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptDialog()));

	connect(exportButton, SIGNAL(clicked()), this, SLOT(exportButton_clicked()));
	connect(importButton, SIGNAL(clicked()), this, SLOT(importButton_clicked()));
}

ShortcutEditorDialog::~ShortcutEditorDialog()
{
}

void ShortcutEditorDialog::removeAllButton_clicked()
{
	model->removeRows(0, model->rowCount());
}

void ShortcutEditorDialog::removeButton_clicked()
{
	model->removeRows(tableView->currentIndex().row(), 1);
}

void ShortcutEditorDialog::addButton_clicked()
{
	model->insertRow();
}

void ShortcutEditorDialog::keysNotUnique(QString value)
{
	QMessageBox::warning(this, tr("Shortcut Error"),
						 tr("The value you entered as a key (%1) is not unique. Enter another one, please.").arg(value));
}

void ShortcutEditorDialog::acceptDialog()
{
	QPair<QString,QString> p;
	foreach (p, model->values())
	{
		if (p.first.isEmpty())
		{
			int b = QMessageBox::question(this, tr("Shortcut Error"),
										  tr("Some of key values are empty. These items will be lost. Do you want to return to the shortcut editor to fix it?"),
										  QMessageBox::Yes | QMessageBox::No,
										  QMessageBox::Yes);
			if (b == QMessageBox::Yes)
				return;
		}
	}
	model->saveValues();
	accept();
}

void ShortcutEditorDialog::exportButton_clicked()
{
#if QT_VERSION >= 0x040300
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Shortcuts"),
			QDir::currentPath(), tr("Sqliteman Shortcuts XML (*.xml);;All Files (*)"));
	if (fileName.isEmpty())
		return;

	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, tr("Export Error"), tr("Cannot open file %1 for writting.").arg(fileName));
		return;
	}

	QXmlStreamWriter xml(&file);
	xml.writeStartDocument();
	xml.writeStartElement("sqliteman-shortcuts");

	QPair<QString,QString> p;
	foreach (p, model->values())
	{
		xml.writeStartElement("pair");
		xml.writeAttribute("key", p.first);
		xml.writeAttribute("value", p.second);
		xml.writeEndElement();
	}
	xml.writeEndElement(); //"sqliteman-shortcuts"
	xml.writeEndDocument();

	file.close();
#endif
}

void ShortcutEditorDialog::importButton_clicked()
{
#if QT_VERSION >= 0x040300
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load Shortcuts"),
			QDir::currentPath(), tr("Sqliteman Shortcuts XML (*.xml);;All Files (*)"));
	if (fileName.isEmpty())
		return;

	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, tr("Load Error"), tr("Cannot open file %1 for reading.").arg(fileName));
		return;
	}

	QXmlStreamReader xml(&file);
	bool isXML = false;

	while (!xml.atEnd())
	{
		xml.readNext();
		if (xml.isStartElement())
		{
			if (xml.name() == "sqliteman-shortcuts")
				isXML = true;
			if (isXML && xml.name() == "pair")
			{
				model->insertRow(xml.attributes().value("key").toString(),
								 xml.attributes().value("value").toString());
			}
		}
	}
	if (xml.error() && xml.error() != QXmlStreamReader::PrematureEndOfDocumentError)
	{
//         qWarning() << "XML ERROR:" << xml.lineNumber() << ": " << xml.errorString();
    }

	file.close();
#endif
}
