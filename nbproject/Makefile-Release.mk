#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=MinGW-Windows
CND_DLIB_EXT=dll
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/WrapCRP.o \
	${OBJECTDIR}/src/crpCaudit.o \
	${OBJECTDIR}/src/crpCmain.o \
	${OBJECTDIR}/src/crpSaudit.o \
	${OBJECTDIR}/src/crpSmain.o \
	${OBJECTDIR}/src/crpXaudit.o \
	${OBJECTDIR}/src/crpXmain.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libCRP.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libCRP.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libCRP.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared

${OBJECTDIR}/src/WrapCRP.o: src/WrapCRP.c
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/WrapCRP.o src/WrapCRP.c

${OBJECTDIR}/src/crpCaudit.o: src/crpCaudit.c
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/crpCaudit.o src/crpCaudit.c

${OBJECTDIR}/src/crpCmain.o: src/crpCmain.c
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/crpCmain.o src/crpCmain.c

${OBJECTDIR}/src/crpSaudit.o: src/crpSaudit.c
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/crpSaudit.o src/crpSaudit.c

${OBJECTDIR}/src/crpSmain.o: src/crpSmain.c
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/crpSmain.o src/crpSmain.c

${OBJECTDIR}/src/crpXaudit.o: src/crpXaudit.c
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/crpXaudit.o src/crpXaudit.c

${OBJECTDIR}/src/crpXmain.o: src/crpXmain.c
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/crpXmain.o src/crpXmain.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
