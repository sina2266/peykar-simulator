/*
 * vcellhybrid.h
 *
 *  Created on: Oct 20, 2015
 *      Author: gweatherby
 */

#ifndef BRIDGEVCELLSMOLDYN_VCELLHYBRID_H_
#define BRIDGEVCELLSMOLDYN_VCELLHYBRID_H_
#include <cassert>
#include <string>
struct simstruct;
class SimTool;
typedef simstruct * simptr;

/**
* manage hybrid / standlone smolydn status
*/
class vcellhybrid {
	public:
		static simptr smoldynInit(SimTool* simTool, std::string& fileName);
		static void smoldynOneStep(simptr sim);
		static void smoldynEnd(simptr sim);
		/**
		 * Whether the Smoldyn input asks for particle positions to be saved: VCell's "save particle files"
		 * option writes `incrementfile <base>.smoldynOutput` and `listmols <base>.smoldynOutput` commands.
		 * In a hybrid run those commands are disabled at load (smoldynInit) and the positions are written
		 * at the PDE's output times instead (writeParticlePositions), because Smoldyn's command clock
		 * counts Smoldyn steps, which differ from PDE steps when SMOLDYN_STEP_MULTIPLIER > 1.
		 */
		static bool savesParticlePositions( ) {
			return bSaveParticlePositions;
		}
		/**
		 * Write every live molecule as `species(state) x y z` (the format of Smoldyn's listmols, which VCell
		 * reads as particle data), one line per molecule, always with three coordinates.
		 */
		static void writeParticlePositions(simptr sim, const std::string& fileName);
		static bool isHybrid( ) {
			return bHybrid;
		}
		static void setHybrid( ) {
			bHybrid = true;
		}
		static void resetHybrid( ) {
			bHybrid = false;
		}
		static void setTaskId(int t) {
			assert(!bHybrid);
			taskID = t;
		}
		static int getTaskId( ) {
			return taskID;
		}
		static bool isMessaging( ) {
			return !bHybrid && (taskID >= 0) ;
		}
	private:
		static bool bHybrid;
		static int taskID;
		static bool bSaveParticlePositions;
};

#endif /* BRIDGEVCELLSMOLDYN_VCELLHYBRID_H_ */
