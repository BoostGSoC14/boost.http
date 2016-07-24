/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_ALGORITHM_HEADER_HPP
#define BOOST_HTTP_ALGORITHM_HEADER_HPP

#include <algorithm>
#include <regex>
#include <type_traits>
#include <limits>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <boost/http/algorithm/header/header_value_any_of.hpp>

namespace boost {
namespace http {

namespace detail {

/* This function was created, because most int <-> string conversion
 * techniques/abstractions can only handle char* or basic_string. They don't
 * support generic strings.
 *
 * Despite this being the critical point to motivate this development, there are
 * other disappointing behaviours with the found abstractions. For instance, I
 * invested a big effort to allocate as little memory on the heap as I could,
 * but most of the abstractions will just use something like standard library
 * string-based stream or other expensive abstractions.
 *
 * However, I'm not advocating the interface developed here is better. This
 * interface is limited to non-negative base-10 conversions and don't properly
 * handle Unicode characters. Even worse, it doesn't check for invalid
 * input.
 *
 * Don't freak out about the lack of valid inputs. This is checked by the
 * regex layer.
 *
 * It's just a workaround to not throw away all the efforts spent in the other
 * layers.*/
template<class Target, class BidirIt>
Target from_decimal_string(BidirIt begin, BidirIt end)
{
    static_assert(std::is_same<
                    typename std::iterator_traits<BidirIt>::value_type,
                    char>::value,
                  "from_decimal_string only supports char type");

    Target ret = 0, digit = 1;
    std::reverse_iterator<BidirIt> it(end), rend(begin);
    for (bool overflow = false;it != rend;++it) {
        Target value = (*it - '0');

        if (value != 0) {
            if (overflow)
                throw std::overflow_error("cannot update digit");

            if (std::numeric_limits<Target>::max() / digit < value)
                throw std::overflow_error("cannot update value");

            value *= digit;

            if (std::numeric_limits<Target>::max() - value < ret)
                throw std::overflow_error("cannot update ret");

            ret += value;
        }

        if (std::numeric_limits<Target>::max() / 10 < digit)
            overflow = true;
        else
            digit *= 10;
    }
    return ret;
}

template<class Target, class BidirIt>
Target from_decimal_submatch(const std::sub_match<BidirIt> &match)
{
    return from_decimal_string<Target>(match.first, match.second);
}

template<class Target, class BidirIt>
Target from_submatch_to_month(std::sub_match<BidirIt> m)
{
    switch (*m.first) {
    case 'A': // "Apr" or "Aug"
        ++m.first;
        return (*m.first == 'p') ? 4 : 8;
    case 'D': // "Dec"
        return 12;
    case 'F': // "Feb"
        return 2;
    case 'J': // "Jan" or "Jun" or "Jul"
        ++m.first;
        if (*m.first == 'a')
            return 1;
        ++m.first;
        return (*m.first == 'n') ? 6 : 7;
    case 'M': // "Mar" or "May"
        ++m.first; ++m.first;
        return (*m.first == 'r') ? 3 : 5;
    case 'N': // "Nov"
        return 11;
    case 'O': // "Oct"
        return 10;
    case 'S': // "Sep"
        return 9;
    default:
        assert(false);
    }
}

template<class StringRef>
bool rfc1123(const StringRef &value, posix_time::ptime &datetime)
{
    using namespace gregorian;
    using namespace posix_time;

    typedef date::year_type::value_type year_type;
    typedef date::month_type::value_type month_type;
    typedef date::day_type::value_type day_type;
    typedef time_duration::hour_type hour_type;
    typedef time_duration::min_type min_type;
    typedef time_duration::sec_type sec_type;

    static const std::basic_regex<typename StringRef::value_type>
        regex("(?:Mon|Tue|Wed|Thu|Fri|Sat|Sun), " // day
              "(\\d{2}) " // day-1
              "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) " // month-2
              "(\\d{4}) " // year-3
              "(\\d{2}):" // hour-4
              "(\\d{2}):" // minutes-5
              "(\\d{2}) " // seconds-6
              "GMT");

    std::match_results<typename StringRef::const_iterator> matches;
    if (!std::regex_match(value.begin(), value.end(), matches, regex))
        return false;

    hour_type hour = from_decimal_submatch<hour_type>(matches[4]);
    min_type min = from_decimal_submatch<min_type>(matches[5]);
    sec_type sec = from_decimal_submatch<sec_type>(matches[6]);

    if (hour > 23 || min > 59 || sec > 60)
        return false;

    try {
        datetime = ptime(date(from_decimal_submatch<year_type>(matches[3]),
                              from_submatch_to_month<month_type>(matches[2]),
                              from_decimal_submatch<day_type>(matches[1])),
                         time_duration(hour, min, sec));
    } catch(std::out_of_range&) {
        return false;
    }

    return true;
}

template<class StringRef>
bool rfc1036(const StringRef &value, posix_time::ptime &datetime)
{
    using namespace gregorian;
    using namespace posix_time;

    typedef date::year_type::value_type year_type;
    typedef date::month_type::value_type month_type;
    typedef date::day_type::value_type day_type;
    typedef time_duration::hour_type hour_type;
    typedef time_duration::min_type min_type;
    typedef time_duration::sec_type sec_type;

    static const std::basic_regex<typename StringRef::value_type>
        regex("(?:Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday), " // day
              "(\\d{2})-" // day-1
              "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)-" // month-2
              "(\\d{2}) " // year-3
              "(\\d{2}):" // hour-4
              "(\\d{2}):" // minutes-5
              "(\\d{2}) " // seconds-6
              "GMT");

    std::match_results<typename StringRef::const_iterator> matches;
    if (!std::regex_match(value.begin(), value.end(), matches, regex))
        return false;

    hour_type hour = from_decimal_submatch<hour_type>(matches[4]);
    min_type min = from_decimal_submatch<min_type>(matches[5]);
    sec_type sec = from_decimal_submatch<sec_type>(matches[6]);

    if (hour > 23 || min > 59 || sec > 60)
        return false;

    try {
        datetime = ptime(date(from_decimal_submatch<year_type>(matches[3])
                              + 1900,
                              from_submatch_to_month<month_type>(matches[2]),
                              from_decimal_submatch<day_type>(matches[1])),
                         time_duration(hour, min, sec));
    } catch(std::out_of_range&) {
        return false;
    }

    return true;
}

template<class StringRef>
bool asctime(const StringRef &value, posix_time::ptime &datetime)
{
    using namespace gregorian;
    using namespace posix_time;

    typedef date::year_type::value_type year_type;
    typedef date::month_type::value_type month_type;
    typedef date::day_type::value_type day_type;
    typedef time_duration::hour_type hour_type;
    typedef time_duration::min_type min_type;
    typedef time_duration::sec_type sec_type;

    static const std::basic_regex<typename StringRef::value_type>
        regex("(?:Mon|Tue|Wed|Thu|Fri|Sat|Sun) " // day
              "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) " // month-1
              "((?:\\d| )\\d) " // day-2
              "(\\d{2}):" // hour-3
              "(\\d{2}):" // minutes-4
              "(\\d{2}) " // seconds-5
              "(\\d{4})" // year-6
              );

    std::match_results<typename StringRef::const_iterator> matches;
    if (!std::regex_match(value.begin(), value.end(), matches, regex))
        return false;

    hour_type hour = from_decimal_submatch<hour_type>(matches[3]);
    min_type min = from_decimal_submatch<min_type>(matches[4]);
    sec_type sec = from_decimal_submatch<sec_type>(matches[5]);

    if (hour > 23 || min > 59 || sec > 60)
        return false;

    try {
        datetime = ptime(date(from_decimal_submatch<year_type>(matches[6]),
                              from_submatch_to_month<month_type>(matches[1]),
                              [&matches]() {
                                  auto m = matches[2];
                                  if (*m.first == ' ')
                                      ++m.first;
                                  return from_decimal_submatch<day_type>(m);
                              }()),
                         time_duration(hour, min, sec));
    } catch(std::out_of_range&) {
        return false;
    }

    return true;
}

template<class String, unsigned N, class Unsigned>
void append_number(String &string, Unsigned value)
{
    char buffer[(N > 0) ? N : 1];
    for (auto i = N;i;--i) {
        buffer[i-1] = '0' + (value % 10);
        value /= 10;
    }
    string.append(buffer, N);
}

} // namespace detail

template<class StringRef>
posix_time::ptime header_to_ptime(const StringRef &value)
{
    using namespace gregorian;
    using namespace posix_time;
    ptime ret(date_time::not_a_date_time);
    if (!detail::rfc1123(value, ret)) {
        if (!detail::rfc1036(value, ret))
            detail::asctime(value, ret);
    }
    return ret;
}

template<class String>
String to_http_date(const posix_time::ptime &datetime)
{
    using detail::append_number;

    typedef gregorian::date::ymd_type::year_type::value_type year_type;
    typedef gregorian::date::ymd_type::day_type::value_type day_type;

    String ret;
    ret.reserve(29);
    ret.append([&datetime]() {
            using namespace gregorian;
            switch(datetime.date().day_of_week()) {
            case Monday: return "Mon, ";
            case Tuesday: return "Tue, ";
            case Wednesday: return "Wed, ";
            case Thursday: return "Thu, ";
            case Friday: return "Fri, ";
            case Saturday: return "Sat, ";
            case Sunday: return "Sun, ";
            default:
                throw std::out_of_range("bad day of week");
            }
        }(), 5);

    {
        gregorian::date::ymd_type ymd = datetime.date().year_month_day();

        append_number<String, 2, day_type>(ret, ymd.day);
        ret.push_back(' ');

        switch (ymd.month) {
        case 1: ret.append("Jan ", 4); break;
        case 2: ret.append("Feb ", 4); break;
        case 3: ret.append("Mar ", 4); break;
        case 4: ret.append("Apr ", 4); break;
        case 5: ret.append("May ", 4); break;
        case 6: ret.append("Jun ", 4); break;
        case 7: ret.append("Jul ", 4); break;
        case 8: ret.append("Aug ", 4); break;
        case 9: ret.append("Sep ", 4); break;
        case 10: ret.append("Oct ", 4); break;
        case 11: ret.append("Nov ", 4); break;
        case 12: ret.append("Dec ", 4); break;
        default:
            throw std::out_of_range("bad month");
        }

        append_number<String, 4, year_type>(ret, ymd.year);
    }
    {
        auto time = datetime.time_of_day();
        ret.push_back(' ');
        append_number<String, 2>(ret, time.hours());
        ret.push_back(':');
        append_number<String, 2>(ret, time.minutes());
        ret.push_back(':');
        append_number<String, 2>(ret, time.seconds());
    }
    ret.append(" GMT", 4);
    return ret;
}

template<class StringRef, class Predicate>
bool header_value_all_of(const StringRef &header_value, const Predicate &p)
{
    typedef typename StringRef::value_type char_type;
    typedef typename StringRef::const_reverse_iterator reverse_iterator;

    auto isspace = [](const char_type &c) {
        return c == ' ' || c == '\t';
    };

    auto comma = header_value.begin();
    decltype(comma) next_comma;
    do {
        next_comma = std::find(comma, header_value.end(), ',');

        auto value_begin = std::find_if_not(comma, next_comma, isspace);

        if (value_begin != next_comma) {
            auto value_end = std::find_if_not(reverse_iterator(next_comma),
                                              reverse_iterator(value_begin),
                                              isspace).base();
            if (value_begin != value_end
                && !p(header_value.substr(value_begin - header_value.begin(),
                                          value_end - value_begin))) {
                return false;
            }
        }

        comma = next_comma;

        /* skip comma, so won't process an empty string in the next iteration
           and enter within an infinite loop afterwards. */
        if (next_comma != header_value.end())
            ++comma;
    } while (comma != header_value.end());
    return true;
}

template<class StringRef, class Predicate>
bool header_value_none_of(const StringRef &header_value, const Predicate &p)
{
    return header_value_all_of(header_value, [&p](const StringRef &s) {
            return !p(s);
        });
}

template<class StringRef, class F>
F header_value_for_each(const StringRef &header_value, F f)
{
    header_value_any_of(header_value, [&f](const StringRef &v) {
            f(v);
            return false;
        });
    return std::move(f);
}

template<class StringRef>
bool etag_match_strong(const StringRef &a, const StringRef &b)
{
    if ((a.size() && a[0] == 'W') || (b.size() && b[0] == 'W'))
        return false;

    return a == b;
}

template<class StringRef>
bool etag_match_weak(const StringRef &a, const StringRef &b)
{
    bool a_is_weak = a.size() > 2 && a[0] == 'W';
    bool b_is_weak = b.size() > 2 && b[0] == 'W';
    return (a_is_weak ? a.substr(2) : a) == (b_is_weak ? b.substr(2) : b);
}

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_ALGORITHM_HEADER_HPP
