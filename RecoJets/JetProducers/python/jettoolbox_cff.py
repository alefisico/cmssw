import FWCore.ParameterSet.Config as cms

from RecoJets.Configuration.RecoPFJets_cff import ca8PFJetsCHSPrunedLinks, ak8PFJetsCHSPrunedLinks

#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#pileupJetID
"""
from RecoJets.JetProducers.pileupjetidproducer_cfi import pileupJetIdCalculator, pileupJetIdEvaluator

pileupJetIdCalculator.jets = cms.InputTag("ak4PFJetsCHS")
pileupJetIdEvaluator.jets = cms.InputTag("ak4PFJetsCHS")
pileupJetIdCalculator.rho = cms.InputTag("fixedGridRhoFastjetAll")
pileupJetIdEvaluator.rho = cms.InputTag("fixedGridRhoFastjetAll")
"""
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#QGTagger
"""
from RecoJets.JetProducers.QGTagger_cfi import QGTagger

QGTagger.srcJets = cms.InputTag("ak4PFJetsCHS")
QGTagger.jetsLabel = cms.string('QGL_AK5PFchs')
"""
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#Njettiness

from RecoJets.JetProducers.nJettinessAdder_cfi import Njettiness

NjettinessCA8 = Njettiness.clone()
NjettinessCA8.src = cms.InputTag("ca8PFJetsCHS")
NjettinessCA8.cone = cms.double(0.8)
NjettinessCA8.Njets = cms.vuint32(1,2,3,4,5)

NjettinessAK8 = NjettinessCA8.clone()
NjettinessAK8.src = cms.InputTag("ak8PFJetsCHS")

#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#QJetsAdder

RandomNumberGeneratorService = cms.Service("RandomNumberGeneratorService",
                                                   QJetsAdderCA8 = cms.PSet(initialSeed = cms.untracked.uint32(7)),
                                                   QJetsAdderAK8 = cms.PSet(initialSeed = cms.untracked.uint32(31)))

from RecoJets.JetProducers.qjetsadder_cfi import QJetsAdder

QJetsAdderCA8 = QJetsAdder.clone()
QJetsAdderCA8.src = cms.InputTag("ca8PFJetsCHS")
QJetsAdderCA8.jetRad = cms.double(0.8)
QJetsAdderCA8.jetAlgo = cms.string('CA')

QJetsAdderAK8 = QJetsAdderCA8.clone()
QJetsAdderAK8.src = cms.InputTag("ak8PFJetsCHS")
QJetsAdderAK8.jetAlgo = cms.string('AK')

#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#Grooming valueMaps

cmsTopTagPFJetsCHSLinksCA8 = ca8PFJetsCHSPrunedLinks.clone()
cmsTopTagPFJetsCHSLinksCA8.src = cms.InputTag("ca8PFJetsCHS")
cmsTopTagPFJetsCHSLinksCA8.matched = cms.InputTag("cmsTopTagPFJetsCHS")

cmsTopTagPFJetsCHSLinksAK8 = cmsTopTagPFJetsCHSLinksCA8.clone()
cmsTopTagPFJetsCHSLinksAK8.src = cms.InputTag("ak8PFJetsCHS")
cmsTopTagPFJetsCHSLinksAK8.matched = cms.InputTag("cmsTopTagPFJetsCHS")

hepTopTagPFJetsCHSLinksCA8 = ca8PFJetsCHSPrunedLinks.clone()
hepTopTagPFJetsCHSLinksCA8.src = cms.InputTag("ca8PFJetsCHS")
hepTopTagPFJetsCHSLinksCA8.matched = cms.InputTag("hepTopTagPFJetsCHS")

hepTopTagPFJetsCHSLinksAK8 = hepTopTagPFJetsCHSLinksCA8.clone()
hepTopTagPFJetsCHSLinksAK8.src = cms.InputTag("ak8PFJetsCHS")
hepTopTagPFJetsCHSLinksAK8.matched = cms.InputTag("hepTopTagPFJetsCHS")

