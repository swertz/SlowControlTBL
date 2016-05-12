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

EXE	=	main

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

all	:	$(EXE)

clean	:
		/bin/rm -f $(OBJS) main.o $(EXE) libCAEN.so

$(EXE)	:	$(OBJS) main.o
		/bin/rm -f $(EXE)
		$(CC) $(FLAGS) -o $(EXE) $(OBJS) main.o $(DEPLIBS)

$(OBJS)	:	Makefile

main.o	:	Makefile

%.o	:	%.cpp
		$(CC) $(COPTS) $(INCLUDEDIR) -c -o $@ $<

libCAEN : $(OBJS)
	$(CC) -shared -o lib/libCAEN.so $(OBJS) $(DEPLIBS)

