## import skeleton process
from PhysicsTools.PatAlgos.patTemplate_cfg import *
#process.Tracer = cms.Service('Tracer')

from PhysicsTools.PatAlgos.tools.jetTools import *

updateJetCollection(
    process,
    labelName = 'AK8PFCHS',
    jetSource = cms.InputTag('slimmedJetsAK8'),
    algo = 'ak8',
    rParam = 0.8,
    jetCorrections = ('AK8PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None')
    )

patJetsAK8 = process.updatedPatJetsAK8PFCHS

process.out.outputCommands += ['keep *_updatedPatJetsAK8PFCHS_*_*']

####################################################################################################
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#Grooming valueMaps

from RecoJets.Configuration.RecoPFJets_cff import ak8PFJetsCHSPruned, ak8PFJetsCHSSoftDrop, ak8PFJetsCHSRecursiveSoftDrop, ak8PFJetsCHSTrimmed, ak8PFJetsCHSFiltered
process.ak8PFJetsCHSSoftDrop = ak8PFJetsCHSSoftDrop.clone()
patAlgosToolsTask.add(process.ak8PFJetsCHSSoftDrop)
process.ak8PFJetsCHSRecursiveSoftDrop = ak8PFJetsCHSRecursiveSoftDrop.clone()
patAlgosToolsTask.add(process.ak8PFJetsCHSRecursiveSoftDrop)
process.ak8PFJetsCHSSoftDrop.src = cms.InputTag("packedPFCandidates")
process.ak8PFJetsCHSRecursiveSoftDrop.src = cms.InputTag("packedPFCandidates")
process.ak8PFJetsCHSRecursiveSoftDrop.nRSD=cms.int32(3)

from RecoJets.Configuration.RecoPFJets_cff import ak8PFJetsCHSPrunedMass, ak8PFJetsCHSSoftDropMass, ak8PFJetsCHSRecursiveSoftDropMass, ak8PFJetsCHSTrimmedMass, ak8PFJetsCHSFilteredMass
process.ak8PFJetsCHSSoftDropMass = ak8PFJetsCHSSoftDropMass.clone()
patAlgosToolsTask.add(process.ak8PFJetsCHSSoftDropMass)
process.ak8PFJetsCHSRecursiveSoftDropMass = ak8PFJetsCHSRecursiveSoftDropMass.clone()
patAlgosToolsTask.add(process.ak8PFJetsCHSRecursiveSoftDropMass)
process.ak8PFJetsCHSSoftDropMass.src = cms.InputTag("slimmedJetsAK8")
process.ak8PFJetsCHSRecursiveSoftDropMass.src = cms.InputTag("slimmedJetsAK8")

patJetsAK8.userData.userFloats.src += ['ak8PFJetsCHSSoftDropMass','ak8PFJetsCHSRecursiveSoftDropMass']
process.out.outputCommands += ['keep *_ak8PFJetsCHSSoftDropMass_*_*',
                               'keep *_ak8PFJetsCHSRecursiveSoftDropMass_*_*',]

### adding PATjets softdrop
addJetCollection(
                  process,
                  labelName = "AK8PATJetsSoftDrop",
                  jetSource = cms.InputTag('ak8PFJetsCHSSoftDrop' ),
                  algo = 'ak8',
                  rParam = 0.8,
                  jetCorrections = ('AK8PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None'),
                  btagDiscriminators = ['None'],
                  getJetMCFlavour = False, # jet flavor should always be disabled for groomed jets
                  genParticles = cms.InputTag('prunedGenParticles'),
                  pvSource = cms.InputTag( 'offlineSlimmedPrimaryVertices' ),
                  genJetCollection = cms.InputTag('slimmedGenJetsAK8'),
                  )

process.out.outputCommands += ['keep *_patJetsAK8PATJetsSoftDrop_*_*']

