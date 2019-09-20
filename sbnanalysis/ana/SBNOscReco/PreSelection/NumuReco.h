#ifndef __sbnanalysis_ana_SBNOsc_NumuReco__
#define __sbnanalysis_ana_SBNOsc_NumuReco__

/**
 * \file NumuReco.h
 *
 * SBN nue selection.
 *
 * Author:
 */

#include <iostream>
#include <array>
#include <vector>

#include "canvas/Utilities/InputTag.h"
#include "core/SelectionBase.hh"
#include "core/Event.hh"
#include "core/ProviderManager.hh"

#include "TDatabasePDG.h"
#include "TGraph.h"

#include "canvas/Persistency/Common/FindManyP.h"
#include "lardataobj/Simulation/GeneratedParticleInfo.h"
#include "nusimdata/SimulationBase/MCTruth.h"
#include "lardataobj/MCBase/MCTrack.h"
#include "lardataobj/RecoBase/Vertex.h"
#include "lardataobj/RecoBase/Track.h"
#include "lardataobj/RecoBase/Hit.h"
#include "larcorealg/Geometry/BoxBoundedGeo.h"

#include "LArReco/TrajectoryMCSFitter.h"
#include "LArReco/TrackMomentumCalculator.h"

#include "sbndcode/CRT/CRTProducts/CRTHit.hh"
#include "sbndcode/CRT/CRTProducts/CRTTrack.hh"

#include "sbndcode/CRT/CRTUtils/CRTT0MatchAlg.h"
#include "sbndcode/CRT/CRTUtils/CRTTrackMatchAlg.h"

#include "../CosmicIDAlgs/ApaCrossCosmicIdAlg.h"
#include "../CosmicIDAlgs/StoppingParticleCosmicIdAlg.h"
#include "OpHitFinder/opHitFinderSBND.hh"


#include "../Data/CRTMatch.h"
#include "../Data/FlashMatch.h"
#include "../Data/Mode.h"
#include "../Data/RecoEvent.h"
#include "../Data/RecoParticle.h"
#include "../Data/RecoTrack.h"
#include "../Data/TruthMatch.h"

class TH2D;
class TH1D;

namespace ana {
  namespace SBNOsc {

/**
 * \class NumuReco
 * \brief Electron neutrino event selection
 */
class NumuReco : public core::SelectionBase {
public:
  /** Constructor. */
  NumuReco();

  /**
   * Initialization.
   *
   * \param config A configuration, as a FHiCL ParameterSet object
   */
  void Initialize(fhicl::ParameterSet* config=NULL);

  /** Finalize and write objects to the output file. */
  void Finalize();

  /**
   * Process one event.
   *
   * \param ev A single event, as a gallery::Event
   * \param Reconstructed interactions
   * \return True to keep event
   */
  bool ProcessEvent(const gallery::Event& ev, const std::vector<event::Interaction> &truth, std::vector<event::RecoInteraction>& reco);



protected:
  /** Configuration parameters */
  struct Config {
    std::vector<geo::BoxBoundedGeo> fiducial_volumes; //!< List of FV containers -- set by "fiducial_volumes"
    std::vector<geo::BoxBoundedGeo> containment_volumes; //!< List of volumes for containment cuts -- set by "containment_volumes"
    std::vector<geo::BoxBoundedGeo> cryostat_volumes; //!< List of cryostat volumes -- retreived from Geometry service
    std::vector<std::vector<geo::BoxBoundedGeo>> tpc_volumes; //!< List of active tpc volumes -- retreived from Geoemtry service
    bool verbose; //!< Whether to print out info associated w/ selection.
    bool shakyMCTracks; //!< How to handle MC tracks with some missing truth information
    std::vector<std::string> uniformWeights; //!< Weights taken from "EventWeight" that should be applied to the weight of each event
    double constantWeight; //!< Constant weight to apply uniformly to each event
    double cosmicWeight; //!< Weight applied to all events matched to a cosmic track

