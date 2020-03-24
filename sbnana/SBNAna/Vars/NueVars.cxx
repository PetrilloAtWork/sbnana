#include "SBNAna/Vars/NueVars.h"
#include "CAFAna/Core/Utilities.h"
#include "StandardRecord/Proxy/SRProxy.h"
#include <cassert>

namespace ana
{

  // Currently assumes shw 0 is the primary
  const Var kRecoShower_BestEnergy(
			[](const caf::SRProxy *sr)
			{
			  double energy = -5.0;
			  if ( sr->reco.nshw ){
			    energy = sr->reco.shw[0].bestplane_energy;
			  }
			  return energy;
			});

  const Var kRecoShower_Density(
			[](const caf::SRProxy *sr)
			{
			  double density = -5.0;
			  if ( sr->reco.nshw && sr->reco.shw[0].len >0 ){
			    density = sr->reco.shw[0].bestplane_energy / sr->reco.shw[0].len;
			  }
			  return density;
			});

  const Var kRecoShower_Energy(
			[](const caf::SRProxy *sr)
			{
			  double energy = -5.0;
			  if ( sr->reco.nshw ){
			    energy = sr->reco.shw[0].energy;
			  }
			  return energy;
			});

  const Var kRecoShower_Len(
			[](const caf::SRProxy *sr)
			{
			  double len = -5.0;
			  if ( sr->reco.nshw ){
			    len = sr->reco.shw[0].len;
			  }
			  return len;
			});

    const Var kRecoShower_OpenAngle(
			[](const caf::SRProxy *sr)
			{
			  double openangle = -5.0;
			  if ( sr->reco.nshw ){
			    openangle = sr->reco.shw[0].openangle;
			  }
			  return openangle;
			});


}
