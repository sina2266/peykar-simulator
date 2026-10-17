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
# Tool: MBSS_MutBndCfg.pl
# Author: Gautier Stoll, gautier_stoll@yahoo.fr
# Date: Jan 2017
#

use strict;

my $MaxRate="1.7976931348623157E+308";
#my $MaBossScript="PlMaBoSS_2.0.pl"; #script that run MaBoSS
my $bnd_file=shift;
#Error test (produce a "help")
if (!$bnd_file){
    printf "Missing .bnd file, MBSS_MutBndCfg.pl  <file.bnd> <file.cfg> \"<node_list>\"\n"; 
    exit;}

my $cfg_file=shift;
#Error test (produce a "help")
if (!$cfg_file){
    printf "Missing .cfg file, MBSS_MutBndCfg.pl  <file.bnd> <file.cfg> \"<node_list>\"\n"; 
    exit;}

my $n_list=shift;
if (!$n_list){
    printf "Missing node list, MBSS_MutBndCfg.pl  <file.bnd> <file.cfg> \"<node_list>\"\n";
    exit;}

my @node_list=split(/\s+/,$n_list);

$MaxRate=$MaxRate."/".($#node_list + 1);

#$_=$cfg_file;
#s/.cfg//;
#my $cfg_name=$_; #not use yet, new version may generates multiple cfg files
open(BND_F,$bnd_file) || die "Cannot find bnd file ".$bnd_file."\n";
my @MutBNDLineList;
my @MutCFGVarList;
do
{
    $_=<BND_F>;
    @MutBNDLineList=(@MutBNDLineList,$_);
    until((/node/ || /Node/ || eof(BND_F))) {$_=<BND_F>;@MutBNDLineList=(@MutBNDLineList,$_);} #catch line starting with "node" 
    if(!eof(BND_F))
    {
	foreach my $node (@node_list) #catch if node name correspond a name in the list of nodes
	{
	    if(/$node[\s\{\n]/)
	    {
		print "Catch node ".$node."\n";
		@MutCFGVarList=(@MutCFGVarList,"\$Low_".$node." = 0;\n");
		@MutCFGVarList=(@MutCFGVarList,"\$High_".$node." = 0;\n");
		my $rate_up_flag=0;
		my $rate_down_flag=0;
		do {
		    $_=<BND_F>;
		    if(/rate_up/)
		    {
			my $up_var=" = ( \$Low_".$node." ? 0.0 : ( \$High_".$node." ? \@max_rate : ("; #Low_node wins
			s/=/$up_var/;
			s/;/)));/;
			@MutBNDLineList=(@MutBNDLineList,$_); #change the rate_up
			$rate_up_flag=1;	
		    }
		    elsif(/rate_down/)
		    {
			my $down_var=" = ( \$Low_".$node." ? \@max_rate : ( \$High_".$node." ? 0.0 : (";#Low_node wins
			s/=/$down_var/;
			s/;/)));/;
			@MutBNDLineList=(@MutBNDLineList,$_); #change the rate_down
			$rate_down_flag=1;
		    }
		    else{@MutBNDLineList=(@MutBNDLineList,$_);}
		}
		until(/\}/);
		if ($rate_up_flag==0)
		{
		    splice(@MutBNDLineList,$#MutBNDLineList-1,0,"\t rate_up = ( \$Low_".$node." ? 0.0 : ( \$High_".$node." ? \@max_rate : (\@logic ? 1.0 : 0.0 )));\n");
		    #if rate_up is absent, create it. Low_node wins
		}
		if ($rate_down_flag==0)
		{
		     splice(@MutBNDLineList,$#MutBNDLineList-1,0,"\t rate_down = ( \$Low_".$node." ? \@max_rate : ( \$High_".$node." ? 0.0 : (\@logic ? 0.0 : 1.0 ))) ;\n");
		     #if rate_down is absent, create it. Low_node wins
		}
		splice(@MutBNDLineList,$#MutBNDLineList,0,"\t max_rate = ".$MaxRate.";\n"); #local definition of Max_rate
		last; #if a node is catched, no need to further run the foreach loop
	    }
	}
    }
}
until(eof(BND_F));
close(BND_F);
$_=$bnd_file;
s/\.bnd/_mut\.bnd/;
open(BND_F_MUT,">".$_);
foreach my $line (@MutBNDLineList)
{print BND_F_MUT $line;}
close(BND_F_MUT);

open(CFG_F,$cfg_file) || die "Cannot find bnd file ".$cfg_file."\n";
$_=$cfg_file;
s/\.cfg/_mut\.cfg/;
open(CFG_F_MUT,">".$_);
while (<CFG_F>)
{print CFG_F_MUT $_;}
close(CFG_F);
foreach my $line (@MutCFGVarList)
{print CFG_F_MUT $line;}
close(CFG_F_MUT);
