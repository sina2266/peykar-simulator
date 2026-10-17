#include <MPoint.h>
#include <iomanip>
#include <MeshElementNode.h>
#include <MovingBoundaryParabolicProblem.h>
#include <World.h>
#include <vcellxml.h>
#include <vector>
#include <TextReportClient.h>
#include <vcellxml.h>
#include <vcellstring.h>
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
using std::endl;
using spatial::cX;
using spatial::cY;

namespace moving_boundary
{
TextReportClient::TextReportClient(const std::string & baseFileName, MovingBoundaryParabolicProblem &mbpp, int precision, int width, long interval)
  : ReportClient(baseFileName),
		fileName(baseFileName + TXT_FILE_EXT),
		problem(mbpp),
		pointConverter(World<CoordinateType, 2>::get().pointConverter()),
		outputPrecision(precision),
		outputWidth(width),
		keeyEvery(interval),
		iterCount(-1),
		lastIteration(false)
{
	remove(fileName.c_str());
	output.open(fileName.c_str());
	output.precision(outputPrecision);
	output.setf(std::ios_base::fixed, std::ios_base::floatfield); //fixed
}

TextReportClient::~TextReportClient()
{
	try
	{
		output.close();
	}
	catch (...)
	{
		// ignore
	}
}
std::string TextReportClient::outputName( ) const
{
	return fileName;
}

std::string TextReportClient::getXML( ) const
{
	return "";
}

bool TextReportClient::shouldReport()
{
	return iterCount == 0 || lastIteration || iterCount % keeyEvery == 0;
}

void TextReportClient::time(double t, unsigned int numIteration, bool last,
		const GeometryInfo<CoordinateType> &geometryInfo)
{
	iterCount = numIteration;
	lastIteration = last;
	if (!shouldReport())
	{
		return;
	}
	if (t == 0)
	{
		output << "%%%%%% MovingBoundarySolver output" << endl;
		output << "% Mesh Size" << endl;
		output << "Nx=" << problem.setup().Nx[0] << ";" << endl;
		output << "% Extent" << endl;
		output << "DomainX=" << problem.setup().extentX << ";" << endl;
		output << "DomainY=" << problem.setup().extentY << ";" << endl;
		output << "Dx=" << (problem.setup().extentX[1] - problem.setup().extentX[0])/problem.setup().Nx[0] << ";" << endl;
		output << "dT=" << problem.frontTimeStep() << ";" << endl;
		output << "Tmax=" << problem.setup().maxTime << ";" << endl;
		output << "precision=  " << outputPrecision << ";" << endl;
		output << "width=  " << outputWidth << ";" << endl;
		output << "%%%%%%%%%%%%%%%%%%%%%%" << endl;
		output << "%%%%%%%%%%%%%%%%%%%%%%" << endl;
	}
	output << "iter= " << numIteration << ";" << endl;
	output << "time= " << std::setw(outputWidth) << t << ";" << endl;

	string variableSuffix;
	if (lastIteration)
	{
		output << "%%% last iteration" << endl;
		variableSuffix = "end";
	}
	else
	{
		variableSuffix = vcell_util::to_string<int>(iterCount);
	}
	output << "nodesAdjusted_" << numIteration << "= " << geometryInfo.nodesAdjusted << ";" << endl;
	std::vector<spatial::TPoint<CoordinateType,2> > front = geometryInfo.boundary;
	output << "front_" << variableSuffix << "_size=" << front.size() << ";" << endl;
	output << "%%% front=[x y Vx Vy];" << endl;
	output << "front_" << variableSuffix << "=[ " << endl;
	for (auto iter = front.begin(); iter != front.end(); ++iter)
	{
		auto simPoint = *iter;
		auto pdPoint = pointConverter(simPoint);
//		spatial::SVector<moving_boundary::VelocityType,2> v = problem.velocity(simPoint);
		//output << simPoint << "; (" << pdPoint << ')' << endl;
		output << pdPoint(cX) << " " << pdPoint(cY)
				//<< " " << v(cX) << " " << v(cY)
						<< endl;
	}
	output << "];" << endl << endl;
	output << "%%% elements=[x y V";
	for (int s = 0; s < problem.physiology()->numVolumeVariables(); ++ s)
	{
		output << " " << problem.physiology()->getVolumeVariable(s)->name();
	}
	output << "];" << endl;
	output << "elements_" << variableSuffix << "=[" << endl;
}

void TextReportClient::element(const MeshElementNode &e)
{
	if (!shouldReport())
	{
		return;
	}
	auto pdPoint = pointConverter(e);
	output << pdPoint(cX) << " " << pdPoint(cY) << " " << std::setw(outputWidth) << e.volumePD();
	for (int s = 0; s < e.numVolumeVariables(); ++ s)
	{
		output << " " << e.concentration(s);
	}
	output << endl;
}

void TextReportClient::iterationComplete()
{
	if (!shouldReport())
	{
		return;
	}
	output << "];" << endl;
	output << "%%% end of generation " << iterCount << endl;
}

void TextReportClient::simulationComplete()
{
	output << "%%% end of simulation " << iterCount << endl;
}
}

