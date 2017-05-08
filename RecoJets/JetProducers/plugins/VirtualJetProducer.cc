// -*- C++ -*-
//
// Package:    RecoJets/JetProducers
// Class:      VirtualJetProducer
// 
/**\class VirtualJetProducer VirtualJetProducer.cc RecoJets/JetProducers/plugins/VirtualJetProducer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Philipp Schieferdecker <philipp.schieferdecker@cern.ch>  04/21/2009 
//     Modified by:  Alejandro Gomez Espinosa
//         Created:  Thu, 20 Apr 2017 10:04:23 GMT
//
//


// user include files
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "RecoJets/JetProducers/interface/VirtualJetProducer.h"
#include "RecoJets/JetProducers/interface/BackgroundEstimator.h"
#include "RecoJets/JetProducers/interface/VirtualJetProducerHelper.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/Common/interface/RefProd.h"
#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/Common/interface/RefVector.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Utilities/interface/isFinite.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/JetReco/interface/CaloJetCollection.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/JetReco/interface/PFJetCollection.h"
#include "DataFormats/JetReco/interface/BasicJetCollection.h"
#include "DataFormats/JetReco/interface/TrackJetCollection.h"
#include "DataFormats/JetReco/interface/PFClusterJetCollection.h"
#include "DataFormats/Candidate/interface/CandidateFwd.h"
#include "DataFormats/Candidate/interface/LeafCandidate.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"

#include "fastjet/SISConePlugin.hh"
#include "fastjet/CMSIterativeConePlugin.hh"
#include "fastjet/ATLASConePlugin.hh"
#include "fastjet/CDFMidPointPlugin.hh"

#include <iostream>
#include <memory>
#include <algorithm>
#include <limits>
#include <cmath>
#include <vdt/vdtMath.h>


using namespace std;
using namespace edm;

namespace reco {
	namespace helper {
		struct GreaterByPtPseudoJet {
			bool operator()( const fastjet::PseudoJet & t1, const fastjet::PseudoJet & t2 ) const { return t1.perp2() > t2.perp2(); }
		};
	}
}                                                                                        

const char *const VirtualJetProducer::JetType::names[] = {
	"BasicJet","GenJet","CaloJet","PFJet","TrackJet","PFClusterJet"
};


VirtualJetProducer::JetType::Type VirtualJetProducer::JetType::byName(const string &name) {

	const char *const *pos = find(names, names + LastJetType, name);
	if (pos == names + LastJetType) {
		string errorMessage="Requested jetType not supported: "+name+"\n";
		throw cms::Exception("Configuration",errorMessage);
	}
	return (Type)(pos-names);
}


void VirtualJetProducer::makeProduces( string alias, string tag ) {

	if ( writeCompound_ ) produces<reco::BasicJetCollection>(); 

	if (makeCaloJet(jetTypeE)) produces<reco::CaloJetCollection>(tag).setBranchAlias(alias);
	else if (makePFJet(jetTypeE)) produces<reco::PFJetCollection>(tag).setBranchAlias(alias);
	else if (makeGenJet(jetTypeE)) produces<reco::GenJetCollection>(tag).setBranchAlias(alias);
	else if (makeTrackJet(jetTypeE)) produces<reco::TrackJetCollection>(tag).setBranchAlias(alias);
	else if (makePFClusterJet(jetTypeE)) produces<reco::PFClusterJetCollection>(tag).setBranchAlias(alias);
	else if (makeBasicJet(jetTypeE)) produces<reco::BasicJetCollection>(tag).setBranchAlias(alias);

}

//
// constructors and destructor
//
VirtualJetProducer::VirtualJetProducer(const edm::ParameterSet& iConfig) {

	moduleLabel_   		= iConfig.getParameter<string> ("module_label");
	jetType_       		= iConfig.getParameter<string> 	("jetType");
	jetAlgorithm_  		= iConfig.getParameter<string>  ("jetAlgorithm");
	rParam_        		= iConfig.getParameter<double>  ("rParam");
	inputEtMin_    		= iConfig.getParameter<double>  ("inputEtMin");
	inputEMin_     		= iConfig.getParameter<double>  ("inputEMin");
	jetPtMin_      		= iConfig.getParameter<double>  ("jetPtMin");
	doPVCorrection_		= iConfig.getParameter<bool>    ("doPVCorrection");
	doAreaFastjet_ 		= iConfig.getParameter<bool>    ("doAreaFastjet");
	doRhoFastjet_  		= iConfig.getParameter<bool>    ("doRhoFastjet");
	jetCollInstanceName_ 	= iConfig.getParameter<string>	("jetCollInstanceName");
	doPUOffsetCorr_		= iConfig.getParameter<bool>	("doPUOffsetCorr");
	puSubtractorName_  	= iConfig.getParameter<string>	("subtractorName");
	useExplicitGhosts_ 	= iConfig.getParameter<bool>	("useExplicitGhosts");  // use explicit ghosts in the fastjet clustering sequence?
	doAreaDiskApprox_ 	= iConfig.getParameter<bool>	("doAreaDiskApprox");
	voronoiRfact_     	= iConfig.getParameter<double>	("voronoiRfact"); 	// Voronoi-based area calculation allows for an empirical scale factor
	rhoEtaMax		= iConfig.getParameter<double>	("Rho_EtaMax"); 		// do fasjet area / rho calcluation? => accept corresponding parameters
	ghostEtaMax 		= iConfig.getParameter<double>	("Ghost_EtaMax");
	activeAreaRepeats 	= iConfig.getParameter<int> 	("Active_Area_Repeats");
	ghostArea 		= iConfig.getParameter<double> 	("GhostArea");
	restrictInputs_ 	= iConfig.getParameter<bool>	("restrictInputs"); 	// restrict inputs to first "maxInputs" towers?
	maxInputs_      	= iConfig.getParameter<unsigned int>("maxInputs");
	writeCompound_ 		= iConfig.getParameter<bool>	("writeCompound"); 	// Check to see if we are writing compound jets for substructure and jet grooming
	doFastJetNonUniform_ 	= iConfig.getParameter<bool>   	("doFastJetNonUniform");
	puWidth_ 		= iConfig.getParameter<double>	("puWidth");
	nExclude_ 		= iConfig.getParameter<unsigned int>("nExclude");
	useDeterministicSeed_ 	= iConfig.getParameter<bool>	("useDeterministicSeed");
	minSeed_ 		= iConfig.getParameter<unsigned int>("minSeed");
	verbosity_ 		= iConfig.getParameter<int>	("verbosity");

	anomalousTowerDef_ = auto_ptr<AnomalousTower>(new AnomalousTower(iConfig));

	input_vertex_token_ = consumes<reco::VertexCollection>(iConfig.getParameter<InputTag>("srcPVs"));
	input_candidateview_token_ = consumes<reco::CandidateView>(iConfig.getParameter<edm::InputTag>("src"));
	input_candidatefwdptr_token_ = consumes<vector<edm::FwdPtr<reco::PFCandidate> > >(iConfig.getParameter<edm::InputTag>("src"));
	input_packedcandidatefwdptr_token_ = consumes<vector<edm::FwdPtr<pat::PackedCandidate> > >(iConfig.getParameter<edm::InputTag>("src"));
	//
	// additional parameters to think about:
	// - overlap threshold (set to 0.75 for the time being)
	// - p parameter for generalized kT (set to -2 for the time being)
	// - fastjet PU subtraction parameters (not yet considered)
	//
	if (jetAlgorithm_=="Kt") 
		fjJetDefinition_= JetDefPtr(new fastjet::JetDefinition(fastjet::kt_algorithm,rParam_));

	else if (jetAlgorithm_=="CambridgeAachen")
		fjJetDefinition_= JetDefPtr(new fastjet::JetDefinition(fastjet::cambridge_algorithm,rParam_) );

	else if (jetAlgorithm_=="AntiKt")
		fjJetDefinition_= JetDefPtr( new fastjet::JetDefinition(fastjet::antikt_algorithm,rParam_) );

	else if (jetAlgorithm_=="GeneralizedKt") 
		fjJetDefinition_= JetDefPtr( new fastjet::JetDefinition(fastjet::genkt_algorithm,rParam_,-2) );

	else if (jetAlgorithm_=="SISCone") {

		fjPlugin_ = PluginPtr( new fastjet::SISConePlugin(rParam_,0.75,0,0.0,false,fastjet::SISConePlugin::SM_pttilde) );
		fjJetDefinition_= JetDefPtr( new fastjet::JetDefinition(&*fjPlugin_) );

	} else if (jetAlgorithm_=="IterativeCone") {

		fjPlugin_ = PluginPtr(new fastjet::CMSIterativeConePlugin(rParam_,1.0));
		fjJetDefinition_= JetDefPtr(new fastjet::JetDefinition(&*fjPlugin_));

	} else if (jetAlgorithm_=="CDFMidPoint") {

		fjPlugin_ = PluginPtr(new fastjet::CDFMidPointPlugin(rParam_,0.75));
		fjJetDefinition_= JetDefPtr(new fastjet::JetDefinition(&*fjPlugin_));

	} else if (jetAlgorithm_=="ATLASCone") {

		fjPlugin_ = PluginPtr(new fastjet::ATLASConePlugin(rParam_));
		fjJetDefinition_= JetDefPtr(new fastjet::JetDefinition(&*fjPlugin_));

	} else {
		throw cms::Exception("Invalid jetAlgorithm") <<"Jet algorithm for VirtualJetProducer is invalid, Abort!\n";
	}

	jetTypeE=JetType::byName(jetType_);

	if ( doPUOffsetCorr_  ) {
		if(puSubtractorName_.empty()){
			LogWarning("VirtualJetProducer") << "Pile Up correction on; however, pile up type is not specified. Using default... \n";
			subtractor_ =  boost::shared_ptr<PileUpSubtractor>(new PileUpSubtractor(iConfig, consumesCollector()));
		} else subtractor_ =  boost::shared_ptr<PileUpSubtractor>(
				PileUpSubtractorFactory::get()->create( puSubtractorName_, iConfig, consumesCollector()));
	}

	// do approximate disk-based area calculation => warn if conflicting request
	if (doAreaDiskApprox_ && doAreaFastjet_)
		throw cms::Exception("Conflicting area calculations") << "Both the calculation of jet area via fastjet and via an analytical disk approximation have been requested. Please decide on one.\n";

	if ( doAreaFastjet_ || doRhoFastjet_ ) {

		if (voronoiRfact_ <= 0) {
			fjActiveArea_     = ActiveAreaSpecPtr(new fastjet::GhostedAreaSpec(ghostEtaMax,activeAreaRepeats,ghostArea));
			fjActiveArea_->set_fj2_placement(true);

			if ( !useExplicitGhosts_ ) {
				fjAreaDefinition_ = AreaDefinitionPtr( new fastjet::AreaDefinition(fastjet::active_area, *fjActiveArea_ ) );
			} else {
				fjAreaDefinition_ = AreaDefinitionPtr( new fastjet::AreaDefinition(fastjet::active_area_explicit_ghosts, *fjActiveArea_ ) );
			}
		}
		fjRangeDef_ = RangeDefPtr( new fastjet::RangeDefinition(rhoEtaMax) );
	} 

	if( doFastJetNonUniform_ ) puCenters_ 		= iConfig.getParameter<vector<double> >("puCenters"); 	/// where is this used? 

	// make the "produces" statements
	makeProduces( moduleLabel_, jetCollInstanceName_ );
	produces<vector<double> >("rhos");
	produces<vector<double> >("sigmas");
	produces<double>("rho");
	produces<double>("sigma");

	/*input_candidateview_token_ = consumes<reco::CandidateView>(src_);
	input_vertex_token_ = consumes<reco::VertexCollection>(srcPVs_);
	input_candidatefwdptr_token_ = consumes<vector<edm::FwdPtr<reco::PFCandidate> > >(src_);
	input_packedcandidatefwdptr_token_ = consumes<vector<edm::FwdPtr<pat::PackedCandidate> > >(src_);*/
  
}


