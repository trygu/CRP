######################################################################################
# Makefile for building: Crplib.dll
# Note: use LPS = CLP or SOPLEX
######################################################################################

####### Compiler, tools and options
CC            = gcc
CXX           = g++
DEFINES       = -DCPLEXV -DSCIPV -DXPRESSV -DDYNAMIC 

#ifeq ($(PROCESSOR_ARCHITECTURE), AMD64) # 64bit windows detected
#    DEFINES = -DCPLEXV -DSCIPV -DXPRESSV -DDYNAMIC -D_LP64
#endif

CXXFLAGS      = -ggdb -Wall $(DEFINES)  
RM = rm -f
MKDIR=mkdir

# Macros
#CND_PLATFORM=MinGW_64-Windows
CND_PLATFORM=MinGW-Windows
CND_DLIB_EXT=dll
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Cplex
DIRCPLEX	  	= ../Cplex/Cplex122

#XPress
DIRXPRESS 	  	= ../Xpress

INCPATH1    = -I$(DIRCPLEX)/include -I$(DIRXPRESS)
LIBS1       = -L$(DIRCPLEX)/lib -lcplex122 -L$(DIRXPRESS) -lxprl -lxprs

# SCIP with CLP
ifeq ($(LPS),CLP)
DIRLPS	    = E:/W7/Users/Peter-Paul/MyDocuments/Thuiswerk/Argus/JJSource/Clp-1.15.5
LIBS        = -L$(DIRLPS)/lib -lscip -llpiclp -lnlpi.cppad -lobjscip -L$(DIRLPS)/lib/clp.mingw.x86.gnu.opt/lib -lClp -lCoinUtils
INCPATH     = -I$(DIRLPS)/src -I$(DIRLPS)/lib/clp.mingw.x86.gnu.opt/Clp/src
endif

# SCIP with soplex
#ifeq ($(LPS),SOPLEX)
DIRLPS	    = ../scip-3.1.1
DIRSOPLEX   = ../soplex-2.0.1
SOPLEXLIB   = soplex-2.0.1.mingw.x86.gnu.opt
NLPILIB     = nlpi.cppad-3.1.1.mingw.x86.gnu.opt
SCIPLIB     = scip-3.1.1.mingw.x86.gnu.opt
OBJSCIPLIB  = objscip-3.1.1.mingw.x86.gnu.opt
LPISPXLIB   = lpispx-3.1.1.mingw.x86.gnu.opt

LIBS        = -L$(DIRLPS)/lib -l$(SCIPLIB) -l$(LPISPXLIB) -L$(DIRSOPLEX)/lib -l$(SOPLEXLIB) -L$(DIRLPS)/lib -l$(NLPILIB) -l$(OBJSCIPLIB)
INCPATH     = -I$(DIRLPS)/src -I$(DIRSOPLEX)/src
#endif

####### Object Files
OBJECTS       = ${OBJECTDIR}/src/crpXmain.o ${OBJECTDIR}/src/crpXaudit.o ${OBJECTDIR}/src/crpSmain.o ${OBJECTDIR}/src/crpSaudit.o ${OBJECTDIR}/src/crpCmain.o ${OBJECTDIR}/src/crpCaudit.o ${OBJECTDIR}/src/WrapCRP.o

LDLIBSOPTIONS  = $(LIBS) $(LIBS1) -static-libgcc -static-libstdc++

####### Compile
${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libCRP.${CND_DLIB_EXT}: ${OBJECTS} 
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	$(CXX) -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libCRP.${CND_DLIB_EXT} ${OBJECTS} $(CXXFLAGS) ${LDLIBSOPTIONS} -shared
	cp ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libCRP.${CND_DLIB_EXT} ../tauargus

${OBJECTDIR}/src/crpCaudit.o: src/crpCaudit.c src/crpdefns.h
	${MKDIR} -p ${OBJECTDIR}/src
	$(CC) -c $(CXXFLAGS) $(INCPATH) $(INCPATH1) -o ${OBJECTDIR}/src/crpCaudit.o src/crpCaudit.c

${OBJECTDIR}/src/crpCmain.o: src/crpCmain.c src/crpmain.h src/crpglob.h src/crpdefns.h
	${MKDIR} -p ${OBJECTDIR}/src
	$(CC) -c $(CXXFLAGS) $(INCPATH) $(INCPATH1) -o ${OBJECTDIR}/src/crpCmain.o src/crpCmain.c

${OBJECTDIR}/src/crpSaudit.o: src/crpSaudit.c src/crpdefns.h
	${MKDIR} -p ${OBJECTDIR}/src
	$(CC) -c $(CXXFLAGS) $(INCPATH) $(INCPATH1) -o ${OBJECTDIR}/src/crpSaudit.o src/crpSaudit.c

${OBJECTDIR}/src/crpSmain.o: src/crpSmain.c src/crpmain.h src/crpglob.h src/crpdefns.h
	${MKDIR} -p ${OBJECTDIR}/src
	$(CC) -c $(CXXFLAGS) $(INCPATH) $(INCPATH1) -o ${OBJECTDIR}/src/crpSmain.o src/crpSmain.c

${OBJECTDIR}/src/crpXaudit.o: src/crpXaudit.c src/crpdefns.h
	${MKDIR} -p ${OBJECTDIR}/src
	$(CC) -c $(CXXFLAGS) $(INCPATH) $(INCPATH1) -o ${OBJECTDIR}/src/crpXaudit.o src/crpXaudit.c

${OBJECTDIR}/src/crpXmain.o: src/crpXmain.c src/crpmain.h src/crpglob.h src/crpdefns.h
	${MKDIR} -p ${OBJECTDIR}/src
	$(CC) -c $(CXXFLAGS) $(INCPATH) $(INCPATH1) -o ${OBJECTDIR}/src/crpXmain.o src/crpXmain.c

${OBJECTDIR}/src/WrapCRP.o: src/WrapCRP.c src/WrapCRP.h
	${MKDIR} -p ${OBJECTDIR}/src
	$(CC) -c $(CXXFLAGS) $(INCPATH) $(INCPATH1) -o ${OBJECTDIR}/src/WrapCRP.o src/WrapCRP.c

clean:
	$(RM) -r ${CND_BUILDDIR}/${CND_CONF}
	$(RM) ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libCRP.${CND_DLIB_EXT}

	#rm *.o
