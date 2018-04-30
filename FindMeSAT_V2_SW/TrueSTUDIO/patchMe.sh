#!/bin/sh

find . -name *.rej -delete
patch -p1 < patchAfterExport.diff

