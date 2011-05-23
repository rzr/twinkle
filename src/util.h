/*
    Copyright (C) 2005-2008  Michel de Boer <michel@twinklephone.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _UTIL_H
#define _UTIL_H

/**
 * @file
 * Utility functions
 */

#include <vector>
#include <string>

using namespace std;

string random_token(int length);
string random_hexstr(int length);

/**
 * Convert a float to a string.
 * @param f [in] Float to convert.
 * @param precision [in] Number of digits after the decimal point in output.
 * @return String representation of the float.
 */
string float2str(float f, int precision);

// Convert an int to a string. format is a printf format
string int2str(int i, const char *format);
string int2str(int i);

// Convert a ulong to a string. format is a printf format
string ulong2str(unsigned long i, const char *format);
string ulong2str(unsigned long i);

// Convert a pointer to a string (hexadecimal)
string ptr2str(void *p);

// Convert a bool to a string: "false", "true"
string bool2str(bool b);

// Convert time/date to string
// The format parameter is a strftime() format string
string time2str(time_t t, const char *format);
string current_time2str(const char *format);

string weekday2str(int wkday);
string month2str(int month);

// Convert a full month name to an int (0-11)
int str2month_full(const string &month);

// Convert a duration in seconds to a string with hours, minutes seconds.
// The hours and minutes are only present if there is at least 1 hour/minute.
// E.g. 65s -> "1m 5s"
//      3601s -> "1h 0m 1s"
string duration2str(unsigned long seconds);

// Convert a timer in seconds to a string (h:mm:ss)
string timer2str(unsigned long seconds);

// Convert a hex string to an integer
unsigned long hex2int(const string &h);

// Convert a string to lower case
string tolower(const string &s);

// Convert a string to upper case
string toupper(const string &s);

// Trim a string
string rtrim(const string &s);
string ltrim(const string &s);
string trim(const string &s);

// Compare 2 strings case insensive, return
// -1 --> s1 < s2
// 0  --> s1 == s2
// 1  --> s1 > s2
int cmp_nocase(const string &s1, const string &s2);

// Return true if a string must be quoted in text encoding
bool must_quote(const string &s);

// Escape character c in string by prepending it with a backslash.
// Backslashed are automatically escaped as well
string escape(const string &s, char c);

// Unescape a string
string unescape(const string &s);

// Escape reserved chars in s by there hex-notation (%HEX)
// All chars that are not in unreserved are considered as reserved.
string escape_hex(const string &s, const string &unreserved);

// Unescape the hex-values in a string
string unescape_hex(const string &s);

// Replace all occurrences of 'from' char 'to' char in s
string replace_char(const string &s, char from, char to);

// Replace first occurrence of 'from'-string to 'to'-string in s
string replace_first(const string &s, const string &from, const string &to);

// Split a string into elements using c as a separator
vector<string> split(const string &s, char c);

// Split a string into elements using separator as a separator
vector<string> split(const string &s, const string &separator);

// Split a string into elements using line breaks as seperator
// If the string contains a CRLF, then CRLF is used as line break.
// Otherwise if the string contains a CR, then CR is used as line break.
// Otherwise LF is used as line break.
vector<string> split_linebreak(const string &s);

// Split a string in two on the first occurence of the separator c.
vector<string> split_on_first(const string &s, char c);

// Split an escaped string into elements using c as a separator
// Escaped means: \c will not be seen as a seperator and backslash is
//                escaped itself (\\)
vector<string> split_escaped(const string &s, char c);

// Split a string into elements using spaces as separator
// If quote_sensitive = true, then spaces within quoted strings will
// not be used to split the string.
vector<string> split_ws(const string &s, bool quote_sensitive = false);

/**
 * Join a vector of strings into one string.
 * @param v Vector of strings.
 * @param separator String to be inserted between the strings to join.
 * @return A string containing the concatenarion of all strings in v.
 *         The invidual strings are separated by separator.
 */
string join_strings(const vector<string> &v, const string &separator);

// Remove surrounding quotes of a string if present.
string unquote(const string &s);

// Check if a string is a number
bool is_number(const string &s);

// Check if a string is an IP address
bool is_ipaddr(const string &s);

// Conversion between yes/no values and bool
bool yesno2bool(const string &yesno);
string bool2yesno(bool b);

// Convert a text string to DTMF digits
// Characters that cannot be converted will be removed
string str2dtmf(const string &s);

// Return true if string s looks like a phone number
// A string looks like a phone number if it consists of digits,
// *, #, special symbols and white space
bool looks_like_phone(const string &s, const string &special_symbols);

// Remove all special symbols from a string
string remove_symbols(const string &s, const string &special_symbols);

// Remove spaces and tabs from a string
string remove_white_space(const string &s);

// Truncate a string. If the string was longer than the truncated
// result, then "..." will be appended.
string dotted_truncate(const string &s, int len);

// Convert a string to a printable representation, i.e. change
// all non-printable chars into dots
string to_printable(const string &s);

/**
 * Get the error message describing an error number.
 * @param errnum [in] The error number.
 * @return The error message.
 */
string get_error_str(int errnum);

#endif
