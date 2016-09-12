# SlowControlTBL
Slow control interface for Test Beam at Louvain

## Install
- Prerequisites:
   - libncurses
   - libqt5
   - cmake >= 2.8
   - pthread
   - libjsoncpp
   - python >= 2.6
   - ROOT >= 6
   - python-opentsdbclient (see below)
- Initialise git repository: `git clone https://github.com/swertz/SlowControlTBL ; cd SlowControlTBL`
- Get [CAEN library](http://www.caen.it/jsp/Template2/CaenProd.jsp?parent=38&idmod=689&downloadSoftwareFileId=11059), install (admin rights needed): `cd lib; sh install_x64`
- Compile Martin's library: `pushd CosmicTrigger; make; popd`
- Build: `mkdir build; cd build; cmake ..; make -j 4`. NB: if on cmslab computer, use `cmake28` instead of `cmake`

## Setting up the database
Instructions to set up the database for logging conditions and displaying in-browser in real time (NOT required to run the interface!).

Once everything is installed, one should make sure HBase and the OpenTSDB daemon are running, otherwise the interface will not be able to connect to the databse.

### HBase
- Install hbase: https://hbase.apache.org/book.html#quickstart
- Edit `conf/hbase-env.sh`: set `JAVA_HOME=/usr`
- Edit `conf/hbase-site.xml`:
```
<configuration>
  <property>
    <name>hbase.rootdir</name>
    <value>file://~/tmp/hbase/hbase</value>
  </property>
  <property>
    <name>hbase.zookeeper.property.dataDir</name>
    <value>~/tmp/hbase/zookeeper</value>
  </property>
</configuration>
```
- Run server: `bin/start-hbase.sh`
- To check that HBase is running, run `jps`: you should see a line contaning `HMaster`

### OpenTSDB
- Download from https://github.com/OpenTSDB/opentsdb/releases
- Install
- Set up DB: `env COMPRESSION=NONE HBASE_HOME=(folder where hbase was extracted) /usr/share/opentsdb/tools/create_table.sh`
- Run daemon: `sudo tsdb tsd`
- To make sure it is running properly, open in a browser `localhost:3000`: you should see a page with the OpenTSDB logo.

### Grafana
- Install from http://docs.grafana.org/installation/
- Start service: `sudo service grafana-server start` (on Ubuntu: `sudo systemctl start grafana-server`)
- To check it is running: `service grafana-server status` (`systemctl status grafana-server`)

### OpenTSDB Python client
- Install: `sudo -H pip install git+https://github.com/delaere/python-opentsdbclient.git`


You should now be able to open in a browser `localhost:4242` and see the Grafana interface. 

FIXME: explain how to load grafana configuration