from RecoJets.Configuration.RecoGenJets_cff import ak4GenJets
process.GenJetsNoNuSoftDrop = ak4GenJets.clone(
                                               src = cms.InputTag('prunedGenParticles'),
                                               useSoftDrop = cms.bool(True),
                                               rParam = 0.8,
                                               jetAlgorithm = 'AntiKt',
                                               useExplicitGhosts=cms.bool(True),
                                               R0= cms.double(0.8),
                                               beta=cms.double(0),
                                               zcut=cms.double(0.1),
                                               writeCompound = cms.bool(True),
                                               jetCollInstanceName=cms.string('SubJets')
                                               )

patAlgosToolsTask.add(process.GenJetsNoNuSoftDrop)

addJetCollection(
                 process,
                 labelName = "PATSubjetsSoftDropLabel",
                 jetSource = cms.InputTag( 'ak8PFJetsCHSSoftDrop', 'SubJets'),
                 algo = 'AntiKt',
                 rParam = 0.8,
                 jetCorrections = ('AK4PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None'),
                 pfCandidates = cms.InputTag( 'packedPFCandidates' ),
                 pvSource = cms.InputTag( 'offlineSlimmedPrimaryVertices' ),
                 svSource = cms.InputTag( 'slimmedSecondaryVertices' ),
                 muSource = cms.InputTag( 'slimmedMuons' ),
                 elSource = cms.InputTag( 'slimmedElectrons' ),
                 btagDiscriminators = ['None'],
                 genJetCollection = cms.InputTag( "GenJetsNoNuSoftDrop",'SubJets'),
                 getJetMCFlavour = False,
                 genParticles = cms.InputTag('prunedGenParticles'),
                 explicitJTA = True,  # needed for subjet b tagging
                 svClustering = True, # needed for subjet b tagging
                 fatJets=cms.InputTag('updatedPatJetsAK8PFCHS'),             # needed for subjet flavor clustering
                 groomedFatJets=cms.InputTag('patJetsAK8PATJetsSoftDrop'), # needed for subjet flavor clustering
                 )

process.selPATJetsSoftDropPacked = cms.EDProducer("BoostedJetMerger",
    jetSrc=cms.InputTag('patJetsAK8PATJetsSoftDrop'),
    subjetSrc=cms.InputTag("patJetsPATSubjetsSoftDropLabel")
  )
process.out.outputCommands += ['keep *_selPATJetsSoftDropPacked_SubJets_*' ]
patAlgosToolsTask.add(process.selPATJetsSoftDropPacked)

#### Pack fat jets with subjets
process.packedPatJetsSoftDrop = cms.EDProducer("JetSubstructurePacker",
                             jetSrc=cms.InputTag('updatedPatJetsAK8PFCHS'),
                             distMax = cms.double( 0.8 ),
                             fixDaughters = cms.bool(False),
                             algoTags = cms.VInputTag("selPATJetsSoftDropPacked"),
                             algoLabels =cms.vstring('SoftDrop')
                             )
process.out.outputCommands += [ 'keep *_packedPatJetsSoftDrop_*_*' ]
patAlgosToolsTask.add(process.packedPatJetsSoftDrop)



####### Recursive softdrop

addJetCollection(
                  process,
                  labelName = "AK8PATJetsRecursiveSoftDrop",
                  jetSource = cms.InputTag('ak8PFJetsCHSRecursiveSoftDrop' ),
                  algo = 'ak8',
                  rParam = 0.8,
                  jetCorrections = ('AK8PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None'),
                  btagDiscriminators = ['None'],
                  getJetMCFlavour = False, # jet flavor should always be disabled for groomed jets
                  genParticles = cms.InputTag('prunedGenParticles'),
                  pvSource = cms.InputTag( 'offlineSlimmedPrimaryVertices' ),
                  genJetCollection = cms.InputTag('slimmedGenJetsAK8'),
                  )

process.out.outputCommands += ['keep *_patJetsAK8PATJetsRecursiveSoftDrop_*_*']

