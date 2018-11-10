# libautomationserver source files

# search path for source files
VPATH = ../../

# source files
SRC =  XAutomationServer.cpp XVideoSourceProcessingGraph.cpp XVideoSourceProcessingStep.cpp

# additional include folders
INCLUDES = -I../../../../afx/afx_types -I../../../../afx/afx_types+ \
	-I../../../../afx/afx_imaging -I../../../iplugin \
	-I../../../../afx/afx_platform+ \
	-I../../../pluginmgr
