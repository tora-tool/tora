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


#ifndef KGRAPHVIEWER_SINGLETON_H
#define KGRAPHVIEWER_SINGLETON_H

/**
 * An implementation of the singleton pattern
 *
 * @short Singleton pattern implementation
 * @author Gaël de Chalendar <kleag@free.fr>
 */
template<typename Object>
class Singleton
{
public:

  /**
    * @brief const singleton accessor
    */
  static const Object& single();

  /**
    * @brief singleton accessor
    */
  static Object& changeable();


private:
  static Object* s_instance;

};

template<typename Object>
Object* Singleton<Object>::s_instance(0);

template<typename Object>
const Object& Singleton<Object>::single()
{
  if (s_instance==0)
  {
    s_instance=new Object();
  }
  return *s_instance;
}

template<typename Object>
Object& Singleton<Object>::changeable()
{
  if (s_instance==0)
  {
    s_instance=new Object();
  }
  return *s_instance;
}

#endif
