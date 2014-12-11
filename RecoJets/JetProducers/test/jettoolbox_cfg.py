import FWCore.ParameterSet.Config as cms

process = cms.Process('jetToolbox')
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100) )

process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.Geometry_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = 'PLS170_V7AN1::All'

process.load('FWCore.MessageLogger.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 10
process.MessageLogger.suppressWarning = cms.untracked.vstring('ecalLaserCorrFilter','manystripclus53X','toomanystripclus53X')
process.options = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )
process.options.allowUnscheduled = cms.untracked.bool(True)

from RecoJets.JetProducers.jetToolbox_cff import *
jetToolbox( process, 'ak8', 'ak8JetSubs', 'out', addSubjets= True ) #, addGroomers = False )
jetToolbox( process, 'ca8', 'ca8JetSubs', 'out', addNsubUpTo5 = True )
jetToolbox( process, 'ca15', 'ca15JetSubs', 'out' )


process.endpath = cms.EndPath(process.out)

process.source = cms.Source("PoolSource",
                            fileNames = cms.untracked.vstring('/store/user/jstupak/ZH_HToBB_ZToLL_M-125_13TeV_powheg-herwigpp/Spring14dr-PU_S14_POSTLS170_V6AN1-v1/140622_185946/0000/miniAOD-prod_PAT_1.root')
                            )

