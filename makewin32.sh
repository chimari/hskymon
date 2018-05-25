#!/bin/sh
touch configure configure.ac aclocal.m4 Makefile.am Makefile.in

./configure --with-win32 \
  'CC=gcc' CFLAGS=-O3 \
  && make \
  && make install-strip prefix=/home/taji/dist32
