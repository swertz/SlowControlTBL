#include "include/VmeUsbBridge.h"
#include "include/TTCvi.h"
#include "include/Discri.h"
#include "include/HV.h"
#include "include/Scaler.h"

#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <unistd.h>
#include<ctime>

long int getDT(struct timeval start,struct timeval stop){
//    cout<<"Start = "<<start.tv_sec<<"."<<start.tv_usec<<endl;
//    cout<<"Stop  = "<<stop.tv_sec<<"."<<stop.tv_usec<<endl;
    return(1000*(stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec)/1000);
}

int main(){
  
  
  
  UsbController myCont(NORMAL);
  ttcVi *myTTC = new ttcVi(&myCont);
  scaler *s = new scaler(&myCont,0xCCCC00);
  
  myTTC->changeChannel(-1);
  myTTC->changeRandomFrequency(1);

  myTTC->resetCounter(); 
  cout<<"Starting..."<<endl;
  cout<<"Testing communication speed..."<<endl;
  for (int i=0; i<5; i++){
      struct timeval start,stop;
      gettimeofday(&start,NULL);
      for (int j=0; j<5000; j++){myTTC->getEventNumber();}
      gettimeofday(&stop,NULL);
      long int dT = getDT(start,stop);
      cout<< "Time to read 5000 values = "<<dT<<"ms "<<"rate = "<<5000./(1.*dT)<<"kHz"<<endl;
  }
  cout<<"done !"<<endl;

  cout<<"Testing random frequencies"<<endl;

  for (int i=0; i<5; i++){
      struct timeval start,stop;
      myTTC->changeRandomFrequency(i);
      int nStart = myTTC->getEventNumber();
      gettimeofday(&start,NULL);

      sleep(30);
      int nStop = myTTC->getEventNumber();
      gettimeofday(&stop,NULL);
      long int dT = getDT(start,stop);
      cout<< "Trig setting = "<<i<<"Ntrig ="<<nStop-nStart<<"    Time = "<<dT<<"ms "<<"    -> rate = "<<(nStop-nStart)/(1.*dT)<<"kHz"<<endl;
  }
}

