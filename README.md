# SlowControlTBL
Slow control interface for Test Beam at Louvain

# Install
- Initialise git repository: `git clone --recursive git@github.com:swertz/SlowControlTBL.git ; cd SlowControlTBL`
- Get CAEN library at http://www.caen.it/jsp/Template2/CaenProd.jsp?parent=38&idmod=689&downloadSoftwareFileId=11059, install (admin rights needed):
```
cd lib
sh install_x64
```
- Compile Martin's library:
```
cd CosmicTrigger
make
```



