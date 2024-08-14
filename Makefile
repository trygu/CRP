######################################################################################
# Makefile for building: Crplib.dll
# use "make 32BIT=true" to compile for 32 bit system
# use "make SOLVER=foo" to compile only for selected LP-solvers
#	    where foo s a comma separated list
#	    Allowed solvers: CP=Cplex, XP=Xpress, SC=SCIP
#	    So e.g. "make SOLVER=SC,CP" would only compile for Cplex and SCIP
######################################################################################

####### Compiler, tools and options
# Environment
CC               = $(GNUDIR)/gcc
CXX              = $(GNUDIR)/g++
WINDRES          = $(GNUDIR)/windres
MKDIR            = mkdir
RM               = rm -f
CP               = cp -p
DEFINES		 = -DDYNAMIC

32BIT            = true
#32BIT           = false

ifeq ($(32BIT), false) # 64 bit assumed
    BITS         = -m64 -D_LP64
    ARCH         = x86_64
    CND_PLATFORM = MinGW-Windows64
    DEFINES      += -Dcplex8
    GNUDIR       = C:/Progra~1/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0/mingw64/bin
else
    BITS         = -m32
    ARCH         = x86
    CND_PLATFORM = MinGW-Windows
    DEFINES      += -Dcplex8
    GNUDIR       = C:/Progra~2/mingw-w64/i686-8.1.0-win32-sjlj-rt_v6-rev0/mingw32/bin
endif

# Macros
CND_DLIB_EXT     = dll
CND_CONF         = Debug
CND_DISTDIR      = dist
CND_BUILDDIR     = build

# Switch to activate debug info in JJ-CRP-files
STAMP            =#-DSTAMP

# Solvers
SOLVER = CP,XP,SC# default is all three
comma:=,
null:=
space:= $(null) #
ifeq (,$(SOLVER)) # if not specified, use defaults
    USEDSOLVERS = CP XP SC
else
    USEDSOLVERS = $(subst $(comma), $(space), $(SOLVER))
endif

# Object Directory
OBJECTDIR        = $(CND_BUILDDIR)/$(CND_CONF)/$(CND_PLATFORM)

####### Object Files
OBJECTS          = $(OBJECTDIR)/src/Cspbridg.o $(OBJECTDIR)/src/Cspcard.o $(OBJECTDIR)/src/Cspmain.o $(OBJECTDIR)/src/Cspnet.o $(OBJECTDIR)/src/Jjsolver.o $(OBJECTDIR)/src/MT1RC.o $(OBJECTDIR)/src/My_time.o $(OBJECTDIR)/src/cspback.o $(OBJECTDIR)/src/cspbranc.o $(OBJECTDIR)/src/cspcapa.o $(OBJECTDIR)/src/cspcover.o $(OBJECTDIR)/src/cspdebug.o $(OBJECTDIR)/src/cspgomo.o $(OBJECTDIR)/src/cspheur.o $(OBJECTDIR)/src/cspprep.o $(OBJECTDIR)/src/cspprice.o $(OBJECTDIR)/src/cspsep.o $(OBJECTDIR)/src/cspsolve.o $(OBJECTDIR)/src/Versioninfo.o

ifeq ($(32BIT),false)
    DIRCPLEX     = ../Solvers/Cplex/Cplex125/Windows/64bits
    CPXLIBS      = -L$(DIRCPLEX) -lcplex125
else
    #DIRCPLEX     = ../Solvers/Cplex/Cplex75
    #CPXLIBS      = -L$(DIRCPLEX)/lib -lcplex75
    DIRCPLEX     = ../Solvers/Cplex/Cplex125/Windows/32bits
    CPXLIBS      = -L$(DIRCPLEX) -lcplex125
endif
CPXINC           = -I$(DIRCPLEX)/include

#DIRXPRESS        = ../Solvers/XPress/XPress_28/$(ARCH)
DIRXPRESS        = ../Solvers/XPress/XPress_19
XPRINC           = -I$(DIRXPRESS)
XPRLIBS          = -L$(DIRXPRESS) -lxprl -lxprs

