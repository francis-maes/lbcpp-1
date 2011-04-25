#!/bin/sh

function launch {
  targetName=`echo $target | sed 's/(/./g'`
  targetName=`echo $targetName | sed 's/)/./g'`
	name="${prefix}${targetName}"

  nbTarget=`echo $targetName | grep -o "-" | wc -l`
	expected_time=`expr ${nbTarget} + 3`
	expected_time=`expr $expected_time '*' 10`
  echo "Expected Time: ${expected_time}"

  sleep 2

qsub << EOF
#$ -l h_vmem=12G
#$ -l h_rt=${expected_time}:00:00

#$ -m eas
#$ -M J.Becker@ULg.ac.be

#$ -cwd

#$ -N ${name}
./${program} ProteinsDirectory /scratch/jbecker/${database} Output ${name} Targets "${target}" NumProteinsToLoad ${nbProt} ${other}
EOF

}

program="SunBox"

target="(DR)10"

prefix="exp"
database="PDB70/protein"
nbProt="140"
#------------------------------------------------

#for targ in `python CombinaisonGenerator.py StAl BBB DR SA SS8 SS3`
#for lr in 0.0001 0.0002 0.0005 0.001 0.002 0.005 0.01 0.02 0.05 0.1 0.2 0.5 1 2 5 10 20 50 100 200 500 1000 2000 5000 10000
#for reg in 0.000001 0.000002 0.000005 0.0000001 0.0000002 0.0000005 0.00000001 0.00000002 0.00000005 
#for ls in 100 200 500 1000 2000 5000 10000 20000 50000 1000000 2000000 5000000 10000000 20000000 50000000

#program="SimpleFeatures"
#database="PredictedDB/supervision400"
#target="(SS3)2"
#other="TestingProteinsDirectory /scratch/jbecker/PredictedDB/supervision100 SaveInference"
#nbProt=400
#launch

#exit

#nbProt="500"
#target="(SS3)10"
#other="$other SaveInference IsExperimentalMode"
#
#prefix="exp2OldPSSM"
#database="PredictedDB/supervision500/"
#launch
#
#prefix="exp2NewNormPSSM"
#database="PredictedDB/newSupervision500/"
#launch
#
#exit

prefix="exp_CM_201-1000_MP"
#database="L50DB/protein"
database="L201-1000DB"
target="(CM)10"
nbProt="1000"
other="IsExperimentalMode SaveInference StoppingIteration 10"
launch

exit

#for multitask in `python CombinaisonGenerator.py VerticalBy2 OblicBy2 HorizontalBy2` -
for multitask in OblicBy2 -
do
for targ in SS3
do
prefix="exp_MultiSimple-SS3_${multitask}"
database="PredictedDB/protein/SS3-SS8-SA-DR-BBB"
nbProt="500"
target="(${targ})1"
other="MultiTaskFeatures $multitask"
other="$other SaveInference IsExperimentalMode"
launch
done
done
