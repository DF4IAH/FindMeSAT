#!/bin/sh

find . -name *.rej -delete
patch -p1 < patchAfterExport01.diff
patch -p1 < patchAfterExport02.diff

