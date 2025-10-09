QT += core gui widgets network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
TARGET = RunMyModelDesktop
TEMPLATE = app

# Source files
SOURCES += src-cpp/src/main.cpp \
           src-cpp/src/mainwindow.cpp \
           src-cpp/src/backend.cpp \
           src-cpp/src/huggingface_client.cpp \
           src-cpp/src/model_manager.cpp \
           src-cpp/src/system_info.cpp

# Header files  
HEADERS += src-cpp/include/mainwindow.h \
           src-cpp/include/backend.h \
           src-cpp/include/huggingface_client.h \
           src-cpp/include/model_manager.h \
           src-cpp/include/system_info.h

# Include path
INCLUDEPATH += src-cpp/include

# Output directory
DESTDIR = build
OBJECTS_DIR = build/obj
MOC_DIR = build/moc

