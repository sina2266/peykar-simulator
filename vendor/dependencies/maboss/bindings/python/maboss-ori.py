# coding: utf-8
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
# Module: maboss.py
# Authors: Eric Viara <viara@sysra.com>
# Date: May 2018

import os, sys, time, signal, socket
import atexit

MABOSS_SERVER = "../../engine/src/MaBoSS-server" # for now

class MaBoSS:

    def __init__(self, host = None, port = None):
        self._host = host
        self._port = port
        self._client = Client(host, port)

    def getHost(self):
        return self._host

    def getPort(self):
        return self._port

    def close(self):
        self._client.close()

class RunConfiguration:
    pass

class BooleanNetwork:
    pass

class StateProba:
    # State
    # Proba
    # ErrorProba

    def __init__(self, state, proba, error_proba = None):
        self._state = state
        self._proba = proba
        self._error_proba = error_proba

    def getState(self):
        return self._state

    def getProba(self):
        return self._proba

    def getErrorProba(self):
        return self._error_proba

class Trajectory:

    def __init(self, num, time, chain, final_state):
        self._num = num
        self._time = time
        self._chain = chain
        self._final_state = final_state

    def getNum(self):
        return self._num

    def getTime(self):
        return self._time

    def getChain(self):
        return self._chain

    def getFinalState(self):
        return self._final_state

class TrajectoryProbability:
    # Time    TH      ErrorTH H       HD=0    State   Proba   ErrorProba      State   Proba   ErrorProba      State   Proba  ErrorProba      State   Proba   ErrorProba      State   Proba   ErrorProba      State   Proba   ErrorProba      State   Proba   ErrorProba      State   Proba   ErrorProba

    def __init__(self, num, time, TH, errorTH, HD, stateProbaList):
        self._num = num
        self._time = time
        self._TH = TH
        self._errorTH = errorTH
        self.stateProbaList = stateProbaList

    def getNum(self):
        return self._num

    def getTime(self):
        return self._time

    def getTH(self):
        return self._TH

    def getErrorTH(self):
        return self._errorTH

    def getStateProbaList(self):
        return self._stateProbaList

class StationaryDistributionItem:
    # Trajectory      State   Proba   State   Proba   State   Proba   State   Proba   State   Proba   State   Proba   State Proba   State   Proba   State   Proba   State   Proba   State   Proba   State   Proba   State   Proba   State   Proba   State   Proba   State   Proba 
    def __init__(self, num, stateProbaList):
        self._num = num
        self.stateProbaList = stateProbaList

    def getNum(self):
        return self._num

    def getStateProbaList(self):
        return self._stateProbaList

class ResultOutput:

    def __init__(self, trajectories, trajectoryProbabilities, stationaryDistribution, fixedPoints, runLog):
        self._trajectories = trajectories
        self._trajectoryProbabilities = trajectoryProbabilities
        self._stationaryDistribution = stationaryDistribution
        self._fixedPoints = fixedPoints
        self._runLog = runLog
        
    def getTrajectories(self):
        return self._trajectories

    def getTrajectoryProbabilities(self):
        return self._trajectoryProbabilities

    def getStationaryDistribution(self):
        return self._stationaryDistribution

    def getFixedPoints(self):
        return self._fixedPoints

    def getRunLog(self):
        return self._runLog

# communication

class DataStreamer:
    pass

class ClientData:

    def __init__(self, network = None, config = None):
        self._network = network
        self._config = config

    def setNetwork(self, network):
        self._network = network

    def setConfig(self, config):
        self._config = config


class ServerData:
    pass

class Client:
    
    SERVER_NUM = 1 # for now

    def __init__(self, host = None, port = None):
        self._host = host
        self._port = port
        self._pid = None
        self._mb = bytearray()
        self._mb.append(0)
        self._pidfile = None

        if host == None:
            if port == None:
                port = '/tmp/MaBoSS_pipe_' + str(os.getpid()) + "_" + str(Client.SERVER_NUM)

            self._pidfile = '/tmp/MaBoSS_pidfile_' + str(os.getpid()) + "_" + str(Client.SERVER_NUM)
            Client.SERVER_NUM += 1

            try:
                pid = os.fork()
            except OSError, e:
                print >> sys.stderr, "error fork:", e
                return

            if pid == 0:
                try:
                    args = [MABOSS_SERVER, "--host", "localhost", "--port", port, "--pidfile", self._pidfile]
                    os.execv(MABOSS_SERVER, args)
                except e:
                    print >> sys.stderr, "error execv:", e

            self._pid = pid
            atexit.register(self.close)
            server_started = False
            MAX_TRIES = 20
            TIME_INTERVAL = 0.1
            for try_cnt in range(MAX_TRIES):
                if os.path.exists(self._pidfile):
                    server_started = True
                    break
                time.sleep(TIME_INTERVAL)

            if not server_started:
                raise Exception("MaBoSS server on port " + port + " not started after " + str(MAX_TRIES*TIME_INTERVAL) + " seconds")

            self._socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            self._socket.connect(port)
        else:
            self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._socket.connect((host, port))
            
        # connection test
        self._socket.send('coucou')
        self._term()

    def send(self, client_data):
        pass

    def _term(self):
        self._socket.send(self._mb)

    def close(self):
        if self._pid != None:
            print "kill", self._pid
            os.kill(self._pid, signal.SIGTERM)
            if self._pidfile:
                os.remove(self._pidfile)
            self._pid = None

#        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#        self._socket.connect((host, port))
#        print "Connection on {}".format(port)
#        self._socket.send('coucou les lapins')

