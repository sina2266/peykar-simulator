#ifdef _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
	  #define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
	  #define NOMINMAX
	#endif
	#ifndef NOGDI
	  #define NOGDI
	#endif
	#include <windows.h>
	// Defensive cleanup for common macro collisions from legacy/project headers
	#ifdef Source
		#undef Source
	#endif
	#ifdef Destination
		#undef Destination
	#endif
	#ifdef POINT
		#undef POINT
	#endif
	#ifdef FIXED
		#undef FIXED
	#endif
#endif

#include <MPoint.h>
#include <map>
#include <set>
#include <iomanip>
#include <limits>
#include <algo.h>
#include <Mesh.h>
#include <MeshElementNode.h>
#include <Logger.h>
#include <Timer.h>
#include <MovingBoundaryParabolicProblem.h>
#include <World.h>
#include <vcellxml.h>
#include <ReportClient.h>
#include <Universe.h>
#include <TextReportClient.h>
#include <flex.h>
#include <vhdf5/dataset.h>
#include <vhdf5/attribute.h>
#include <vhdf5/suppressor.h>
#include <vhdf5/facade.h>
#include <vhdf5/file.h>
#include <vhdf5/vlen.h>
#include <vhdf5/exception.h>
#include <MBridge/MatlabDebug.h>
#include <MBridge/FronTierAdapt.h>
#include <MBridge/Figure.h>
#include <Hdf5OutputWriter.h>
#include <JobMessaging.h>
#include <VCELL/GitDescribe.h>
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
using moving_boundary::World ;
using moving_boundary::CoordinateType;
using moving_boundary::Volume2DClass;
namespace {

	/**
	* Plain old data point type
	*/
	template <class T>
	struct PODPoint {
		T x;
		T y;
		PODPoint(T x_ = 0, T y_ = 0)
			:x(x_),
			y(y_) {}
		static H5::DataType getType( ) {
			H5::CompType pointType( sizeof (PODPoint) );
			pointType.insertMember("x",HOFFSET(PODPoint,x),vcellH5::TPredType<T>::predType( ));
			pointType.insertMember("y",HOFFSET(PODPoint,y),vcellH5::TPredType<T>::predType( ));
			return pointType;
		}
		PODPoint(const spatial::TPoint<double,2> & in)
			:x(in(spatial::cX)),
			y(in(spatial::cY)) {}
		/**
		* singleton function for var len type; must be method due to static initialization
		* dependencies
		*/ 
		static vcellH5::VarLen<PODPoint<T> > & vectorType( );
	};

	struct HElementRecord {
		double volume;
		std::vector<double> mass;
		std::vector<double> concentration;
		char  boundaryPosition; 
		std::vector<PODPoint<double> > controlVolume;
		static const char inactivePosition = 'X';
		HElementRecord( )
			:volume(),
			mass(),
			concentration(),
			boundaryPosition(inactivePosition),
			controlVolume( ){ }
		/**
		* size mass and concentration vectors
		* @param s new size
		*/
		void resize(size_t s) {
			mass.resize(s);
			concentration.resize(s);
		}
		void clear( ) {
			volume = 0;
			mass.assign(mass.size( ), 0);
			concentration.assign(mass.size( ), 0);
		}
	};

	/**
	* aggregates results for Hdf5 writing
	*/
	struct ResultPoint {
		ResultPoint( )
			:volume(0),
			boundaryPosition(0),
			volumePoints( )
		{ }
		void set(const HElementRecord & er) {
			static vcellH5::VarLen<PODPoint<double> > & vpointType = PODPoint<double>::vectorType( );
			static vcellH5::VarLenSimple<double>  valueType;
			volume = er.volume;
			boundaryPosition = er.boundaryPosition;
			volumePoints = vpointType.adapt(er.controlVolume);
		}
		double volume;
		char boundaryPosition;
		hvl_t  volumePoints; 
		static H5::CompType getType() {
			using H5::PredType;
			H5::PredType dtype = vcellH5::TPredType<double>::predType( ); 
			H5::PredType ctype = vcellH5::TPredType<char>::predType( ); 
			vcellH5::VarLenSimple<double> vlen;
			H5::DataType arrayType = vlen.getType( );
			H5::CompType resultPointType(sizeof(ResultPoint));
			resultPointType.insertMember("volume", HOFFSET(ResultPoint,volume),dtype);
			resultPointType.insertMember("boundaryPosition", HOFFSET(ResultPoint,boundaryPosition),ctype);
			resultPointType.insertMember("volumePoints", HOFFSET(ResultPoint,volumePoints),PODPoint<double>::vectorType( ).getType( ));
			return resultPointType;
		}
	};

	/**
	* aggregates results for Hdf5 writing
	*/
	struct SpeciesData {
		SpeciesData( )
			:mass( ),
			concentrationNumeric( )
		{ }
		void set(const HElementRecord & er, size_t index) {
			static vcellH5::VarLenSimple<double>  valueType;
			mass = er.mass[index];
			concentrationNumeric = er.concentration[index];
		}
		double mass;
		double concentrationNumeric;
		static H5::CompType getType() {
			H5::PredType dtype = vcellH5::TPredType<double>::predType( ); 
			H5::CompType resultPointType(sizeof(SpeciesData));
			resultPointType.insertMember("mass", HOFFSET(SpeciesData,mass),dtype);
			resultPointType.insertMember("uNumeric", HOFFSET(SpeciesData,concentrationNumeric),dtype);
			return resultPointType;
		}
	};

