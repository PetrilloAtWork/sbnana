#include "TruthMatch.h"
#include "nusimdata/SimulationBase/MCTruth.h"

numu::TruthMatch numu::InteractionTruthMatch(const std::vector<numu::RecoInteraction> &truth,  const std::map<size_t, numu::RecoTrack> &reco_tracks, const numu::RecoInteraction &reco) {
  TruthMatch match;
  // Try to match against truth events
  // Vertices
  match.event_vertex_id = -1;
  match.truth_vertex_distance = -1;
  for (int truth_i = 0; truth_i < truth.size(); truth_i++) {
    // find the closest vertex
    if (match.truth_vertex_distance < 0 || (truth[truth_i].position - reco.position).Mag() < match.truth_vertex_distance) {
      match.truth_vertex_distance = (truth[truth_i].position - reco.position).Mag();
      match.event_vertex_id = truth_i;
    }
  }

  // TODO: better way to do matching
  // only keep match if distance is less than 5cm
  if (match.truth_vertex_distance < 0. || match.truth_vertex_distance > 5.) {
    std::cout << "Vertex not matched well.\n";
  }
  else {
      std::cout << "Vertex matched to neutrino.\n";
  }

  // Match to the primary track of the event
  match.event_track_id = -1;
  if (reco.slice.primary_track_index >= 0) {
    const numu::TrackTruthMatch &ptrack_match = reco_tracks.at(reco.slice.primary_track_index).match;
    if (ptrack_match.has_match) {
      for (int truth_i = 0; truth_i < truth.size(); truth_i++) {
        if ((truth[truth_i].position - ptrack_match.mctruth_vertex).Mag() < 1.) {
          match.event_track_id = truth_i;
        }
      }
    }
    std::cout << "Primary track index: " << reco.slice.primary_track_index << std::endl;

    // return the interaction mode
    if (ptrack_match.has_match && ptrack_match.mctruth_origin == simb::Origin_t::kCosmicRay) {
      match.tmode = numu::tmCosmic;
      std::cout << "Track matched to cosmic.\n";
    }
    else if (ptrack_match.has_match && ptrack_match.mctruth_origin == simb::Origin_t::kBeamNeutrino) {
      match.tmode = numu::tmNeutrino;
      std::cout << "Track matched to neutrino.\n";
    }
    else if (ptrack_match.has_match) {
      match.tmode = numu::tmOther;
      std::cout << "Track matched to other: " << ptrack_match.mctruth_origin << std::endl;
      std::cout << "Match PDG: " << ptrack_match.match_pdg << std::endl;
      std::cout << "Match completion: " << ptrack_match.completion << std::endl;
      std::cout << "Is primary: " << ptrack_match.is_primary << std::endl;
      const numu::RecoTrack &track = reco_tracks.at(reco.slice.primary_track_index);
      std::cout << "reco start: " << track.start.X() << " " << track.start.Y() << " " << track.start.Z() << std::endl;
      std::cout << "reco end: " << track.end.X() << " " << track.end.Y() << " " << track.end.Z() << std::endl;
    }
    else {
      match.tmode = numu::tmOther;
      std::cout << "Track not matched.\n";
    }
  }
  else {
    match.tmode = numu::tmOther;
    std::cout << "No track to match.\n";
  }

  // Use the track to match the mode of the interaction
  if (reco.slice.primary_track_index >= 0 && 
    reco_tracks.at(reco.slice.primary_track_index).match.has_match) {
    const numu::TrackTruthMatch &ptrack_match = reco_tracks.at(reco.slice.primary_track_index).match;
    if (ptrack_match.mctruth_origin == simb::Origin_t::kBeamNeutrino && ptrack_match.mctruth_ccnc == simb::kCC) {
      if (ptrack_match.is_primary) {
        match.mode = numu::mCC;
      }
      else {
        match.mode = numu::mCCNonPrimary;
      }
    }
    else if (ptrack_match.mctruth_origin == simb::Origin_t::kBeamNeutrino && ptrack_match.mctruth_ccnc == simb::kNC) {
      if (ptrack_match.is_primary) {
        match.mode = numu::mNC;
      }
      else {
        match.mode = numu::mNCNonPrimary;
      }
    }
    else if (ptrack_match.mctruth_origin == simb::Origin_t::kCosmicRay) {
      match.mode = numu::mCosmic;
    }
    else {
      match.mode = numu::mOther;
    }
  }
  // **shrug**
  else {
    match.mode = numu::mOther;
  }
  match.has_match = true;

  // bad vertex
  match.is_misreconstructed = match.tmode == numu::tmNeutrino && !match.has_match;

  // get the mctruth indexes from the event indexes
  if (match.event_vertex_id != -1) match.mctruth_vertex_id = truth[match.event_vertex_id].match.mctruth_vertex_id;
  else match.mctruth_vertex_id = -1;
  if (match.event_track_id != -1) match.mctruth_track_id = truth[match.event_track_id].match.mctruth_track_id;
  else match.mctruth_track_id = -1;

  return match;
}

