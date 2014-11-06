miniAODInput=True

#Prepare all the toolbox inputs
if miniAODInput:
    from RecoJets.JetProducers.jettoolboxMiniHelper_cff import *
else:
    from RecoJets.JetProducers.jettoolboxHelper_cff import *

#Load the toolbox
process.load('RecoJets.JetProducers.jettoolbox_cff')

if miniAODInput: 
    #QGTagger.srcVertexCollection = 'unpackedTracksAndVertices'
    pass

#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
#Keep statements for valueMaps (link Reco::Jets to associated quantities)
#You don't have to keep them to deswizzle below

#out.outputCommands += ['keep *_pileupJetIdEvaluator_*_*',
#                       'keep *_QGTagger_*_*']

process.out.outputCommands += ['keep *_NjettinessCA8_*_*',
                               'keep *_QJetsAdderCA8_*_*',
                               'keep *_ca8PFJetsCHSPrunedLinks_*_*','keep *_ca8PFJetsCHSTrimmedLinks_*_*','keep *_ca8PFJetsCHSFilteredLinks_*_*',
                               'keep *_cmsTopTagPFJetsCHSLinksCA8_*_*']
                               
process.out.outputCommands += ['keep *_NjettinessAK8_*_*',
                               'keep *_QJetsAdderAK8_*_*',
                               'keep *_ak8PFJetsCHSPrunedLinks_*_*','keep *_ak8PFJetsCHSTrimmedLinks_*_*','keep *_ak8PFJetsCHSFilteredLinks_*_*',
                               'keep *_cmsTopTagPFJetsCHSLinksAK8_*_*']

#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#Deswizzle valueMaps and attach to PAT::Jets as userFloats

#process.patJetsAK4PFCHS.userData.userFloats.src += ['pileupJetIdEvaluator:fullDiscriminant','QGTagger:qgLikelihood']
#process.patJetsAK4PFCHS.userData.userInts.src   += ['pileupJetIdEvaluator:cutbasedId','pileupJetIdEvaluator:fullId']

process.patJetsCA8PFCHS.userData.userFloats.src += ['NjettinessCA8:tau1','NjettinessCA8:tau2','NjettinessCA8:tau3',
                                                    'QJetsAdderCA8:QjetsVolatility',
                                                    'ca8PFJetsCHSPrunedLinks','ca8PFJetsCHSTrimmedLinks','ca8PFJetsCHSFilteredLinks',
                                                    'cmsTopTagPFJetsCHSLinksCA8']

process.patJetsAK8PFCHS.userData.userFloats.src += ['NjettinessAK8:tau1','NjettinessAK8:tau2','NjettinessAK8:tau3',
                                                    'QJetsAdderAK8:QjetsVolatility',
                                                    'ak8PFJetsCHSPrunedLinks','ak8PFJetsCHSTrimmedLinks','ak8PFJetsCHSFilteredLinks',
                                                    'cmsTopTagPFJetsCHSLinksAK8']