    bool requireMatched; //!< Apply cut that requires each reconstructed vertex to be matched to a truth vertex
    bool requireTrack; //!< Apply cut that requires each reconstructed vertex to have an associated primary track
    bool requireContained; //!< Apply cut that requires each primary track to be contained inside the containment volume
    double trackMatchContainmentCut;

    bool CRTHitinOpHitRange;
    double CRT2OPTimeWidth;

    bool MakeOpHits;
    double CRTHitDist;

    int FlashMatchMethod;
    int TSMode;
    double flashMatchTimeDifference;

    double beamCenterX;
    double beamCenterY;

    std::string HitTag; //!< art tag for hits
    std::string RecoSliceTag;
    std::string RecoTrackTag; //!< art tag for reconstructed tracks
    std::string RecoVertexTag; //!< art tag for reconstructed vertices
    std::string CaloTag;
    std::string PIDTag;
    std::string PFParticleTag; //!< art tag for PFParticles
    std::string CorsikaTag; //!< art tag for corsika MCTruth
    std::string CRTTrackTag; //!< art tag for CRT tracks
    std::string CRTHitTag;
    std::string OpFlashTag;
    std::string MCParticleTag; //!< art tag for MCParticle 

  };


  // Internal functions

  /**
 * Produce all reconstruction information from the gallery event
 * \param ev the gallery Event
 * \param truth the list of truth vertices for this event
 * \return the RecoEvent object for this event
 */
  numu::RecoEvent Reconstruct(const gallery::Event &ev, std::vector<numu::RecoInteraction> truth);

  /**
 * Gathers together reconstruction information on each individual particle.
 * \param ev the gallery Event
 *
 * \return the list of RecoParticle objects for every reconstructed particle in the event
 */
  std::vector<numu::RecoParticle> RecoParticleInfo(const gallery::Event &event);

  /**
 * Selects which RecoParticles are neutrino interaction candidates
 * \param reco_particles the list of all reconstructed particles to be considered
 *
 * \return the list of reconstructed particles which might be neutrinos
 */
  std::vector<numu::RecoSlice> SelectSlices(const std::vector<numu::RecoSlice> &reco_slices);
  std::vector<numu::RecoTrack> RecoTrackInfo(const gallery::Event &event);

  std::vector<numu::RecoSlice> RecoSliceInfo(
    const gallery::Event &event,
    const std::vector<numu::RecoTrack> &reco_tracks,
    const std::vector<numu::RecoParticle> &particles);

  int SelectPrimaryTrack(const numu::RecoSlice &slice);

  std::map<size_t, numu::RecoTrack> RecoSliceTracks(
    const gallery::Event &event, 
    const std::vector<numu::RecoTrack> &reco_tracks, 
    const std::map<size_t, numu::RecoParticle> &particles, 
    int primary_index);

  /**
 *  Converts the NumuReco::RecoInteraction information into reco information used by sbncode core
 *  \param truth the list of truth vertices produced by sbncode core
 *  \param vertex the reconstructed vertex produced by this selection
 *  \param weight the calculated weight for this interaction
 *
 *  \return Reconstruction information as needed by the sbncode core class
 */
  event::RecoInteraction CoreRecoInteraction(const std::vector<event::Interaction> &truth, const numu::RecoInteraction &vertex, double weight);

  /**
 * Produced vertex information from truth information
 * \param ev the gallery Event
 *
 * \return the list of truth neutrino interactions for this event 
 */
  std::vector<numu::RecoInteraction> MCTruthInteractions(const gallery::Event &ev, std::map<size_t, numu::RecoTrack> &true_tracks);

  std::map<size_t, numu::RecoTrack> MCParticleTracks(const gallery::Event &event);


  std::vector<numu::FlashMatch> FlashMatching(const gallery::Event &ev, const recob::Track &pandora_track, const numu::RecoTrack &track); 
  std::vector<numu::CRTMatch> CRTMatching(const numu::RecoTrack &track, const recob::Track &pandora_track, const std::vector<art::Ptr<recob::Hit>> &track_hits);

