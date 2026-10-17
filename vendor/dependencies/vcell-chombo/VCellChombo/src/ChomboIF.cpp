
#include <VCELL/DataSet.h>
#include <VCELL/ChomboIF.h>
#include <VCELL/ChomboGeometry.h>
#include <Expression.h>
#include <SimpleSymbolTable.h>
#include <fstream>
#include <sstream>

ChomboIF::ChomboIF(Feature* f, int phaseIndex, VCell::Expression* exp, VCell::Expression* uexp)
{
	this->feature = f;
	this->phaseIndex = phaseIndex;
	ifExp = exp;
	userExp = uexp;
	distanceMap = NULL;
}

ChomboIF::ChomboIF(Feature* f, int phaseIndex, string& distanceMapFile)
{
	feature = f;
	this->phaseIndex = phaseIndex;
	distanceMap = NULL;
	ifExp = NULL;
	userExp = NULL;
	readDistanceMap(distanceMapFile);
}

ChomboIF::ChomboIF(const ChomboIF* another)
{
	distanceMap = NULL;
	ifExp = NULL;
	userExp = NULL;
	feature = another->feature;
	phaseIndex = another->phaseIndex;
	if (another->distanceMap != NULL)
	{
		//distanceMapFile = another->distanceMapFile;
		dimension = another->dimension;
		sampleH = another->sampleH;
		sampleN = another->sampleN;
		firstSamplePoint = another->firstSamplePoint;
		sampleOffsetXY = another->sampleOffsetXY;
		long dataLength = sampleOffsetXY;
		if (dimension > 2)
		{
			dataLength *= sampleN[2];
		}
		distanceMap = new double[dataLength];
		memcpy(distanceMap, another->distanceMap, dataLength * sizeof(double));
	} 
	else
	{
		ifExp = new VCell::Expression(another->ifExp->infix());
		ifExp->bindExpression(ChomboGeometry::getGeometrySymbolTable());
		userExp = new VCell::Expression(another->userExp->infix());
		userExp->bindExpression(ChomboGeometry::getGeometrySymbolTable());
	}
}

ChomboIF::~ChomboIF()
{
	delete[] distanceMap;
	delete ifExp;
	delete userExp;
}

void ChomboIF::readDistanceMap(string& distanceMapFile) {
	// header
	// dimension
	// N (Nx, Ny, Nz)
	// H (dx, dy, dz)
	// first point (x, y, z)
	const int headerLength = 10;
	const string headerNames[headerLength] = {
		"dimension",
		"Nx",
		"Ny",
		"Nz",
		"Dx",
		"Dy",
		"Dz",
		"firstX",
		"firstY",
		"firstZ",
	};
	double header[headerLength];
	FILE* fp;
	if ((fp=fopen(distanceMapFile.c_str(), "rb"))==NULL){
			char errmsg[512];
			sprintf(errmsg, "ChomboIF::readDistanceMap() - could not open file '%s'.", distanceMapFile.c_str());
			throw errmsg;
	}
	DataSet::readDoubles(fp, header, headerLength);
	for (int i = 0; i < headerLength; i ++) {
		cout << headerNames[i] << " " << header[i] << endl;
	}
	int count = -1;
	dimension = int(header[++count]);

	sampleN[0] = int(header[++count]);
	sampleN[1] = int(header[++count]);
	++count;
	if (dimension > 2) {
		sampleN[2] = int(header[count]);
	}

	sampleH[0] = header[++count];
	sampleH[1] = header[++count];
	++count;
	if (dimension > 2) {
		sampleH[2] = header[count];
	}

	firstSamplePoint[0] = header[++count];
	firstSamplePoint[1] = header[++count];
	++count;
	if (dimension > 2) {
		firstSamplePoint[2] = header[count];
	}

	sampleOffsetXY = sampleN[0] * sampleN[1];
	long dataLength = sampleOffsetXY;
	if (dimension > 2) {
		dataLength *= sampleN[2];
	}
	distanceMap = new double[dataLength];
	DataSet::readDoubles(fp, distanceMap, dataLength);
	fclose(fp);
}

Real ChomboIF::value(const RealVect& a_point) const
{
	return value(a_point, false);
}

Real ChomboIF::value(const RealVect& a_point, bool validate) const
{
	static string helpMessage = "Try rewriting geometry description without nested boolean expressions.";
	if (distanceMap == NULL)
	{
		double values[3] = {a_point[0], a_point[1], SpaceDim < 3 ? 0.5 : a_point[2]};
		double ifValue = ifExp->evaluateVector(values);
		if (validate)
		{
			double userValue = userExp->evaluateVector(values);
			// userValue != 0 means inside
			// ifValue < 0 means inside
      if (userValue != 0 && ifValue > 0)
			{
				stringstream ss;
				ss << "In feature " << feature->getName() << ", point " << a_point << " is inside but IF function value is >0 (outside). " << helpMessage;
				throw ss.str();
			}
      if (userValue == 0 && ifValue < 0)
			{
				stringstream ss;
				ss << "In feature " << feature->getName() << ", point " << a_point << " is outside but IF function value is <0 (inside). " << helpMessage;
				throw ss.str();
			}
		}
		return ifValue;
	} 
	else
	{
		double v = interpolateDistance(a_point);
		return v;
	}
}

BaseIF* ChomboIF::newImplicitFunction() const
{
  	ChomboIF* ifPtr = new ChomboIF(this);
  	return static_cast<BaseIF*>(ifPtr);
}

double ChomboIF::interpolateDistance(const RealVect& a_point) const {
	//find i,j,k for gridpoint to the left of point
	IntVect Nijk;
	RealVect fpoint;
	for (int i = 0; i < SpaceDim; ++i) {
		double d = (a_point[i] - firstSamplePoint[i])/sampleH[i];
		Nijk[i] = (int)floor(d);
		fpoint[i] = d - Nijk[i];
	}

    // calculate global index of first corner point using i,j,k
    int index0 = Nijk[0] + Nijk[1] * sampleN[0];
    if (SpaceDim > 2) {
    	index0 = index0 + Nijk[2] * sampleOffsetXY;
    }
    // the list of indexes of all corners of the square/cube containing the
    // point:
    //    V000 V100 V010 V110 V001 V101 V011 V111
	//    Vxyz = V000 (1 - x) (1 - y) (1 - z) +
	//				V100 x (1 - y) (1 - z) +
	//				V010 (1 - x) y (1 - z) +
	//				V110 x y (1 - z) +
	//				V001 (1 - x) (1 - y) z +
	//				V101 x (1 - y) z +
	//				V011 (1 - x) y z +
	//				V111 x y z
    const int baseNumCorners = 4;
    int baseIList[baseNumCorners] = {index0, index0 + 1, index0 + sampleN[0], index0 + sampleN[0] + 1};
    double P1 = 1 - fpoint[0];
    double P2 = 1 - fpoint[1];
    double Xprd[] = {P1 * P2, fpoint[0] * P2, P1 * fpoint[1], fpoint[0] * fpoint[1]};
    double f2 = 0, f3 = 0;
    for (int i = 0; i < baseNumCorners; ++i) {
    	int gilo = baseIList[i];
			f2 += distanceMap[gilo]* Xprd[i] ;
			if (SpaceDim > 2) {
				int gihi = gilo  + sampleOffsetXY;
				f3 += distanceMap[gihi]*Xprd[i];
			}
    }
    double f = SpaceDim == 2 ? f2 : f2*(1 - fpoint[2]) + f3 * fpoint[2];
    return f;
}

