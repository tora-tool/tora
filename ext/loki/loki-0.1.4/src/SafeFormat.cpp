////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 by Andrei Alexandrescu
// Copyright (c) 2006 Peter Kümmel
// Permission to use, copy, modify, distribute, and sell this software for any
//     purpose is hereby granted without fee, provided that the above copyright
//     notice appear in all copies and that both that copyright notice and this
//     permission notice appear in supporting documentation.
// The author makes no representations about the suitability of this software 
//     for any purpose. It is provided "as is" without express or implied 
//     warranty.
////////////////////////////////////////////////////////////////////////////////

// $Header: /cvsroot/loki-lib/loki/src/SafeFormat.cpp,v 1.2 2006/01/16 20:59:53 rich_sposato Exp $

#include <loki/SafeFormat.h>


namespace Loki
{

    // Crude writing method: writes straight to the file, unbuffered
    // Must be combined with a buffer to work properly (and efficiently)

    void write(std::FILE* f, const char* from, const char* to) {
        assert(from <= to);
        fwrite(from, 1, to - from, f);
    }

    // Write to a string

    void write(std::string& s, const char* from, const char* to) {
        assert(from <= to);
        s.append(from, to);
    }

  

    ////////////////////////////////////////////////////////////////////////////////
    // PrintfState class template
    // Holds the formatting state, and implements operator() to format stuff
    // Todo: make sure errors are handled properly
    ////////////////////////////////////////////////////////////////////////////////


    PrintfState<std::FILE*, char> Printf(const char* format) {
        return PrintfState<std::FILE*, char>(stdout, format);
    }

    PrintfState<std::FILE*, char> Printf(const std::string format) {
        return PrintfState<std::FILE*, char>(stdout, format.c_str());
    }

    PrintfState<std::FILE*, char> FPrintf(FILE* f, const char* format) {
        return PrintfState<std::FILE*, char>(f, format);
    }

    PrintfState<std::FILE*, char> FPrintf(FILE* f, const std::string& format) {
        return PrintfState<std::FILE*, char>(f, format.c_str());
    }

    PrintfState<std::string&, char> SPrintf(std::string& s, const char* format) {
        return PrintfState<std::string&, char>(s, format);
    }

    PrintfState<std::string&, char> SPrintf(std::string& s, const std::string& format) {
        return PrintfState<std::string&, char>(s, format.c_str());
    }


}// namespace Loki

// $Log: SafeFormat.cpp,v $
// Revision 1.2  2006/01/16 20:59:53  rich_sposato
// Added cvs keywords.
//
