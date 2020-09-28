#include "SBNAna/Cuts/NueCuts.h"

#include "StandardRecord/Proxy/SRProxy.h"

namespace ana{

	// Basic reconstruction 
	const Cut kRecoShower(
		[](const caf::SRProxy* sr)
		{
		  return (sr->reco.nshw > 0 && // need a shower
				  sr->reco.shw[0].bestplane_energy > 0 && // nothing is terribly wrong
				  sr->reco.shw[0].len > 0 );
	}
	);

	// Basic reconstruction 
	const Cut kNueBasicCut(
		[](const caf::SRProxy* sr)
		{
		  return (sr->reco.shw[0].bestplane_energy < 250. &&
				  sr->reco.shw[0].bestplane_dEdx < 2.7 &&
				  sr->reco.shw[0].len < 42.);
	}
	);

}
