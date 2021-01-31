#include "date/date.h"
#include <iostream>
#include <chrono>
#include <sstream>

using namespace std;
using namespace date;

auto ask_date(const string& order) {
    string in;
    year_month_day date{};
    // need to flush here in order to get proper cursor behaviour
    cout << "Enter " << order << " date (year/month/day): " << endl;
    // prefer `getline` over `cin`:
    // http://www.cplusplus.com/forum/articles/6046/
    getline(cin, in);
    // https://stackoverflow.com/a/41613816/3486684
    istringstream(in) >> parse("%Y/%m/%d", date);
    return date;
}

auto days_in_date_year(year_month_day& ymd) {
    // see https://github.com/HowardHinnant/date/issues/178
    auto year = ymd.year();
    auto next_year = year + years(1);
    sys_days first_of_year = January/1/year;
    sys_days first_of_next = January/1/next_year;
    // Calculate the number of days this year for a date using
    // Howard Hinnant's date.h: https://stackoverflow.com/a/57617969/3486684
    return first_of_next - first_of_year;
}

auto day_of_date_year(year_month_day& ymd) {
    sys_days first_of_year = January / 1 / ymd.year();
    sys_days this_day = ymd;
    return (this_day - first_of_year);
}

int main() {
    auto d1 = ask_date("first");
    sys_days tp1 = d1;
    auto d2 = ask_date("second");
    sys_days tp2 = d2;

    auto days_in_y1 = days_in_date_year(d1);
    cout << "Number of days in year " << "(" << d1.year() << ")" << " of date "
    << d1 << ": " << days_in_y1 << ".\n";

    auto day_num_y1 = day_of_date_year(d1);
    cout << d1 << " is day " << day_num_y1.operator++() << " of year " << d1.year()
    << ".\n";

    cout << "Days between dates " << d1 << " and " << d2 << ": "
         << tp2 - tp1 << ".\n";;
}
