#!/bin/sh
#
# test1.sh
#

. `dirname $0`/share.sh

return_code=0

if [ ! -x $MABOSS_MPI ]
then
    echo $MABOSS_MPI not found
    exit 1
fi

check_file()
{
    if [ $? = 0 ]; then
	echo "File $1 OK"
    else
	echo "File $1 ** error: differences found **"
    return_code=1
    fi
}

echo
echo "Ensemble test"
rm -rf tmp; mkdir -p tmp
/usr/bin/time -p $LAUNCHER mpirun -np $1 --oversubscribe $MABOSS_MPI --ensemble --save-individual -c ensemble/ensemble.cfg -o tmp/res ensemble/invasion/Invasion_0.bnet ensemble/invasion/Invasion_200.bnet ensemble/invasion/Invasion_400.bnet ensemble/invasion/Invasion_600.bnet ensemble/invasion/Invasion_800.bnet  ensemble/invasion/Invasion_1000.bnet

if [ $? != 0 ]; then exit 1; fi

python compare_probtrajs.py ensemble/refer/res_probtraj.csv tmp/res_probtraj.csv --exact
check_file "projtraj"
python compare_probtrajs.py ensemble/refer/res_model_0_probtraj.csv tmp/res_model_0_probtraj.csv --exact
check_file "projtraj_model_0"
python compare_probtrajs.py ensemble/refer/res_model_1_probtraj.csv tmp/res_model_1_probtraj.csv --exact
check_file "projtraj_model_1"
python compare_probtrajs.py ensemble/refer/res_model_2_probtraj.csv tmp/res_model_2_probtraj.csv --exact
check_file "projtraj_model_2"
python compare_probtrajs.py ensemble/refer/res_model_3_probtraj.csv tmp/res_model_3_probtraj.csv --exact
check_file "projtraj_model_3"
python compare_probtrajs.py ensemble/refer/res_model_4_probtraj.csv tmp/res_model_4_probtraj.csv --exact
check_file "projtraj_model_4"
python compare_probtrajs.py ensemble/refer/res_model_5_probtraj.csv tmp/res_model_5_probtraj.csv --exact
check_file "projtraj_model_5"

# rm -rf tmp; 

exit $return_code
