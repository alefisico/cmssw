import FWCore.ParameterSet.Config as cms

process = cms.Process('jetToolbox')
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(10) )

process.load('PhysicsTools.PatAlgos.producersLayer1.patCandidates_cff')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.Geometry_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = 'START70_V6::All'

process.chs = cms.EDFilter('CandPtrSelector', src = cms.InputTag('packedPFCandidates'), cut = cms.string('fromPV'))

process.load('RecoJets.Configuration.RecoPFJets_cff')
process.load('RecoJets.Configuration.RecoGenJets_cff')

process.ak4PFJets.src = 'packedPFCandidates'
process.ak4PFJets.doAreaFastjet = True

process.ak4PFJetsCHS = process.ak4PFJetsCHS.clone(src = 'chs', doAreaFastjet = True)
process.ak8PFJetsCHS = process.ak4PFJetsCHS.clone(src = 'chs', doAreaFastjet = True, rParam = 0.8)
process.ca8PFJetsCHS = process.ca4PFJets.clone(src = 'chs', doAreaFastjet = True, rParam = 0.8)

process.ak4GenJets.src = 'packedGenParticles'
process.ak8GenJets = process.ak4GenJets.clone(src = 'packedGenParticles', rParam = 0.8)
process.ca8GenJets = process.ca4GenJets.clone(src = 'packedGenParticles', rParam = 0.8)

process.fixedGridRhoFastjetAll.pfCandidatesTag = 'packedPFCandidates'

process.ak8PFJetsCHSPruned.src = 'chs'
process.ak8PFJetsCHSTrimmed.src = 'chs'
process.ak8PFJetsCHSFiltered.src = 'chs'

process.ca8PFJetsCHSPruned.src = 'chs'
process.ca8PFJetsCHSTrimmed.src = 'chs'
process.ca8PFJetsCHSFiltered.src = 'chs'

process.cmsTopTagPFJetsCHS.src = 'chs'

from PhysicsTools.PatAlgos.tools.jetTools import addJetCollection
from PhysicsTools.PatAlgos.tools.jetTools import switchJetCollection

addJetCollection(
    process,
    labelName = 'AK4PFCHS',
    jetSource = cms.InputTag('ak4PFJetsCHS'),
    algo = 'ak4',
    rParam = 0.4,
    jetCorrections = ('AK5PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None'),
    trackSource = cms.InputTag('unpackedTracksAndVertices'),
    pvSource = cms.InputTag('unpackedTracksAndVertices'),
    btagDiscriminators = ['combinedSecondaryVertexBJetTags'],
    ) 

addJetCollection(
    process,
    labelName = 'CA8PFCHS',
    jetSource = cms.InputTag('ca8PFJetsCHS'),
    algo = 'ca8',
    rParam = 0.8,
    jetCorrections = ('AK7PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None'),
    trackSource = cms.InputTag('unpackedTracksAndVertices'),
    pvSource = cms.InputTag('unpackedTracksAndVertices'),
    )                                                                                                                                                                                  
addJetCollection(
    process,
    labelName = 'AK8PFCHS',
    jetSource = cms.InputTag('ak8PFJetsCHS'),
    algo = 'ak8',
    rParam = 0.8,
    jetCorrections = ('AK7PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None'),
    trackSource = cms.InputTag('unpackedTracksAndVertices'),
    pvSource = cms.InputTag('unpackedTracksAndVertices'),
    ) 
"""
switchJetCollection(
    process,
    jetSource = cms.InputTag('ak4PFJets'),
    algo = 'ak4',
    rParam = 0.4,
    jetCorrections = ('AK5PF', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'Type-1'),
    # btagDiscriminators = ['jetBProbabilityBJetTags',
    #                       'jetProbabilityBJetTags',
    #                       'trackCountingHighPurBJetTags',
    #                       'trackCountingHighEffBJetTags',
    #                       'simpleSecondaryVertexHighEffBJetTags',
    #                       'simpleSecondaryVertexHighPurBJetTags',
    #                       'combinedSecondaryVertexBJetTags'
    #                       ],
    trackSource = cms.InputTag('unpackedTracksAndVertices'),
    pvSource = cms.InputTag('unpackedTracksAndVertices'),
    )
"""

