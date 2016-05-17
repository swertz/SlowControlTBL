#pragma once

#include <string>
#include <ctime>
#include <json/json.h>

/*
 * Helper functions for time management
 *
 */

template<typename T>
inline std::string timeNowString() {
    std::time_t c_time = T::to_time_t(T::now());
    std::string time_str(ctime(&c_time));
    return time_str.substr(0, time_str.size() - 1);
}

template<typename T>
inline std::string timeToString(typename T::time_point m_time) {
    std::time_t c_time = T::to_time_t(m_time);
    std::string time_str(ctime(&c_time));
    return time_str.substr(0, time_str.size() - 1);
}

template<typename T>
Json::UInt64 timeToJson(typename T::time_point m_time) {
    return static_cast<Json::UInt64>(m_time.time_since_epoch().count());
}
