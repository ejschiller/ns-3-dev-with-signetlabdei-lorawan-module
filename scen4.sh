#!/bin/bash
nEndDevices='10 1250 2500 3750 5000'
delays='120 91.25 62.5 33.75 5'

for number in $nEndDevices
do
  for delay in $delays
  do
    echo '================================================================='
    echo 'In progress: nDevices = '$number', inter-/intraTransactionDelay = '$delay
    ./waf --run "scenario4 --nDevices=$number --intraDelay=$delay --interDelay=$delay"
    echo 'Finished: nDevices = '$number', inter-/intraTransactionDelay = '$delay
    echo -e '=================================================================\n\n'
  done
done

echo "Batch simulation for scenario 4 complete."
