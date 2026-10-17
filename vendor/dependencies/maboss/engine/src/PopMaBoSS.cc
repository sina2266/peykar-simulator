/*
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

  Module:
  PopMaBoSS.cc

  Authors:
  Vincent Noël <vincent.noel@curie.fr>
 
  Date:
  March 2021
*/

#include <ctime>
#include "PopMaBEstEngine.h"
#include "Function.h"
#include <fstream>
#include <stdlib.h>
#include "Utils.h"
#include "RandomGenerator.h"
#include "ProbTrajDisplayer.h"
#include "PopProbTrajDisplayer.h"

const char* prog = "PopMaBoSS";

static int usage(std::ostream& os = std::cerr)
{
  os << "\nUsage:\n\n";
  os << "  " << prog << " [-h|--help]\n\n";
  os << "  " << prog << " [-V|--version]\n\n";
  os << "  " << prog << " [-c|--config CONF_FILE] [-v|--config-vars VAR1=NUMERIC[,VAR2=...]] [-e|--config-expr CONFIG_EXPR] -o|--output OUTPUT BOOLEAN_NETWORK_FILE\n\n";
  os << "  " << prog << " [-c|--config CONF_FILE] [-v|--config-vars VAR1=NUMERIC[,VAR2=...]] [-e|--config-expr CONFIG_EXPR] -d|--dump-config BOOLEAN_NETWORK_FILE\n\n";
  os << "  " << prog << " [-c|--config CONF_FILE] [-v|--config-vars VAR1=NUMERIC[,VAR2=...]] [-e|--config-expr CONFIG_EXPR] -l|--generate-logical-expressions BOOLEAN_NETWORK_FILE\n\n";
  os << "  " << prog << " -t|--generate-config-template BOOLEAN_NETWORK_FILE\n";
  os << "  " << prog << " [-q|--quiet]\n";
  os << "  " << prog << " [--verbose level\n";
  os << "  " << prog << " [--format csv|json]\n";
  os << "  " << prog << " [--check]\n";
  os << "  " << prog << " [--override]\n";
  os << "  " << prog << " [--augment]\n";
  os << "  " << prog << " [--hexfloat]\n";
  return 1;
}

static int help()
{
  //  std::cout << "\n=================================================== " << prog << " help " << "===================================================\n";
  (void)usage(std::cout);
  std::cout << "\nOptions:\n\n";
  std::cout << "  -V --version                            : displays PopMaBoSS version\n";
  std::cout << "  -c --config CONF_FILE                   : uses CONF_FILE as a configuration file\n";
  std::cout << "  -v --config-vars VAR=NUMERIC[,VAR2=...] : sets the value of the given variables to the given numeric values\n";
  //  std::cout << "                                        the VAR value in the configuration file (if present) will be overriden\n";
  std::cout << "  -e --config-expr CONFIG_EXPR            : evaluates the configuration expression; may have multiple expressions\n";
  std::cout << "                                            separated by semi-colons\n";
  std::cout << "  --format csv|json                       : if set, format output in the given option: csv (tab delimited) or json; csv being the default\n";
  std::cout << "  --override                              : if set, a new node definition will replace a previous one\n";
  std::cout << "  --augment                               : if set, a new node definition will complete (add non existing attributes) / override (replace existing attributes) a previous one\n";
  std::cout << "  --verbose level                         : Set level of verbose (0-3, default 0)\n";
  std::cout << "  -o --output OUTPUT                      : prefix to be used for output files; when present run MaBoSS simulation process\n";
  std::cout << "  -d --dump-config                        : dumps configuration and exits\n";
  std::cout << "  -t --generate-config-template           : generates template configuration and exits\n";
  std::cout << "  -l --generate-logical-expressions       : generates the logical expressions and exits\n";
  std::cout << "  -q|--quiet                              : no notices and no warnings will be displayed\n";
  std::cout << "  --check                                 : checks network and configuration files and exits\n";
  std::cout << "  --hexfloat                              : displays double in hexadecimal format\n";
  std::cout << "  -h --help                               : displays this message\n";
  std::cout << "\nNotices:\n";
  std::cout << "\n1. --config and --config-expr options can be used multiple times;\n";
  std::cout << "   multiple --config and/or --config-expr options are managed in the order given at the command line;\n";
  std::cout << "   --config-vars VAR=VALUE always overrides any VAR assignment in a configuration file or expression\n";
  std::cout << "\n2. --dump-config, --generate-config-template, --generate-logical-expressions and --output are exclusive options\n";
  std::cout << '\n';

  std::cout << "Builtin functions:\n\n";
  Function::displayFunctionDescriptions(std::cout);

  return 0;
}

