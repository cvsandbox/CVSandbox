# afx_platform+ source files

# search path for source files
VPATH = ../../ ../../internal

# source files
SRC =  XMutex.cpp XThread.cpp XManualResetEvent.cpp XTimer.cpp \
       XMutexImpl_Win32.cpp XThreadImpl_Win32.cpp XManualResetEventImpl_Win32.cpp XTimerImpl_Win32.cpp

# additional include folders
INCLUDES += -I../../../afx_types -I../../../afx_types+