DIRLPS           = ../Solvers/scip-3.1.1
DIRSOPLEX        = ../Solvers/soplex-2.0.1
SOPLEXLIB        = soplex-2.0.1.mingw.$(ARCH).gnu.opt
NLPILIB          = nlpi.cppad-3.1.1.mingw.$(ARCH).gnu.opt
SCIPLIB          = scip-3.1.1.mingw.$(ARCH).gnu.opt
OBJSCIPLIB       = objscip-3.1.1.mingw.$(ARCH).gnu.opt
LPISPXLIB        = lpispx-3.1.1.mingw.$(ARCH).gnu.opt
SCIPINC          = -I$(DIRLPS)/src -I$(DIRSOPLEX)/src
SCIPLIBS         = -L$(DIRLPS)/lib -L$(DIRSOPLEX)/lib -L$(DIRLPS)/lib -l$(OBJSCIPLIB) -l$(SCIPLIB) -l$(NLPILIB) -l$(LPISPXLIB) -l$(SOPLEXLIB)

ifneq (,$(findstring CP,$(USEDSOLVERS)))
	DEFINES += -DCPLEXV 
	DEFINES += -DBUILD_CPXSTATIC #Needed to circumvent deprication errors
	INCPATH += $(CPXINC)
	LIBS += $(CPXLIBS)
endif
ifneq (,$(findstring XP,$(USEDSOLVERS)))
	DEFINES += -DXPRESSV
	INCPATH += $(XPRINC)
	LIBS += $(XPRLIBS)
endif
ifneq (,$(findstring SC,$(USEDSOLVERS)))
	DEFINES += -DSCIPV
	INCPATH += $(SCIPINC)
	LIBS += $(SCIPLIBS)
endif

####### Object Files
OBJECTS          = $(OBJECTDIR)/src/crpXmain.o $(OBJECTDIR)/src/crpXaudit.o $(OBJECTDIR)/src/crpSmain.o $(OBJECTDIR)/src/crpSaudit.o $(OBJECTDIR)/src/crpCmain.o $(OBJECTDIR)/src/crpCaudit.o $(OBJECTDIR)/src/WrapCRP.o $(OBJECTDIR)/src/Versioninfo.o


CXXFLAGS         = -g -O2 -Wall $(DEFINES) $(BITS)
#CXXFLAGS         = -ggdb -g -Og -Wall $(DEFINES) $(BITS)
LDLIBSOPTIONS    = $(LIBS) -static-libgcc -static-libstdc++

####### Compile
all:
	$(MKDIR) -p $(OBJECTDIR)/src
	$(MKDIR) -p $(CND_DISTDIR)/$(CND_CONF)/$(CND_PLATFORM)
	$(WINDRES) ./src/Versioninfo.rc $(CND_BUILDDIR)/$(CND_CONF)/$(CND_PLATFORM)/src/Versioninfo.o

	$(CC) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTDIR)/src/crpCaudit.o src/crpCaudit.c
	$(CC) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTDIR)/src/crpCmain.o src/crpCmain.c
	$(CC) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTDIR)/src/crpSaudit.o src/crpSaudit.c
	$(CC) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTDIR)/src/crpSmain.o src/crpSmain.c
	$(CC) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTDIR)/src/crpXaudit.o src/crpXaudit.c
	$(CC) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTDIR)/src/crpXmain.o src/crpXmain.c
	$(CC) -c $(CXXFLAGS) $(INCPATH) -o $(OBJECTDIR)/src/WrapCRP.o src/WrapCRP.c
	$(CXX) -o $(CND_DISTDIR)/$(CND_CONF)/$(CND_PLATFORM)/libCRP.$(CND_DLIB_EXT) $(OBJECTS) $(CXXFLAGS) $(LDLIBSOPTIONS) -shared
	$(CP) $(CND_DISTDIR)/$(CND_CONF)/$(CND_PLATFORM)/libCRP.$(CND_DLIB_EXT) ../tauargus
	
clean:
	$(RM) -r $(CND_BUILDDIR)/$(CND_CONF)
	$(RM) $(CND_DISTDIR)/$(CND_CONF)/$(CND_PLATFORM)/libCRP.$(CND_DLIB_EXT)