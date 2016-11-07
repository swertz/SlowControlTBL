#pragma once

#include <string>
#include <limits>
#include <ctime>
#include <cstdint>
#include <list>
#include <chrono>
#include <iostream>

#include <json/json.h>

/*
 * Numbers for the scaler channels
 */
enum class ScalerChannel {
    PM0 = 1,
    PM1,
    NIM,
    VME,
    TTC,
    Ileak 
};

/*
 * Small helper class to handle arguments for main
 * Everything is static...
 */
class Arguments {
    public:
        Arguments(int argc, char **argv):
            log_path("./"),
            use_fake_setup(false)
        {
            for (std::size_t i = 1; i < argc; i++)
                parseArgument(argv[i]);
        }

        std::string log_path;
        bool use_fake_setup;

    private:
        void parseArgument(std::string arg) {
            if (arg == "-f" || arg == "--fake") {
                std::cout << "Will use fake setup no matter what." << std::endl;
                use_fake_setup = true;
                return;
            } else if (arg == "-h" || arg == "--help") {
                std::cout << "--- Slow control interface for test beam at Louvain ---\n\n";
                std::cout << "List of available options:\n";
                std::cout << " - '-f'/'--fake': Use fake setup even if real setup is connected (default false)\n";
                std::cout << " - '-h'/'--help': Display this help\n";
                std::cout << " - Unnamed argument: specify path to directory where log files will be stored (fault to current directory)\n\n";
            } else {
                std::cout << "Will write log files to " << arg << std::endl;
                log_path = arg;
            }
        }
};


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
std::uint64_t timeNowStamp(typename T::time_point m_time) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(m_time.time_since_epoch()).count();
}

template<typename T>
Json::UInt64 timeToJson(typename T::time_point m_time) {
    return static_cast<Json::UInt64>(timeNowStamp<T>(m_time));
}

/*
 * Class helping to compute a running average on a set of values
 */

template<typename T>
class MovingAverage {
    public:
        MovingAverage(std::size_t size):
            m_size(size),
            m_avg_buffer(0),
            m_need_update(false) 
            {}

        void add(T val) {
            m_list.push_back(val);
            if (m_list.size() > m_size)
                m_list.pop_front();
            m_need_update = true;
        }

        double operator()() {
            if (!m_need_update)
                return m_avg_buffer;
            m_need_update = false;
            double sum;
            for (auto const& i: m_list)
                sum += i;
            m_avg_buffer = sum / m_list.size();
            return m_avg_buffer;
        }
        
        double operator()(T val) {
            add(val);
            return operator();
        }

        void setSize(std::size_t size) {
            m_size = size;
        }
        
        void clear() {
            m_list.clear();
            m_avg_buffer = 0;
            m_need_update = false;
        }

    private:
        std::list<T> m_list;
        std::size_t m_size;
        double m_avg_buffer;
        bool m_need_update;
};

template<typename T>
class MovingMinimum {
    public:
        MovingMinimum(std::size_t size):
            m_size(size),
            m_min_buffer(0),
            m_need_update(false) 
            {}

        void add(T val) {
            m_list.push_back(val);
            if (m_list.size() > m_size)
                m_list.pop_front();
            m_need_update = true;
        }

        T operator()() {
            if (!m_need_update)
                return m_min_buffer;
            T min = std::numeric_limits<T>::max();
            for (auto const& i: m_list) {
                if (i < min)
                    min = i;
            }
            m_min_buffer = min;
            return min;
        }
        
        T operator()(T val) {
            add(val);
            return operator()();
        }

        void setSize(std::size_t size) {
            m_size = size;
        }
        
        void clear() {
            m_list.clear();
            m_min_buffer = std::numeric_limits<T>::min();
            m_need_update = false;
        }

    private:
        std::list<T> m_list;
        std::size_t m_size;
        T m_min_buffer;
        bool m_need_update;
};

/*
 * Helper class to compute a rate given two measurements A and B at different times: 
 * Returns Cst*(B-A)/(t_B-t_A) where Cst is a conversion constant (default 1)
 */
template<typename T>
class Rate {
    public:
        Rate(double constant=1):
            m_cst(constant),
            first(0),
            last(0)
        {}

        void add(double val, typename T::time_point now=T::now()) {
            first = last;
            last = val;
            first_time = last_time;
            last_time = now;
        }

        double operator()(double val, typename T::time_point now=T::now()) {
            add(val, now);
            return operator()();
        }

        double operator()() {
            uint64_t delta_t_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(last_time - first_time).count();
            if (delta_t_ns == 0)
                return 0;
            return m_cst * (last - first) / (1e9 * delta_t_ns);
        }

    private:
        double m_cst;
        double first, last;
        typename T::time_point first_time, last_time;
};
