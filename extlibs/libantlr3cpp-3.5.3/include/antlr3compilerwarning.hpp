#ifndef _ANTLR3_WARNIN_PUSH
#define _ANTLR3_WARNIN_PUSH

//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifdef _MSC_VER
//#pragma warning (push)
#pragma warning( disable: 4189 )    // local variable is initialized but not referenced
#pragma warning( disable: 4296 )    // expression is always true
#pragma warning( disable: 4625 )    // copy constructor could not be generated
#pragma warning( disable: 4626 )    // assignment operator could not be generated
#pragma warning( disable: 4640 )    // a local static object is not thread-safe
#pragma warning( disable: 4710 )    // call was not inlined
#pragma warning( disable: 4711 )    // call was inlined
#pragma warning( disable: 4820 )    // some padding was added
#pragma warning( disable: 4127 )    // conditional expression is constant
// These are only really displayed at warning level /W4 but that is the code ideal I am aiming at
// and the codegen must generate some of these warnings by necessity, apart from 4100, which is
// usually generated when a parser rule is given a parameter that it does not use. Mostly though
// this is a matter of orthogonality hence I disable that one.
#pragma warning( disable : 4100 )  // unreferenced parameter
#pragma warning( disable : 4101 )  // unreferenced local variable
#endif  

#if defined(__GNUC__) && !defined(__clang__)
//#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#if __GNUC_MINOR__ >= 8
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

#ifdef __clang__
//#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wparentheses-equality"
#pragma clang diagnostic ignored "-Wtautological-compare"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#endif

#endif
