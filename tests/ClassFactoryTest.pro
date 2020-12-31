!include(../src/ClassFactory.pri) {
	error("Couldn't find the pri file!")
}

QT += testlib

SOURCES += \
	main.cpp