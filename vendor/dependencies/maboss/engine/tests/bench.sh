POPMABOSSDIR=../src
ICDDIR=../../incoming/qui_dure

uname -a
for opt in unordered_map_orig std_map_no_optim unordered_map_ev_optim std_map_ev_optim
do
    for in in $(seq 2)
    do
	echo using PopMaBoSS_${opt} >&2
	/usr/bin/time -p  ./${POPMABOSSDIR}/PopMaBoSS_${opt} -c ${ICDDIR}/ICD_phenomenologicalPM.cfg ${ICDDIR}/ICD_phenomenologicalPM.pbnd --output ICD_phenomenologicalPM_${opt}
    done
done

