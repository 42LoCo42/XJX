TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
		main.cpp \
		utils.cpp \
		xjx.cpp

HEADERS += \
	utils.h \
	xjx.h

unix:!macx: LIBS += -lncurses
