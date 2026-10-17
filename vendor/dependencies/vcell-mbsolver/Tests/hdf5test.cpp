#pragma warning ( disable : 4996 )
#include <array>
#include <limits>
#include "gtest/gtest.h"
#include <H5Cpp.h>
#include <flex.h>
#include <vhdf5/dataset.h>
#include <vhdf5/attribute.h>
#include <vhdf5/facade.h>
#include <vhdf5/vlen.h>
using std::cout;
using std::endl;

const H5std_string	FILE_NAME( "SDSd.h5" );
const H5std_string	DATASET_NAME( "DoubleArray" );
const int 	NX = 5;                    // dataset dimensions
const int 	NY = 6;
const int 	RANK = 2;

using namespace H5;

TEST(hdf5,create) {

	/*
	* Data initialization.
	*/
	int i, j;
	double data[NX][NY];          // buffer for data to write
	for (j = 0; j < NX; j++)
	{
		for (i = 0; i < NY; i++)
			data[j][i] = i + j;
	}
	/*
	* 0 1 2 3 4 5
	* 1 2 3 4 5 6
	* 2 3 4 5 6 7
	* 3 4 5 6 7 8
	* 4 5 6 7 8 9
	*/

	// Try block to detect exceptions raised by any of the calls inside it
	try
	{
		/*
		* Turn off the auto-printing when failure occurs so that we can
		* handle the errors appropriately
		*/
		Exception::dontPrint();

		/*
		* Create a new file using H5F_ACC_TRUNC access,
		* default file creation properties, and default file
		* access properties.
		*/
		H5File file( FILE_NAME, H5F_ACC_TRUNC );

		/*
		* Define the size of the array and create the data space for fixed
		* size dataset.
		*/
		hsize_t     dimsf[2];              // dataset dimensions
		dimsf[0] = NX;
		dimsf[1] = NY;
		DataSpace dataspace( RANK, dimsf );

		/*
		* Define datatype for the data in the file.
		* We will store little endian INT numbers.
		*/
		FloatType datatype( PredType::NATIVE_DOUBLE );
		//datatype.setOrder( H5T_ORDER_LE );

		/*
		* Create a new dataset within the file using defined dataspace and
		* datatype and default dataset creation properties.
		*/
		DataSet dataset = file.createDataSet( DATASET_NAME, datatype, dataspace );

		/*
		* Write the data to the dataset using default memory space, file
		* space, and transfer properties.
		*/
		dataset.write( data, PredType::NATIVE_DOUBLE );
	}  // end of try block

	// catch failure caused by the H5File operations
	catch( FileIException error )
	{
		error.printErrorStack();
	}

	// catch failure caused by the DataSet operations
	catch( DataSetIException error )
	{
		error.printErrorStack();
	}

	// catch failure caused by the DataSpace operations
	catch( DataSpaceIException error )
	{
		error.printErrorStack();
	}

	// catch failure caused by the DataSpace operations
	catch( DataTypeIException error )
	{
		error.printErrorStack();
	}
}



