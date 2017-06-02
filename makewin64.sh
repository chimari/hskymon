#!/bin/sh

./configure --with-win32 \
  'CC=gcc' CFLAGS=-O2 \
  && make \
  && make install-strip prefix=/home/taji/dist
