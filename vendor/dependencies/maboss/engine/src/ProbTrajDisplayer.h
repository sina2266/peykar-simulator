
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
     ProbTrajDisplayer.h

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     Decembre 2020
*/

#ifndef _PROBTRAJ_DISPLAYER_H_
#define _PROBTRAJ_DISPLAYER_H_

#include <iostream>
#include "BooleanNetwork.h"
#include "Utils.h"
#include <iomanip>

template <typename S>
class ProbTrajDisplayer {

public:
  Network* network;
  bool hexfloat;
  bool compute_errors;
  size_t maxcols;
  size_t max_simplecols;
  size_t refnode_count;

  size_t current_line;
  // current line
  double time_tick;
  double TH, err_TH, H;
  double* HD_v;
  
  struct Proba {
    S state;
    double proba;
    double err_proba;

    Proba(const S& state, double proba, double err_proba) : state(state), proba(proba), err_proba(err_proba) { }
  };

  std::vector<Proba> proba_v;

  ProbTrajDisplayer(Network* network, bool hexfloat = false) : network(network), hexfloat(hexfloat), current_line(0), HD_v(NULL) { }

// public:
  void begin(bool compute_errors, size_t maxcols, size_t max_simplecols, size_t refnode_count) {
    this->compute_errors = compute_errors;
    this->refnode_count = refnode_count;
    this->maxcols = maxcols;
    this->max_simplecols = max_simplecols;
    this->HD_v = new double[refnode_count+1];
    beginDisplay();
  }

  void beginTimeTick(double time_tick) {
    this->time_tick = time_tick;
    proba_v.clear();
    beginTimeTickDisplay();
  }

  void setTH(double TH) {
    this->TH = TH;
  }

  void setErrorTH(double err_TH) {
    this->err_TH = err_TH;
  }

  void setH(double H) {
    this->H = H;
  }

  void setHD(unsigned int ind, double HD) {
    this->HD_v[ind] = HD;
  }

  void addProba(const S& state, double proba, double err_proba) {
    proba_v.push_back(Proba(state, proba, err_proba));
  }

  void endTimeTick() {
    endTimeTickDisplay();
    current_line++;
  }

  void end() {
    endDisplay();
  }

  virtual void beginDisplay() = 0;
  virtual void beginTimeTickDisplay() = 0;

  virtual void endTimeTickDisplay() = 0;
  virtual void endDisplay() = 0;

  virtual ~ProbTrajDisplayer() { delete[] HD_v; }
};


template <typename S>
class CSVProbTrajDisplayer : public ProbTrajDisplayer<S> {

  std::ostream& os_probtraj;

public:
  CSVProbTrajDisplayer(Network* network, std::ostream& os_probtraj, bool hexfloat = false) : ProbTrajDisplayer<S>(network, hexfloat), os_probtraj(os_probtraj) { }

  virtual void beginDisplay() {
    os_probtraj << "Time\tTH" << (this->compute_errors ? "\tErrorTH" : "") << "\tH";
    for (unsigned int jj = 0; jj <= this->refnode_count; ++jj) {
      os_probtraj << "\tHD=" << jj;
    }

    for (unsigned int nn = 0; nn < this->maxcols; ++nn) {
      os_probtraj << "\tState\tProba" << (this->compute_errors ? "\tErrorProba" : "");
    }

    os_probtraj << '\n';
  }
  virtual void beginTimeTickDisplay() {}
  virtual void endTimeTickDisplay() {
    os_probtraj << std::setprecision(4) << std::fixed << this->time_tick;
  #ifdef HAS_STD_HEXFLOAT
    if (this->hexfloat) {
      os_probtraj << std::hexfloat;
    }
  #endif
    if (this->hexfloat) {
      os_probtraj << '\t' << fmthexdouble(this->TH);
      os_probtraj << '\t' << fmthexdouble(this->err_TH);
      os_probtraj << '\t' << fmthexdouble(this->H);
    } else {
      os_probtraj << '\t' << this->TH;
      os_probtraj << '\t' << this->err_TH;
      os_probtraj << '\t' << this->H;
    }

    for (unsigned int nn = 0; nn <= this->refnode_count; nn++) {
      os_probtraj << '\t';
      if (this->hexfloat) {
        os_probtraj << fmthexdouble(this->HD_v[nn]);
      } else {
        os_probtraj << this->HD_v[nn];
      }
    }

    for (const typename ProbTrajDisplayer<S>::Proba &proba : this->proba_v) {
      os_probtraj << '\t';
      proba.state.displayOneLine(os_probtraj, this->network);
      if (this->hexfloat) {
        os_probtraj << '\t' << fmthexdouble(proba.proba);
        os_probtraj << '\t' << fmthexdouble(proba.err_proba);
      } else {
        os_probtraj << '\t' << std::setprecision(6) << proba.proba;
        os_probtraj << '\t' << proba.err_proba;
      }
    }
    os_probtraj << '\n';
  }
  virtual void endDisplay() {}
};