	class TimeReport {
		double startTime;
	public:
		static const unsigned int STD_PRIORITY = 10;
		TimeReport(double startTime_)
			:startTime(startTime_) {}
		virtual ~TimeReport( ){};
		double getStartTime( ) const {
			return startTime;
		}
		/**
		* allow object to expire itself
		* @return false (default)
		*/
		virtual bool expired(double time, int generation) const {
			return false;
		}
		/**
		* return relative priority for objects with same start time. Lower is higher priority
		* @returns 10
		*/
		virtual unsigned int priority( ) const {
			return STD_PRIORITY;
		}
		virtual bool needReport(unsigned int generation, unsigned int lastReportGeneration, double time, double lastReportTime) const = 0;
	};

	class TimeReportStep : public TimeReport {
		unsigned long step;
	public:
		TimeReportStep(double startTime, unsigned long step_)
			:TimeReport(startTime),
			step(step_) {}
		virtual bool needReport(unsigned int generation, unsigned int lastReportGeneration, double time, double lastReportTime) const {
			return generation - lastReportGeneration >= step;
		}
	};

	class TimeReportInterval : public TimeReport {
		double interval;
	public:
		TimeReportInterval(double startTime, double interval_)
			:TimeReport(startTime),
			interval(interval_) {}
		virtual bool needReport(unsigned int generation, unsigned int lastReportGeneration, double time, double lastReportTime) const {
			return std::abs(time - lastReportTime - interval) < 1e-8;
		}
	};

	/**
	* TimeReport which reports beginning generation
	*/
	struct TimeReportBegin : public TimeReport {
		TimeReportBegin( )
			:TimeReport(0) {}
		virtual bool needReport(unsigned int generation, unsigned int lastReportGeneration, double time, double lastReportTime) const {
			return generation == 0; 
		}
		/**
		* expire after first time
		*/
		virtual bool expired(double time, int generation) const {
			return time > 0.0;
		}
		/**
		* @return 0 (highest) 
		*/
		virtual unsigned int priority( ) const {
			return 0;
		}
	};

	/**
	* TimeReport which never reports; acts as placeholder
	* @param startTime
	* @param priority lower is higher priority, "standard" value is 10
	*/
	struct TimeReportQuiet : public TimeReport {

		TimeReportQuiet(long startTime, unsigned int priority = STD_PRIORITY)
			:TimeReport(startTime),
			priorityValue(priority) {}

		/**
		* never wants report
		*/
		virtual bool needReport(unsigned int generation, unsigned int lastReportGeneration, double time, double lastReportTime) const {
			return false; 
		}
		/**
		* never expires 
		*/
		virtual bool expired(double time, int generation) const {
			return false; 
		}
		/**
		* @return priorty constructed with 
		*/
		virtual unsigned int priority( ) const {
			return priorityValue; 
		}
	private:
		const unsigned int priorityValue;
	};

	/**
	* dummy placeholder so set iterator always has next element
	*/
	struct CollectionTail : public TimeReport {
		CollectionTail( ) 
			:TimeReport(std::numeric_limits<unsigned long>::max( )) {}
		virtual bool needReport(unsigned int generation, unsigned int lastReportGeneration, double time, double lastReportTime) const {
			return false; 
		}
	};

	struct TimeSorter {
		bool operator( )(const TimeReport * lhs, const TimeReport * rhs) {
			const double left =  lhs->getStartTime( );
			const double right =  rhs->getStartTime( );
			if (left < right) {
				return true;
			}
			if (right < left) {
				return false;
			}
			//if equal, use priorities
			const unsigned int lPriority = lhs->priority( );
			const unsigned int rPriority = rhs->priority( );
			if (lPriority < rPriority) {
				return true;
			}
			if (rPriority < lPriority) {
				return false;
			}
			VCELL_EXCEPTION(logic_error, "Duplicate start time " << left << " and priority " << lPriority);
		}
	};

	struct HDF5Client :public moving_boundary::ReportClient {
		typedef World<CoordinateType,2> WorldType;
		/**
		* HDF5 spatial chunk size (x and y dimensions)
		*/
		static const size_t spatialChunkSize = 10;
		/**
		* HDF5 spatial chunk size (time dimension)
		*/
		static const size_t timeChunkSize = 50;

		/**
		* worldDim, speciesDim index
		*/
		static const size_t timeArrayIndex = 0;

		/**
		* worldDim, speciesDim index
		*/
		static const size_t xArrayIndex = 1;

		/**
		* worldDim, speciesDim index
		*/
		static const size_t yArrayIndex = 2;

		/**
		* speciesDim index
		*/
		static const size_t speciesIndex = 3;

		static constexpr const char* H5_FILE_EXT = ".h5";
		static constexpr const char* LOG_FILE_EXT = ".log";

