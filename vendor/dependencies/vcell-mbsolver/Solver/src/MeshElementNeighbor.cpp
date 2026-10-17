#include <MeshElementNode.h>
#include <cstring>
using namespace moving_boundary;

MeshElementNeighbor::MeshElementNeighbor(MeshElementNode *e)
			:element(e),
			distanceTo( ),
			edgeLength( )
{
	halfAdvectionFlux = new BioQuanType[e->numVolumeVariables()];
	std::memset(halfAdvectionFlux, 0, e->numVolumeVariables() * sizeof(BioQuanType));
}

MeshElementNeighbor::MeshElementNeighbor(std::istream & is,  const MeshElementNode & client)  {
	vcell_persist::Token::check<MeshElementNeighbor>(is);
	spatial::ElementOffset<2> eo(is);
	if (!eo.allZero( )) {
		element = client.neighbor(eo);
	}
	else {
		element = nullptr; 
	}
	vcell_persist::binaryRead(is,distanceTo);
	vcell_persist::binaryRead(is,edgeLength);
}

void MeshElementNeighbor::persist(std::ostream &os, const MeshElementNode &client) const {
	vcell_persist::Token::insert<MeshElementNeighbor>(os);
	spatial::ElementOffset<2> eo;
	if (element != nullptr) {
		eo = client.offset(*element);
	}
	eo.persist(os);
	vcell_persist::binaryWrite(os,distanceTo);
	vcell_persist::binaryWrite(os,edgeLength);
}
