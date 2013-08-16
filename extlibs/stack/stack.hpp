// Copyright 2007 Edd Dawson.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef STACK_HPP_0022_01092007
#define STACK_HPP_0022_01092007

#include <string>
#include <list>
#include <iosfwd>

namespace dbg
{
    //! stack_frame objects are collected by a stack object. They contain information about the instruction pointer,
    //! the name of the corresponding function and the "module" (executable or library) in which the function resides.
    struct stack_frame
    {
        stack_frame(const void *instruction, const std::string &function, const std::string &module);

        const void *instruction;
        std::string function;
        std::string module;
    };

    //! Allows you to write a stack_frame object to an std::ostream
    std::ostream &operator<< (std::ostream &out, const stack_frame &frame);

    //! Instantiate a dbg::stack object to collect information about the current call stack. Once created, a stack object
    //! may be freely copied about and will continue to contain the information about the scope in which collection occurred.
    class stack
    {
        public:
            typedef std::list<stack_frame>::size_type depth_type;
            typedef std::list<stack_frame>::const_iterator const_iterator;

            //! Collect information about the current call stack. Information on the most recent frames will be collected
            //! up to the specified limit. 0 means unlimited.
            //! An std::runtime_error may be thrown on failure.
            stack(depth_type limit = 0);

            //! Returns an iterator referring to the "top" stack frame
            const_iterator begin() const;

            //! Returns an iterator referring to one past the "bottom" stack frame
            const_iterator end() const;

            //! Returns the number of frames collected
            depth_type depth() const;
            
        private:
            std::list<stack_frame> frames_;
    };

} // close namespace dbg

#endif // STACK_HPP_0022_01092007
