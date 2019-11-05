#include "HistoList.h"
#include "TH1.h"
namespace ana {
 namespace SBNOsc {

void HistoList::Write() {
  for (TH1 *hist: fAllHistos) hist->Write();
}

void HistoList::Scale(double scale) {
  for (TH1 *hist: fAllHistos) hist->Scale(scale);
}

void HistoList::Add(const HistoList &other) {
  for (unsigned i = 0; i < fAllHistos.size(); i++) {
    fAllHistos[i]->Add(other.fAllHistos[i]);
  }
}
  }
}
