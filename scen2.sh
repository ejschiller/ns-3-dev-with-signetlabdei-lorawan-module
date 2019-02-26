#!/bin/bash
nEndDevices='10 1250 2500 3750 5000'
delays='120 91.25 62.5 33.75 5'

for number in $nEndDevices
do
  for delay in $delays
  do
    echo '========================================================='
    echo 'In progress: nDevices = '$number', interTransmissionDelay = '$delay
    ./waf --run "scenario2 --nDevices=$number --delay=$delay"
    echo 'Finished: nDevices = '$number', interTransmissionDelay = '$delay
    echo -e '=========================================================\n\n'
  done
done

echo "Batch simulation for scenario 2 complete."