		/**
		* @param f file to write to
		* @param mbpp the problem
		* @param baseName name of dataset in HDF5 file if not default
		*/
		//template <typename R>
		HDF5Client(std::string xml_,std::string& baseFileName, int steps, double interval,
			WorldType & world_,
			const moving_boundary::MovingBoundaryParabolicProblem &mbpp)
			: ReportClient(baseFileName),
			xml(xml_),
			theProblem(mbpp),
			constantSim(mbpp.noReaction( )),
			currentTime(0),
			totalStuff(0),
			oldStuff(0),
			meshDef(mbpp.meshDef( )),
			numVolumeVariables(mbpp.physiology( )->numVolumeVariables( )),
			eRecords(),
			genTimes(),
			moveTimes(),
			timeStepTimes( ),
			lastTimeStep(-1), //invalid value, to trigger recording
			elementStorage(),
			elementDataset( ),
			worldDim( ),
			speciesDataset( ),
			speciesDim( ),
			boundaryDataset( ),
			boundaryDim( ),
			//reportStep(calcReportStep(mbpp,startTime_,numberReports)),
			//reportCounter(0),
			//reportBegan(startTime_ == 0), //if beginning at zero, we've "begun" at the start
//			reportActive(true),
			timer( ),
			world(world_),
//			reportControllers( ),
			lastReportTime(0),
			lastReportGeneration(0),
//			reportControl(nullptr),
			nextReportControlTime(-1),
			pointconverter(world.pointConverter( ))
		{
			string h5FileName = baseFileName + H5_FILE_EXT;
			remove(h5FileName.c_str());
			h5File = vcellH5::VH5File(h5FileName.c_str( ), H5F_ACC_TRUNC|H5F_ACC_RDWR);
			baseGroup = h5File;

			hdf5OutputWriter = new Hdf5OutputWriter(xml_, baseFileName, h5File, steps, interval, world_, mbpp);

			using spatial::cX;
			using spatial::cY;
			int numberGenerations = mbpp.numberTimeSteps( );
			timer.start( );
//			reportControllers.insert(new TimeReportBegin( ));
//			reportControllers.insert(new TimeReportQuiet(0, std::numeric_limits<unsigned int>::max(  )) );
//			for (typename R::iterator iter = timeReports.begin( );iter != timeReports.end( ); ++iter) {
//				reportControllers.insert(*iter);
//			}
//			reportControllers.insert(new CollectionTail( ));
//			determineReportControl(0,0);

			{ //create group

				//std::string groupName;
				//if (baseName != nullptr) {
				//	groupName = baseName;
				//}
				//else {
				//	std::ostringstream oss;
				//	oss << "result-" << numberGenerations << '-' <<  meshDef.numCells(cX)  << '-' <<meshDef.numCells(cY);
				//	groupName = oss.str( );
				//}
				//{
				//	vcellH5::Suppressor s; //no error message if not there
				//	H5Ldelete( file.getLocId(), groupName.c_str( ), H5P_DEFAULT );
				//}
				//std::cerr << "creating " << groupName << std::endl;
				//baseGroup = file.createGroup(groupName);
				const double & bts = mbpp.frontTimeStep( );
				vcellH5::writeAttribute(baseGroup,"requestedTimeStep",bts);
				const double scaleFactor = world.theScale( );
				vcellH5::writeAttribute(baseGroup,"scaleFactor",scaleFactor);
				const double halfStep = world.distanceToProblemDomain(1) / 2.0;
				vcellH5::writeAttribute(baseGroup,"precision",halfStep);
			} //create group


			{ //create element dataset
				const size_t xSize = meshDef.numCells(cX);
				const size_t ySize = meshDef.numCells(cY);
				worldDim[timeArrayIndex] = timeChunkSize;
				worldDim[xArrayIndex] = xSize;
				worldDim[yArrayIndex] = ySize;
				hsize_t     maxdim[3]= {H5S_UNLIMITED,xSize,ySize};
				H5::DataSpace dataspace(3,worldDim,maxdim);

				H5::DSetCreatPropList  prop;
				hsize_t     chunkDim[3]  = {timeChunkSize,spatialChunkSize,spatialChunkSize};
				prop.setChunk(3, chunkDim);
				H5::CompType dataType = ResultPoint::getType( );

				elementDataset = baseGroup.createDataSet( "elements", dataType, dataspace ,prop);


				//record attributes
				const double startx = world.toProblemDomain( meshDef.startCorner(spatial::cX), spatial::cX);
				const double starty = world.toProblemDomain( meshDef.startCorner(spatial::cY), spatial::cY);
				const double hx = world.distanceToProblemDomain( meshDef.interval(spatial::cX) );
				const double hy = world.distanceToProblemDomain( meshDef.interval(spatial::cY) );

				spatial::TGeoLimit<moving_boundary::CoordinateType> limit =  world.limits( )[spatial::cX];
				const double beginx = world.toProblemDomain (limit.low( ), spatial::cX);
				const double endx = world.toProblemDomain (limit.high( ), spatial::cX);
				assert(beginx == startx);

				limit =  world.limits( )[spatial::cY];
				const double beginy = world.toProblemDomain (limit.low( ), spatial::cY);
				const double endy = world.toProblemDomain (limit.high( ), spatial::cY);
				assert(beginy == starty);

				vcellH5::writeAttribute(elementDataset,"startX",startx);
				vcellH5::writeAttribute(elementDataset,"startY",starty);
				vcellH5::writeAttribute(elementDataset,"endX",endx);
				vcellH5::writeAttribute(elementDataset,"endY",endy);
				vcellH5::writeAttribute(elementDataset,"numX",xSize);
				vcellH5::writeAttribute(elementDataset,"numY",ySize);
				vcellH5::writeAttribute(elementDataset,"hx",hx);
				vcellH5::writeAttribute(elementDataset,"hy",hy);
				const std::string layout("time x X x Y (transposed in MATLAB)");
				vcellH5::writeAttribute(elementDataset,"layout",layout);
				std::string desc = theProblem.frontDescription( );
				vcellH5::writeAttribute(elementDataset,"front description",desc);
			} //create element dataset

			{ //meshX & mesh Y
				std::vector<moving_boundary::CoordinateType> xvalues = meshDef.coordinateValues(spatial::cX);
				std::vector<double> dv(xvalues.size( ));
				WorldType::XConverter xconverter(world);
				std::transform(xvalues.begin( ),xvalues.end( ),dv.begin( ),xconverter);
				vcellH5::SeqFacade<std::vector<double> > axisSF(dv);
				vcellH5::facadeWrite(baseGroup,"meshXvalues",axisSF);

				std::vector<moving_boundary::CoordinateType> yvalues = meshDef.coordinateValues(spatial::cY);
				dv.resize(yvalues.size( ));
				WorldType::YConverter yconverter(world);
				std::transform(yvalues.begin( ),yvalues.end( ),dv.begin( ),yconverter);
				vcellH5::facadeWrite(baseGroup,"meshYvalues",axisSF);
			} //create element dataset

			{ //create species  dataset
				speciesDim[timeArrayIndex] = worldDim[timeArrayIndex];
				speciesDim[xArrayIndex] = worldDim[xArrayIndex];
				speciesDim[yArrayIndex] = worldDim[yArrayIndex];
				speciesDim[speciesIndex] = numVolumeVariables;
				hsize_t     maxsdim[4] = {H5S_UNLIMITED, speciesDim[1],speciesDim[2],speciesDim[3]};
				H5::DataSpace dataspace(4,speciesDim,maxsdim);

				H5::DSetCreatPropList  prop;
				hsize_t     chunkDim[4]  = {timeChunkSize,spatialChunkSize,spatialChunkSize,numVolumeVariables};
				prop.setChunk(4, chunkDim);
				H5::CompType dataType = SpeciesData::getType( );

				speciesDataset = baseGroup.createDataSet( "species", dataType, dataspace ,prop);
			}

			{ //create boundary dataset
				boundaryDim[0] = timeChunkSize;
				hsize_t     maxdim[1]= {H5S_UNLIMITED};
				H5::DataSpace dataspace(1,boundaryDim,maxdim);

				H5::DSetCreatPropList  prop;
				hsize_t     chunkDim[1]  = {timeChunkSize};
				prop.setChunk(1, chunkDim);
				vcellH5::VarLen<PODPoint<double> > & vtype = PODPoint<double>::vectorType( );

				boundaryDataset = baseGroup.createDataSet( "boundaries", vtype.getType( ), dataspace ,prop);
			} //create boundary dataset
		}


