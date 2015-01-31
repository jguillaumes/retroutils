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
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/BasicHandler.o \
	${OBJECTDIR}/DecnetListener.o \
	${OBJECTDIR}/DecnetListenerPcap.o \
	${OBJECTDIR}/FileSaver.o \
	${OBJECTDIR}/PcapReader.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-Os
CXXFLAGS=-Os

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lpcap

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/decnetlistenerplus

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/decnetlistenerplus: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/decnetlistenerplus ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/BasicHandler.o: BasicHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/BasicHandler.o BasicHandler.cpp

${OBJECTDIR}/DecnetListener.o: DecnetListener.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DecnetListener.o DecnetListener.cpp

${OBJECTDIR}/DecnetListenerPcap.o: DecnetListenerPcap.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DecnetListenerPcap.o DecnetListenerPcap.cpp

${OBJECTDIR}/FileSaver.o: FileSaver.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FileSaver.o FileSaver.cpp

${OBJECTDIR}/PcapReader.o: PcapReader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PcapReader.o PcapReader.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/decnetlistenerplus

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