void numu::CorrectMultiMatches(const numu::RecoEvent &event, std::vector<numu::RecoInteraction> &recos) {
  // check for no two reco matched to same truth
  std::map<int, unsigned> matched_vertices;
  for (unsigned reco_i = 0; reco_i < recos.size(); reco_i++) {
    numu::RecoInteraction &reco = recos[reco_i];
    unsigned set_i = reco_i; 

    if (reco.primary_track.match.has_match && reco.primary_track.match.is_primary && reco.match.event_track_id >= 0 && reco.match.event_track_id < event.truth.size()-1) {
      if (matched_vertices.count(reco.match.event_track_id)) {
        std::cout << "BAAAAAAAAD: two reco matched to same truth." << std::endl;
        std::cout << "This id: " << reco.match.event_track_id << " N ids: " << recos.size() << " N truth: " << event.truth.size() << std::endl;
        for (const numu::RecoInteraction &reco: recos) {
          std::cout << "Interaction x: " << reco.position.X() << " match: " << reco.match.event_track_id << " primary track pdg: " << reco.primary_track.match.match_pdg << std::endl;
        }

        numu::RecoInteraction &other = recos[matched_vertices[reco.match.event_track_id]];

        float dist_reco = std::min( (reco.primary_track.start - event.true_tracks.at(reco.primary_track.match.mcparticle_id).start).Mag(),
                                    (reco.primary_track.end   - event.true_tracks.at(reco.primary_track.match.mcparticle_id).start).Mag());

        float dist_other = std::min( (other.primary_track.start - event.true_tracks.at(other.primary_track.match.mcparticle_id).start).Mag(),
                                    (other.primary_track.end   - event.true_tracks.at(other.primary_track.match.mcparticle_id).start).Mag());

        // Handle if one of the matched particles in not a muon
        if (abs(reco.primary_track.match.match_pdg) != 13 && abs(other.primary_track.match.match_pdg) == 13) {
          set_i = matched_vertices[reco.match.event_track_id];
          reco.primary_track.match.is_primary = false; 
          std::cout << "Corrected -- matched track is non-muon\n";
        } 
        else if (abs(reco.primary_track.match.match_pdg) == 13 && abs(other.primary_track.match.match_pdg) != 13) {
          other.primary_track.match.is_primary = false; 
          std::cout << "Corrected -- matched track is non-muon\n";
        } 
        // Handle both two different particles neither of which are muons
        else if (reco.primary_track.match.mcparticle_id != other.primary_track.match.mcparticle_id) {
          std::cout << "Reco track 1 length: " << reco.primary_track.length << " pdg: " << reco.primary_track.match.match_pdg << std::endl; 
          std::cout << "Reco track 2 length: " << other.primary_track.length << " pdg: " << other.primary_track.match.match_pdg << std::endl; 
          if (reco.primary_track.length > other.primary_track.length) {
            other.primary_track.match.is_primary = false;
            std::cout << "Corrected -- used longer track. Track 1 is primary.\n";
          }
          else {
            reco.primary_track.match.is_primary = false;
            std::cout << "Corrected -- used longer track. Track 2 is primary.\n";
          }
        }
        // Handle if the priamry track was split in two
        else if (reco.primary_track.match.mcparticle_id == other.primary_track.match.mcparticle_id) {
          std::cout << "Reco track 1 pos: " << reco.primary_track.start.X() << " " << reco.primary_track.start.Y() << " " << reco.primary_track.start.Z() << " to: " << reco.primary_track.end.X() << " " << reco.primary_track.end.Y() << " " << reco.primary_track.end.Z() << std::endl;
          std::cout << "Reco track 2 pos: " << other.primary_track.start.X() << " " << other.primary_track.start.Y() << " " << other.primary_track.start.Z() << " to: " << other.primary_track.end.X() << " " << other.primary_track.end.Y() << " " << other.primary_track.end.Z() << std::endl;
          TVector3 match_start = event.true_tracks.at(other.primary_track.match.mcparticle_id).start;
          std::cout << "Match start pos: " << match_start.X() << " " << match_start.Y() << " " << match_start.Z() << std::endl;
          if (dist_reco <= dist_other) {
            std::cout << "Corrected -- split muon. Track 1 is primary.\n";
            other.primary_track.match.is_primary = false; 
          }
          else {
            std::cout << "Corrected -- split muon. Track 2 is primary.\n";
            set_i = matched_vertices[reco.match.event_track_id];
            reco.primary_track.match.is_primary = false; 
          }
        }
        else { 
          std::cout << "Unable to correct!\n";
          std::cerr << "Exiting on failure.\n";
          assert(false);
        }
      }
      matched_vertices[reco.match.event_track_id] = set_i;
    } 
  }
}

