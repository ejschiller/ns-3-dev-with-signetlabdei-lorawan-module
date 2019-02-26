#!/bin/bash
nEndDevices='10 1250 2500 3750 5000'
delays='120 91.25 62.5 33.75 5'

for number in $nEndDevices
do
  for delay in $delays
  do
    start=`date +%s`
    echo '================================================================='
    echo 'In progress: nDevices = '$number', inter-/intraTransactionDelay = '$delay
    ./waf --run "scenario3 --nDevices=$number --intraDelay=$delay --interDelay=$delay"
    end=`date +%s`
    echo 'Finished: nDevices = '$number', inter-/intraTransactionDelay = '$delay
    echo "Execution time: "$((end-start))" s"
    echo -e '=================================================================\n\n'
  done
done

echo "Batch simulation for scenario 3 complete."