TEST(hdf5,compound) {
	struct SolutionPoint {
		double x;
		double y;
		double time;
		double concentrationNumeric;
		double concentrationExact;
	};

	try {
		CompType solutionPointType( sizeof(SolutionPoint) );
		solutionPointType.insertMember("x", HOFFSET(SolutionPoint,x),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("y", HOFFSET(SolutionPoint,y),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("time", HOFFSET(SolutionPoint,time),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("uNumeric", HOFFSET(SolutionPoint,concentrationNumeric),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("uExact", HOFFSET(SolutionPoint,concentrationExact),PredType::NATIVE_DOUBLE);

		H5File file( "compound.h5", H5F_ACC_TRUNC );

		const int N = 100;
		SolutionPoint sps[N];
		hsize_t     dimsf[1];              // dataset dimensions
		dimsf[0] = N;
		DataSpace dataspace( 1, dimsf );

		SolutionPoint scratch;
		scratch.x = scratch.y = scratch.time = 0.8;
		scratch.concentrationNumeric= .5;
		scratch.concentrationExact = .45;

		for (int i = 0; i < N; i++) {
			scratch.x +=  .4;
			if (i%10 == 0) {
				scratch.y += .7;
				scratch.x = 0;
			}
			if (i%20 == 0) {
				scratch.time += .3;
			}
			scratch.concentrationExact += .02;
			scratch.concentrationNumeric += .021;
			sps[i] = scratch;
		}

		DataSet dataset = file.createDataSet( "demo", solutionPointType, dataspace );
		dataset.write(sps, solutionPointType); 
		//add an attribute
		DataSpace attrSpace(H5S_SCALAR);
		Attribute hxAttr = dataset.createAttribute("hx",PredType::NATIVE_DOUBLE,attrSpace);
		double hx = 0.4;
		hxAttr.write(PredType::NATIVE_DOUBLE,&hx);

	}
	catch (H5::Exception &e) {
		std::cerr << e.getDetailMsg( ) << std::endl;
	}
}

TEST(hdf5,slab) {
	struct SolutionPoint {
		double x;
		double y;
		double concentrationNumeric;
		double concentrationExact;
	};

	try {
		const int worldSize = 90;
		const int bufferSize = 40;
		const int chunkSize = 10;
		CompType solutionPointType( sizeof(SolutionPoint) );
		solutionPointType.insertMember("x", HOFFSET(SolutionPoint,x),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("y", HOFFSET(SolutionPoint,y),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("uNumeric", HOFFSET(SolutionPoint,concentrationNumeric),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("uExact", HOFFSET(SolutionPoint,concentrationExact),PredType::NATIVE_DOUBLE);

		H5File file( "slab.h5", H5F_ACC_RDWR );

		vcell_util::Flex2<SolutionPoint,hsize_t> buffer(bufferSize,bufferSize);

		//SolutionPoint buffer[bufferSize][bufferSize]; 
		hsize_t     worldDim[2]  = {worldSize,worldSize};
		hsize_t     chunkDim[2]  = {chunkSize, chunkSize};             // dataset dimensions
		hsize_t     bufferDim[2]  = {bufferSize, bufferSize};             // dataset dimensions
		//hsize_t     maxdim[2]= {H5S_UNLIMITED,H5S_UNLIMITED};              // dataset dimensions
		DataSpace dataspace(2,worldDim); 
		DataSpace memoryspace(2,bufferDim); 

		SolutionPoint scratch;
		scratch.x = scratch.y = 0.8;
		scratch.concentrationNumeric= .5;
		scratch.concentrationExact = .45;

		DSetCreatPropList  prop;
		prop.setChunk(2, chunkDim);
		file.unlink("sparse2");
		DataSet dataset = file.createDataSet( "sparse2", solutionPointType, dataspace ,prop);
		std::cout << "world size " << sizeof(SolutionPoint) *worldSize * worldSize << std::endl;
		std::cout << "buffer size " << sizeof(buffer) << std::endl;
		size_t left = 15;
		size_t top = 25; 

		for (int i = 0; i < bufferSize; i++) {
			for (int j = 0; j < bufferSize; j++) {
				SolutionPoint & current = buffer[i][j];
				current.x = left + i + .1;
				current.y = top + j + .2; 
				current.concentrationNumeric = i + j;
				current.concentrationExact = i + j + .2;
			}
		}
		//const test
		{
			const vcell_util::Flex2<SolutionPoint,hsize_t> copy(buffer); 
			std::cout << copy[5][10].x << std::endl;
			//copy[5][10].x = .3;  //fails, like it should
		}


		hsize_t    offset[2] = {top,left}; 
		//hsize_t    stride[2] = {1,1}; 
		//hsize_t    block[2] = {1,1}; 
		//DataSpace setdataspace = dataset.getSpace( ); 
		//dataspace.selectHyperslab(H5S_SELECT_SET,bufferDim,offset,stride,block);
		dataspace.selectHyperslab(H5S_SELECT_SET,bufferDim,offset);
		hsize_t   zeros[2] = {0,0}; 
		//memoryspace.selectHyperslab(H5S_SELECT_SET,bufferDim,zeros,stride,block);
		memoryspace.selectHyperslab(H5S_SELECT_SET,bufferDim,zeros);
		dataset.write(buffer.ptr( ),solutionPointType,memoryspace,dataspace);


		//second write, near boundary
		bufferDim[1] = 10;
		buffer.reindex(bufferDim);
		DataSpace memoryspace2(2,bufferDim); 
		left = 80;
		top = 0;
		for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 10; j++) {
				SolutionPoint & current = buffer[i][j];
				current.x = 100 + i; 
				current.y = 100 + j; 
			}
		}
		hsize_t    nextOffset[2] = {top,left}; 
		dataspace.selectHyperslab(H5S_SELECT_SET,bufferDim,nextOffset);
		//memoryspace.selectHyperslab(H5S_SELECT_SET,bufferDim,zeros,stride,block);
		memoryspace2.selectHyperslab(H5S_SELECT_SET,bufferDim,zeros);
		dataset.write(buffer.ptr( ),solutionPointType,memoryspace2,dataspace);

	}
	catch (H5::Exception &e) {
		std::cerr << e.getDetailMsg( ) << std::endl;
	}
}

TEST(hdf5,slabExtend) {
	struct SolutionPoint {
		double x;
		double y;
		double concentrationNumeric;
		double concentrationExact;
	};

	try {
		const int worldSize = 90;
		const int bufferSize = 40;
		const int chunkSize = 10;
		CompType solutionPointType( sizeof(SolutionPoint) );
		solutionPointType.insertMember("x", HOFFSET(SolutionPoint,x),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("y", HOFFSET(SolutionPoint,y),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("uNumeric", HOFFSET(SolutionPoint,concentrationNumeric),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("uExact", HOFFSET(SolutionPoint,concentrationExact),PredType::NATIVE_DOUBLE);

		H5File file( "slabExtend.h5", H5F_ACC_TRUNC);

		vcell_util::Flex2<SolutionPoint,hsize_t> buffer(bufferSize,bufferSize);

		//SolutionPoint buffer[bufferSize][bufferSize]; 
		hsize_t     worldDim[3]  = {1,worldSize,worldSize};
		hsize_t     chunkDim[3]  = {1,chunkSize, chunkSize};             // dataset dimensions
		hsize_t     bufferDim[3]  = {1,bufferSize, bufferSize};             // dataset dimensions
		hsize_t     maxdim[3]= {H5S_UNLIMITED,worldSize,worldSize};
		DataSpace dataspace(3,worldDim,maxdim); 
		DataSpace memoryspace(3,bufferDim); 

		hsize_t timeIndex = 0;
		SolutionPoint scratch;
		scratch.x = scratch.y = 0.8;
		scratch.concentrationNumeric= .5;
		scratch.concentrationExact = .45;

		DSetCreatPropList  prop;
		prop.setChunk(3, chunkDim);
		DataSet dataset = file.createDataSet( "sparse", solutionPointType, dataspace ,prop);
		//quick test
		double hx = 5;
		vcellH5::writeAttribute(dataset,"hx",hx);
		hx = 10;
		vcellH5::writeAttribute(dataset,"hx",hx, true);
		size_t left = 15;
		size_t top = 25; 

		for (int i = 0; i < bufferSize; i++) {
			for (int j = 0; j < bufferSize; j++) {
				SolutionPoint & current = buffer[i][j];
				current.x = left + i + .1;
				current.y = top + j + .2; 
				current.concentrationNumeric = i + j;
				current.concentrationExact = i + j + .2;
			}
		}

		hsize_t    offset[3] = {timeIndex,top,left}; 
		dataspace.selectHyperslab(H5S_SELECT_SET,bufferDim,offset);
		hsize_t   zeros[3] = {0,0,0}; 
		//memoryspace.selectHyperslab(H5S_SELECT_SET,bufferDim,zeros,stride,block);
		memoryspace.selectHyperslab(H5S_SELECT_SET,bufferDim,zeros);
		dataset.write(buffer.ptr( ),solutionPointType,memoryspace,dataspace);

		//extend dataset
		timeIndex = 10;
		worldDim[0] = timeIndex + 1;
		dataset.extend(worldDim);
		dataspace = dataset.getSpace( );
		left = 10;
		top = 20; 
		offset[0] = timeIndex;
		offset[1] = top; 
		offset[2] = left; 


		for (int i = 0; i < bufferSize; i++) {
			for (int j = 0; j < bufferSize; j++) {
				SolutionPoint & current = buffer[i][j];
				current.x = 200 + left + i + .1;
				current.y = 200 + top + j + .2; 
				current.concentrationNumeric = i + j;
				current.concentrationExact = i + j + .2;
			}
		}

		dataspace.selectHyperslab(H5S_SELECT_SET,bufferDim,offset);
		//memoryspace.selectHyperslab(H5S_SELECT_SET,bufferDim,zeros,stride,block);
		//memoryspace.selectHyperslab(H5S_SELECT_SET,bufferDim,zeros);
		dataset.write(buffer.ptr( ),solutionPointType,memoryspace,dataspace);

	}
	catch (H5::Exception &e) {
		std::cerr << e.getDetailMsg( ) << std::endl;
	}
}

TEST(hdf5,extendCompound) {
	struct SolutionPoint {
		double x;
		double y;
		double time;
		double concentrationNumeric;
		double concentrationExact;
	};

	try {
		const int chunkSize = 10;
		CompType solutionPointType( sizeof(SolutionPoint) );
		solutionPointType.insertMember("x", HOFFSET(SolutionPoint,x),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("y", HOFFSET(SolutionPoint,y),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("time", HOFFSET(SolutionPoint,time),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("uNumeric", HOFFSET(SolutionPoint,concentrationNumeric),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("uExact", HOFFSET(SolutionPoint,concentrationExact),PredType::NATIVE_DOUBLE);

		H5File file( "compoundExtend.h5", H5F_ACC_TRUNC );

		const int N = 100;
		SolutionPoint sps[N];
		hsize_t     currentDim[1]  = {chunkSize};             // dataset dimensions
		hsize_t     chunkDim[1]  = {chunkSize};             // dataset dimensions
		hsize_t     maxdim[1]= {H5S_UNLIMITED};              // dataset dimensions
		DataSpace dataspace( 1, currentDim, maxdim); 
		DataSpace memoryspace( 1, chunkDim);

		SolutionPoint scratch;
		scratch.x = scratch.y = scratch.time = 0.8;
		scratch.concentrationNumeric= .5;
		scratch.concentrationExact = .45;

		DSetCreatPropList  prop;
		prop.setChunk( 1, chunkDim);
		DataSet dataset = file.createDataSet( "edemo", solutionPointType, dataspace ,prop);
		size_t offset = 0;
		size_t idx =0;
		for (int i = 0; i < N; i++) {
			scratch.x +=  .4;
			if (i%10 == 0) {
				scratch.y += .7;
				scratch.x = 0;
			}
			if (i%20 == 0) {
				scratch.time += .3;
			}
			scratch.concentrationExact += .02;
			scratch.concentrationNumeric += .021;
			sps[idx++] = scratch;
			if (i % 10 == (chunkSize - 1)) {
				if (offset == 0) {
					dataset.write(sps, solutionPointType); 
				}
				else {
					currentDim[0] += chunkSize;
					dataset.extend(currentDim);
					DataSpace extendSpace = dataset.getSpace( );
					hsize_t offseta[1];
					offseta[0] = offset;
					extendSpace.selectHyperslab(H5S_SELECT_SET,chunkDim,offseta);
					dataset.write(sps,solutionPointType,memoryspace,extendSpace);
				}
				offset += chunkSize;
				idx = 0;
			}
		}

	}
	catch (H5::Exception &e) {
		std::cerr << e.getDetailMsg( ) << std::endl;
	}

}

TEST(hdf5,static3D) {
	const int N = 2;
	struct Data {
		double x;
		double y;
		double concentrationNumeric[N];
		double concentrationExact[N];
	};

	try {
		hsize_t arraydim[] = {N};
		ArrayType arrayType(PredType::NATIVE_DOUBLE,1,arraydim);
		CompType solutionPointType( sizeof(Data) );
		solutionPointType.insertMember("x", HOFFSET(Data,x),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("y", HOFFSET(Data,y),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("uNumeric", HOFFSET(Data,concentrationNumeric),arrayType);
		solutionPointType.insertMember("uExact", HOFFSET(Data,concentrationExact),arrayType);

		H5File file( "compound3d.h5", H5F_ACC_TRUNC );

		const int dataSize = 50; 
		Data buffer;
		hsize_t     currentDim[2]  = {dataSize,dataSize};
		hsize_t     maxdim[2]= {H5S_UNLIMITED,H5S_UNLIMITED};              // dataset dimensionsH5S_UNLIMITED};              // dataset dimensions
		DataSpace dataspace( 2, currentDim);

		hsize_t     oneElement[2]= {1,1};
		DataSpace memoryspace( 1, oneElement); 

		DataSet dataset = file.createDataSet("data3d",solutionPointType,dataspace);
		const double hx = 0.2;
		const double hy = 0.3;
		for (size_t i = 0; i< dataSize; i++) {
			for (size_t j = 0; j< dataSize; j++) {
				buffer.x = hx *i;
				buffer.y = hy *j;
				size_t base = 100 * i + 10000 * j;
				for (size_t idx = 0; idx < N; idx++) {
					buffer.concentrationExact[idx] = static_cast<double>(base + idx);
					buffer.concentrationNumeric[idx] = static_cast<double>(base + idx) + 0.5;
				}
				DataSpace &dSpace = dataspace; 
				//DataSpace dSpace = dataset.getSpace( );
				hsize_t coord[2] = {i,j};
				dSpace.selectElements(H5S_SELECT_SET,1,coord);
				dataset.write(&buffer,solutionPointType,memoryspace,dSpace);
			}
		}
	}
	catch (H5::Exception &e) {
		std::cerr << e.getDetailMsg( ) << std::endl;
	}

}
namespace {
	template<int N>
	struct TSolutionPoint {
		double x;
		double y;
		double concentrationNumeric[N];
		double concentrationExact[N];
	};
}
TEST(hdf5,compoundArray) {


	try {
		typedef TSolutionPoint<100> SolutionPoint;
		hsize_t arraydim[] = {100};
		ArrayType arrayType(PredType::NATIVE_DOUBLE,1,arraydim);
		CompType solutionPointType( sizeof(SolutionPoint) );
		solutionPointType.insertMember("x", HOFFSET(SolutionPoint,x),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("y", HOFFSET(SolutionPoint,y),PredType::NATIVE_DOUBLE);
		solutionPointType.insertMember("uNumeric", HOFFSET(SolutionPoint,concentrationNumeric),arrayType);
		solutionPointType.insertMember("uExact", HOFFSET(SolutionPoint,concentrationExact),arrayType);

		H5File file( "compoundArray.h5", H5F_ACC_TRUNC );

		const int N = 100;
		std::array<SolutionPoint,N> sps;
		hsize_t     dimsf[1];              // dataset dimensions
		dimsf[0] = N;
		DataSpace dataspace( 1, dimsf );

		size_t idx = 0;
		for (double x = 0; x < 1.2; x+=.2)
			for (double y = 0; y < 1.5; y+=.3) {
				SolutionPoint scratch;
				scratch.x = x;
				scratch.y = y;
				for (int i = 0; i < N; i++) {
					scratch.concentrationNumeric[i] = i * .1;
					scratch.concentrationExact[i] = i * .1 + i/100.0;
				}
				sps[idx++] =  scratch;
			}

			DataSet dataset = file.createDataSet( "arraydemo", solutionPointType, dataspace );
			dataset.write(sps.data( ), solutionPointType); 
	}
	catch (H5::Exception &e) {
		std::cerr << e.getDetailMsg( ) << std::endl;
	}
}
namespace {
	struct DemoPoint {
		double x;
		double y;
		hvl_t varData;
	};
}
TEST(hdf5,compoundWithVar) {


	try {
		std::vector<double> data;
		vcellH5::VarLenSimple<double> vlen;

		CompType demoType( sizeof(DemoPoint) ); 
		demoType.insertMember("x", HOFFSET(DemoPoint,x),vcellH5::TPredType<double>::predType( ));
		demoType.insertMember("y", HOFFSET(DemoPoint,y),vcellH5::TPredType<double>::predType( ));
		demoType.insertMember("v", HOFFSET(DemoPoint,varData),vlen.getType( ));

		H5File file( "compoundWithVar.h5", H5F_ACC_TRUNC );

		hsize_t     dimsf[1];              // dataset dimensions
		dimsf[0] = 1;
		DataSpace dataspace( 1, dimsf );
		DemoPoint dp;
		dp.x = 3;
		dp.y = 4;
		for (int i = 0; i < 7; i++) {
			data.push_back(i*i);
		}
		dp.varData = vlen.adapt(data);
		Group group = file.createGroup("mydata");
		DataSet dataset = group.createDataSet( "blob", demoType, dataspace );
		dataset.write(&dp,demoType); 
		DataSet dataset2 = group.createDataSet( "blob2", demoType, dataspace );
		dataset2.write(&dp,demoType); 
	}
	catch (H5::Exception &e) {
		std::cerr << e.getDetailMsg( ) << std::endl;
	}
}

struct CoordPair {
	typedef double CType;
	CType x;
	CType y;
	static H5::DataType getType( ) {
		CompType ourType(sizeof(CoordPair)); 
		ourType.insertMember("x", HOFFSET(CoordPair,x), vcellH5::TPredType<CType>::predType( )); 
		ourType.insertMember("y", HOFFSET(CoordPair,y), vcellH5::TPredType<CType>::predType( )); 
		return ourType;
	}
};

TEST(hdf5,variableArray) {
	try {
		const int N = 100;
		H5::DataType * pt= new H5::DataType(CoordPair::getType( ));
		H5::VarLenType vtype(pt);
		delete pt;
		hsize_t arraydim[] = {N};
		ArrayType arrayType(vtype,1,arraydim);
		H5File file( "variableArray.h5", H5F_ACC_TRUNC );

		hsize_t     dimsf[1];              // dataset dimensions
		dimsf[0] = 1;
		DataSpace dataspace( 1, dimsf );
		hvl_t vdata[100];


		for (int i = 0; i < N; i++) {
			CoordPair * d = new CoordPair[i+1];   
			for (int j=0;j<=i;j++) {
				d[j].x = j;
				d[j].y = j*3;
			}
			vdata[i].len = i + 1;
			vdata[i].p = d;
		}

		DataSet dataset = file.createDataSet( "arraydemo", arrayType, dataspace );
		dataset.write(vdata,arrayType);
	}
	catch (H5::Exception &e) {
		std::cerr << e.getDetailMsg( ) << std::endl;
	}
}

TEST(hdf5,vcellHelper) {
	std::vector<double> myVec;
	typedef vcellH5::VectorFacade<double> DTRAIT; 
	DTRAIT t(myVec);
	H5File file("helper.h5", H5F_ACC_TRUNC );
	vcellH5::facadeWrite(file,"empty",t);

	myVec.push_back(9); 
	myVec.push_back(0);
	myVec.push_back(2); 
	myVec.push_back(1);
	myVec.push_back(0); 
	vcellH5::facadeWrite(file,"BeverlyHills",t);
	double hx = 12;
	H5::DataSet ds = vcellH5::primitiveWrite(file,"hx",hx);
	vcellH5::facadeWriteAttribute(ds,"BeverlyHills",t);
	vcellH5::writeAttribute(ds,"hx",hx);
	int n = std::numeric_limits<int>::max( ); 
	std::cerr << "int is " << n << " with size " << sizeof(n) << std::endl;
	vcellH5::primitiveWrite(file,"n",n);
	vcellH5::writeAttribute(ds,"n",n);
	short s= 5;
	vcellH5::primitiveWrite(file,"s",s);
	vcellH5::writeAttribute(ds,"s",s);
	std::string p("peachy");
	vcellH5::writeAttribute(ds,"fruit",p);
}
TEST(hdf5,bugDemo) {
	H5File file("hdf5viewDemo.h5", H5F_ACC_TRUNC );
	int answer = 42;
	H5::DataSet ds = vcellH5::primitiveWrite(file,"answer",answer);

	std::vector<double> myVec;
	vcellH5::VectorFacade<double> trait(myVec); 
	myVec.push_back(9); 
	myVec.push_back(0);
	myVec.push_back(2); 
	myVec.push_back(1);
	myVec.push_back(0); 
	vcellH5::facadeWriteAttribute(ds,"BeverlyHills",trait);
}
namespace {
	inline double testValue(size_t x, size_t y, size_t z) {
		return static_cast<double>(10000 * x + 100 * y + z);
	}


}
TEST(hdf5,flex3) {
	using namespace vcellH5;
	vcell_util::Flex3<double,hsize_t> trial(3,4,5);
	for (int x = 0; x < trial.depthSize( ); x++) 
		for (int y = 0; y < trial.columnSize( ); y ++)
			for (int z = 0; z < trial.rowSize( ); z++) {
				trial[x][y][z] = testValue(x,y,z);
	}
	for (int x = 0; x < trial.depthSize( ); x++) 
		for (int y = 0; y < trial.columnSize( ); y ++)
			for (int z = 0; z < trial.rowSize( ); z++) {
				double back = testValue(x,y,z);
				ASSERT_TRUE(trial[x][y][z] == testValue(x,y,z));
	}
	const vcell_util::Flex3<double,hsize_t> copy(trial); 
	for (int x = 0; x < trial.depthSize( ); x++) 
		for (int y = 0; y < trial.columnSize( ); y ++)
			for (int z = 0; z < trial.rowSize( ); z++) {
				ASSERT_TRUE(copy[x][y][z] == testValue(x,y,z));
	}
}

