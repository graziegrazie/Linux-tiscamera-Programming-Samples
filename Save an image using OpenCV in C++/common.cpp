#include <common.h>
#include <sstream>

bool make_directory_with_current_date_as_name(std::string& directory_name)
{
    bool result = false;

    time_t t = time(nullptr);
    const tm* lt = localtime(&t);

    std::stringstream ss;
    ss << "20";
    ss << lt->tm_year - 100; ss << "-";
    ss << std::setfill('0') << std::setw(2) << lt->tm_mon  + 1; ss << "-";
    ss << std::setfill('0') << std::setw(2) << lt->tm_mday;     ss << "-";
    ss << std::setfill('0') << std::setw(2) << lt->tm_hour;     ss << "-";
    ss << std::setfill('0') << std::setw(2) << lt->tm_min;      ss << "-";
    ss << std::setfill('0') << std::setw(2) << lt->tm_sec;

    directory_name = ss.str();

    // TODO: permission should be masked
    if( -1 == mkdir(directory_name.c_str(), 0777) )
    {
        result = false;
    }
    else
    {
        result = true;
    }

    return result;
}
