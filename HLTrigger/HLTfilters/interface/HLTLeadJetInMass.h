#ifndef HLTLeadJetInMass_h
#define HLTLeadJetInMass_h

/** \class HLTLeadJetInMass
 *
 *
 *  This class is an HLTFilter (-> EDFilter) implementing a basic HLT
 *  trigger for the leading jet ordered in mass, cutting on
 *  variables relating to their 4-momentum representation
 *
 *
 *  \author Alejandro Gomez Espinosa
 *
 */

#include<vector>
#include "DataFormats/HLTReco/interface/TriggerTypeDefs.h"
#include "HLTrigger/HLTcore/interface/HLTFilter.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"

//
// class declaration
//

template<typename T>
class HLTLeadJetInMass : public HLTFilter {

   public:
      explicit HLTLeadJetInMass(const edm::ParameterSet&);
      ~HLTLeadJetInMass();
      static void fillDescriptions(edm::ConfigurationDescriptions & descriptions);
      virtual bool hltFilter(edm::Event&, const edm::EventSetup&, trigger::TriggerFilterObjectWithRefs & filterproduct) const override;

   private:
      const edm::InputTag                    inputTag_;     // input tag identifying product
      const edm::EDGetTokenT<std::vector<T>> inputToken_;   // token identifying product
      const int    triggerType_ ;                           // triggerType configured
      const int    min_N_;                                  // number of objects passing cuts required
      const double min_E_;                                  // energy threshold in GeV
      const double min_Pt_;                                 // pt threshold in GeV
      const double min_Mass_;                               // mass threshold in GeV
      const double max_Eta_;                                // eta range (symmetric)
};

#endif // HLTLeadJetInMass_h
