import time
import random
from opentsdbclient.client import RESTOpenTSDBClient
from opentsdbclient.opentsdbobjects import OpenTSDBMeasurement
from opentsdbclient.opentsdbobjects import OpenTSDBTimeSeries

def get_ts_hash(name, tags):
    return str(name) + str(set(tags.items()))

class OTSDBInterface:
    def __init__(self):
        self.client = RESTOpenTSDBClient("localhost", 4242)
        print "-- OpenTSDB -- Client initialized successfully!"

    def add_ts(self, name, tags):
        ts = OpenTSDBTimeSeries(name, tags)
        ts.assign_uid(self.client)
        print "-- OpenTSDB -- Successfully added timeseries for {} with tags {}".format(name, tags)
        return ts

    def put_value(self, ts, val, m_time = 1000*time.time(), verbose=False):
        meas = OpenTSDBMeasurement(ts, int(m_time), val)
        result = self.client.put_measurements([meas])
        
        if verbose:
            print "-- OpenTSDB -- Successfully put value {} at time {}!".format(val, m_time)
