//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "toprofiler.h"
#include "tosql.h"

#include "toprofiler.moc"

static toSQL SQLProfilerRuns("toProfiler:ProfilerRuns",
			     "create table plsql_profiler_runs\n"
			     "(\n"
			     "  runid           number primary key,\n"
			     "  related_run     number,\n"
			     "  run_owner       varchar2(32),\n"
			     "  run_date        date,\n"
			     "  run_comment     varchar2(2047),\n"
			     "  run_total_time  number,\n"
			     "  run_system_info varchar2(2047),\n"
			     "  run_comment1    varchar2(2047),\n"
			     "  spare1          varchar2(256)\n"
			     ")",
			     "Create table plsql_profiler_runs");

static toSQL SQLProfilerUnits("toProfiler:ProfilerUnits",
			      "create table plsql_profiler_units\n"
			      "(\n"
			      "  runid              number references plsql_profiler_runs,\n"
			      "  unit_number        number,\n"
			      "  unit_type          varchar2(32),\n"
			      "  unit_owner         varchar2(32),\n"
			      "  unit_name          varchar2(32),\n"
			      "  unit_timestamp     date,\n"
			      "  total_time         number DEFAULT 0 NOT NULL,\n"
			      "  spare1             number,\n"
			      "  spare2             number,\n"
			      "  primary key (runid, unit_number)\n"
			      ")",
			      "Create table plsql_profiler_unites");

static toSQL SQLProfilerData("toProfiler:ProfilerData",
			     "create table plsql_profiler_data\n"
			     "(\n"
			     "  runid           number,\n"
			     "  unit_number     number,\n"
			     "  line#           number not null,\n"
			     "  total_occur     number,\n"
			     "  total_time      number,\n"
			     "  min_time        number,\n"
			     "  max_time        number,\n"
			     "  spare1          number,\n"
			     "  spare2          number,\n"
			     "  spare3          number,\n"
			     "  spare4          number,\n"
			     "  primary key (runid, unit_number, line#),\n"
			     "  foreign key (runid, unit_number) references plsql_profiler_units\n"
			     ")",
			     "Create table plsql_profiler_data");

static toSQL SQLProfilerNumber("toProfiler:ProfilerNumber",
			       "create sequence plsql_profiler_runnumber start with 1 nocache",
			       "Create sequence plsql_profiler_runnumber");

static toSQL SQLProfilerDetect("toProfiler:ProfilerDetect",
			       "select count(1)\n"
			       "  from plsql_profiler_data a,\n"
			       "       plsql_profiler_units b,\n"
			       "       plsql_profiler_runs c\n"
			       " where a.runid = null and b.runid = null and c.runid = null",
			       "Detect if profiler tables seems to exist");