VirtualJetProducer::~VirtualJetProducer() {
}


//
// member functions
//

// ------------ method called to produce the data  ------------
void VirtualJetProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
 
	// If requested, set the fastjet random seed to a deterministic function
	// of the run/lumi/event. 
	// NOTE!!! The fastjet random number sequence is a global singleton.
	// Thus, we have to create an object and get access to the global singleton
	// in order to change it. 
	if ( useDeterministicSeed_ ) {
		fastjet::GhostedAreaSpec gas;
		vector<int> seeds(2);
		unsigned int runNum_uint = static_cast <unsigned int> (iEvent.id().run());
		unsigned int evNum_uint = static_cast <unsigned int> (iEvent.id().event()); 
		seeds[0] = max(runNum_uint,minSeed_ + 3) + 3 * evNum_uint;
		seeds[1] = max(runNum_uint,minSeed_ + 5) + 5 * evNum_uint;
		gas.set_random_status(seeds);
	}

	LogDebug("VirtualJetProducer") << "Entered produce\n";
	//determine signal vertex2
	vertex_=reco::Jet::Point(0,0,0);
	if ( (makeCaloJet(jetTypeE) || makePFJet(jetTypeE)) && doPVCorrection_) {
		LogDebug("VirtualJetProducer") << "Adding PV info\n";
		edm::Handle<reco::VertexCollection> pvCollection;
		iEvent.getByToken(input_vertex_token_ , pvCollection);
		if (pvCollection->size()>0) vertex_=pvCollection->begin()->position();
	}

	// For Pileup subtraction using offset correction:
	// set up geometry map
	if ( doPUOffsetCorr_ ) subtractor_->setupGeometryMap(iEvent, iSetup);

	// clear data
	LogDebug("VirtualJetProducer") << "Clear data\n";
	fjInputs_.clear();
	fjJets_.clear();
	inputs_.clear();  

	// get inputs and convert them to the fastjet format (fastjet::PeudoJet)
	edm::Handle<reco::CandidateView> inputsHandle;

	edm::Handle< vector<edm::FwdPtr<reco::PFCandidate> > > pfinputsHandleAsFwdPtr; 
	edm::Handle< vector<edm::FwdPtr<pat::PackedCandidate> > > packedinputsHandleAsFwdPtr; 

	bool isView = iEvent.getByToken(input_candidateview_token_, inputsHandle);
	if ( isView ) {

		if ( inputsHandle->size() == 0) {
			output( iEvent, iSetup );
			return;
		}
		for (size_t i = 0; i < inputsHandle->size(); ++i) {
			inputs_.push_back(inputsHandle->ptrAt(i));
		}

	} else {

		bool isPF = iEvent.getByToken(input_candidatefwdptr_token_, pfinputsHandleAsFwdPtr);
		if ( isPF ) {
			if ( pfinputsHandleAsFwdPtr->size() == 0) {
				output( iEvent, iSetup );
				return;
			}
			for (size_t i = 0; i < pfinputsHandleAsFwdPtr->size(); ++i) {
				if ( (*pfinputsHandleAsFwdPtr)[i].ptr().isAvailable() ) {
					inputs_.push_back( (*pfinputsHandleAsFwdPtr)[i].ptr() );
				} else if ( (*pfinputsHandleAsFwdPtr)[i].backPtr().isAvailable() ) {
					inputs_.push_back( (*pfinputsHandleAsFwdPtr)[i].backPtr() );
				}
			}
		} else {
			iEvent.getByToken(input_packedcandidatefwdptr_token_, packedinputsHandleAsFwdPtr);
			if ( packedinputsHandleAsFwdPtr->size() == 0) {
				output( iEvent, iSetup );
				return;
			}
			for (size_t i = 0; i < packedinputsHandleAsFwdPtr->size(); ++i) {
				if ( (*packedinputsHandleAsFwdPtr)[i].ptr().isAvailable() ) {
					inputs_.push_back( (*packedinputsHandleAsFwdPtr)[i].ptr() );
				} else if ( (*packedinputsHandleAsFwdPtr)[i].backPtr().isAvailable() ) {
					inputs_.push_back( (*packedinputsHandleAsFwdPtr)[i].backPtr() );
				}
			}
		}
	}
	LogDebug("VirtualJetProducer") << "Got inputs\n";

	// Convert candidates to fastjet::PseudoJets.
	// Also correct to Primary Vertex. Will modify fjInputs_
	// and use inputs_
	fjInputs_.reserve(inputs_.size());
	inputTowers();
	LogDebug("VirtualJetProducer") << "Inputted towers\n";

	// For Pileup subtraction using offset correction:
	// Subtract pedestal. 
	if ( doPUOffsetCorr_ ) {
		subtractor_->setDefinition(fjJetDefinition_);
		subtractor_->reset(inputs_,fjInputs_,fjJets_);
		subtractor_->calculatePedestal(fjInputs_); 
		subtractor_->subtractPedestal(fjInputs_);    
		LogDebug("VirtualJetProducer") << "Subtracted pedestal\n";
	}
	// Run algorithm. Will modify fjJets_ and allocate fjClusterSeq_. 
	// This will use fjInputs_
	runAlgorithm( iEvent, iSetup );

	// if ( doPUOffsetCorr_ ) {
	//    subtractor_->setAlgorithm(fjClusterSeq_);
	// }

	LogDebug("VirtualJetProducer") << "Ran algorithm\n";
	// For Pileup subtraction using offset correction:
	// Now we find jets and need to recalculate their energy,
	// mark towers participated in jet,
	// remove occupied towers from the list and recalculate mean and sigma
	// put the initial towers collection to the jet,   
	// and subtract from initial towers in jet recalculated mean and sigma of towers 
	if ( doPUOffsetCorr_ ) {
		LogDebug("VirtualJetProducer") << "Do PUOffsetCorr\n";
		vector<fastjet::PseudoJet> orphanInput;
		subtractor_->calculateOrphanInput(orphanInput);
		subtractor_->calculatePedestal(orphanInput);
		subtractor_->offsetCorrectJets();
	}
	// Write the output jets.
	// This will (by default) call the member function template
	// "writeJets", but can be overridden. 
	// this will use inputs_
	output( iEvent, iSetup );
	LogDebug("VirtualJetProducer") << "Wrote jets\n";

	// Clear the work vectors so that memory is free for other modules.
	// Use the trick of swapping with an empty vector so that the memory
	// is actually given back rather than silently kept.
	decltype(fjInputs_)().swap(fjInputs_);
	decltype(fjJets_)().swap(fjJets_);
	decltype(inputs_)().swap(inputs_);  

	return;
}

