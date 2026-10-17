#!/bin/bash
#
#############################################################################
#                                                                           #
# BSD 3-Clause License (see https://opensource.org/licenses/BSD-3-Clause)   #
#                                                                           #
# Copyright (c) 2011-2020 Institut Curie, 26 rue d'Ulm, Paris, France       #
# All rights reserved.                                                      #
#                                                                           #
# Redistribution and use in source and binary forms, with or without        #
# modification, are permitted provided that the following conditions are    #
# met:                                                                      #
#                                                                           #
# 1. Redistributions of source code must retain the above copyright notice, #
# this list of conditions and the following disclaimer.                     #
#                                                                           #
# 2. Redistributions in binary form must reproduce the above copyright      #
# notice, this list of conditions and the following disclaimer in the       #
# documentation and/or other materials provided with the distribution.      #
#                                                                           #
# 3. Neither the name of the copyright holder nor the names of its          #
# contributors may be used to endorse or promote products derived from this #
# software without specific prior written permission.                       #
#                                                                           #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       #
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED #
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           #
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER #
# OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  #
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       #
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        #
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    #
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      #
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        #
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              #
#                                                                           #
#############################################################################
#
# Module: check-requirements.sh
# Author: Eric Viara
# Date: Jan 2017
#
# Check MaBoSS requirements
#

typeset -i error=0

RED="\033[0;31m"
NOCOLOR="\033[0m"

display_error()
{
    echo -e "  ${RED}$*${NOCOLOR}"
    error=${error}+1
}

display_ok()
{
    echo "  $*: OK"
}

check_prog()
{
    type -p $1 > /dev/null 2>&1
    if [ $? != 0 ]; then
	display_error "$1: MISSING"
	return 1
    else
	display_ok $1
    fi
}

tbchecked="MaBoSS requirements"
echo Checking ${tbchecked}...
echo

check_prog flex
check_prog bison
check_prog gcc
check_prog g++

tmpfile=/tmp/MaBoSS
trap "rm -f $tmpfile ${tmpfile}.cpp ${tmpfile}.py" 0 1 2 3

cat > ${tmpfile}.cpp <<EOF
#include <iostream>
int main() 
{
  std::cout << "hello world\n";
  return 0;
}
EOF

g++ -o $tmpfile ${tmpfile}.cpp > /dev/null 2>&1
if [ $? != 0 ]; then
    display_error "bad g++ installation"
fi

$tmpfile > /dev/null 2>&1
if [ $? != 0 ]; then
    display_error "bad g++ installation"
fi

echo

if [ $error = 0 ]; then
    echo ${tbchecked}: OK
else
    echo ${tbchecked}: $error errors found
fi

echo

error=0
tbchecked="MaBoSS tools requirements"
echo Checking ${tbchecked}...
echo

check_python_module()
{
    cat > ${tmpfile}.py <<EOF
import $1
EOF
    python ${tmpfile}.py > /dev/null 2>&1
    if [ $? != 0 ]; then
	display_error "python module $1: MISSING"
    else
	display_ok "python module $1"
    fi
}

check_prog perl

check_prog python

if [ $? = 0 ]; then
    for module in matplotlib matplotlib.gridspec matplotlib.patches matplotlib.pylab numpy pandas seaborn xlsxwriter
    do
	check_python_module $module
    done
fi

echo

if [ $error = 0 ]; then
    echo ${tbchecked}: OK
else
    echo ${tbchecked}: $error errors found
fi
