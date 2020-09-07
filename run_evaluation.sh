#!/bin/bash

if [ "$#" -lt 8 ] || [ "$#" -gt 9 ] ; then
    echo "Illegal number of parameters"
    echo "Usage: ./run_evaluation.sh [branch_pred] [l1d_pref] [l2c_pref] [llc_pref] [llc_repl] [num_core] [N_WARM] [N_SIM] [OPTION]"
    exit 1
fi

BRANCH=${1}
L1D_PREFETCHER=${2}
L2C_PREFETCHER=${3}
LLC_PREFETCHER=${4}
LLC_REPLACEMENT=${5}
NUM_CORE=${6}
N_WARM=${7}
N_SIM=${8}
OPTION=${9}

function BINARY_NAME(){
   local L1I_PREFETCHER=$1
   echo "${BRANCH}-${L1I_PREFETCHER}-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${LLC_REPLACEMENT}-${NUM_CORE}core"
}

function BUILD_COMMAND(){
   local L1I_PREFETCHER=$1
   echo "./build_champsim.sh ${BRANCH} ${L1I_PREFETCHER} ${L1D_PREFETCHER} ${L2C_PREFETCHER} ${LLC_PREFETCHER} ${LLC_REPLACEMENT} ${NUM_CORE}"
}

function RUN_COMMAND(){
   local BINARY=$1
   local TRACE=$2
   echo "./run_champsim_nosuffix.sh ${BINARY} ${N_WARM} ${N_SIM} ${TRACE}"
}

function STATS_NAME(){
   local BINARY=$1
   local TRACE=$2
   echo "${BINARY}.${TRACE}.${N_SIM}.${N_WARM}.stats"
}

TRACE_DIR=$PWD/dpc3_traces

mkdir -p ${TRACE_DIR}

EVAL_ROOT_DIR=$PWD/eval
EVAL_SOURCE_DIR=$PWD/eval/prefetcher
EVAL_STATS_DIR=$PWD/eval/stats/not-formatted
EVAL_STATS_FORMATTED_DIR=$PWD/eval/stats/formatted
EVAL_SUMM_DIR=$PWD/eval/summary

mkdir -p ${EVAL_ROOT_DIR}
mkdir -p ${EVAL_SOURCE_DIR}
mkdir -p ${EVAL_STATS_DIR}
mkdir -p ${EVAL_STATS_FORMATTED_DIR}
mkdir -p ${EVAL_SUMM_DIR}

TRACE_TYPE=("client" "server" "spec")
TRACE_NUM=$(ls -1 ${TRACE_DIR} | wc -l)

CYAN="\033[0;36m"
BOLD=$(tput bold)
NORMAL=$(tput sgr0)

printf "${CYAN}${BOLD}Generating executables for evaluation...\n${NORMAL}"
for FILE in ${EVAL_SOURCE_DIR}/*.l1i_pref
do
    [ -f "$FILE" ] || continue
    cp ${FILE} prefetcher/
    PREF=`basename ${FILE} .l1i_pref`
    if [ -f $PWD/bin/$(BINARY_NAME ${PREF}) ] ; then
        printf "$(BUILD_COMMAND ${PREF}) ${BOLD}(Skipped)${NORMAL}\n"
    else
        printf "$(BUILD_COMMAND ${PREF})\n"
        bash $(BUILD_COMMAND ${PREF})
    fi
    rm -f $PWD/prefetcher/${PREF}.l1i_pref
done

if [ -f $PWD/bin/$(BINARY_NAME baseline) ] ; then
    printf "$(BUILD_COMMAND no) ${BOLD}(Skipped)${NORMAL}\n"
else
    printf "$(BUILD_COMMAND no)\n"
    bash $(BUILD_COMMAND no)
    mv $PWD/bin/$(BINARY_NAME no) $PWD/bin/$(BINARY_NAME baseline)
fi

function PROGRESS_BAR(){
    local n=$1
    local i=$2
    ((j=n-i))
    printf "\r["
    [ "$i" -ne 0 ] && printf "=%0.s" $(seq 1 $i)
    [ "$j" -ne 0 ] && printf " %0.s" $(seq 1 $j)
    printf "] $i / $n"
}

printf "${CYAN}${BOLD}Evaluating each executable on traces in ${TRACE_DIR}...${NORMAL}"
for BINARY in $PWD/bin/*
do
    [ -f "$BINARY" ] || continue
    BINARY=`basename ${BINARY}`
    printf "\n${BOLD}Simulate ${BINARY}...\n${NORMAL}"
    ((COUNT=1))
    for TRACE in ${TRACE_DIR}/*
    do
        printf "$(PROGRESS_BAR ${TRACE_NUM} ${COUNT})\n"
        TRACE=`basename ${TRACE} .champsimtrace.xz`
        for TYPE in ${TRACE_TYPE[@]}
        do
            if [[ ${TRACE} == ${TYPE}* ]] ; then
                RESULT_DIR=${EVAL_STATS_DIR}/${BINARY}/${TYPE}
                RESULT_FORMATTED_DIR=${EVAL_STATS_FORMATTED_DIR}/${BINARY}/${TYPE}
                mkdir -p ${RESULT_DIR}
                mkdir -p ${RESULT_FORMATTED_DIR}
                RESULT=$(STATS_NAME ${BINARY} ${TRACE})
                if [ -f ${RESULT_DIR}/${RESULT} ] && [ -f ${RESULT_FORMATTED_DIR}/${RESULT} ] ; then
                    printf "$(RUN_COMMAND ${BINARY} ${TRACE}.champsimtrace.xz) ${BOLD}(Skipped)${NORMAL}"
                else
                    printf "$(RUN_COMMAND ${BINARY} ${TRACE}.champsimtrace.xz)"
                    bash $(RUN_COMMAND ${BINARY} ${TRACE}.champsimtrace.xz)
                    cp $PWD/results_${N_SIM}M/${TRACE}.champsimtrace.xz-${BINARY}${OPTION}.txt ${RESULT_DIR}/${RESULT}
                    cp $PWD/champsim-stats.json ${RESULT_FORMATTED_DIR}/${RESULT}
                fi
            fi
        done
        ((COUNT++))
        tput el
        if [ ! "$COUNT" -gt $TRACE_NUM ]; then
            tput cuu1
        fi
    done
done
printf "\n"

rm -rf $PWD/results_${N_SIM}M
[ -f champsim-stats.json ] && rm champsim-stats.json

printf "${CYAN}${BOLD}Summarizing all the results...\n${NORMAL}"
printf "python eval.py\n\n"
python eval.py
