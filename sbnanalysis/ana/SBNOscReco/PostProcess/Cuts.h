#ifndef __sbnanalysis_CURS_HH
#define __sbnanalysis_CURS_HH

#include <array>
#include "../Data/RecoEvent.h"

namespace ana {
 namespace SBNOsc {


class Cuts {
public:
  static const unsigned nCuts = 4; //!< total number of cuts
  void Initialize(double trackMatchCompletionCut);
  /** 
 * Process each cut associated with reconstructed events
 * \param event The reconstructed event information
 * \param reco_vertex_index The index of the candidate reconstructed neutrino vertex into the list of such vertices in the RecoEvent
 *
 * \return A list of bool's of whether the reco event passes each cut
 */
  std::array<bool, nCuts> ProcessRecoCuts(const numu::RecoEvent &event, unsigned reco_vertex_index);

  /**
 * Select a reco event based on the cut values provided by ProcessRecoCuts
 * \param cuts the list of cuts returned by ProcessRecoCuts
 *
 * \return whether to select this reconstructed neutrino vertex candidate
 */
  bool SelectReco(std::array<bool, nCuts> &cuts);

private:

  struct Config {
    double trackMatchCompletionCut;
  };

  Config fConfig;

};

  }
}

#endif