enum OutputFormat {
  CSV_FORMAT = 1,
  JSON_FORMAT
};

static std::string format_extension(OutputFormat format) {
  switch(format) {
  case CSV_FORMAT:
    return ".csv";
  case JSON_FORMAT:
    return ".json";
  default:
    return NULL;
  }
}

int main(int argc, char* argv[])
{
  const char* output = NULL;
  std::vector<ConfigOpt> runconfig_file_or_expr_v;
  std::vector<std::string> runconfig_var_v;
  const char* ctbndl_file = NULL;
  bool dump_config = false;
  bool generate_config_template = false;
  bool generate_logical_expressions = false;
  bool hexfloat = false;
  bool check = false;
  dont_shrink_logical_expressions = false; // global flag
  OutputFormat format = CSV_FORMAT;
  PopMaBEstEngine::init();

  // for debug
  if (getenv("MABOSS_VERBOSE") != NULL) {
#if USE_DYNAMIC_BITSET_STD_ALLOC
    std::cerr << "PopMaBoSS use dynamic_bitset [std allocator]\n";
#elif defined(USE_DYNAMIC_BITSET)
    std::cerr << "PopMaBoSS use MaBoSS dynamic bitset [experimental allocator]\n";
#elif defined(USE_STATIC_BITSET)
    std::cerr << "PopMaBoSS use standard bitset\n";
#else
    std::cerr << "PopMaBoSS use long long mask\n";
#endif
#ifdef HAS_UNORDERED_MAP
    bool unordered = true;
    std::cerr << "PopMaBoSS use std::unordered_map\n";
#else
    bool unordered = false;
    std::cerr << "PopMaBoSS use standard std::map\n";
#endif
#ifdef POPNETWORKSTATE_STD_MAP
    std::cerr << "PopMaBoSS use std::map<PopNetworkState, double>\n";
#else
    std::cerr << "PopMaBoSS use " << (unordered ? "std::unordered_map" : "std::map" ) << "<PopNetworkState, double>\n";
#endif
#ifdef EV_OPTIM_2021_10
    std::cerr << "PopMaBoSS use EV_OPTIM_2021_10\n";
#endif
  }

  for (int nn = 1; nn < argc; ++nn) {
    const char* s = argv[nn];
    if (s[0] == '-') {
      if (!strcmp(s, "-version") || !strcmp(s, "--version") || !strcmp(s, "-V")) { // keep -version for backward compatibility
	std::cout << prog << " version " + PopMaBEstEngine::VERSION << " [networks up to " << MAXNODES << " nodes]\n";
	return 0;
      } else if (!strcmp(s, "--config-vars") || !strcmp(s, "-v")) {
	if (nn == argc-1) {std::cerr << '\n' << prog << ": missing value after option " << s << '\n'; return usage();}
	runconfig_var_v.push_back(argv[++nn]);
      } else if (!strcmp(s, "--config-expr") || !strcmp(s, "-e")) {
	if (nn == argc-1) {std::cerr << '\n' << prog << ": missing value after option " << s << '\n'; return usage();}
	runconfig_file_or_expr_v.push_back(ConfigOpt(argv[++nn], true));
      } else if (!strcmp(s, "--dump-config") || !strcmp(s, "-d")) {
	dump_config = true;
      } else if (!strcmp(s, "--generate-config-template") || !strcmp(s, "-t")) {
	generate_config_template = true;
      } else if (!strcmp(s, "--generate-logical-expressions") || !strcmp(s, "-l")) {
	generate_logical_expressions = true;
      } else if (!strcmp(s, "--dont-shrink-logical-expressions")) {
	dont_shrink_logical_expressions = true;
      } else if (!strcmp(s, "--format")) {
	const char* fmt = argv[++nn];
	if (!strcasecmp(fmt, "CSV")) {
	  format = CSV_FORMAT;
	} else if (!strcasecmp(fmt, "JSON")) {
	  format = JSON_FORMAT;
	} else {
          std::cerr << "\n" << prog << ": unknown format " << fmt << std::endl;
	  return usage();
	}	  
      } else if (!strcmp(s, "--verbose")) {
        if ((nn+1) < argc) {
          const char* level_chr = argv[++nn];
          if (isdigit(*level_chr)) {
            const int level = atoi(level_chr);
            PopMaBEstEngine::setVerbose(level);
          } else {
            std::cerr << "\n" << prog << ": unknown level for verbose " << level_chr << std::endl;
            return usage();
          }        
        } else {
          std::cerr << "\n" << prog << ": please choose a level of verbosity (0-3) " << std::endl;
          return usage();
        }
  	  
      } else if (!strcmp(s, "--load-user-functions")) {
	if (nn == argc-1) {std::cerr << '\n' << prog << ": missing value after option " << s << '\n'; return usage();}
	PopMaBEstEngine::loadUserFuncs(argv[++nn]);
      } else if (!strcmp(s, "-o") || !strcmp(s, "--output")) {
	if (nn == argc-1) {std::cerr << '\n' << prog << ": missing value after option " << s << '\n'; return usage();}
	output = argv[++nn];
      } else if (!strcmp(s, "-c") || !strcmp(s, "--config")) {
	if (nn == argc-1) {std::cerr << '\n' << prog << ": missing value after option " << s << '\n'; return usage();}
	runconfig_file_or_expr_v.push_back(ConfigOpt(argv[++nn], false));
      } else if (!strcmp(s, "--override")) {
	if (Node::isAugment()) {
	  std::cerr << '\n' << prog << ": --override and --augment are exclusive options\n"; return usage();
	}
	Node::setOverride(true);
      } else if (!strcmp(s, "--augment")) {
	if (Node::isOverride()) {
	  std::cerr << '\n' << prog << ": --override and --augment are exclusive options\n"; return usage();
	}
	Node::setAugment(true);
      } else if (!strcmp(s, "--check")) {
	check = true;
      } else if (!strcmp(s, "-q") || !strcmp(s, "--quiet")) {
	MaBoSS_quiet = true;
      } else if (!strcmp(s, "--hexfloat")) {
	hexfloat = true;
      } else if (!strcmp(s, "--help") || !strcmp(s, "-h")) {
	return help();
      } else {
	std::cerr << '\n' << prog << ": unknown option " << s << std::endl;
	return usage();
      }
    } else if (ctbndl_file == NULL) {
      ctbndl_file = argv[nn];
    } else {
      std::cerr << '\n' << prog << ": boolean network file is already set to " << ctbndl_file << " [" << s << "]" << std::endl;
    }
  }
  
  if (!dump_config && !generate_config_template && !generate_logical_expressions && !check && output == NULL) {
    std::cerr << '\n' << prog << ": --output option is not set\n";
    return usage();
  }

  if (dump_config && generate_config_template) {
    std::cerr << '\n' << prog << ": --dump-config and --generate-config-template are exclusive options\n";
    return usage();
  }

  if (dump_config && output) {
    std::cerr << '\n' << prog << ": --dump-config and -o|--output are exclusive options\n";
    return usage();
  }

  if (generate_config_template && output) {
    std::cerr << '\n' << prog << ": --generate-config-template and -o|--output are exclusive options\n";
    return usage();
  }

  if (generate_config_template && ConfigOpt::getFileCount() > 0) {
    std::cerr<< '\n'  << prog << ": --generate-config-template and -c|--config are exclusive options\n";
    return usage();
  }

  if (generate_config_template && ConfigOpt::getExprCount() > 0) {
    std::cerr << '\n' << prog << ": --generate-config-template and --config-expr are exclusive options\n";
    return usage();
  }

  if (generate_config_template && runconfig_var_v.size() > 0) {
    std::cerr << '\n' << prog << ": --generate-config-template and --config-vars are exclusive options\n";
    return usage();
  }

  if (check && output) {
    std::cerr << '\n' << prog << ": --check and -o|--output are exclusive options\n";
    return usage();
  }

  std::ostream* output_run = NULL;
  std::ostream* output_traj = NULL;
  
  std::ostream* output_fp = NULL;
  std::ostream* output_pop_probtraj = NULL;
  std::ostream* output_simple_pop_probtraj = NULL;
  
#ifdef USE_DYNAMIC_BITSET
  MBDynBitset::init_pthread();
#endif

  try {
    time_t start_time, end_time;

    PopNetwork* pop_network = new PopNetwork();
    pop_network->parse(ctbndl_file);

    RunConfig* runconfig = new RunConfig();

    if (generate_config_template) {
      IStateGroup::checkAndComplete(pop_network);
      runconfig->generateTemplate(pop_network, std::cout, PopMaBEstEngine::VERSION);
      return 0;
    }

    if (setConfigVariables(pop_network, prog, runconfig_var_v)) {
      return 1;
    }      

    for (const auto & cfg : runconfig_file_or_expr_v) {
      if (cfg.isExpr()) {
        runconfig->parseExpression(pop_network, (cfg.getExpr() + ";").c_str());
      } else {
        runconfig->parse(pop_network, cfg.getFile().c_str());
      }
    }

    IStateGroup::checkAndComplete(pop_network);

    pop_network->getSymbolTable()->checkSymbols();

    if (check) {
      return 0;
    }

    if (generate_logical_expressions) {
      pop_network->generateLogicalExpressions(std::cout);
      return 0;
    }

    if (dump_config) {
      runconfig->dump(pop_network, std::cout, PopMaBEstEngine::VERSION);
      return 0;
    }

    if (runconfig->displayTrajectories()) {
      if (runconfig->getThreadCount() > 1) {
        if (!MaBoSS_quiet) {
          std::cerr << '\n' << prog << ": warning: cannot display trajectories in multi-threaded mode\n";
        }
      } else {
	        output_traj = new std::ofstream((std::string(output) + "_traj.txt").c_str());
      }
    }

    output_run = new std::ofstream((std::string(output) + "_run.txt").c_str());
    output_fp = new std::ofstream((std::string(output) + "_fp" + format_extension(format)).c_str());
    output_pop_probtraj = new std::ofstream((std::string(output) + "_pop_probtraj" + format_extension(format)).c_str());
    output_simple_pop_probtraj = new std::ofstream((std::string(output) + "_simple_pop_probtraj" + format_extension(format)).c_str());
    
    time(&start_time);
    PopMaBEstEngine mabest(pop_network, runconfig);
    mabest.run(output_traj);
    
    ProbTrajDisplayer<PopNetworkState>* pop_probtraj_displayer;
    FixedPointDisplayer* fp_displayer;
    
    if (format == CSV_FORMAT) {
      pop_probtraj_displayer = new CSVSimplePopProbTrajDisplayer(pop_network, *output_pop_probtraj, *output_simple_pop_probtraj, hexfloat);
      fp_displayer = new CSVFixedPointDisplayer(pop_network, *output_fp, hexfloat);
    } else if (format == JSON_FORMAT) {
      pop_probtraj_displayer = new JSONSimpleProbTrajDisplayer(pop_network, *output_pop_probtraj, *output_simple_pop_probtraj, hexfloat);
      // Use CSV displayer for fixed points as the Json one is not fully implemented
      fp_displayer = new CSVFixedPointDisplayer(pop_network, *output_fp, hexfloat);
    } else {
      pop_probtraj_displayer = NULL;
      fp_displayer = NULL;
    }

    mabest.display(pop_probtraj_displayer, fp_displayer);
    
    time(&end_time);

    mabest.displayRunStats(*output_run, start_time, end_time);

    ((std::ofstream*)output_run)->close();
    delete output_run;
    if (NULL != output_traj) {
      ((std::ofstream*)output_traj)->close();
      delete output_traj;
    }
    
    ((std::ofstream*)output_pop_probtraj)->close();
    delete output_pop_probtraj;
    
    ((std::ofstream*)output_fp)->close();
    delete output_fp;
      
    ((std::ofstream*)output_simple_pop_probtraj)->close();
    delete output_simple_pop_probtraj;
    
    delete runconfig;
    delete pop_network;

    Function::destroyFuncMap();  
    
  } catch(const BNException& e) {
    std::cerr << '\n' << prog << ": " << e;
    return 1;
  }

#ifdef USE_DYNAMIC_BITSET
  MBDynBitset::end_pthread();
  MBDynBitset::stats();
#endif
  return 0;
}