		/**
		* delete TimeReport objects
		*/
		~HDF5Client( ) {
//			std::for_each(reportControllers.begin( ),reportControllers.end( ),cleanup);
			delete hdf5OutputWriter;
		}

		/**
		* add information from MovingBoundarySetup; currently just the concentration string
		*/
		void addInitial(const moving_boundary::MovingBoundarySetup & mbs) {
			//XRAY const std::string s = mbs.concentrationFunctionStr;
			//vcellH5::writeAttribute(baseGroup,"concentrationFunction",s);
		}
		/**
		* free form annotation of data set for including notes in HDF file
		* @param attributeName
		* @param value
		*/
		void annotate(const char *attributeName, const std::string & value) {
			vcellH5::writeAttribute(baseGroup,attributeName,value);
		}

		/**
		* set #reportControl and #nextReportControlTime
		*/
		/*
		void determineReportControl(double t, unsigned int generation) {
			std::set<const TimeReport *,TimeSorter>::const_iterator iter = reportControllers.begin( );
			bool pastTime = t>=nextReportControlTime;
			//delete any expired or past time
			while (iter != reportControllers.end( )) {
				const TimeReport *tr = *iter;
				std::set<const TimeReport *,TimeSorter>::iterator eraseIter(iter);
				++iter;
				if (tr->expired(t,generation) || (pastTime && tr->getStartTime( ) < nextReportControlTime )) {
					const TimeReport *dtr = *eraseIter;
					reportControllers.erase(eraseIter);
					delete dtr;
				}
			}
			nextReportControlTime = std::numeric_limits<double>::max( );
			for (iter = reportControllers.begin( );iter != reportControllers.end( ); ++iter) {
				double st = (*iter)->getStartTime( );
				if (st > t) {
					nextReportControlTime = st;
					break;
				}
			}
			iter = reportControllers.begin( );
			if (iter == reportControllers.end( )) {
				VCELL_EXCEPTION(logic_error,"No time reporter for time " << t << ", generation " << generation);
			}
			reportControl = *iter;
		}
		*/

