## import skeleton process
from PhysicsTools.PatAlgos.patTemplate_cfg import *
## switch to uncheduled mode
process.options.allowUnscheduled = cms.untracked.bool(True)

process.load('PhysicsTools.PatAlgos.producersLayer1.patCandidates_cff')
process.load('PhysicsTools.PatAlgos.selectionLayer1.selectedPatCandidates_cff')
process.load("RecoJets.Configuration.RecoGenJets_cff")
process.load("RecoJets.Configuration.GenJetParticles_cff")

process.load('RecoJets.Configuration.RecoPFJets_cff')

from PhysicsTools.PatAlgos.tools.jetTools import addJetCollection
from PhysicsTools.PatAlgos.tools.jetTools import switchJetCollection

addJetCollection(
    process,
    labelName = 'AK4PFCHS',
    jetSource = cms.InputTag('ak4PFJetsCHS'),
    algo = 'ak4',
    rParam = 0.4,
    jetCorrections = ('AK5PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None')
    )

addJetCollection(
    process,
    labelName = 'CA8PFCHS',
    jetSource = cms.InputTag('ca8PFJetsCHS'),
    algo = 'ca8',
    rParam = 0.8,
    jetCorrections = ('AK7PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None')
    )

addJetCollection(
    process,
    labelName = 'AK8PFCHS',
    jetSource = cms.InputTag('ak8PFJetsCHS'),
    algo = 'ak8',
    rParam = 0.8,
    jetCorrections = ('AK7PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None')
    )

switchJetCollection(
    process,
    jetSource = cms.InputTag('ak4PFJets'),
    rParam = 0.4,
    jetCorrections = ('AK5PF', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'Type-1'),
    btagDiscriminators = ['jetBProbabilityBJetTags',
                          'jetProbabilityBJetTags',
                          'trackCountingHighPurBJetTags',
                          'trackCountingHighEffBJetTags',
                          'simpleSecondaryVertexHighEffBJetTags',
                          'simpleSecondaryVertexHighPurBJetTags',
                          'combinedSecondaryVertexBJetTags'
                          ],
    )

from PhysicsTools.PatAlgos.patInputFiles_cff import filesRelValProdTTbarAODSIM
process.source.fileNames = filesRelValProdTTbarAODSIM

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(10) )

process.out.outputCommands += ['keep *_ak4PFJetsCHS_*_*',
                               'keep *_patJetsAK4PFCHS_*_*',
                               'keep *_ca8PFJetsCHS_*_*',
                               'keep *_patJetsCA8PFCHS_*_*',
                               'keep *_ak8PFJetsCHS_*_*',
                               'keep *_patJetsAK8PFCHS_*_*']

process.out.fileName = 'jettoolbox.root'
