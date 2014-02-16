/* This file is part of KGraphViewer.
   Copyright (C) 2007 Gael de Chalendar <kleag@free.fr>

   KGraphViewer is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

#include "graphelement.h"
#include "canvaselement.h"
#include "dotdefaults.h"

#include <math.h>

///#include <kdebug.h>

#include <QRegExp>
#ifdef GV_LIB
#include <graphviz/gvc.h>
#endif

///namespace KGraphViewer
///{
  
GraphElement::GraphElement() :
    QObject(),
    m_attributes(),
    m_originalAttributes(),
    m_ce(0),
    m_z(1.0),
    m_renderOperations(),
    m_selected(false)
{
/*  label("");
  id("");
  style(DOT_DEFAULT_STYLE);
  shape(DOT_DEFAULT_SHAPE);
  lineColor(DOT_DEFAULT_LINECOLOR);
  backColor(DOT_DEFAULT_BACKCOLOR);
  fontName(DOT_DEFAULT_FONTNAME);
  fontColor(DOT_DEFAULT_FONTCOLOR);
  url("");
  shapeFile("");*/
  setFontSize(DOT_DEFAULT_FONTSIZE);
}

GraphElement::GraphElement(const GraphElement& element) : QObject(),
  m_attributes(),
  m_originalAttributes(),
  m_ce(element.m_ce),
  m_z(element.m_z),
  m_renderOperations(),
  m_selected(element.m_selected)
{
  ///kDebug() ;
  updateWithElement(element);
}

void GraphElement::updateWithElement(const GraphElement& element)
{
  ///kDebug() << element.id();
  bool modified = false;
  if (element.z() != m_z)
  {
    m_z = element.z();
    modified = true;
  }
  foreach (const QString &attrib, element.attributes().keys())
  {
    if ( (!m_attributes.contains(attrib)) || (m_attributes[attrib] != element.attributes()[attrib]) )
    {
      m_attributes[attrib] = element.attributes()[attrib];
      if (attrib == "z")
      {
        bool ok;
        setZ(m_attributes[attrib].toDouble(&ok));
      }
      modified = true;
    }
  }
  if (modified)
  {
    ///kDebug() << "modified: update render operations";
    m_renderOperations = element.m_renderOperations;
/*    foreach (DotRenderOp op, m_renderOperations)
    {
      QString msg;
      QTextStream dd(&msg);
      dd << "an op: " << op.renderop << " ";
      foreach (int i, op.integers)
      {
        dd << i << " ";
      }
      dd << op.str;
      ///kDebug() << msg;
    }
    ///kDebug() << "modified: emiting changed";*/
    emit changed();
  }
  ///kDebug() << "done" << m_renderOperations.size();
}


QString GraphElement::backColor() const
{
  if (m_attributes.find("fillcolor") != m_attributes.end())
  {
    return m_attributes["fillcolor"];
  }
  else if ( (m_attributes.find("color") != m_attributes.end())
    && (m_attributes["style"] == "filled") )
  {
    return m_attributes["color"];
  }
  else
  {
    return DOT_DEFAULT_NODE_BACKCOLOR;
  }
}

void GraphElement::removeAttribute(const QString& attribName)
{
  ///kDebug() << attribName;
  m_attributes.remove(attribName);
  emit changed();
}

#ifdef GV_LIB
void GraphElement::exportToGraphviz(void* element) const
{
  QMap<QString,QString>::const_iterator it, it_end;
  it = attributes().begin(); it_end = attributes().end();
  for (;it != it_end; it++)
  {
    if (!it.value().isEmpty())
    {
      if (it.key() == "label")
      {
        QString label = it.value();
        if (label != "label")
        {
          label.replace(QRegExp("\n"),"\\n");
          //           kDebug() << it.key() << "=\"" << label << "\",";
          agsafeset(element, it.key().toUtf8().data(), label.toUtf8().data(), QString().toUtf8().data());
        }
      }
      else if (it.key() == "_draw_" || it.key() == "_ldraw_")
      {
      }
      else if (originalAttributes().isEmpty() || originalAttributes().contains(it.key()))
      {
        //         kDebug() << it.key() << it.value();
        
        agsafeset(element, it.key().toUtf8().data(), it.value().toUtf8().data(), QString().toUtf8().data());
      }
    }
  }
}
#endif

QTextStream& operator<<(QTextStream& s, const GraphElement& n)
{
  QMap<QString,QString>::const_iterator it, it_end;
  it = n.attributes().begin(); it_end = n.attributes().end();
  for (;it != it_end; it++)
  {
    if (!it.value().isEmpty())
    {
      if (it.key() == "label")
      {
        QString label = it.value();
        if (label != "label")
        {
          label.replace(QRegExp("\n"),"\\n");
//           kDebug() << it.key() << "=\"" << label << "\",";
          s << it.key() << "=\"" << label << "\",";
        }
      }
      else if (it.key() == "_draw_" || it.key() == "_ldraw_")
      {
      }
      else if (n.originalAttributes().isEmpty() || n.originalAttributes().contains(it.key()))
      {
//         ///kDebug() << it.key() << it.value();
        
          s << it.key() << "=\"" << it.value() << "\",";
      }
    }
  }
  return s;
}


///#include "graphelement.moc"