void VirtualJetProducer::inputTowers() {

	auto inBegin = inputs_.begin(),	inEnd = inputs_.end(), i = inBegin;
	for (; i != inEnd; ++i ) {

		auto const & input = **i;
		// cout << "CaloTowerVI jets " << input->pt() << " " << input->et() << ' '<< input->energy() << ' ' << (isAnomalousTower(input) ? " bad" : " ok") << endl; 
		if (edm::isNotFinite(input.pt()))           continue;
		if (input.et()    <inputEtMin_)  continue;
		if (input.energy()<inputEMin_)   continue;
		if (isAnomalousTower(*i))      continue;
		// Change by SRR : this is no longer an error nor warning, this can happen with PU mitigation algos.
		// Also switch to something more numerically safe. (VI: 10^-42GeV????)
		if (input.pt() < 100 * numeric_limits<double>::epsilon() ) continue;

		if (makeCaloJet(jetTypeE)&&doPVCorrection_) {

			const CaloTower & tower = dynamic_cast<const CaloTower &>(input);
			auto const &  ct = tower.p4(vertex_);  // very expensive as computed in eta/phi
			fjInputs_.emplace_back(ct.px(),ct.py(),ct.pz(),ct.energy());
			//cout << "tower:" << *tower << '\n';

		} else {
			/*
			if(makePFJet(jetTypeE)) {
			reco::PFCandidate& pfc = (reco::PFCandidate&)input;
			cout << "PF cand:" << pfc << '\n';
			}
			*/
			fjInputs_.emplace_back(input.px(),input.py(),input.pz(), input.energy());
		}

		fjInputs_.back().set_user_index(i - inBegin);
	}

	if ( restrictInputs_ && fjInputs_.size() > maxInputs_ ) {

		reco::helper::GreaterByPtPseudoJet   pTComparator;
		sort(fjInputs_.begin(), fjInputs_.end(), pTComparator);
		fjInputs_.resize(maxInputs_);
		LogWarning("JetRecoTooManyEntries") << "Too many inputs in the event, limiting to first " << maxInputs_ << ". Output is suspect.";

	}

}

