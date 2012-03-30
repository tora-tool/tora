/* This file is part of KGraphViewer.
   Copyright (C) 2005-2007 Gael de Chalendar <kleag@free.fr>

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


#ifndef KGRAPHVIEWER_FONTSCACHE_H
#define KGRAPHVIEWER_FONTSCACHE_H

#include "Singleton.h"

#include <qfont.h>
#include <qmap.h>
#include <qstring.h>

/**
 * This is a map of fonts used in KgraphViewer
 *
 * @short A fonts map
 * @author Gaël de Chalendar <kleag@free.fr>
 */
class FontsCache : 
  public Singleton<FontsCache>,
  public QMap<QString, QFont*>
{
friend class Singleton<FontsCache>;

public:
  virtual ~FontsCache();

  QFont* cachedFont(const QFont& font);
  QFont* fromName(const QString& font);

private:
  FontsCache() {}

  QMap<QString, QFont*> m_namesToFonts;
};

#endif
