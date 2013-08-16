/*
A helper co-called API generator for code completion.
It takes TOra internals DefaultKeywords and function definition
from templates - then is the result written into "cpmpletion.api".

compile: qmake; make

Petr Vanek <petr@scribus.info>
*/

#include <QFile>
#include <QTextStream>
#include <QtDebug>

#include "../../src/todefaultkeywords.h"

int main(int /*argc*/, char** /*argv*/)
{
	qDebug() << "Code completion generator for TOra started";

	QFile of("../../src/templates/completion.api");
	of.open(QIODevice::WriteOnly);
	QTextStream out(&of);

	qDebug() << "DefaultKeywords...";
	// default keywords
	for (int i = 0; DefaultKeywords[i]; i++)
	{
		out << DefaultKeywords[i] << "\n";
	}
	qDebug() << "done.";

	qDebug() << "Additional functions...";
	// functions and the other stuff
	QFile f("../../src/templates/sqlfunctions.tpl");
	f.open(QIODevice::ReadOnly | QIODevice::Text);
	while (!f.atEnd())
	{
		QString s(f.readLine());
		int start = s.indexOf(":") + 1;
		int end = s.indexOf(" ", start);
		if (end == -1)
			end = s.indexOf("(", start);

		out << s.mid(start, end - start) << "\n";
	}
	qDebug() << "done.";

	of.close();
	qDebug() << "Code completion generator for TOra finished";
}