bool VirtualJetProducer::isAnomalousTower(reco::CandidatePtr input) {
	if (!makeCaloJet(jetTypeE)) return false;
	else return (*anomalousTowerDef_)(*input);
}
 

void VirtualJetProducer::copyConstituents(const vector<fastjet::PseudoJet>& fjConstituents, reco::Jet* jet) {

	for (unsigned int i=0;i<fjConstituents.size();++i) { 
		int index = fjConstituents[i].user_index();
		if ( index >= 0 && static_cast<unsigned int>(index) < inputs_.size() )
		jet->addDaughter(inputs_[index]);
	}
}

vector<reco::CandidatePtr> VirtualJetProducer::getConstituents(const vector<fastjet::PseudoJet>&fjConstituents) {

	vector<reco::CandidatePtr> result; result.reserve(fjConstituents.size()/2);
	for (unsigned int i=0;i<fjConstituents.size();i++) {
		auto index = fjConstituents[i].user_index();
		if ( index >= 0 && static_cast<unsigned int>(index) < inputs_.size() ) {
			result.emplace_back(inputs_[index]);
		}
	}
	return result;
}


void VirtualJetProducer::output(edm::Event & iEvent, edm::EventSetup const& iSetup) {

	// Write jets and constitutents. Will use fjJets_, inputs_
	// and fjClusterSeq_

	if ( !writeCompound_ ) {

		switch( jetTypeE ) {
			case JetType::CaloJet :
				writeJets<reco::CaloJet>( iEvent, iSetup);
				break;
			case JetType::PFJet :
				writeJets<reco::PFJet>( iEvent, iSetup);
				break;
			case JetType::GenJet :
				writeJets<reco::GenJet>( iEvent, iSetup);
				break;
			case JetType::TrackJet :
				writeJets<reco::TrackJet>( iEvent, iSetup);
				break;
			case JetType::PFClusterJet :
				writeJets<reco::PFClusterJet>( iEvent, iSetup);
				break;
			case JetType::BasicJet :
				writeJets<reco::BasicJet>( iEvent, iSetup);
				break;
			default:
				throw cms::Exception("InvalidInput") << "invalid jet type in VirtualJetProducer\n";
				break;
		};

	} else {

		// Write jets and constitutents.
		switch( jetTypeE ) {
			case JetType::CaloJet :
				writeCompoundJets<reco::CaloJet>( iEvent, iSetup );
				break;
			case JetType::PFJet :
				writeCompoundJets<reco::PFJet>( iEvent, iSetup );
				break;
			case JetType::GenJet :
				writeCompoundJets<reco::GenJet>( iEvent, iSetup );
				break;
			case JetType::BasicJet :
				writeCompoundJets<reco::BasicJet>( iEvent, iSetup );
				break;
			default:
				throw cms::Exception("InvalidInput") << "invalid jet type in CompoundJetProducer\n";
				break;
		};
	}
}