  /**
 * Get the primary track associated with a truth neutrino interaction.
 * \param ev the gallery Event
 * \param mctruth the MCTruth object for the considered truth neutrino interaction
 *
 * \return the index into the list of MCTrack's containing this track. Equal to -1 if no such track exists.
 */
  int MCTruthPrimaryTrack(const simb::MCTruth &mc_truth, const std::vector<simb::MCParticle> &mcparticle_list);

  /**
 * Get the primary track information associated with an mctruth object
 * \param ev the gallery Event
 * \param mc_truth the MCTruth object for the considered truth neutrino interaction
 *
 * \return information associated with the primary track. Set to nonsense if no such track exists.
 */
  std::map<size_t, numu::RecoTrack> MCTruthTracks(
    std::map<size_t, numu::RecoTrack> &true_tracks, 
    const art::FindManyP<simb::MCParticle, sim::GeneratedParticleInfo> &truth_to_particles, 
    const simb::MCTruth &mc_truth, 
    int mc_truth_index);

  int TrueTrackMultiplicity(const simb::MCTruth &mc_truth, const std::vector<simb::MCParticle> &mcparticle_list);

  /**
 * Get the TrackInfo information associated with an MCTrack
 * \param truth The neutrino interaction which produced this track
 * \param track The MCTrack information for this track
 *
 * \return reconstruction information associated with the truth track
 */
  numu::RecoTrack MCTrackInfo(const simb::MCParticle &track);

  /** Helper function -- whether point is contained in fiducial volume list
 * \param v The point vector.
 *
 * \return Whether the point is contained in the configured list of fiducial volumes.
 * */
  bool containedInFV(const TVector3 &v);
  bool containedInFV(const geo::Point_t &v);

  /**
 * Calculate the associated topology of each primary track (i.e. whether the track is contained in things)
 * 
 * \param The reconstructed track information
 *
 * \return A list of bools associated with track topology. See the code for what is what.
 */
  std::array<bool, 4> RecoTrackTopology(const art::Ptr<recob::Track> &track);

  numu::TrackTruthMatch MatchTrack2Truth(const gallery::Event &ev, size_t pfp_track_id);
  double TrackCompletion(int mcparticle_id, const std::vector<art::Ptr<recob::Hit>> &reco_track_hits);

  void CollectPMTInformation(const gallery::Event &ev);
  void CollectCRTInformation(const gallery::Event &ev);

  unsigned _event_counter;  //!< Count processed events
  unsigned _nu_count;  //!< Count selected events
  TGraph *_cut_counts; //!< Keep track of neutrinos per cut

  Config _config; //!< The config

  // calculators for Reco things
  trkf::TrackMomentumCalculator *_track_momentum_calculator;
  trkf::TrajectoryMCSFitter *_mcs_fitter;
  

  numu::RecoEvent _recoEvent; //!< Branch container for the RecoEvent
  std::vector<numu::RecoInteraction> *_selected; //!< Branch container for the list of selected reco vertices

  sbnd::CRTTrackMatchAlg *_crt_track_matchalg; //!< Algorithm for matching reco Tracks -> CRT Tracks
  sbnd::CRTT0MatchAlg *_crt_hit_matchalg; //!< Algorithm for matching reco Tracks -> CRT hits (T0's)
  ApaCrossCosmicIdAlg _apa_cross_cosmic_alg;
  StoppingParticleCosmicIdAlg _stopping_cosmic_alg;
  opdet::opHitFinderSBND *_op_hit_maker;

  // holders for CRT information
  const std::vector<sbnd::crt::CRTTrack> *_crt_tracks;
  std::vector<sbnd::crt::CRTTrack> _crt_tracks_local;
  std::vector<sbnd::crt::CRTHit> _crt_hits_local; 
  const std::vector<sbnd::crt::CRTHit> *_crt_hits; 
  bool _has_crt_hits;
  bool _has_crt_tracks;

  // holders for PMT information
  std::vector<art::Ptr<recob::OpHit>> _op_hit_ptrs;
  std::vector<recob::OpHit> _op_hits_local;
};

  }  // namespace SBNOsc
}  // namespace ana

#endif  // __sbnanalysis_ana_SBNOsc_NumuReco__

