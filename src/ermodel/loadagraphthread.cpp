/*
    This file is part of KGraphViewer.
    Copyright (C) 2010  Gael de Chalendar <kleag@free.fr>

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

#include "loadagraphthread.h"

///#include <kdebug.h>

void LoadAGraphThread::run()
{
  ///kDebug() << m_dotFileName;
  GVC_t *gvc;
  FILE* fp;
  gvc = gvContext();
  fp = fopen(m_dotFileName.toUtf8().data(), "r");
  m_g = agread(fp);
}

void LoadAGraphThread::loadFile(const QString& dotFileName)
{
  ///kDebug();
  if (isRunning())
    return;
  m_dotFileName = dotFileName;
  start();
}