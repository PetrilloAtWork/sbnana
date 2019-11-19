#include <iostream>

#include "PMTTrigger.h"

#include "sbndcode/OpDetSim/sbndPDMapAlg.h"

std::vector<numu::FlashTriggerPrimitive> numu::TriggerPrimitives(const std::vector<raw::OpDetWaveform> &waveforms, double tick_period, std::pair<double, double> &window, int thresh, bool is_sbnd) {
  std::vector<numu::FlashTriggerPrimitive> ret;

  opdet::sbndPDMapAlg mapalg;
  for (const raw::OpDetWaveform &wvf: waveforms) {
    // check if this is a PMT
    bool is_pmt = (!is_sbnd) || mapalg.pdType(wvf.ChannelNumber(), "pmt");

    if (!is_pmt) continue;

    // look for any overlap with the enable window
    double waveform_start = wvf.TimeStamp();
    int waveform_index_start = std::max((int)((window.first - waveform_start) / tick_period), 0);
    int waveform_index_end = std::min((int)((window.second - waveform_start) / tick_period), (int) wvf.size());

    if (waveform_index_start < waveform_index_end) {
      numu::FlashTriggerPrimitive prim;
      prim.channel = wvf.ChannelNumber();
      for (int i = waveform_index_start; i < waveform_index_end; i++) {
        if (wvf[i] <= thresh) { // PMT waveforms go down
          std::pair<int, unsigned> this_prim {wvf[i], i + (int)(waveform_start - window.first) / tick_period};
          prim.triggers.push_back(this_prim);
        }
      }
      ret.push_back(prim);
    }
  }
  return ret;
}

bool numu::HasTrigger(const std::vector<numu::FlashTriggerPrimitive> &primitives, int threshold, unsigned n_above_threshold) {
  std::map<unsigned, unsigned> above_threshold;

  for (const numu::FlashTriggerPrimitive &primitive: primitives) {
    for (const std::pair<unsigned, unsigned> &trig: primitive.triggers) {
      if (trig.first > threshold) {
        if (above_threshold.count(trig.second)) above_threshold[trig.second] = 1;
        else above_threshold[trig.second] += 1;
      }
    }
  }

  for (auto const &pair: above_threshold) {
    if (pair.second >= n_above_threshold) {
      return true;
    }
  }

  return false;
}