from RecoJets.Configuration.RecoGenJets_cff import ak4GenJets
process.GenJetsNoNuRecursiveSoftDrop = ak4GenJets.clone(
                                               src = cms.InputTag('prunedGenParticles'),
                                               useRecursiveSoftDrop = cms.bool(True),
                                               rParam = 0.8,
                                               jetAlgorithm = 'AntiKt',
                                               useExplicitGhosts=cms.bool(True),
                                               R0= cms.double(0.8),
                                               beta=cms.double(0),
                                               zcut=cms.double(0.1),
                                               nRSD=cms.int32(3),
                                               writeCompound = cms.bool(True),
                                               jetCollInstanceName=cms.string('SubJets')
                                               )

patAlgosToolsTask.add(process.GenJetsNoNuRecursiveSoftDrop)

addJetCollection(
                 process,
                 labelName = "PATSubjetsRecursiveSoftDropLabel",
                 jetSource = cms.InputTag( 'ak8PFJetsCHSRecursiveSoftDrop', 'SubJets'),
                 algo = 'AntiKt',
                 rParam = 0.8,
                 jetCorrections = ('AK4PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None'),
                 pfCandidates = cms.InputTag( 'packedPFCandidates' ),
                 pvSource = cms.InputTag( 'offlineSlimmedPrimaryVertices' ),
                 svSource = cms.InputTag( 'slimmedSecondaryVertices' ),
                 muSource = cms.InputTag( 'slimmedMuons' ),
                 elSource = cms.InputTag( 'slimmedElectrons' ),
                 btagDiscriminators = ['None'],
                 genJetCollection = cms.InputTag( "GenJetsNoNuRecursiveSoftDrop",'SubJets'),
                 getJetMCFlavour = False,
                 genParticles = cms.InputTag('prunedGenParticles'),
                 explicitJTA = True,  # needed for subjet b tagging
                 svClustering = True, # needed for subjet b tagging
                 fatJets=cms.InputTag('updatedPatJetsAK8PFCHS'),             # needed for subjet flavor clustering
                 groomedFatJets=cms.InputTag('patJetsAK8PATJetsRecursiveSoftDrop'), # needed for subjet flavor clustering
                 )

process.selPATJetsRecursiveSoftDropPacked = cms.EDProducer("BoostedJetMerger",
    jetSrc=cms.InputTag('patJetsAK8PATJetsRecursiveSoftDrop'),
    subjetSrc=cms.InputTag("patJetsPATSubjetsRecursiveSoftDropLabel")
  )
process.out.outputCommands += ['keep *_selPATJetsRecursiveSoftDropPacked_SubJets_*' ]
patAlgosToolsTask.add(process.selPATJetsRecursiveSoftDropPacked)

#### Pack fat jets with subjets
process.packedPatJetsRecursiveSoftDrop = cms.EDProducer("JetSubstructurePacker",
                             jetSrc=cms.InputTag('updatedPatJetsAK8PFCHS'),
                             distMax = cms.double( 0.8 ),
                             fixDaughters = cms.bool(False),
                             algoTags = cms.VInputTag("selPATJetsRecursiveSoftDropPacked"),
                             algoLabels =cms.vstring('RecursiveSoftDrop')
                             )
process.out.outputCommands += [ 'keep *_packedPatJetsRecursiveSoftDrop_*_*' ]
patAlgosToolsTask.add(process.packedPatJetsRecursiveSoftDrop)

####################################################################################################

## ------------------------------------------------------
#  In addition you usually want to change the following
#  parameters:
## ------------------------------------------------------
#
#   process.GlobalTag.globaltag =  ...    ##  (according to https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideFrontierConditions)
#from Configuration.AlCa.GlobalTag import GlobalTag
#process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc')
#                                         ##
import PhysicsTools.PatAlgos.patInputFiles_cff
from PhysicsTools.PatAlgos.patInputFiles_cff import filesRelValTTbarPileUpMINIAODSIM
process.source.fileNames = filesRelValTTbarPileUpMINIAODSIM
#                                         ##
process.maxEvents.input = 200
#                                         ##
#   process.out.outputCommands = [ ... ]  ##  (e.g. taken from PhysicsTools/PatAlgos/python/patEventContent_cff.py)
#                                         ##
process.out.fileName = 'testJetTools.root'
#                                         ##
#   process.options.wantSummary = False   ##  (to suppress the long output at the end of the job)
