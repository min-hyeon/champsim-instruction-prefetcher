#ifndef THROW_H
#define THROW_H

#include <string>
#include <sstream>

namespace fmt {

    template <typename ...Args>
    std::string sprintf_static(const char* format_expr, Args... args) {
        /**
         * 7.21.6.5 [The snprintf function] N1570 (C11 draft)
         * 
         * <\code>
         *     int snprintf(
         *         char* restrict s,
         *         size_t n,
         *         const char * restrict format, ...);
         * <\endcode>
         * 
         * The snprintf function is equivalent to fprintf, except that the
         * output is written into an array (speciﬁed by argument s) rather
         * than to a stream. If n is zero, nothing is written, and s may be
         * a null pointer.
         * 
         * Otherwise, output characters beyond the n-1st are discarded rather
         * than being written to the array, and a null character is written
         * at the end of the characters actually written into the array.
         * If copying takes place between objects that overlap, the behavior
         * is undefined.
         */
        int size = snprintf(nullptr, 0, format_expr, args...);
        if (size < 0) {
            throw std::runtime_error("[Error] Format encoding error.");
        }

        // temporary buffer (with null terminator)
        char* p_buf = new char[size+1];
        snprintf(p_buf, size+1, format_expr, args...);

        // copy "p_buf" into string without null terminator
        std::string formatted(p_buf, p_buf + size); 

        delete[] p_buf;
        return formatted;
    }

    namespace impl
    {
        template <typename Arg>
        void __impl_sprintf(
            std::stringstream& stream,
            const std::string& format_expr, const std::string& sep,
            size_t begin, size_t end,
            Arg arg)
        {
            if ((end = format_expr.find(sep, begin)) == std::string::npos)
                throw std::runtime_error("[Error] Encoding error occurs.");
            stream << format_expr.substr(begin, end - begin) << arg;
            begin = end + sep.length();
            if (format_expr.find(sep, begin) != std::string::npos)
                throw std::runtime_error("[Error] Encoding error occurs.");
        }

        template <typename Arg, typename ...Args>
        void __impl_sprintf(
            std::stringstream& stream,
            const std::string& format_expr, const std::string& sep,
            size_t begin, size_t end,
            Arg arg, Args... args)
        {
            if ((end = format_expr.find(sep, begin)) == std::string::npos)
                throw std::runtime_error("[Error] Encoding error occurs.");
            stream << format_expr.substr(begin, end - begin) << arg;
            begin = end + sep.length();
            __impl_sprintf(stream, format_expr, sep, begin, end, args...);
        }

    }   // namespace impl
    
    /**
     * This function dynamically formats and returns a string using
     * variadic template arguements.
     * 
     * Each unpacked arguement from the given parameter pack "args"
     * replaces each substr that matches "sep". Thus, the size of the
     * parameter pack and the number of "sep"'s found in the "format_expr"
     * should be same.
     * 
     * <\code>
     *     auto str = format::sprintf(
     *         "{} {} and {} {} are ${} in total.",   // format
     *         "{}",   // seperator
     *         2, "apple", 3, "banna", 2*0.8.0+3*0.75);
     * <\endcode>
     * 
     * If you know the types of the arguments that you want to format
     * and they are all supported in C, then it's okay to use sprintf_static,
     * but otherwise, use this function; i.e. if some of entries that
     * you want to convert into string are templated type.
     * 
     * NOTE)
     * This function is one simple version of python-like string formatters,
     * and it doesn't support more complex string formattings.
     * 
     * IMPORTANT)
     * If you want to use some user-defined type that is not supported
     * by default "std::stringstream::operator<<", then the appropriate "<<"
     * operator should be overloaded.
     */
    template <typename ...Args>
    std::string sprintf(const std::string& format_expr, const std::string& sep, Args... args)
    {
        std::stringstream buffer;
        impl::__impl_sprintf(buffer, format_expr, sep, 0, 0, args...);
        return buffer.str();
    }

}   // namespace fmt

#endif   // THROW_H