########################################################################
#                                                                      
#              --- CAEN SpA - Computing Division ---                   
#                                                                      
#   CAENVMElib Software Project                                        
#                                                                      
#   Created  :  March 2004      (Rel. 1.0)                                             
#                                                                      
#   Auth: S. Coluccini                                                 
#                                                                      
########################################################################

CC	=	g++

COPTS	=	-fPIC -DLINUX -Wall 
#COPTS	=	-g -fPIC -DLINUX -Wall 

FLAGS	=	-Wall -s
#FLAGS	=	-Wall

DEPLIBS	=       -l CAENVME -l ncurses -lc -lm

LIBS	=	

INCLUDEDIR =	-I.

OBJS	=	include/Discri.o include/HV.o include/TDC.o include/TTCvi.o include/VmeBoard.o include/VmeController.o include/VmeUsbBridge.o include/CommonDef.o include/Scaler.o


#########################################################################

all	:	libCAEN

clean	:
		/bin/rm -f $(OBJS)  lib/libCAEN.so
		if [ -d lib ]; then rmdir lib; fi

$(OBJS)	:	Makefile

%.o	:	%.cpp
		$(CC) $(COPTS) $(INCLUDEDIR) -c -o $@ $<

libCAEN : $(OBJS)
	if [ ! -d lib ]; then mkdir lib; fi
	$(CC) -shared -o lib/libCAEN.so $(OBJS) $(DEPLIBS)

