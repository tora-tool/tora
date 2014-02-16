/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef LAYOUTAGRAPHTHREAD_H
#define LAYOUTAGRAPHTHREAD_H

#include <QThread>
#ifdef GV_LIB
#include <graphviz/gvc.h>
#endif

class LayoutAGraphThread : public QThread
{
public:
  inline const QString& layoutCommand() const {return m_layoutCommand;}
#ifdef GV_LIB
  void layoutGraph(graph_t* graph, const QString& layoutCommand);
  inline GVC_t* gvc() {return m_gvc;}
  inline graph_t* g() {return m_g;}
#endif  
protected:
virtual void run();

private:
  QString m_layoutCommand;
#ifdef GV_LIB
  graph_t* m_g;
  GVC_t *m_gvc;
#endif
};

#endif // LAYOUTAGRAPHTHREAD_H