		virtual void time(double t, unsigned int generationCounter, bool last, const moving_boundary::GeometryInfo<moving_boundary::CoordinateType> & geometryInfo) {
			hdf5OutputWriter->time(t, generationCounter, last, geometryInfo);

			double ts = theProblem.frontTimeStep( );
			if (ts != lastTimeStep) {
				timeStepTimes.push_back(t);
				timeStep.push_back(ts);
				lastTimeStep = ts;
			}
			if (geometryInfo.nodesAdjusted) {
				moveTimes.push_back(t);
			}
//			if (reportControl->expired(t,generationCounter) || t >= nextReportControlTime) {
//				determineReportControl(t,generationCounter);
//			}
//
//			reportActive = last || reportControl->needReport(generationCounter,lastReportGeneration,t,lastReportTime);

			if (hdf5OutputWriter->shouldReport()) {
				writeBoundary(genTimes.size( ),geometryInfo.boundary);
				currentTime = t;
				totalStuff = 0;
				std::cout << "generation " << std::setw(2) <<  generationCounter << " time " << currentTime << " end " << std::endl;
				VCELL_KEY_LOG(trace,Key::generationTime,"generation " << std::setw(2) <<  generationCounter << " time " << currentTime);
				genTimes.push_back(t);
				lastReportGeneration = generationCounter;
				lastReportTime = t;
			}
		}

		void writeBoundary(hsize_t timeIndex, const std::vector<spatial::TPoint<moving_boundary::CoordinateType,2> > & boundary) {
			try {
				moving_boundary::WorldToPDPointConverter<moving_boundary::CoordinateType,2> converter = world.pointConverter( );
				std::vector<PODPoint<double> > outVector(boundary.size( ));
				//std::transform(boundary.begin( ),boundary.end( ),outVector.begin( ),converter);
				std::transform(boundary.begin( ),boundary.end( ),outVector.begin( ),convertFrontToPOD);

				vcellH5::VarLen<PODPoint<double> > & vtype = PODPoint<double>::vectorType( );

				hvl_t variableBoundaryData = vtype.adapt(outVector);

				const hsize_t singleTimeSlice = 1;
				hsize_t  bufferDim[1] = {singleTimeSlice};
				H5::DataSpace memoryspace(1,bufferDim);

				//is dataset big enough in time dimension?
				if (timeIndex >= boundaryDim[0]) {
					boundaryDim[0] += timeChunkSize;
					boundaryDataset.extend(boundaryDim);
				}

				hsize_t offset[1] = {timeIndex};
				H5::DataSpace dataspace = boundaryDataset.getSpace( );
				dataspace.selectHyperslab(H5S_SELECT_SET,bufferDim,offset);

				boundaryDataset.write(&variableBoundaryData,vtype.getType( ),memoryspace,dataspace);
			} catch (H5::Exception & e) {
				throw vcellH5::Exception(e);
			}
		}


		static PODPoint<double> convertFrontToPOD(const spatial::TPoint<moving_boundary::CoordinateType,2> &in) {
			moving_boundary::World<moving_boundary::CoordinateType,2> world =  moving_boundary::World<moving_boundary::CoordinateType,2>::get( );
			moving_boundary::WorldToPDPointConverter<moving_boundary::CoordinateType,2> converter = world.pointConverter( );
			PODPoint<double> p = converter(in);
			return p;
		};

		char encodePosition(const moving_boundary::MeshElementNode &e) {
			switch (e.mPos( )) {
			case spatial::interiorSurface:
				return 'I';
			case spatial::boundarySurface:
				return 'B';
			case spatial::outsideSurface:
			case spatial::unsetPosition:
			default:
				assert(0);  //we don't currently expect any of these
				return HElementRecord::inactivePosition;
			}
		}

