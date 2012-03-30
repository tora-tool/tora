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


#include "FontsCache.h"

#include "dot2qtconsts.h"

FontsCache::~FontsCache()
{
  FontsCache::iterator it, it_end;
  it = begin(); it_end = end();
  for (; it != it_end; it++)
  {
    delete (*it);
  }
}

QFont* FontsCache::cachedFont(const QFont& font)
{
  if (find(font.key()) == end())
  {
    (*this)[font.key()] = new QFont(font);
  }
  return (*this)[font.key()];
}

QFont* FontsCache::fromName(const QString& fontName)
{
  if (m_namesToFonts.find(fontName) == m_namesToFonts.end())
  {
    QFont font(Dot2QtConsts::componentData().qtFont(fontName));
    m_namesToFonts[fontName] = cachedFont(font);
  }
  return m_namesToFonts[fontName];
}
