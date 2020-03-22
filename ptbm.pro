TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
  ptbm-emscripten.cpp \
  ptbm.cpp

HEADERS += \
  cxxopts.hpp \
  ptbm.h
