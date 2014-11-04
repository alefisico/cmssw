/** \class HLTLeadJetInMass
 *
 * See header file for documentation
 *
 *
 *  \author Martin Grunewald
 *
 */

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/HLTReco/interface/TriggerFilterObjectWithRefs.h"

#include "HLTrigger/HLTfilters/interface/HLTLeadJetInMass.h"

#include <typeinfo>


//
// constructors and destructor
//
template<typename T>
HLTLeadJetInMass<T>::HLTLeadJetInMass(const edm::ParameterSet& iConfig) : HLTFilter(iConfig),
  inputTag_    (iConfig.template getParameter<edm::InputTag>("inputTag")),
  inputToken_  (consumes<std::vector<T> >(inputTag_)),
  triggerType_ (iConfig.template getParameter<int>("triggerType")),
  min_N_    (iConfig.template getParameter<int>          ("MinN"    )),
  min_E_    (iConfig.template getParameter<double>       ("MinE"    )),
  min_Pt_   (iConfig.template getParameter<double>       ("MinPt"   )),
  min_Mass_ (iConfig.template getParameter<double>       ("MinMass" )),
  max_Eta_  (iConfig.template getParameter<double>       ("MaxEta"  ))
{
   LogDebug("") << "Input/ptcut/etacut/ncut : "
		<< inputTag_.encode() << " "
		<< min_E_ << " " << min_Pt_ << " " << min_Mass_ << " "
		<< max_Eta_ << " " << min_N_ ;
}

template<typename T>
HLTLeadJetInMass<T>::~HLTLeadJetInMass()
{
}

template<typename T>
void
HLTLeadJetInMass<T>::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  makeHLTFilterDescription(desc);
  desc.add<edm::InputTag>("inputTag",edm::InputTag("hltCollection"));
  desc.add<int>("triggerType",0);
  desc.add<double>("MinE",-1.0);
  desc.add<double>("MinPt",-1.0);
  desc.add<double>("MinMass",-1.0);
  desc.add<double>("MaxEta",-1.0);
  desc.add<int>("MinN",1);
  descriptions.add(std::string("hlt")+std::string(typeid(HLTLeadJetInMass<T>).name()),desc);
}

//
// member functions
//

// ------------ method called to produce the data  ------------
template<typename T>
bool
HLTLeadJetInMass<T>::hltFilter(edm::Event& iEvent, const edm::EventSetup& iSetup, trigger::TriggerFilterObjectWithRefs & filterproduct) const
{
   using namespace std;
   using namespace edm;
   using namespace reco;
   using namespace trigger;

   typedef vector<T> TCollection;
   typedef Ref<TCollection> TRef;

   // All HLT filters must create and fill an HLT filter object,
   // recording any reconstructed physics objects satisfying (or not)
   // this HLT filter, and place it in the Event.

   // The filter object
   if (saveTags()) filterproduct.addCollectionTag(inputTag_);

   // Ref to Candidate object to be recorded in filter object
   TRef ref;


   // get hold of collection of objects
   Handle<TCollection> objects;
   iEvent.getByToken(inputToken_,objects);

   // look at all objects, check cuts 
   vector<T> selectedObjects;
   for(const T &j : *objects){
	   if( (j.pt() <= min_Pt_) || ( TMath::Abs( j.eta() ) > max_Eta_ ) || j.energy() <= min_E_ ) continue;
	   selectedObjects.push_back( j );
   }
   // sorting in mass
   sort( selectedObjects.begin(), selectedObjects.end(), [](const T &j1, const T &j2){ return ( j1.mass() > j2.mass() ); });

   int n(0);
   // Final mass cut and add to filter object
   typename TCollection::const_iterator i ( selectedObjects.begin() );
     if ( i->mass() >= min_Mass_ ) { 
       n++;
       ref=TRef(objects,distance(objects->begin(),i));
       filterproduct.addObject(triggerType_, ref);
     }


   // filter decision
   bool accept(n>=min_N_);

   return accept;
}