		/**
		* state of inside / boundary nodes
		*/
		virtual void element(const moving_boundary::MeshElementNode &e) {
			if (hdf5OutputWriter->shouldReport()) {
				using spatial::cX;
				using spatial::cY;
				spatial::TPoint<size_t,2> key(e.indexOf(0),e.indexOf(1));
				if (eRecords.find(key) == eRecords.end( )) {
					eRecords[key] = HElementRecord( );
					eRecords[key].resize(numVolumeVariables);
				}
				HElementRecord & er = eRecords[key];
				er.volume = e.volumePD( );
				for (int i = 0; i < numVolumeVariables; i++) {
					er.mass[i] = e.mass(i);
					er.concentration[i] = e.concentration(i);
				}
				er.boundaryPosition = encodePosition(e);
				Volume2DClass::VectorOfVectors vOfv = e.getControlVolume().points( );
				if (vOfv.size( ) > 1) {
					//throw std::domain_error("multi region control volumes not supported yet");
					std::cerr << "multi region warning" << std::endl;
					{
						static int multiCounter = 0;
						std::string mcounter("multiregion");
						mcounter.pop_back( ); //remove null
						mcounter += multiCounter;
						std::string filename(mcounter);
						filename += ".m";
						std::ofstream mr(filename);

						using matlabBridge::MatLabDebug;
						matlabBridge::Scatter nbplot('b',2);
						frontTierAdapt::copyPointInto(nbplot,e);
						std::stringstream ss;
						ss << e.indexOf(cX) << ',' << e.indexOf(cY);
						mr <<  matlabBridge::FigureName(mcounter) <<
							nbplot << matlabBridge::Text(e(cX),e(cY),ss.str( ).c_str( ));

						matlabBridge::Polygons vs("-g",2);
						frontTierAdapt::copyVectorsInto(vs,vOfv);
						mr << vs;
					}

				}
				Volume2DClass::PointVector & pVec = vOfv.front( );
				er.controlVolume.resize(pVec.size( ));
				std::transform(pVec.begin( ),pVec.end( ),er.controlVolume.begin( ),pointconverter);

				if (constantSim) {
					totalStuff += er.mass[0];
				}
			}
		}
		/**
		* notify client they've received all elements
		*/
		virtual void iterationComplete( ) {
			if (hdf5OutputWriter->shouldReport()) {
				VCELL_LOG(info,"Time " << currentTime << " total mass " << totalStuff);
				//VCELL_LOG(info,"Time " << currentTime);
				if (eRecords.size( ) > 0) {

					try {
						//determine size of buffer needed for current generation
						size_t minI, maxI, minJ, maxJ;
						minI = minJ = std::numeric_limits<size_t>::max( );
						maxI = maxJ = std::numeric_limits<size_t>::min( );
						for (RecordMap::const_iterator iter = eRecords.begin( ); iter != eRecords.end( ); ++iter) {
							if (iter->second.boundaryPosition != HElementRecord::inactivePosition) {
								size_t i = iter->first(spatial::cX);
								size_t j = iter->first(spatial::cY);
								minI = std::min(minI,i);
								minJ = std::min(minJ,j);
								maxI = std::max(maxI,i);
								maxJ = std::max(maxJ,j);
							}
						}
						const size_t iSpan = maxI - minI + 1;
						const size_t jSpan = maxJ - minJ + 1;
						elementStorage.reindex(iSpan,jSpan).reset( );
						speciesStorage.reindex(iSpan,jSpan,numVolumeVariables).reset( );

						size_t timeIndex = genTimes.size( ) - 1;

						for (RecordMap::iterator iter = eRecords.begin( ); iter != eRecords.end( ); ++iter) {
							HElementRecord & er = iter->second;
							if (er.boundaryPosition != HElementRecord::inactivePosition) {
								const spatial::TPoint<size_t,2> & index = iter->first;
								hsize_t i = index(spatial::cX);
								hsize_t j = index(spatial::cY);
								elementStorage[i - minI][j - minJ].set(er);
								for (int s = 0; s < numVolumeVariables; ++s) {
									speciesStorage[i - minI][j - minJ][s].set(er,s);
								}
							}
							er.boundaryPosition = HElementRecord::inactivePosition;
						}
						const size_t singleTimeSlice = 1;
						//are datasets big enough in time dimension?
						if (timeIndex >= worldDim[timeArrayIndex]) {
							worldDim[timeArrayIndex] += timeChunkSize;
							elementDataset.extend(worldDim);
							speciesDim[timeArrayIndex] += timeChunkSize;
							speciesDataset.extend(speciesDim);
						}
						{ //first write out the 3D element data
							hsize_t  bufferDim[3] = {singleTimeSlice,iSpan, jSpan};
							H5::DataSpace memoryspace(3,bufferDim);

							hsize_t offset[3] = {timeIndex ,minI,minJ};
							H5::DataSpace dataspace = elementDataset.getSpace( );
							dataspace.selectHyperslab(H5S_SELECT_SET,bufferDim,offset);

							H5::CompType dataType = ResultPoint::getType( );
							elementDataset.write(elementStorage.ptr( ),dataType,memoryspace,dataspace);
						}

						{ //next the 4D species data
							hsize_t  bufferDim[4] = {singleTimeSlice,iSpan, jSpan,numVolumeVariables};
							H5::DataSpace memoryspace(4,bufferDim);

							hsize_t offset[4] = {timeIndex ,minI,minJ,0};
							H5::DataSpace dataspace = speciesDataset.getSpace( );
							dataspace.selectHyperslab(H5S_SELECT_SET,bufferDim,offset);

							H5::CompType dataType = SpeciesData::getType( );
							speciesDataset.write(speciesStorage.ptr( ),dataType,memoryspace,dataspace);
						}
					}

					catch (H5::Exception &e) {
						throw vcellH5::Exception(e);
					}
				}
				/*
				std::ofstream fa("finalareas.m");
				std::ofstream fp("finalpolygons.m");
				theProblem.plotAreas(fa);
				theProblem.plotPolygons(fp);
				*/


				if (constantSim) {
					if (oldStuff != 0 && !spatial::nearlyEqual(oldStuff,totalStuff,1e-3)) {
						simulationComplete( ); //write out final info
						VCELL_EXCEPTION(logic_error, "mass not conserved old"<< oldStuff << " , new " << totalStuff
							<< ", gain(+)/loss(-) " << (totalStuff - oldStuff));
					}
					oldStuff = totalStuff;
				}
				h5File.flush(H5F_SCOPE_GLOBAL);
				if (SimulationMessaging *messaging = moving_boundary::jobMessaging()) {
					messaging->setWorkerEvent(JobEvent::JOB_DATA, currentTime/theProblem.endTime(), currentTime);
				}
			}
		}

