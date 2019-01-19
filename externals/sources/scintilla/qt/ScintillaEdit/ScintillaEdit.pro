#-------------------------------------------------
#
# Project created by QtCreator 2011-05-05T12:41:23
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ScintillaEdit
TEMPLATE = lib
CONFIG += lib_bundle
CONFIG += embed_manifest_dll

unix {
    # <regex> requires C++11 support
    greaterThan(QT_MAJOR_VERSION, 4){
        CONFIG += c++11
    } else {
        QMAKE_CXXFLAGS += -std=c++0x
    }
}

VERSION = 3.7.5

CONFIG (debug, debug|release) {
    SCINTILLA_EDIT_LIBDIR = $$PWD/lib/unknow/debug

    msvc:SCINTILLA_EDIT_LIBDIR = $$PWD/lib/msvc/debug
    mingw:SCINTILLA_EDIT_LIBDIR = $$PWD/lib/mingw/debug

} else {
    SCINTILLA_EDIT_LIBDIR = $$PWD/lib/unknow/release

    msvc:SCINTILLA_EDIT_LIBDIR = $$PWD/lib/msvc/release
    mingw:SCINTILLA_EDIT_LIBDIR = $$PWD/lib/mingw/release
}

msvc:QMAKE_CXXFLAGS += /FS

DESTDIR = $$SCINTILLA_EDIT_LIBDIR
#DLLDESTDIR = $$SCINTILLA_EDIT_LIBDIR/bin

SOURCES += \
    ScintillaEdit.cpp \
    ScintillaDocument.cpp \
    ../ScintillaEditBase/PlatQt.cpp \
    ../ScintillaEditBase/ScintillaQt.cpp \
    ../ScintillaEditBase/ScintillaEditBase.cpp \
    ../../src/XPM.cxx \
    ../../src/ViewStyle.cxx \
    ../../src/UniConversion.cxx \
    ../../src/Style.cxx \
    ../../src/Selection.cxx \
    ../../src/ScintillaBase.cxx \
    ../../src/RunStyles.cxx \
    ../../src/RESearch.cxx \
    ../../src/PositionCache.cxx \
    ../../src/PerLine.cxx \
    ../../src/MarginView.cxx \
    ../../src/LineMarker.cxx \
    ../../src/KeyMap.cxx \
    ../../src/Indicator.cxx \
    ../../src/ExternalLexer.cxx \
    ../../src/EditView.cxx \
    ../../src/Editor.cxx \
    ../../src/EditModel.cxx \
    ../../src/Document.cxx \
    ../../src/Decoration.cxx \
    ../../src/ContractionState.cxx \
    ../../src/CharClassify.cxx \
    ../../src/CellBuffer.cxx \
    ../../src/Catalogue.cxx \
    ../../src/CaseFolder.cxx \
    ../../src/CaseConvert.cxx \
    ../../src/CallTip.cxx \
    ../../src/AutoComplete.cxx \
    ../../lexlib/WordList.cxx \
    ../../lexlib/StyleContext.cxx \
    ../../lexlib/PropSetSimple.cxx \
    ../../lexlib/LexerSimple.cxx \
    ../../lexlib/LexerNoExceptions.cxx \
    ../../lexlib/LexerModule.cxx \
    ../../lexlib/LexerBase.cxx \
    ../../lexlib/CharacterSet.cxx \
    ../../lexlib/CharacterCategory.cxx \
    ../../lexlib/Accessor.cxx \
    ../../lexers/*.cxx

HEADERS  += \
    ScintillaEdit.h \
    ScintillaDocument.h \
    ../ScintillaEditBase/ScintillaEditBase.h \
    ../ScintillaEditBase/ScintillaQt.h

OTHER_FILES +=

INCLUDEPATH += ../ScintillaEditBase ../../include ../../src ../../lexlib

DEFINES += SCINTILLA_QT=1 MAKING_LIBRARY=1 SCI_LEXER=1 _CRT_SECURE_NO_DEPRECATE=1
CONFIG(release, debug|release) {
    DEFINES += NDEBUG=1
}

macx {
	QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/
}
