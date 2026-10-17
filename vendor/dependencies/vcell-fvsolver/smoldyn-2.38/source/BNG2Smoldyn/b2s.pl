$filename = $ARGV[0];
$c2p = $ARGV[1];

@filedata = get_file_data($filename);

$filename =~ s/\.net//;
$txtname = $filename.".txt";
$outname = $filename."out.txt";
open(OUTPUT, '>'.$txtname);
print OUTPUT "dim 2\n\n";
open(LIST, '>species.list');
open(OBS, '>obs.txt');
%para = %species = %rxn = %mol = %obs = ();
%substant = ();
$flag_para = $flag_species = $flag_rxn = $flag_obs = 0;
$data_para = $data_species = $data_rxn = $data_obs = '';
foreach $line(@filedata){
	if($line =~ /^begin parameters/){
		$flag_para = 1;
	}
	elsif($line =~ /^end parameters/){
		$flag_para = 0;
	}
	elsif($line =~ /^begin species/){
		$flag_species = 1;
	}
	elsif($line =~ /^end species/){
		$flag_species = 0;
	}
	elsif($line =~ /^begin reactions/){
		$flag_rxn = 1;
	}
	elsif($line =~ /^end reactions/){
		$flag_rxn = 0;
	}
	elsif($line =~ /^begin groups/){
		$flag_obs = 1;
	}
	elsif($line =~ /end groups/){
		$flag_obs = 0;	
	}
	else{
		if($flag_para == 1){
			$data_para .= $line;
		}
		elsif($flag_species == 1){
			$data_species .= $line;
		}
		elsif($flag_rxn == 1){
			$data_rxn .= $line;
		}
		elsif($flag_obs == 1){
			$data_obs .= $line;	
		}
	}
}

@spd_para = split("\n", $data_para);
@spd_species = split("\n", $data_species);
@spd_rxn = split("\n", $data_rxn);
@spd_obs = split("\n", $data_obs);

foreach $each_para(@spd_para){
	@spe_para = split(' ', $each_para);
	$para{$spe_para[1]} = $spe_para[2];
}

$species_index = 1;
$track_index = 1;
$str_track = '';
$str_species = '';
foreach $each_species(@spd_species){
	@spe_species = split(' ', $each_species);
	if($spe_species[1] =~ /,track~/){
		$bak_species = $spe_species[1];
		if($spe_species[1] =~ /,track~[1-9]/){
			$bak_species =~ s/,track~[0-9]+//g;
			if(exists $substant{$bak_species}){
				$sub = $substant{$bak_species};
			}
			else{
				$sub = 'N/A';
			}
			$str_track .= 't'.$track_index."\t".$spe_species[1]."\t".$sub."\n";
			$species{$spe_species[0]}= 't'.$track_index;
			print OUTPUT 'species t'.$track_index."\n";
			$track_index++;
		}
		else{
			$bak_species =~ s/,track~0//g;
			$str_species .= 's'.$species_index."\t".$bak_species."\n";
			$species{$spe_species[0]} = 's'.$species_index;
			$substant{$bak_species} = $species_index;
			print OUTPUT 'species s'.$species_index."\n";
			$species_index++;
		}
	}
	else{
		$str_species .= 's'.$species_index."\t".$spe_species[1]."\n";
		$species{$spe_species[0]} = 's'.$species_index;
		print OUTPUT 'species s'.$species_index."\n";
		$species_index++;
	}
}

#output the species.list and track.list
print LIST $str_species;
if(!($str_track eq '')){
	open(TRACK, ">track.list");
	print TRACK $str_track;
	close TRACK;
}

print OUTPUT "\nread_file mol.txt\nread_file domain.txt\nread_file para_spe.txt\nread_file para_time.txt\n\n";

$track_count = 0;
print OBS "species";
for($i = 1; $i <= scalar(@spd_species); $i++){
	print OBS " ".$species{$i};
}
print OBS "\n";
foreach $each_obs(@spd_obs){
	@spe_obs = split(' ', $each_obs);
	@s2_obs = split(',', $spe_obs[2]);
	@m = ();
	for($i = 0; $i < scalar(@spd_species); $i++){
		$m[$i] = 0;
	}
	foreach $s2e(@s2_obs){
		if($s2e =~ /\*/){
			@s = split('\*', $s2e);
			$m[$s[1] - 1] = $s[0];
		}
		else{
			$m[$s2e - 1] = 1;
		}
	}
	print OBS $spe_obs[1];
	for($i = 0; $i < scalar(@spd_species); $i++){
		print OBS ' '.$m[$i];
	}
	print OBS "\n";
}

foreach $each_rxn(@spd_rxn){
	@spe_rxn = split(' ', $each_rxn);
	$rxn_smol = 'reaction rxn'.$spe_rxn[0].' ';
	@reactants = split(',', $spe_rxn[1]);
	foreach $reactant(@reactants){
		$reactant = $species{$reactant};
	}
	$rxn_smol .= join(' + ', @reactants);
	$rxn_smol .= ' -> ';
	@products = split(',', $spe_rxn[2]);
	foreach $product(@products){
		$product = $species{$product};
	}
	$rxn_smol .= join(' + ', @products);
	if($spe_rxn[3] =~ /\*/){
		@rate = split('\*', $spe_rxn[3]);
		$rate_c = $rate[0] * $para{$rate[1]};
	}
	else{
		$rate_c = $para{$spe_rxn[3]};
	}
	if(scalar(@reactants) > 1){
		$rate_c *= $c2p * $c2p * $c2p;
	}
	$rxn_smol .= ' '.$rate_c;
	print OUTPUT $rxn_smol."\n";
}
print OUTPUT "\nend_file\n";
close OUTPUT;
close LIST;
close OBS;

print "--------------------\nBNGL is translated into Smoldyn compliler\n--------------------\n";

sub get_file_data 
{
    my($filename) = @_;
    my @filedata = (  );
    unless( open(GET_FILE_DATA, $filename) ) 
    {
        print STDERR "Cannot open file \"$filename\"\n\n";
        exit;
    }
    @filedata = <GET_FILE_DATA>;
    close GET_FILE_DATA;
    return @filedata;
}

