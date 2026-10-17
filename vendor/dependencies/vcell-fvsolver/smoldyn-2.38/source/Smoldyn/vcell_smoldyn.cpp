/* Originating from code written by Steven Andrews, started 10/22/2001.
This is the VCell entry point for the Smoldyn program.
 See documentation called Smoldyn_doc1.pdf and Smoldyn_doc2.pdf, and the Smoldyn
 website, which is at www.smoldyn.org.
 This work is distributed under the terms of the Gnu Lesser General Public License (LGPL). */


#include "opengl2.h"
#include "smoldyn.h"
#include "random2.h"
#include "smoldynfuncs.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sstream>
#include <cstdarg>
#include <VCELL/SimulationMessaging.h>
#include <VCELL/GitDescribe.h>
#include "SimpleValueProvider.h"
#include "SimpleMesh.h"
#include "vcellhybrid.h"
#include <iostream>
#include <vector>

namespace {
    void slogger(simptr, int code, const char *, ...);

    const std::string warnMessage();
}

/* ***************************************************************** */
/* ********************** main() segment *************************** */
/* ***************************************************************** */


#ifdef USE_MESSAGING
	const char * const Variant = "messaging";
#else
const char *const Variant = "stdout";
#endif



/* runSmoldyn*/
int runSmoldyn(int argc, char *argv[]) {
    std::vector<std::string> providedArgs{};
    providedArgs.reserve(argc); // pre-allocate space for efficiency
    for (int i = 0; i < argc; i++) providedArgs.emplace_back(argv[i]);

    const int errMsgLen = 2048;
    char errorMsg[errMsgLen];
    int exitCode = 0;
    simptr sim;
    try {
        simSetThrowing(10);
        simSetLogging(stdout, slogger);
        SimulationMessaging::create();

        //clear the errormesage
        memset(errorMsg, 0, errMsgLen * sizeof(char));

        int i, er, pflag, qflag, wflag, tflag, Vflag, oflag;
        char root[STRCHAR], fname[STRCHAR], flags[STRCHAR], *cptr;

        for (i = 0; i < STRCHAR; i++) root[i] = fname[i] = flags[i] = '\0';
        er = 0;
        if (argc <= 1) {
            fprintf(stderr, "Welcome to Smoldyn version %s.\n\n",VERSION);
            fprintf(stderr, "Enter name of configuration file: ");
            fgets(root,STRCHAR,stdin);
            if (strchr(root, '\n')) *(strchr(root, '\n')) = '\0';
            fprintf(stderr, "Enter runtime flags (q=quiet, p=parameters only), or '-'=none: ");
            fgets(flags,STRCHAR,stdin);
            if (strchr(flags, '\n')) *(strchr(flags, '\n')) = '\0';
        }
        if (argc > 1) {
            strncpy(root, argv[1],STRCHAR - 1);
            root[STRCHAR - 1] = '\0';
            argc--;
            argv++;
        } // Remove the executable name from sight!
        er = Parse_CmdLineArg(&argc, argv,NULL);
        if (er) {
            if (er == 1) fprintf(stderr, "Out of memory");
            else fprintf(stderr, "Follow command line '--define' options with key=replacement\n");
            return 0;
        }
        if (argc > 1) {
            if (argv[1][0] == '-') {
                // -tid is always at the end of argument list
                // and it is used in conjunction with PBS
                // should not be used when called from command line manually
                if (strcmp(argv[1], "-tid")) {
                    strncpy(flags, argv[1],STRCHAR - 1);
                    flags[STRCHAR - 1] = '\0';
                    strcpy(SimFlags, flags);
                    argc--;
                    argv++;
                }
            } else {
                fprintf(stderr, "Command line format: smoldyn [config_file] [-options] [-OpenGL_options]\n");
                return 0;
            }
        }

        cptr = strrpbrk(root, ":\\/");
        if (cptr) cptr++;
        else cptr = root;
        strcpy(fname, cptr);
        *cptr = '\0';

        oflag = strchr(flags, 'o') ? 1 : 0;
        pflag = strchr(flags, 'p') ? 1 : 0;
        qflag = strchr(flags, 'q') ? 1 : 0;
        Vflag = strchr(flags, 'V') ? 1 : 0;
        if (!strcmp(fname, "-V")) Vflag = 1;
        wflag = strchr(flags, 'w') ? 1 : 0;
        tflag = strchr(flags, 't') ? 1 : 0;

        if (Vflag) {
            simLog(NULL, 4, "%s\n",VERSION);
            return 0;
        }
        sim = NULL;

        if (argc > 2) {
            //-tid [int]
            if (!strcmp(argv[1], "-tid")) {
                {
                    int taskID = -1;
                    sscanf(argv[2], "%d", &taskID);
                    vcellhybrid::setTaskId(taskID);
                    argc -= 2;
                    argv += 2; //pointer arithmetic; remove vcell messaging args before passing to smoldyn
                }
            }
        }

        er = simInitAndLoad(root, fname, &sim, flags, NULL, NULL);
        if (!er) {
            if (!tflag && sim->graphss && sim->graphss->graphics != 0)
                gl2glutInit(&argc, argv);
            er = simUpdateAndDisplay(sim);
        }
        if (!oflag && !pflag && !er) {
            wflag = 1; //always overwrite smoldyn output for VCell
            er = scmdopenfiles((cmdssptr) sim->cmds, wflag);
        }
        if (pflag || er) {
            simLog(sim, 4, "Simulation skipped\n");
            strcpy(errorMsg, "Invalid input arguments");
            exitCode = 2;
        } else {
            fflush(stdout);
            fflush(stderr);
            if (tflag || !sim->graphss || sim->graphss->graphics == 0) {
                er = smolsimulate(sim);
                WorkerEvent *we;
                endsimulate(sim, er);
                if (er <= 1) {
                    we = new WorkerEvent(JOB_COMPLETED, 1.0, sim->time);
                } else {
                    const double completeRatio = sim->time / sim->tmax;
                    const std::string warn = warnMessage();
                    we = new WorkerEvent(JOB_COMPLETED, completeRatio, sim->time, warn.c_str());
                }
                SimulationMessaging::getInstVar()->setWorkerEvent(we);
            } else {
                smolsimulategl(sim);
            }
        }
        simfree(sim);
        simfuncfree();
    } catch (const char *errmsg) {
        fprintf(stderr, "%s\n", errmsg);
        exitCode = 1;
    }
    catch (int stop) {
        // stopped by user
    }
    catch (...) {
        fprintf(stderr, "unknown error\n");
        exitCode = 1;
    }

    if (SimulationMessaging::getInstVar() == NULL) {
        if (exitCode != 0) {
            simLog(sim, 10, "%s\n", errorMsg);
        }
    } else if (!SimulationMessaging::getInstVar()->isStopRequested()) {
        if (exitCode != 0) {
            SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_FAILURE, errorMsg));
        }
#ifdef USE_MESSAGING
		SimulationMessaging::getInstVar()->waitUntilFinished();
#endif
    }
    delete SimulationMessaging::getInstVar();

    return exitCode;
}

namespace {
    std::ostringstream warning;

    void slogger(simptr, int code, const char *fmt, ...) {
        if (code >= 5) {
            const size_t bsize = 1024;
            char buff[bsize];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buff, bsize, fmt, args);
            va_end(args);
            warning << buff;
        }
    }

    const std::string warnMessage() {
        warning << std::ends;
        std::string tmp(warning.str());
        return tmp;
    }
}