namespace {
	template< typename T >
	struct Area { static float get(T const &) {return 0;}};

	template<>
	struct Area<reco::CaloJet>{ static float get(reco::CaloJet const & jet) { 
		return jet.getSpecific().mTowersArea; 
		} 
	};
}


template< typename T > 
void VirtualJetProducer::writeJets( edm::Event & iEvent, edm::EventSetup const& iSetup ) {

	// cout << "writeJets " << typeid(T).name() 
	//          << (doRhoFastjet_ ? " doRhoFastjet " : "")
	//          << (doAreaFastjet_ ? " doAreaFastjet " : "")
	//          << (doAreaDiskApprox_ ? " doAreaDiskApprox " : "")
	//          << endl;

	if (doRhoFastjet_) {

		// declare jet collection without the two jets, 
		// for unbiased background estimation.
		vector<fastjet::PseudoJet> fjexcluded_jets;
		fjexcluded_jets=fjJets_;

		if(fjexcluded_jets.size()>2) fjexcluded_jets.resize(nExclude_);

		if(doFastJetNonUniform_){

			auto rhos = make_unique<vector<double>>();
			auto sigmas = make_unique<vector<double>>();
			int nEta = puCenters_.size();
			rhos->reserve(nEta);
			sigmas->reserve(nEta);
			fastjet::ClusterSequenceAreaBase const* clusterSequenceWithArea =
			dynamic_cast<fastjet::ClusterSequenceAreaBase const *> ( &*fjClusterSeq_ );

			if (clusterSequenceWithArea ==nullptr ){
				if (fjJets_.size() > 0) 
					throw cms::Exception("LogicError")<<"fjClusterSeq is not initialized while inputs are present\n ";
			} else {
				for(int ie = 0; ie < nEta; ++ie){
					double eta = puCenters_[ie];
					double etamin=eta-puWidth_;
					double etamax=eta+puWidth_;
					fastjet::RangeDefinition range_rho(etamin,etamax);
					fastjet::BackgroundEstimator bkgestim(*clusterSequenceWithArea,range_rho);
					bkgestim.set_excluded_jets(fjexcluded_jets);
					rhos->push_back(bkgestim.rho());
					sigmas->push_back(bkgestim.sigma());
				}
			}

			iEvent.put(move(rhos),"rhos");
			iEvent.put(move(sigmas),"sigmas");

		}else{
			auto rho = make_unique<double>(0.0);
			auto sigma = make_unique<double>(0.0);
			double mean_area = 0;

			fastjet::ClusterSequenceAreaBase const* clusterSequenceWithArea =
			dynamic_cast<fastjet::ClusterSequenceAreaBase const *> ( &*fjClusterSeq_ );
			/*
			const double nemptyjets = clusterSequenceWithArea->n_empty_jets(*fjRangeDef_);
			if(( nemptyjets  < -15 ) || ( nemptyjets > fjRangeDef_->area()+ 15)) {
			LogWarning("StrangeNEmtpyJets") << "n_empty_jets is : " << clusterSequenceWithArea->n_empty_jets(*fjRangeDef_) << " with range " << fjRangeDef_->description() << ".";
			}
			*/
			if (clusterSequenceWithArea ==nullptr ){
				if (fjJets_.size() > 0) 
					throw cms::Exception("LogicError")<<"fjClusterSeq is not initialized while inputs are present\n ";
			} else {

				clusterSequenceWithArea->get_median_rho_and_sigma(*fjRangeDef_,false,*rho,*sigma,mean_area);
				if((*rho < 0)|| (edm::isNotFinite(*rho))) {
					LogError("BadRho") << "rho value is " << *rho 
						<< " area:" << mean_area 
						<< " and n_empty_jets: " << clusterSequenceWithArea->n_empty_jets(*fjRangeDef_) 
						<< " with range " << fjRangeDef_->description()
						<<". Setting rho to rezo.";
					*rho = 0;
				}
			}
			iEvent.put(move(rho),"rho");
			iEvent.put(move(sigma),"sigma");
		}
	} // doRhoFastjet_


	// produce output jet collection
	using namespace reco;

	// allocate fjJets_.size() Ts in vector  
	auto jets = make_unique<vector<T>>(fjJets_.size());

	// Distance between jet centers and overlap area -- for disk-based area calculation
	using RIJ = pair<double,double>; 
	vector<RIJ>   rijStorage(fjJets_.size()*(fjJets_.size()/2));
	RIJ * rij[fjJets_.size()];
	unsigned int k=0;
	for (unsigned int ijet=0;ijet<fjJets_.size();++ijet) {
		rij[ijet] = &rijStorage[k]; k+=ijet;
	}

	float etaJ[fjJets_.size()],  phiJ[fjJets_.size()];

	auto orParam_ = 1./rParam_;
	// fill jets 
	for (unsigned int ijet=0;ijet<fjJets_.size();++ijet) {

		auto & jet = (*jets)[ijet];
		// get the fastjet jet
		const fastjet::PseudoJet& fjJet = fjJets_[ijet];
		// get the constituents from fastjet
		vector<fastjet::PseudoJet> const & fjConstituents = fastjet::sorted_by_pt(fjJet.constituents());
		// convert them to CandidatePtr vector
		vector<CandidatePtr> const & constituents = getConstituents(fjConstituents);

		// write the specifics to the jet (simultaneously sets 4-vector, vertex).
		// These are overridden functions that will call the appropriate
		// specific allocator.
		writeSpecific(jet,
			  Particle::LorentzVector(fjJet.px(),
						  fjJet.py(),
						  fjJet.pz(),
						  fjJet.E()),
			  vertex_,
			  constituents, iSetup);
		phiJ[ijet] = jet.phi();
		etaJ[ijet] = jet.eta();
	}

	// calcuate the jet area
	for (unsigned int ijet=0;ijet<fjJets_.size();++ijet) {
		// calcuate the jet area
		double jetArea=0.0;
		// get the fastjet jet
		const auto & fjJet = fjJets_[ijet];
		if ( doAreaFastjet_ && fjJet.has_area() ) jetArea = fjJet.area();
		else if ( doAreaDiskApprox_ ) {

			// Here it is assumed that fjJets_ is in decreasing order of pT, 
			// which should happen in FastjetJetProducer::runAlgorithm() 
			jetArea   = M_PI;
			RIJ *  distance  = rij[ijet];
			for (unsigned jJet = 0; jJet < ijet; ++jJet) {
				distance[jJet].first      = sqrt(reco::deltaR2(etaJ[ijet],phiJ[ijet], etaJ[jJet],phiJ[jJet]))*orParam_;
				distance[jJet].second = reco::helper::VirtualJetProducerHelper::intersection(distance[jJet].first);
				jetArea            -=distance[jJet].second;
				for (unsigned kJet = 0; kJet < jJet; ++kJet) {
					jetArea += reco::helper::VirtualJetProducerHelper::intersection(distance[jJet].first, 
							distance[kJet].first, rij[jJet][kJet].first, distance[jJet].second, distance[kJet].second, rij[jJet][kJet].second);
				  } // end loop over harder jets
			} // end loop over harder jets
			jetArea  *= (rParam_*rParam_);
		} 

		auto & jet = (*jets)[ijet]; 
		jet.setJetArea (jetArea);

		if(doPUOffsetCorr_) jet.setPileup(subtractor_->getPileUpEnergy(ijet));
		else jet.setPileup (0.0);

		// cout << "area " << ijet << " " << jetArea << " " << Area<T>::get(jet) << endl;
		// cout << "JetVI " << ijet << ' '<< jet.pt() << " " << jet.et() << ' '<< jet.energy() << ' '<< jet.mass() << endl;

	}
	// put the jets in the collection
	iEvent.put(move(jets),jetCollInstanceName_);
}