		void simulationComplete( ) {
			try {
				timer.stop( );
				const double totalTime = timer.elapsed( );
				vcellH5::primitiveWrite(baseGroup,"endTime",currentTime);
				H5::DataSet runTime = vcellH5::primitiveWrite(baseGroup,"runTime",totalTime);
				//annotate version info - we place on runTime node to allow easy exclusion when doing
				//h5diff (see --exclude-path)
                std::string gitVersion(g_GIT_DESCRIBE);
				vcellH5::writeAttribute(runTime,"gitVersion", gitVersion);
				unsigned int lastTimeIndex = static_cast<unsigned int>(genTimes.size( ));
				vcellH5::primitiveWrite(baseGroup,"lastTimeIndex",lastTimeIndex);

				VCELL_KEY_LOG(info,Key::generationTime,"logging " << genTimes.size( ) << " generation times");
				vcellH5::SeqFacade<std::vector<double> > gt(genTimes);
				vcellH5::facadeWrite(baseGroup,"generationTimes",gt);

				vcellH5::SeqFacade<std::vector<double> > mt(moveTimes);
				vcellH5::facadeWrite(baseGroup,"moveTimes",mt);

				vcellH5::SeqFacade<std::vector<double> > tst(timeStepTimes);
				vcellH5::facadeWrite(baseGroup,"timeStepTimes",tst);

				vcellH5::SeqFacade<std::vector<double> > ts(timeStep);
				vcellH5::facadeWrite(baseGroup,"timeStep",ts);
				const double sts = theProblem.solverTimeStep( );
				vcellH5::primitiveWrite(baseGroup,"solverTimeStep",sts);
			}
			catch (H5::Exception &e) {
				throw vcellH5::Exception(e);
			}
		}
		virtual std::string getXML( ) const {
			return xml;
		}
		virtual std::string outputName( ) const {
			return h5File.getFileName( );
		}

	private:
		/**
		* for_each function for destructor
		* @param tr to delete
		*/
		static void cleanup(const TimeReport *tr) {
			delete tr;
		}

		const moving_boundary::MovingBoundaryParabolicProblem &theProblem;
		const bool constantSim;

		/**
		* XML used to create
		*/
		std::string xml;
		H5::H5File h5File;
		double currentTime;
		double totalStuff;
		double oldStuff;
		const spatial::MeshDef<moving_boundary::CoordinateType,2> meshDef;
		const size_t numVolumeVariables;
		typedef std::map<spatial::TPoint<size_t,2>, HElementRecord> RecordMap;
		RecordMap eRecords;
		std::vector<double> genTimes;
		std::vector<double> moveTimes;
		std::vector<double> timeStepTimes;
		std::vector<double> timeStep;
		double lastTimeStep;
		//H5::Group baseGroup; if we want to put individual data sets in groups
		H5::H5File baseGroup; //synonum for "file"
		/**
		* element information for a single time slice
		*/
		vcell_util::Flex2<ResultPoint> elementStorage;
		/**
		* 3 dimensional HDF data
		*/
		H5::DataSet elementDataset;
		hsize_t worldDim[3];

		/**
		* species information for a single time slice
		*/
		vcell_util::Flex3<SpeciesData> speciesStorage;
		/**
		* 4 dimensional HDF data
		*/
		H5::DataSet speciesDataset;
		hsize_t speciesDim[4];

		H5::DataSet boundaryDataset;
		hsize_t boundaryDim[1];
//		bool reportActive;
		vcell_util::Timer timer;
		const WorldType & world;
//		std::set<const TimeReport *,TimeSorter> reportControllers;
		double lastReportTime;
		unsigned long lastReportGeneration;
//		const TimeReport * reportControl;
		/**
		* start time of next TimeReport object not currently active
		*/
		double nextReportControlTime;

		WorldType::PointConverter pointconverter;

		Hdf5OutputWriter *hdf5OutputWriter;
	};


	/**
	* declare static function of ResultPoint. Can't be a static field because
	* H5:PredType constants may have been initialized yet when fields initialized due to
	* order of initialization of static fields
	* see http://stackoverflow.com/questions/1005685/c-static-initialization-order
	*/
	using vcellH5::VarLen;
	template <class T>
	VarLen<PODPoint<T> > &  PODPoint<T>::vectorType( ) {
		static VarLen<PODPoint<double> > instance(PODPoint<double>::getType( ));
		return instance;
	}
}
namespace tinyxml2 {
	//credit: http://sourceforge.net/p/tinyxml/discussion/42748/thread/820b0377/?limit=25
	XMLNode *deepCopy( const XMLNode & src, XMLDocument & destDoc )
	{
		XMLNode *current = src.ShallowClone( &destDoc );
		for( const XMLNode *child=src.FirstChild(); child != nullptr ; child=child->NextSibling() )
		{
			current->InsertEndChild( deepCopy( *child, destDoc ) );
		}

		return current;
	}
}

