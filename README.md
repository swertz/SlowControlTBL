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
- Build: `mkdir build; cd build; cmake ..; make -j 4`.
- **NB**: If on cmslab computer, do:
   - `source /home/xtaldaq/software/root-gh-master/builddir/bin/thisroot.sh` (to be run each time you'll run the interface)
   - `cmake3 .. -DCMAKE_PREFIX_PATH="/home/xtaldaq/software/root-gh-master/" -DPYTHON_INCLUDE_DIR="/usr/include/python2.7/" -DPYTHON_LIBRARY="/usr/lib64/libpython2.7.so" -DPYTHON_EXECUTABLE="/usr/bin/python2.7"`

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
- To check that HBase is running, run `jps`: you should see a line containing `HMaster`
- **NB**: On cmslab machine, make sure the file `/etc/hosts` contains the line `127.0.0.1 psiwks1.fynu.ucl.ac.be`. HBase has trouble starting if it's not the case.

### OpenTSDB
- Download from https://github.com/OpenTSDB/opentsdb/releases
- Install
- Set up DB: `env COMPRESSION=NONE HBASE_HOME=/home/xtaldaq/software/hbase-1.2.3/ /usr/share/opentsdb/tools/create_table.sh` (the folder where HBase is installed might have to be changed) (only needs to be run once)
- Run daemon: `sudo tsdb tsd &> /dev/null &`
- To make sure it is running properly, open in a browser `localhost:4242`: you should see a page with the OpenTSDB logo.
- **NB**: To start a browser from remote on cmslab computer, run `firefox --no-remote &> /dev/null &`

### OpenTSDB Python client
- Install: `sudo pip install git+https://github.com/delaere/python-opentsdbclient.git`
- **NB**: On cmslab machine, you need to do: `sudo python2.7 /usr/bin/pip install git+https://github.com/delaere/python-opentsdbclient.git`

### Grafana
- Install from http://docs.grafana.org/installation/
- Start service: `sudo service grafana-server start` (on Ubuntu: `sudo systemctl start grafana-server`)
- To check it is running: `sudo service grafana-server status` (`sudo systemctl status grafana-server`)

You should now be able to open in a browser `localhost:3000` and see the Grafana interface. 

### Configure Grafana
- If prompt to login, simply use 'admin'/'admin'
- Click on 'home', 'import', and select the `json` file in `grafana` directory of SlowControlTBL
- Click on the grafana logo, 'Data Sources', 'Add data source', then:
   - Name: 'SlowControlTSDB'
   - Type: 'OpenTSDB'
   - Url: 'localhost:4242'
   - Access: 'proxy'
   - Version: '2.2'
   - Save & test

You can now go back to the dashboard displaying the metrics. Remember to save any modifications made to the `json` in the SlowControlTBL repository!