process.patJetsAK4PFCHS.addJetCharge   = False
process.patJetsAK4PFCHS.addBTagInfo    = True
process.patJetsAK4PFCHS.getJetMCFlavour = False
process.patJetsAK4PFCHS.addAssociatedTracks = False
process.patJetPartonMatchAK4PFCHS.matched='prunedGenParticles'
process.patJetCorrFactorsAK4PFCHS.primaryVertices = 'offlineSlimmedPrimaryVertices'

process.patJetsCA8PFCHS.addJetCharge   = False
process.patJetsCA8PFCHS.addBTagInfo    = False   #For some reason this has to be False
process.patJetsCA8PFCHS.getJetMCFlavour = False
process.patJetsCA8PFCHS.addAssociatedTracks = False
process.patJetPartonMatchCA8PFCHS.matched='prunedGenParticles'
process.patJetCorrFactorsCA8PFCHS.primaryVertices = 'offlineSlimmedPrimaryVertices'

process.patJetsAK8PFCHS.addJetCharge   = False
process.patJetsAK8PFCHS.addBTagInfo    = False    #For some reason this has to be False
process.patJetsAK8PFCHS.getJetMCFlavour = False
process.patJetsAK8PFCHS.addAssociatedTracks = False
process.patJetPartonMatchAK8PFCHS.matched='prunedGenParticles'
process.patJetCorrFactorsAK8PFCHS.primaryVertices = 'offlineSlimmedPrimaryVertices'

process.patJets.addJetCharge   = False
process.patJets.addBTagInfo    = True
process.patJets.getJetMCFlavour = False
process.patJets.addAssociatedTracks = False
process.patJetPartonMatch.matched = 'prunedGenParticles'
process.patJetCorrFactors.primaryVertices = 'offlineSlimmedPrimaryVertices'


process.load('RecoBTag.Configuration.RecoBTag_cff')
process.load('RecoJets.Configuration.RecoJetAssociations_cff')
process.load('PhysicsTools.PatAlgos.slimming.unpackedTracksAndVertices_cfi')

process.ak4JetTracksAssociatorAtVertexPF.jets = cms.InputTag('ak4PFJetsCHS')
process.ak4JetTracksAssociatorAtVertexPF.tracks = cms.InputTag('unpackedTracksAndVertices')
process.ak8JetTracksAssociatorAtVertexPF=process.ak4JetTracksAssociatorAtVertexPF.clone(jets = cms.InputTag('ak8PFJetsCHS'),
                                                                                        coneSize = 0.8)
process.ca8JetTracksAssociatorAtVertexPF=process.ak4JetTracksAssociatorAtVertexPF.clone(jets = cms.InputTag('ca8PFJetsCHS'),
                                                                                        coneSize = 0.8)

process.impactParameterTagInfos.primaryVertex = cms.InputTag('unpackedTracksAndVertices')
process.inclusiveSecondaryVertexFinderTagInfos.extSVCollection = cms.InputTag('unpackedTracksAndVertices','secondary','')
process.combinedSecondaryVertex.trackMultiplicityMin = 1 #silly sv, uses un filtered tracks.. i.e. any pt

process.load('FWCore.MessageLogger.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 10
process.MessageLogger.suppressWarning = cms.untracked.vstring('ecalLaserCorrFilter','manystripclus53X','toomanystripclus53X')
process.options = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )
process.options.allowUnscheduled = cms.untracked.bool(True)

process.out = cms.OutputModule('PoolOutputModule',
                               fileName = cms.untracked.string('jettoolbox.root'),
                               outputCommands = cms.untracked.vstring(['keep *_ak4PFJetsCHS_*_*',
                                                                       'keep *_patJetsAK4PFCHS_*_*',
                                                                       'keep *_ca8PFJetsCHS_*_*',
                                                                       'keep *_patJetsCA8PFCHS_*_*',
                                                                       'keep *_ak8PFJetsCHS_*_*',
                                                                       'keep *_patJetsAK8PFCHS_*_*',
                                                                       ])
                               )
process.endpath = cms.EndPath(process.out)

process.source = cms.Source("PoolSource",
                            fileNames = cms.untracked.vstring('/store/user/jstupak/ZH_HToBB_ZToLL_M-125_13TeV_powheg-herwigpp/Spring14dr-PU_S14_POSTLS170_V6AN1-v1/140622_185946/0000/miniAOD-prod_PAT_1.root')
                            )
