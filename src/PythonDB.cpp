#include <python2.7/Python.h>

#include <memory>
#include <iostream>
#include <string>
#include <map>

#include "PythonDB.h"

PyObject *p_loadModule(std::string name) {
    PyObject *pModName = PyString_FromString(name.c_str());
    PyObject *pModule = PyImport_Import(pModName);
    Py_DECREF(pModName);
    return pModule;
}


OpenTSDBInterface::OpenTSDBInterface() {
    Py_Initialize();
    
    PyObject *sys = PyImport_ImportModule("sys");
    PyObject *path = PyObject_GetAttrString(sys, "path");
    PyList_Append(path, PyUnicode_FromString("../python"));
    Py_DECREF(path);
    
    m_pModule = p_loadModule("otsdb_client");
    if (m_pModule == NULL) {
        PyErr_Print();
        throw initialise_error("Failed to load module!");
    }
    
    PyObject *pInit = PyObject_GetAttrString(m_pModule, "OTSDBInterface");
    if (pInit == NULL) {
        PyErr_Print();
        Py_DECREF(m_pModule);
        throw initialise_error("Failed to find class OTSDBInterface!");
    }
 
    m_pInterface = PyObject_CallObject(pInit, NULL);
    if (m_pInterface == NULL) {
        PyErr_Print();
        Py_DECREF(m_pModule);
        Py_DECREF(pInit);
        throw initialise_error("Failed to instantiate class OTSDBInterface!");
    }
    
    Py_DECREF(pInit);
}

std::shared_ptr<TimeSeries> OpenTSDBInterface::addTimeSeries(std::string name, TSTags_t tags) {
    PyObject *pTags = PyDict_New();
    for (const auto& tag: tags) {
        PyObject *pVal = PyString_FromString(tag.second.c_str());
        PyDict_SetItemString(pTags, tag.first.c_str(), pVal);
        Py_DECREF(pVal);
    }

    PyObject *pRet = PyObject_CallMethod(m_pInterface, const_cast<char*>("add_ts"), const_cast<char*>("sO"), name.c_str(), pTags);
    if (pRet == NULL) {
        PyErr_Print();
        std::cerr << "Failed to add time series " << name << std::endl;
        Py_DECREF(pTags);
        return NULL;
    }

    Py_DECREF(pTags);

    return std::shared_ptr<TimeSeries>(new TimeSeries(name, tags, pRet));
}

bool OpenTSDBInterface::putValue(std::shared_ptr<TimeSeries> ts, double val, uint64_t time) {
    if (m_pInterface == NULL) {
        std::cerr << "Interface not valid anymore!" << std::endl;
        return false;
    }
    if (ts->get() == NULL) {
        std::cerr << "Time series not valid anymore!" << std::endl;
        return false;
    }
    
    PyObject *pRet = PyObject_CallMethod(m_pInterface, const_cast<char*>("put_value"), const_cast<char*>("Odk"), ts->get(), val, time);
    if (pRet == NULL) {
        PyErr_Print();
        std::cerr << "Failed to call put_value!" << std::endl;
        return false;
    }
    
    Py_DECREF(pRet);
    
    return true;
}

OpenTSDBInterface::~OpenTSDBInterface() {
    Py_DECREF(m_pInterface);
    Py_DECREF(m_pModule);
    Py_Finalize();
}
