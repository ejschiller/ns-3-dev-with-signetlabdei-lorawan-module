#!/bin/bash
nEndDevices='10 400 800 1200 1600'
delays='160 120 80 40 10'

for delay in $delays
do
  for number in $nEndDevices
  do
    start=`date +%s`
    echo '================================================================='
    echo 'In progress: nDevices = '$number', inter-/intraTransactionDelay = '$delay
    mkdir -p logs
    ./waf --run "scenario4 --nDevices=$number --intraDelay=$delay --interDelay=$delay" > logs/scen4_${delay}_${number}.txt 2>&1
    end=`date +%s`
    echo 'Finished: nDevices = '$number', inter-/intraTransactionDelay = '$delay
    date +%d.%m", "%X
    echo "Execution time: "$((end-start))" s"
    echo -e '=================================================================\n\n'
  done
done

echo "Batch simulation for scenario 4 complete."
date +%d.%m", "%X
