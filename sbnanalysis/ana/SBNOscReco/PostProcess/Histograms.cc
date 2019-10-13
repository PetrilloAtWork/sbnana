#include <TH1D.h>
#include <TH2D.h>

#include "nusimdata/SimulationBase/MCTruth.h"

#include "Histograms.h"
#include "Derived.h"

namespace ana {
  namespace SBNOsc {

void FillTrack(
  const numu::RecoTrack &track, 
  const std::map<size_t, numu::RecoTrack> &true_tracks, 
  TrackHistos* track_histos,
  const Cuts &cuts) {

  std::vector<bool> do_fill {true, false, false, false};
  if (track.match.has_match) {
    do_fill[1] = track.match.mctruth_origin == simb::Origin_t::kCosmicRay;
    do_fill[2] = track.match.mctruth_origin == simb::Origin_t::kBeamNeutrino;
  }
  if (!do_fill[1] && !do_fill[2]) do_fill[3] = true;
  for (int i = 0; i < do_fill.size(); i++) {
    if (do_fill[i]) {
      track_histos[i].Fill(track, true_tracks, cuts);
    }
  }
}

void Histograms::Fill(const numu::RecoEvent &event, const event::Event &core, const Cuts &cutmaker, bool fill_all_tracks) { 

  if (fill_all_tracks) {
    for (const auto &track_pair: event.reco_tracks) {
      const numu::RecoTrack &track = track_pair.second;
      FillTrack(track, event.true_tracks, fAllTracks, cutmaker);
    }
  }

  for (unsigned i = 0; i < event.reco.size(); i++) {
    const numu::RecoInteraction &interaction = event.reco[i];

    if (event.reco_tracks.size() > (unsigned)interaction.slice.primary_track_index) {
      FillTrack(event.reco_tracks.at(interaction.slice.primary_track_index), event.true_tracks, fPrimaryTracks, cutmaker);
    }

    std::array<bool, Cuts::nCuts> cuts = cutmaker.ProcessRecoCuts(event, i);
    // fill histos
    for (size_t cut_i=0; cut_i < Cuts::nCuts; cut_i++) {
      int mode = interaction.match.mode; 
      if (cuts[cut_i]) {
        fInteraction[cut_i+Cuts::nTruthCuts][mode].Fill(i, false, event, core.truth);
        fInteraction[cut_i+Cuts::nTruthCuts][numu::mAll].Fill(i, false, event, core.truth);
      }
    }
  }

  for (unsigned i = 0; i < event.truth.size(); i++) {
    const numu::RecoInteraction &truth = event.truth[i];
    std::array<bool, Cuts::nTruthCuts> cuts = cutmaker.ProcessTruthCuts(event, i); 
    for (int cut_i = 0; cut_i < Cuts::nTruthCuts; cut_i++) {
      int mode = truth.match.mode;
      if (cuts[cut_i]) {
        fInteraction[cut_i][mode].Fill(i, true, event, core.truth);
        fInteraction[cut_i][numu::mAll].Fill(i, true, event, core.truth);
      }
    }
  }
}

void InteractionHistos::Write() {
  for (TH1 *hist: all_histos) hist->Write();
}

void InteractionHistos::Scale(double scale) {
  for (TH1 *hist: all_histos) hist->Scale(scale);
}

void InteractionHistos::Add(const InteractionHistos &other) {
  for (unsigned i = 0; i < all_histos.size(); i++) {
    all_histos[i]->Add(other.all_histos[i]);
  }
}

InteractionHistos::~InteractionHistos() {
  for (TH1 *hist: all_histos) delete hist;
}

void TrackHistos::Scale(double scale) {
  for (TH1 *hist: all_histos) hist->Scale(scale);
}

void TrackHistos::Write() {
  for (TH1 *hist: all_histos) hist->Write();
}

void TrackHistos::Add(const TrackHistos &other) {
  for (unsigned i = 0; i < all_histos.size(); i++) {
    all_histos[i]->Add(other.all_histos[i]);
  }
}

TrackHistos::~TrackHistos() {
  for (TH1 *hist: all_histos) delete hist;
}


Histograms::Histograms(const std::string &prefix) {
  for (unsigned i = 0; i < InteractionHistos::nHistos; i++) {
    for (const auto mode: InteractionHistos::allModes) {
      fInteraction[i][mode].Initialize(prefix, mode, i); 
    }
  }

  for (unsigned i = 0; i < TrackHistos::nTrackHistos; i++) {
    fAllTracks[i].Initialize(prefix + "All", i);
    fPrimaryTracks[i].Initialize(prefix + "Primary", i);
  } 
}

void Histograms::Write() {
  for (unsigned i = 0; i < InteractionHistos::nHistos; i++) {
    for (const auto mode: InteractionHistos::allModes) {
      fInteraction[i][mode].Write();
    }
  }

  for (unsigned i = 0; i < TrackHistos::nTrackHistos; i++) {
    fAllTracks[i].Write();
    fPrimaryTracks[i].Write();
  } 
}

void Histograms::Scale(double scale) {
  for (unsigned i = 0; i < InteractionHistos::nHistos; i++) {
    for (const auto mode: InteractionHistos::allModes) {
      fInteraction[i][mode].Scale(scale);
    }
  }

  for (unsigned i = 0; i < TrackHistos::nTrackHistos; i++) {
    fAllTracks[i].Scale(scale);
    fPrimaryTracks[i].Scale(scale);
  } 
}

void Histograms::Add(const Histograms &other) {
  for (unsigned i = 0; i < InteractionHistos::nHistos; i++) {
    for (const auto mode: InteractionHistos::allModes) {
      fInteraction[i][mode].Add(other.fInteraction[i][mode]);
    }
  }

  for (unsigned i = 0; i < TrackHistos::nTrackHistos; i++) {
    fAllTracks[i].Add(other.fAllTracks[i]);
    fPrimaryTracks[i].Add(other.fPrimaryTracks[i]);
  } 

}


void InteractionHistos::Initialize(const std::string &prefix, numu::InteractionMode mode, unsigned i) {
#define INT_HISTO(name, n_bins, lo, hi)    name = new TH1D((#name"_" + mode2Str(mode) + prefix + histoNames[i]).c_str(), #name, n_bins, lo, hi); all_histos.push_back(name)

  INT_HISTO(track_length, 101, -10, 1000);
  INT_HISTO(track_p, 50, 0., 5.);
  INT_HISTO(nuE, 50, 0., 5.);
  INT_HISTO(beam_center_distance, 60, 0., 300.);
  INT_HISTO(Q2, 50, 0., 10.);
  INT_HISTO(true_contained_length, 101, -10., 1000.);
  INT_HISTO(true_track_multiplicity, 10, 0., 10);
  INT_HISTO(crosses_tpc, 2, -0.5, 1.5);
  INT_HISTO(dist_to_match, 101, -1., 100.);
  INT_HISTO(primary_track_completion, 100, 0., 1.);

#undef INT_HISTO
}

void TrackHistos::Initialize(const std::string &prefix, unsigned i) {
#define TRACK_HISTO(name, n_bins, lo, hi)    name = new TH1D((#name"_" + prefix + trackHistoNames[i]).c_str(), #name, n_bins, lo, hi); all_histos.push_back(name)
#define TRACK_2DHISTO(name, binx, lo_x, hi_x, biny, lo_y, hi_y)  name = new TH2D((#name"_" + prefix + trackHistoNames[i]).c_str(), #name, binx, lo_x, hi_x, biny, lo_y, hi_y); all_histos.push_back(name)

  TRACK_HISTO(chi2_muon_diff, 100, 0., 100.);
  
  TRACK_HISTO(chi2_proton_diff, 101, -0.1, 10);
  TRACK_HISTO(chi2_kaon_diff, 101, -0.1, 10);
  TRACK_HISTO(chi2_pion_diff, 101, -0.1, 10);
  
  TRACK_HISTO(range_p, 100, 0., 2.);
  TRACK_HISTO(mcs_p, 100, 0., 2.);
  TRACK_HISTO(deposited_e_max, 100, 0., 2.);
  TRACK_HISTO(deposited_e_avg, 100, 0., 2.);
  
  TRACK_HISTO(range_p_minus_truth, 100, -2., 2);
  TRACK_HISTO(mcs_p_minus_truth, 100, -2., 2.);
  TRACK_HISTO(deposited_e_max_minus_truth, 100, -2., 2.);
  TRACK_HISTO(deposited_e_avg_minus_truth, 100, -2., 2.);
  TRACK_HISTO(deposited_e_med_minus_truth, 100, -2., 2.); 

  TRACK_HISTO(length, 100, 0., 500.);
  TRACK_HISTO(is_contained, 2, -0.5, 1.5);
  
  TRACK_2DHISTO(range_p_diff, 25, 0, 2.5, 40, -2., 2.); 
  TRACK_2DHISTO(mcs_p_diff, 25, 0., 2.5, 40, -2., 2.);
  TRACK_2DHISTO(deposited_e_max_diff, 25, 0., 2.5, 40, -2., 2.);

  TRACK_2DHISTO(range_p_comp, 25, 0, 2.5, 25, 0., 2.5);
  TRACK_2DHISTO(mcs_p_comp, 25, 0., 2.5, 25, 0., 2.5);
  TRACK_2DHISTO(deposited_e_max_comp,  25, 0., 2.5, 25, 0., 2.5);
  
  // timing histos
  TRACK_HISTO(has_crt_track_match, 3, -0.5, 1.5);
  TRACK_HISTO(has_crt_hit_match, 3, -0.5, 1.5); 
  TRACK_HISTO(has_flash_match, 3, -0.5, 1.5);
  
  double min_matchtime_t = -1640;
  double max_matchtime_t =  3280;
  int n_matchtime_bins = 1000;
  
  double min_comptime = -0.5;
  double max_comptime = 0.5;
  int n_comptime_bins = 1000;
  
  TRACK_HISTO(crt_match_time, n_matchtime_bins, min_matchtime_t, max_matchtime_t);
  TRACK_HISTO(flash_match_time, n_matchtime_bins, min_matchtime_t, max_matchtime_t);
  TRACK_HISTO(crt_v_flash_match_time, n_comptime_bins, min_comptime, max_comptime);

  TRACK_HISTO(completion, 200, -1, 1);

  TRACK_HISTO(stopping_chisq_start, 100, 0., 10.);
  TRACK_HISTO(stopping_chisq_finish, 100, 0., 10.);
  TRACK_HISTO(stopping_chisq, 100., 0., 10.);

#undef TRACK_HISTO
#undef TRACK_2DHISTO
}

void TrackHistos::Fill(
    const numu::RecoTrack &track, 
    const std::map<size_t, numu::RecoTrack> &true_tracks,
    const Cuts &cuts) {

  // Primary track histos
  if (track.min_chi2 > 0) {
    chi2_proton_diff->Fill(track.chi2_proton - track.min_chi2);
    chi2_muon_diff->Fill(track.chi2_muon - track.min_chi2);
    chi2_pion_diff->Fill(track.chi2_pion - track.min_chi2);
    chi2_kaon_diff->Fill(track.chi2_kaon - track.min_chi2);
  }
	  
  range_p->Fill(track.range_momentum_muon); 
  double mcs_p_val = track.mcs_is_backward ? track.bwd_mcs_momentum_muon : track.fwd_mcs_momentum_muon;
  mcs_p->Fill(mcs_p_val);
  deposited_e_max->Fill(track.deposited_energy_max);
  
  length->Fill(track.length);
  is_contained->Fill(track.is_contained);
  
  has_crt_hit_match->Fill(cuts.HasCRTHitMatch(track));
  has_crt_track_match->Fill(cuts.HasCRTTrackMatch(track));
  if (cuts.HasCRTTrackMatch(track) || cuts.HasCRTHitMatch(track)) {
    crt_match_time->Fill(cuts.CRTMatchTime(track));
  }
  bool has_flash_match_val = track.flash_match.size() > 0;
  has_flash_match->Fill(has_flash_match_val);
  if (has_flash_match_val) {
    const numu::FlashMatch &flash_match = track.flash_match.at(0);
    double flash_time = flash_match.match_time_first;
    std::cout << "FLASH match time: " << flash_time << std::endl;
    flash_match_time->Fill(flash_time);
    if (cuts.HasCRTTrackMatch(track) || cuts.HasCRTHitMatch(track)) {
      crt_v_flash_match_time->Fill(cuts.CRTMatchTime(track) - flash_time);  
      std::cout << "CRT match time: " << crt_match_time << std::endl;
    }
  }
  
  // check if truth match
  if (track.match.has_match && track.match.mcparticle_id >= 0) {
    const numu::RecoTrack &true_track = true_tracks.at(track.match.mcparticle_id);
    range_p_minus_truth->Fill(track.range_momentum_muon - true_track.momentum);
    mcs_p_minus_truth->Fill(mcs_p_val - true_track.momentum); 
    deposited_e_max_minus_truth->Fill(track.deposited_energy_max - true_track.energy);
    deposited_e_avg_minus_truth->Fill(track.deposited_energy_avg - true_track.energy);
    deposited_e_med_minus_truth->Fill(track.deposited_energy_med - true_track.energy);
    
    //std::cout << "AT FILL -- is contained: " << true_track.is_contained << std::endl;      
    
    range_p_diff->Fill(true_track.momentum, track.range_momentum_muon - true_track.momentum);
    mcs_p_diff->Fill(true_track.momentum, mcs_p_val - true_track.momentum);
    deposited_e_max_diff->Fill(true_track.energy, track.deposited_energy_max - true_track.energy);
    
    range_p_comp->Fill(true_track.momentum, track.range_momentum_muon);
    mcs_p_comp->Fill(true_track.momentum, mcs_p_val);
    deposited_e_max_comp->Fill(true_track.energy, track.deposited_energy_max);

    completion->Fill(track.match.completion);
  }
  else {
    completion->Fill(-0.5);
  }

  stopping_chisq_start->Fill(track.stopping_chisq_start);
  stopping_chisq_finish->Fill(track.stopping_chisq_finish);

  if (!cuts.InContainment(track.end) && track.end.Y() > track.start.Y()) {
    stopping_chisq->Fill(track.stopping_chisq_start);
  }
  else if (!cuts.InContainment(track.start) && track.start.Y() > track.end.Y()) {
    stopping_chisq->Fill(track.stopping_chisq_finish);
  }

}

void InteractionHistos::Fill(
  unsigned vertex_index,
  bool is_truth,
  const numu::RecoEvent &event,
  const std::vector<event::Interaction> &core_truth) {

  const numu::RecoInteraction &vertex = (is_truth) ? event.truth[vertex_index] : event.reco[vertex_index];

  const std::vector<numu::RecoInteraction> &truth = event.truth;

  const std::map<size_t, numu::RecoTrack> *vertex_tracks;
  if (is_truth) {
    // find the closest reconstructed vertex to this one
    double dist = numu::dist2Match(vertex, event.reco);
    std::cout << "Truth dist to match: " << dist << " mode: " << vertex.match.mode << std::endl;
    dist_to_match->Fill(dist);
    primary_track_completion->Fill(numu::trackMatchCompletion(vertex_index, event));
    vertex_tracks = &event.true_tracks;
  }
  // closest reconstructed vertex to this one (already contained in object)
  else {
    dist_to_match->Fill(vertex.match.truth_vertex_distance);
    primary_track_completion->Fill(vertex.primary_track.match.completion);
    vertex_tracks = &event.reco_tracks;
  }

  double track_length_val = vertex.slice.primary_track_index >= 0 ? vertex_tracks->at(vertex.slice.primary_track_index).length: -1;
  track_length->Fill(track_length_val);
  if (vertex.slice.primary_track_index >= 0 && vertex_tracks->at(vertex.slice.primary_track_index).match.has_match) {
    int mcparticle_id = vertex_tracks->at(vertex.slice.primary_track_index).match.mcparticle_id;
  
    double true_track_momentum = event.true_tracks.at(mcparticle_id).momentum; 
    track_p->Fill(true_track_momentum);

    int crosses_tpc_val = event.true_tracks.at(mcparticle_id).crosses_tpc;
    crosses_tpc->Fill(crosses_tpc_val);

    double length = event.true_tracks.at(mcparticle_id).length;
    true_contained_length->Fill(length);
  }
   
  if (vertex.match.has_match) {
    int event_id = vertex.match.event_track_id;
    int mctruth_id = vertex.match.mctruth_track_id;

    true_track_multiplicity->Fill(truth[event_id].multiplicity);
    
    if (mctruth_id >= 0) {
      nuE->Fill(core_truth[mctruth_id].neutrino.energy);
      Q2->Fill(core_truth[mctruth_id].neutrino.Q2);
      // get the distance from the beam center
      /*
      float beam_center_distance = sqrt( (core_truth[mctruth_id].neutrino.position.X() - _config.beamCenterX) * 
        (core_truth[mctruth_id].neutrino.position.X() - _config.beamCenterX) +
        (core_truth[mctruth_id].neutrino.position.Y() - _config.beamCenterY) *
        (core_truth[mctruth_id].neutrino.position.Y() - _config.beamCenterY));

      beam_center_distance->Fill(beam_center_distance);
      */
    }
  }
}

  } // namespace SBNOsc
} // namespace ana
