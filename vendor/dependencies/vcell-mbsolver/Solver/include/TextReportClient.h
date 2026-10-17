#ifndef TextReportClient_h
#define TextReportClient_h

#include <ReportClient.h>
namespace moving_boundary
{
	struct TextReportClient: public ReportClient
	{
		TextReportClient(const std::string & filename, MovingBoundaryParabolicProblem &mbpp, int precision, int width,
				long interval);
		~TextReportClient();

		virtual std::string outputName( ) const;
		virtual std::string getXML( ) const;

		virtual void time(double t, unsigned int numIteration, bool last, const GeometryInfo<CoordinateType> &geometryInfo);

		virtual void element(const MeshElementNode &e);

		virtual void iterationComplete();

		virtual void simulationComplete();

		static constexpr const char* TXT_FILE_EXT = ".m";

		std::string fileName;
		std::ofstream output;
		const MovingBoundaryParabolicProblem &problem;
		WorldToPDPointConverter<CoordinateType, 2> pointConverter;
		const int outputPrecision;
		const int outputWidth;
		int iterCount;
		bool lastIteration;
		long keeyEvery;

		bool shouldReport();
	};
}

#endif
