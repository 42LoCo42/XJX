TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
		fileloader.cpp \
		main.cpp \
		xjx.cpp

HEADERS += \
	fileloader.h \
	xjx.h

unix:!macx: LIBS += -lncurses
