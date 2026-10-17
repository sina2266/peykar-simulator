#!/usr/bin/env perl
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
# Tool: MBSS_SensitivityAnalysis.pl
# Author: Gautier Stoll, gautier_stoll@yahoo.fr
# Date: Jan 2017
#

use strict;

#my $MaBossScript="PlMaBoSS_3.0.pl"; #script that run MaBoSS
my $bnd_file=shift;
if (!$bnd_file){
    printf "Missing bnd file, MBSS_SensitivityAnalysis.pl  <file.bnd> <file.cfg> \"<add_string>\", the string will be added to every external variable\n"; 
    exit;}
my $cfg_file=shift;
if (!$cfg_file){
    printf "Missing cfg file, MBSS_SensitivityAnalysis.pl  <file.bnd> <file.cfg> \"<add_string>\", the string will be added to every external variable\n";
    exit;}
my $mod_string=shift;
if (!$mod_string){
    printf "Missing string to add, MBSS_SensitivityAnalysis.pl  <file.bnd> <file.cfg> \"<add_string>\", the string will be added to every external variable\n";
    exit;}
$mod_string=$mod_string.";";
my $MaBossScript="MBSS_FormatTable.pl";
#if (!$mod_string){
#    printf "Missing string to add, ParamSensFormat.pl  <file.bnd> <file.cfg> \"<add_string>\", the string will be added to every external variable\n";
#    exit;}

$_=$cfg_file;
s/.cfg//;
my $cfg_name=$_;

my @CfgLineList;
my @CfgHitLineIndex;
my @CfgVarList;
open(CFG_F,$cfg_file);
my $LineIndex=0;
while(<CFG_F>)
{
    @CfgLineList=(@CfgLineList,$_);
    if (/^\$/)
    {
	s/^\$//;
	@CfgHitLineIndex=(@CfgHitLineIndex,$LineIndex);
	my @LineSplit=split(/=/);
	$_=$LineSplit[0];
	s/\s//g;
	@CfgVarList=(@CfgVarList,$_);
    }

    $LineIndex++;	    
}
close(CFG_F);
system("mkdir Sensitivity_".$cfg_name);   #added to renamed script"
system("cp ".$bnd_file." Sensitivity_".$cfg_name."/"); #added to renamed script
open(SRC_F,">Sensitivity_".$cfg_name."/Sensitivity_".$cfg_name.".sh"); #modified to renamed script
print SRC_F "#!/bin/bash\n"; #added to renamed script
for (my $i=0;$i<=$#CfgHitLineIndex;$i++)
{
    my $tmpCfgFile=$cfg_name."_".$CfgVarList[$i].".cfg";
    print SRC_F $MaBossScript." ".$bnd_file." ".$tmpCfgFile." 0.01\n"; #line that lauch MaBoSS
#    print SRC_F $MaBossScript." -c ".$tmpCfgFile." -o Out_".$CfgVarList[$i]." ".$bnd_file."\n"; #line that lauch MaBoSS
    open(CFG_F,">Sensitivity_".$cfg_name."/".$tmpCfgFile); #modified to renamed script
    for ($LineIndex=0;$LineIndex<$CfgHitLineIndex[$i];$LineIndex++)
    {
	print CFG_F $CfgLineList[$LineIndex];
    }
    $_=$CfgLineList[$CfgHitLineIndex[$i]];
    
    s/;/$mod_string/;
    print CFG_F $_;
    
    for ($LineIndex=$CfgHitLineIndex[$i]+1;$LineIndex<=$#CfgLineList;$LineIndex++)
    {
	print CFG_F $CfgLineList[$LineIndex];
    }
    close(CFG_F);
}
close(SRC_F);
system("chmod a+x Sensitivity_".$cfg_name."/Sensitivity_".$cfg_name.".sh"); #added to renamed script

	
    

