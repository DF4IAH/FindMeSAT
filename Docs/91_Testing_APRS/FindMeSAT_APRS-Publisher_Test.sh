#!/bin/bash

nc -q1 karlsruhe.aprs2.net 8080 < FindMeSAT-APRS-Data_Test$NR.txt >/dev/null 2>&1 &
nc -q1 nuremberg.aprs2.net 8080 < FindMeSAT-APRS-Data_Test$NR.txt >/dev/null 2>&1 & 

