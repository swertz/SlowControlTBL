#pragma once

#include <python2.7/Python.h>
#include <string>
#include <map>

PyObject *p_loadModule(std::string name);

typedef std::map<std::string, std::string> TSTags_t;

class TimeSeries {
    public:
        TimeSeries(std::string name, TSTags_t tags, PyObject* pTS):
            m_name(name),
            m_tags(tags),
            m_pTS(pTS)
        {}
        ~TimeSeries() { Py_DECREF(m_pTS); }

        PyObject* get() { return m_pTS; }
        
    private:
        const std::string m_name;
        const TSTags_t m_tags;
        PyObject* m_pTS;
};

/*
 * Basic interface to the Python client for OpenTSDB
 *
 * This class loads the module 'python/otsdb_client.py', takes care of creating an instance of the OTSDBInterface python class, and forwarding requests to it.
 */
class OpenTSDBInterface {
    public:
        class initialise_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        /*
         * Create client and connect to server
         * The server address and port are specified in 'python/otsdb_client.py'
         */
        OpenTSDBInterface();
        ~OpenTSDBInterface();

        /*
         * Add a time series with name @name and a number of tags (@tags)
         * \return Pointer to a TimeSeries instance
         */
        std::shared_ptr<TimeSeries> addTimeSeries(std::string name, TSTags_t tags);
        
        /*
         * Insert a measurement @val into a time series @ts, at time @time
         */
        bool putValue(std::shared_ptr<TimeSeries> ts, int64_t val, uint64_t time);


    private:

        PyObject *m_pModule;
        PyObject *m_pInterface;
};