using moving_boundary::ReportClient; 
using moving_boundary::TextReportClient;
using tinyxml2::XMLElement;
void ReportClient::setup(const XMLElement &root, const std::string & h5filename , moving_boundary::MovingBoundaryParabolicProblem &mbpp) {
	std::string xmlCopy;
	{
		tinyxml2::XMLDocument doc;
		//tinyxml2::XMLNode *xn = DeepClone(root,&doc);
		tinyxml2::XMLNode *xn = deepCopy(root,doc); 
		doc.InsertFirstChild(xn);
		tinyxml2::XMLPrinter printer;
		doc.Print( &printer);
		xmlCopy = printer.CStr( ); 
	}

	try {
		const moving_boundary::MovingBoundarySetup & mbs = mbpp.setup( );
		const XMLElement & report = vcell_xml::get(root,"report");
		H5::H5File output;
		std::string h5FileName = h5filename;
		std::string outputFilePrefix = vcell_xml::convertChildElement<std::string>(report,"outputFilePrefix");
#ifdef INCLUDE_DETAILS_IN_OUTPUT_FILE_NAME
		const std::array<Universe<2>::CountType, 2>& numNodes = Universe<2>::get().numNodes();
		const std::array<spatial::GeoLimit, 2>& limits = Universe<2>::get().limits();
		std::stringstream ss;
		ss << outputFilePrefix << "_Nx" << numNodes[0] << "_t" << mbs.maxTime * 10;
		outputFilePrefix = ss.str();
#endif
		bool deleteExisting = vcell_xml::convertChildElementWithDefault<bool>(report,"deleteExisting",false);
		if (h5filename.empty( )) {
			h5FileName = outputFilePrefix + HDF5Client::H5_FILE_EXT;
			if (deleteExisting) {
				remove(h5FileName.c_str( ));
			}
		}
//		const char *datasetName = nullptr;
//		std::pair<bool,std::string> dnq = vcell_xml::queryElement<std::string>(report,"datasetName");
//		if (dnq.first) {
//			datasetName = dnq.second.c_str( );
//		}

		std::vector<TimeReport *> timeReports;

		std::pair<bool,unsigned int> nr = vcell_xml::queryElement<unsigned int>(report,"numberReports");
		if (nr.first) {
			std::cerr << "numberReports XML element deprecated" << std::endl;
			const double startTime = vcell_xml::convertChildElementWithDefault<double>(report,"startTime",0);
			unsigned int nts = mbpp.numberTimeSteps( );
			unsigned int rs = nts /  nr.second;
			if (startTime >0) { //if not beginning, scale based on fraction of time reported
				const double end = mbpp.endTime( );
				unsigned int scaled = static_cast<unsigned int>(nts * (end -startTime) / end);
				rs = scaled / nr.second; 
			}
			if (rs < 1) {
				rs = 1;
			}
			timeReports.push_back( new TimeReportStep(startTime,rs));
		}

		const XMLElement * timeReport = report.FirstChildElement("timeReport"); 
		long step = -1;
		double interval = -1;
		if(timeReport != nullptr) {
			const int NOT_THERE = -1;
			const double startTime = vcell_xml::convertChildElement<double>(*timeReport,"startTime");
			step = vcell_xml::convertChildElementWithDefault<long>(*timeReport,"step", NOT_THERE);
			interval = vcell_xml::convertChildElementWithDefault<double>(*timeReport,"interval",NOT_THERE);
			bool quiet = timeReport->FirstChildElement("quiet") != nullptr;
			int nsubs = 0;

//			double cfl = 1;
//			step = (int)(0.1/(cfl*mbpp.meshInterval()*mbpp.meshInterval()/4))+1;
//			if (step != NOT_THERE) {
//				nsubs++;
//				timeReports.push_back( new TimeReportStep(startTime,step) );
//			}
//			if (interval != NOT_THERE) {
//				nsubs++;
//				timeReports.push_back( new TimeReportInterval(startTime,interval) );
//			}
//			if (quiet) {
//				nsubs++;
//				timeReports.push_back( new TimeReportQuiet(startTime) );
//			}
//			if (nsubs != 1) {
//				throw std::invalid_argument("XML error exactly one of <step>, <interval>, or <quiet> must be specified per timeReport element");
//			}
//			timeReport = timeReport->NextSiblingElement("timeReport");
		}

		moving_boundary::World<moving_boundary::CoordinateType,2> &world = moving_boundary::World<moving_boundary::CoordinateType,2>::get( );
		HDF5Client *hdf5Client = new HDF5Client(xmlCopy, outputFilePrefix, (int)step, interval, world, mbpp);
		mbpp.add(*hdf5Client);

		const tinyxml2::XMLElement *tr = vcell_xml::query(report, "textReport");
		if (tr != nullptr) {
			const int p = vcell_xml::convertChildElementWithDefault<unsigned int>(*tr, "precision", 6);
			const int w = vcell_xml::convertChildElementWithDefault<unsigned int>(*tr, "width", 6);
			TextReportClient* trcClient = new TextReportClient(outputFilePrefix, mbpp, p, w, step);
			mbpp.add(*trcClient);
		}
	} 
	catch (H5::Exception &h5e) {
		std::string h5Msg = h5e.getDetailMsg( );
		h5Msg += " ";
		h5Msg += h5e.getFuncName( );
		throw std::runtime_error(h5Msg);
	}
}


namespace {
	template struct PODPoint<double>;
}