template <typename S>
class JSONProbTrajDisplayer : public ProbTrajDisplayer<S> {

  std::ostream& os_probtraj;

public:
  JSONProbTrajDisplayer(Network* network, std::ostream& os_probtraj, bool hexfloat = false) : ProbTrajDisplayer<S>(network, hexfloat), os_probtraj(os_probtraj) { }

  virtual void beginDisplay() {
    // void JSONProbTrajDisplayer<N, S>::beginDisplay() {
    os_probtraj << '[';
  }

  virtual void beginTimeTickDisplay() {
    if (this->current_line > 0) {
      os_probtraj << ',';
    }
    os_probtraj << '{';
  }
  
  virtual void endTimeTickDisplay() {
    os_probtraj << "\"tick\":" << std::setprecision(4) << std::fixed << this->time_tick << ",";
    if (this->hexfloat) {
      os_probtraj << "\"TH\":" << fmthexdouble(this->TH, true) << ",";
      os_probtraj << "\"ErrorTH\":"  << fmthexdouble(this->err_TH, true) << ",";
      os_probtraj << "\"H\":" << fmthexdouble(this->H, true) << ",";
    } else {
      os_probtraj << "\"TH\":" << this->TH << ",";
      os_probtraj << "\"ErrorTH\":" << this->err_TH << ",";
      os_probtraj << "\"H\":" << this->H << ",";
    }
    
    os_probtraj << "\"HD\":[";
    for (unsigned int nn = 0; nn <= this->refnode_count; nn++) {
      if (this->hexfloat) {
        os_probtraj << fmthexdouble(this->HD_v[nn], true);
      } else {
        os_probtraj << this->HD_v[nn];
      }
      if (nn != this->refnode_count) {
        os_probtraj << ",";
      }
    }
    os_probtraj << "],";

    os_probtraj << "\"probas\":[";
    unsigned int idx = 0;
    for (const typename ProbTrajDisplayer<S>::Proba &proba : this->proba_v) {
      os_probtraj << "{\"state\":\"";
      proba.state.displayJSON(os_probtraj, this->network);
      os_probtraj << "\",";
      if (this->hexfloat) {
        os_probtraj << "\"proba\":" << fmthexdouble(proba.proba, true) << ",";
        os_probtraj << "\"err_proba\":" << fmthexdouble(proba.err_proba, true);
      } else {
        os_probtraj << "\"proba\":" << std::setprecision(6) << proba.proba << ",";
        os_probtraj << "\"err_proba\":" << proba.err_proba;
      }
      os_probtraj << "}";
      if (idx < this->proba_v.size()-1) {
        os_probtraj << ",";
      }
      idx++;
    }
    os_probtraj << "]";
    os_probtraj << '}';
  }
  virtual void endDisplay() {
    os_probtraj << ']';
  }
};

/*
class NumPyProbTrajDisplayer : public ProbTrajDisplayer {

  std::ostream& os_probtraj;
  std::ostream& os_probtraj_summary;
  NumPyInfo* info;
  std::map<...> cumul_info;

public:
  NumPyProbTrajDisplayer(Network* network, NumPyInfo* info, std::ostream& os_probtraj, std::ostream& os_probtraj_summary, bool hexfloat = false) : ProbTrajDisplayer(network, hexfloat), os_probtraj(os_probtraj) { }

  virtual void beginDisplay();
  virtual void beginTimeTickDisplay();
  virtual void endTimeTickDisplay();
  virtual void endDisplay();
};
*/

#endif
