#include "InteractionHisto.h"
#include "Derived.h"

#include "TH1D.h"
#include "TH2D.h"

namespace ana {
  namespace SBNOsc {

void InteractionHistos::Initialize(const std::string &postfix, const geo::BoxBoundedGeo &detector_volume, const std::vector<double> &tagger_volume) {
#define INT_HISTO(name, n_bins, lo, hi)    name = new TH1D((#name"_" + postfix).c_str(), #name, n_bins, lo, hi); fAllHistos.push_back(name)
#define INT_HISTO2D(name, n_binsx, xlo, xhi, n_binsy, ylo, yhi) name = new TH2D((#name"_" + postfix).c_str(), #name, n_binsx, xlo, xhi, n_binsy, ylo, yhi); fAllHistos.push_back(name)

  INT_HISTO(track_length, 100, 0., 600.);
  INT_HISTO(track_p, 50, 0., 5.);
  INT_HISTO(true_deposited_energy, 50., 0., 5.);
  INT_HISTO(nuE, 50, 0., 5.);
  INT_HISTO(beam_center_distance, 60, 0., 300.);
  INT_HISTO(Q2, 50, 0., 10.);
  INT_HISTO(true_contained_length, 100, 0., 600.);
  INT_HISTO(true_track_multiplicity, 10, 0., 10);
  INT_HISTO(crosses_tpc, 2, -0.5, 1.5);
  INT_HISTO(dist_to_match, 101, -1., 100.);
  INT_HISTO(primary_track_completion, 100, 0., 1.);
  INT_HISTO(n_reco_vertices, 10, -0.5, 9.5);
  INT_HISTO(maxpe_crt_intime_hit, 1000, 0., 10000.);

  INT_HISTO(crt_hit_times, 300, -20., 10.);
  INT_HISTO(closest_crt_hit_time, 300, -20., 10.);

  INT_HISTO2D(intime_crt_hits_xy, 100, tagger_volume[0], tagger_volume[3], 100, tagger_volume[1], tagger_volume[4]);
  INT_HISTO2D(intime_crt_hits_xz, 100, tagger_volume[0], tagger_volume[3], 100, tagger_volume[2], tagger_volume[5]);
  INT_HISTO2D(intime_crt_hits_yz, 100, tagger_volume[1], tagger_volume[4], 100, tagger_volume[2], tagger_volume[5]);

  INT_HISTO2D(vertex_xy, 200, detector_volume.MinX(), detector_volume.MaxX(), 200, detector_volume.MinY(), detector_volume.MaxY());
  INT_HISTO2D(vertex_yz, 200, detector_volume.MinY(), detector_volume.MaxY(), 200, detector_volume.MinZ(), detector_volume.MaxZ());
  INT_HISTO2D(vertex_xz, 200, detector_volume.MinX(), detector_volume.MaxX(), 200, detector_volume.MinZ(), detector_volume.MaxZ());

#undef INT_HISTO
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

  vertex_xy->Fill(vertex.position.X(), vertex.position.Y());
  vertex_yz->Fill(vertex.position.Y(), vertex.position.Z());
  vertex_xz->Fill(vertex.position.X(), vertex.position.Z());

  n_reco_vertices->Fill(event.reco.size());

  double maxpe = 0.;
  double closest_time_dist = -1;
  double closest_time = 0.;
  for (const numu::CRTHit &hit: event.in_time_crt_hits) {
    intime_crt_hits_xy->Fill(hit.location.X(), hit.location.Y());
    intime_crt_hits_xz->Fill(hit.location.X(), hit.location.Z());
    intime_crt_hits_yz->Fill(hit.location.Y(), hit.location.Z());

    std::cout << "PE: " << hit.pes << std::endl;
    std::cout << "Time: " << hit.time << std::endl;
    if (hit.pes > maxpe) maxpe = hit.pes;
    crt_hit_times->Fill(hit.time);

    if (closest_time_dist < 0. || closest_time_dist > 1e-3) {
       double this_time_dist = -1;
       if (hit.time > -0.2 && hit.time < 1.8) this_time_dist = 0.;
       else if (hit.time < 0.) this_time_dist = -hit.time -0.2;
       if (this_time_dist >= 0. && (this_time_dist < closest_time_dist || closest_time_dist < 0.)) {
         closest_time = hit.time;
         closest_time_dist = this_time_dist;
       }
    }
    if (closest_time_dist >= 0.) {
      closest_crt_hit_time->Fill(closest_time);
    }
    else {
      closest_crt_hit_time->Fill(30.);
    }
  }
  std::cout << "Max PE: " << maxpe << std::endl; 
  maxpe_crt_intime_hit->Fill(maxpe);


  double track_length_val = vertex.slice.primary_track_index >= 0 ? vertex_tracks->at(vertex.slice.primary_track_index).length: -1;
  track_length->Fill(track_length_val);
  if (vertex.slice.primary_track_index >= 0 && vertex_tracks->at(vertex.slice.primary_track_index).match.has_match) {
    int mcparticle_id = vertex_tracks->at(vertex.slice.primary_track_index).match.mcparticle_id;
  
    double true_track_momentum = event.true_tracks.at(mcparticle_id).momentum; 
    track_p->Fill(true_track_momentum);
    true_deposited_energy->Fill(event.true_tracks.at(mcparticle_id).deposited_energy);

    int crosses_tpc_val = event.true_tracks.at(mcparticle_id).crosses_tpc;
    crosses_tpc->Fill(crosses_tpc_val);

    double length = event.true_tracks.at(mcparticle_id).length;
    true_contained_length->Fill(length);
  }
   
  if (vertex.match.event_track_id >= 0 && vertex.primary_track.match.is_primary) {
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