/// function template to write out the outputs
template< class T>
void VirtualJetProducer::writeCompoundJets( edm::Event & iEvent, edm::EventSetup const& iSetup ) {

	if ( verbosity_ >= 1 ) 
		cout << "<VirtualJetProducer::writeCompoundJets (moduleLabel = " << moduleLabel_ << ")>:" << endl;

	// get a list of output jets
	auto jetCollection = make_unique<reco::BasicJetCollection>();
	// get a list of output subjets
	auto subjetCollection = make_unique<vector<T>>();

	// This will store the handle for the subjets after we write them
	edm::OrphanHandle< vector<T> > subjetHandleAfterPut;
	// this is the mapping of subjet to hard jet
	vector< vector<int> > indices;
	// this is the list of hardjet 4-momenta
	vector<math::XYZTLorentzVector> p4_hardJets;
	// this is the hardjet areas
	vector<double> area_hardJets;

	// Loop over the hard jets
	vector<fastjet::PseudoJet>::const_iterator it = fjJets_.begin(),
	iEnd = fjJets_.end(),
	iBegin = fjJets_.begin();
	indices.resize( fjJets_.size() );
	for ( ; it != iEnd; ++it ) {
		fastjet::PseudoJet const & localJet = *it;
		unsigned int jetIndex = it - iBegin;
		// Get the 4-vector for the hard jet
		p4_hardJets.push_back( math::XYZTLorentzVector(localJet.px(), localJet.py(), localJet.pz(), localJet.e() ));
		double localJetArea = 0.0;
		if ( doAreaFastjet_ && localJet.has_area() ) localJetArea = localJet.area();
		area_hardJets.push_back( localJetArea );

		// create the subjet list
		vector<fastjet::PseudoJet> constituents;
		if ( it->has_pieces() ) constituents = it->pieces();
		else if ( it->has_constituents() ) constituents = it->constituents();

		vector<fastjet::PseudoJet>::const_iterator itSubJetBegin = constituents.begin(),
		itSubJet = itSubJetBegin, itSubJetEnd = constituents.end();
		for (; itSubJet != itSubJetEnd; ++itSubJet ){

			fastjet::PseudoJet const & subjet = *itSubJet;      
			if ( verbosity_ >= 1 ) {
				cout << "subjet #" << (itSubJet - itSubJetBegin) 
					<< ": Pt = " << subjet.pt() 
					<< ", eta = " << subjet.eta() 
					<< ", phi = " << subjet.phi() 
					<< ", mass = " << subjet.m() 
					<< " (#constituents = " << subjet.constituents().size() 
					<< ")" << endl;
				vector<fastjet::PseudoJet> subjet_constituents = subjet.constituents();
				int idx_constituent = 0;
				for ( vector<fastjet::PseudoJet>::const_iterator constituent = subjet_constituents.begin();
				constituent != subjet_constituents.end(); ++constituent ) {
					if ( constituent->pt() < 1.e-3 ) continue; // CV: skip ghosts
					cout << "  constituent #" << idx_constituent 
						<< ": Pt = " << constituent->pt() 
						<< ", eta = " << constituent->eta() 
						<< ", phi = " << constituent->phi() 
						<< ", mass = " << constituent->m() << endl;
					++idx_constituent;
				}
			}

			if ( verbosity_ >= 1 ) {
				cout << "subjet #" << (itSubJet - itSubJetBegin) 
					<< ": Pt = " << subjet.pt() 
					<< ", eta = " << subjet.eta() 
					<< ", phi = " << subjet.phi() 
					<< ", mass = " << subjet.m() 
					<< " (#constituents = " << subjet.constituents().size() 
					<< ")" << endl;
				vector<fastjet::PseudoJet> subjet_constituents = subjet.constituents();
				int idx_constituent = 0;
				for ( vector<fastjet::PseudoJet>::const_iterator constituent = subjet_constituents.begin(); 
				constituent != subjet_constituents.end(); ++constituent ) {

					if ( constituent->pt() < 1.e-3 ) continue; // CV: skip ghosts
					cout << " constituent #" << idx_constituent 
						<< ": Pt = " << constituent->pt() 
						<< ", eta = " << constituent->eta() 
						<< ", phi = " << constituent->phi() 
						<< ", mass = " << constituent->m() << endl;
					++idx_constituent;
				}
			}

			math::XYZTLorentzVector p4Subjet(subjet.px(), subjet.py(), subjet.pz(), subjet.e() );
			reco::Particle::Point point(0,0,0);

			// This will hold ptr's to the subjets
			vector<reco::CandidatePtr> subjetConstituents;

			// Get the transient subjet constituents from fastjet
			vector<fastjet::PseudoJet> subjetFastjetConstituents = subjet.constituents();
			vector<reco::CandidatePtr> constituents = getConstituents(subjetFastjetConstituents );    
			indices[jetIndex].push_back( subjetCollection->size() );

			// Add the concrete subjet type to the subjet list to write to event record
			T jet;
			reco::writeSpecific( jet, p4Subjet, point, constituents, iSetup);
			double subjetArea = 0.0;
			if ( doAreaFastjet_ && itSubJet->has_area() ) subjetArea = itSubJet->area();
			jet.setJetArea( subjetArea );
			subjetCollection->push_back( jet );
		}
	}

	// put subjets into event record
	subjetHandleAfterPut = iEvent.put(move(subjetCollection), jetCollInstanceName_);

	// Now create the hard jets with ptr's to the subjets as constituents
	vector<math::XYZTLorentzVector>::const_iterator ip4 = p4_hardJets.begin(),
	ip4Begin = p4_hardJets.begin(),
	ip4End = p4_hardJets.end();
	for ( ; ip4 != ip4End; ++ip4 ) {
		int p4_index = ip4 - ip4Begin;
		vector<int> & ind = indices[p4_index];
		vector<reco::CandidatePtr> i_hardJetConstituents;
		// Add the subjets to the hard jet
		for( vector<int>::const_iterator isub = ind.begin(); isub != ind.end(); ++isub ) {
			reco::CandidatePtr candPtr( subjetHandleAfterPut, *isub, false );
			i_hardJetConstituents.push_back( candPtr );
		}   
		reco::Particle::Point point(0,0,0);
		reco::BasicJet toput( *ip4, point, i_hardJetConstituents);
		toput.setJetArea( area_hardJets[ip4 - ip4Begin] );
		jetCollection->push_back( toput );
	}

	// put hard jets into event record
	// Store the Orphan handle for adding HTT information
	edm::OrphanHandle<reco::BasicJetCollection>  oh = iEvent.put(move(jetCollection));

	//////////// IS THIS EVER TRUE?
	if (fromHTTTopJetProducer_) addHTTTopJetTagInfoCollection( iEvent, iSetup, oh);
	///////////////////////////////////////////////

}



// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void VirtualJetProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {

	edm::ParameterSetDescription desc;
	desc.add<string> ("module_label",	"" );
	desc.add<edm::InputTag>("src",		edm::InputTag("particleFlow") );
	desc.add<edm::InputTag>("srcPVs",	edm::InputTag("") );
	desc.add<string>("jetType",		"PFJet" );
	desc.add<string>("jetAlgorithm",	"AntiKt" );
	desc.add<double>("rParam",		0.4 );
	desc.add<double>("inputEtMin",		0.0 );
	desc.add<double>("inputEMin",		0.0 );
	desc.add<double>("jetPtMin",		5. );
	desc.add<bool> 	("doPVCorrection",	false );
	desc.add<bool> 	("doAreaFastjet",	false );
	desc.add<bool>  ("doRhoFastjet",	false );
	desc.add<string>("jetCollInstanceName", ""	);
	desc.add<bool> 	("doPUOffsetCorr", 	false	);
	desc.add<string>("subtractorName", 	""	);
	desc.add<bool> 	("useExplicitGhosts", 	false	);
	desc.add<bool> 	("doAreaDiskApprox", 	false 	);
	desc.add<double>("voronoiRfact", 	-0.9 	);
	desc.add<double>("Rho_EtaMax", 	 	4.4 	);
	desc.add<double>("Ghost_EtaMax",	5. 	);
	desc.add<int> 	("Active_Area_Repeats",	1 	);
	desc.add<double>("GhostArea",	 	0.01 	);
	desc.add<bool> 	("restrictInputs", 	false 	);
	desc.add<unsigned int> 	("maxInputs", 	1 	);
	desc.add<bool> 	("writeCompound", 	false 	);
	desc.add<bool> 	("doFastJetNonUniform", false 	);
	desc.add<bool> 	("useDeterministicSeed",false 	);
	desc.add<unsigned int> 	("minSeed", 	14327 	);
	desc.add<int> 	("verbosity", 		1 	);
	desc.add<double>("puWidth",	 	0. 	);
	desc.add<unsigned int>("nExclude", 	0 	);
	desc.add<unsigned int>("maxBadEcalCells", 	9999999	);
	desc.add<unsigned int>("maxBadHcalCells",	9999999 );
	desc.add<unsigned int>("maxProblematicEcalCells",	9999999 );
	desc.add<unsigned int>("maxProblematicHcalCells",	9999999 );
	desc.add<unsigned int>("maxRecoveredEcalCells",	9999999 );
	desc.add<unsigned int>("maxRecoveredHcalCells",	9999999 );
	descriptions.addDefault(desc);
}

