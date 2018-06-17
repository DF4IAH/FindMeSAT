#/bin/bash

SR=44100; rtl_fm -f 433.92e6 -l 0 -M am -s $SR -r $SR - | tee WirelessSensor_Analytic_SAMPLE.raw | play -r $SR -t s16 -L -c 1  -

