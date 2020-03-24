#include "SBNAna/Vars/NumuVars.h"
#include "CAFAna/Core/Utilities.h"
#include "StandardRecord/Proxy/SRProxy.h"
#include <cassert>

namespace ana
{


  const Var kPrimMuonIdx([](const caf::SRProxy *sr)
                        {       //Find the most muon-like track			  
			  if( sr->reco.ntrk <= 0 ) return -5.0;

                          double best_idx   = -5.0;
                          //double best_score = -5.0;
                          double best_len   = -5.0;			  
			  for( unsigned int trkIdx = 0; trkIdx < sr->reco.ntrk; trkIdx++ ){
			    auto &trk = sr->reco.trk[trkIdx];
			    
			    //Find longest trk w/Chi2 for muon < Chi2 for pion
			    double score = trk.chi2pid.chi2_pion < trk.chi2pid.chi2_muon;
			    if( score > 0 && trk.len > best_len ){ //Chi2 muon < Chi2 pion
			      // best_score = score;
			      best_len = trk.len;
			      best_idx = trkIdx;
			    }
			  } // end loop over trks
			  
                          return best_idx;
                        });


  // Currently assumes trk 0 is the primary
  const Var kPrimTrkLen(
			[](const caf::SRProxy *sr)
			{
			  double len = -5.0;
			  if ( sr->reco.ntrk ){
			    unsigned int muIdx = (unsigned int)kPrimMuonIdx(sr);
			    len = sr->reco.trk[muIdx].len;
			  }
			  return len;
			});

}
