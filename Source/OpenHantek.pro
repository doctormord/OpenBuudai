TEMPLATE = app

# Configuration
CONFIG += warn_on \
    qt
QT += opengl printsupport


INCLUDEPATH += C:/Qt/lib/libusb/include/ # Find .h files
LIBS += -LC:/Qt/lib/libusb/MS32/dll/ -llibusb-1.0 # Find .lib files
# LIBS += -LC:/Qt/lib/libusb/MS32/dll/ -lusb-1.0 # Find .lib files Linux Build

INCLUDEPATH += C:/Qt/lib/fftw-3.3.4-dll32 # Find .h files
LIBS += -LC:/Qt/lib/fftw-3.3.4-dll32/ -lfftw3-3 # Find .lib files
# LIBS += -LC:/Qt/lib/fftw-3.3.4-dll32/ -lfftw3 # Find .lib files Linux Build

# Source files
SOURCES += src/colorbox.cpp \
    src/configdialog.cpp \
    src/configpages.cpp \
    src/dataanalyzer.cpp \
    src/dockwindows.cpp \
    src/dsocontrol.cpp \
    src/dsowidget.cpp \
    src/exporter.cpp \
    src/glgenerator.cpp \
    src/glscope.cpp \
    src/helper.cpp \
    src/levelslider.cpp \
    src/main.cpp \
    src/openhantek.cpp \
    src/settings.cpp \
    src/hantek/hantek_control.cpp \
    src/hantek/hantek_device.cpp \
    src/hantek/hantek_types.cpp \
    src/buudai/buudai_control.cpp \
    src/buudai/buudai_device.cpp \
    src/buudai/buudai_types.cpp \
    src/dso.cpp
HEADERS += src/colorbox.h \
    src/configdialog.h \
    src/configpages.h \
    src/dataanalyzer.h \
    src/dockwindows.h \
    src/dsocontrol.h \
    src/dsowidget.h \
    src/exporter.h \
    src/glscope.h \
    src/glgenerator.h \
    src/helper.h \
    src/levelslider.h \
    src/openhantek.h \
    src/settings.h \
    src/hantek/hantek_control.h \
    src/hantek/hantek_device.h \
    src/hantek/hantek_types.h \
    src/buudai/buudai_control.h \
    src/buudai/buudai_device.h \
    src/buudai/buudai_types.h \
    src/dso.h

# Ressource files
RESOURCES += res/application.qrc \
    res/configdialog.qrc

# Doxygen files
DOXYFILES += Doxyfile \
    mainpage.dox \
    roadmap.dox

# Files copied into the distribution package
DISTFILES += ChangeLog \
    COPYING \
    INSTALL \
    res/images/*.png \
    res/images/*.icns \
    res/images/*.svg \
    translations/*.qm \
    translations/*.ts \
    $${DOXYFILES}

# Translations
TRANSLATIONS += translations/openhantek_de.ts

# Program version
VERSION = 0.3.1

# Destination directory for built binaries
DESTDIR = bin

# Prefix for installation
PREFIX = $$(PREFIX)

# Build directories
OBJECTS_DIR = build/obj
UI_DIR = build/ui
MOC_DIR = build/moc

# Include directory
QMAKE_CXXFLAGS += "-iquote $${IN_PWD}/src"

# libusb version
LIBUSB_VERSION = $$(LIBUSB_VERSION)
contains(LIBUSB_VERSION, 0): LIBS += -lusb
else { 
    LIBUSB_VERSION = 1
#    LIBS += -lusb-1.0
}
DEFINES += LIBUSB_VERSION=$${LIBUSB_VERSION}

# Debug output
CONFIG(debug, debug|release): DEFINES += DEBUG

CONFIG += debug_and_release

# Settings for different operating systems
unix:!macx { 
    isEmpty(PREFIX):PREFIX = /usr/local
    TARGET = openhantek
    
    # Installation directories
    target.path = $${PREFIX}/bin
    translations.path = $${PREFIX}/share/apps/openhantek/translations
    INCLUDEPATH += /usr/include/libusb
    DEFINES += QMAKE_TRANSLATIONS_PATH=\\\"$${translations.path}\\\" \
        OS_UNIX VERSION=\\\"$${VERSION}\\\"
}
macx { 
    isEmpty(PREFIX):PREFIX = OpenHantek.app
    TARGET = OpenHantek
    
    # Installation directories
    target.path = $${PREFIX}/Contents/MacOS
    translations.path = $${PREFIX}/Contents/Resources/translations
    INCLUDEPATH += src /opt/local/include
    LIBS += -framework IOKit -framework CoreFoundation -L/opt/local/lib
    ICON = res/images/openhantek.icns
    DEFINES += QMAKE_TRANSLATIONS_PATH=\\\"Contents/Resources/translations\\\" \
        OS_DARWIN VERSION=\\\"$${VERSION}\\\"
}
win32 { 
    isEmpty(PREFIX):PREFIX = OpenHantek
    TARGET = OpenHantek
    
    # Installation directories
    target.path = $${PREFIX}
    translations.path = $${PREFIX}/translations
    DEFINES += QMAKE_TRANSLATIONS_PATH=\\\"translations\\\" \
        OS_WINDOWS VERSION=\\\"$${VERSION}\\\"
}
translations.files += translations/*.qm
INSTALLS += target \
    translations

# Custom target "doc" for Doxygen
doxygen.target = doc
doxygen.commands = "rm -r doc/; env DEFINES=\"$${DEFINES}\" doxygen Doxyfile"
doxygen.depends = $${SOURCES} \
    $${HEADERS} \
    $${DOXYFILES}
QMAKE_EXTRA_TARGETS += doxygen
