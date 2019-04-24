#!/bin/bash
nEndDevices='200 400 600 800 1000 1200 1400 1600'
delays='120 95 65 35 14 9'
dPktsPerTrans='12 8'

touch battLevels.csv

for delay in $delays
do
  for number in $nEndDevices
  do
    for packets in $dPktsPerTrans
    do
      start=`date +%s`
      echo '================================================================='
      echo 'In progress: nDevices = '$number', inter-/intraTransactionDelay = '$delay', nDataPkts = '$packets
      ./waf --run "energy --nDevices=$number --intraDelay=$delay --interDelay=$delay
      --dPktsPerTrans=$packets"
      end=`date +%s`
      echo 'Finished: nDevices = '$number', inter-/intraTransactionDelay = '$delay', nDataPkts = '$packets
      date +%d.%m", "%X
      echo "Execution time: "$((end-start))" s"
      echo "Appending final end device battery level (J) to battLevels.csv ..."
      level=$(tail -n1 battery-level.txt)
      stringarray=($level)
      echo ${stringarray[1]} >> battLevels.csv
      echo -e '=================================================================\n\n'
    done
  done
done

echo "Batch simulation for energy measurement scenario complete."
date +%d.%m", "%X
