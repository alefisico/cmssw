import FWCore.ParameterSet.Config as cms

from PhysicsTools.PatAlgos.producersLayer1.patCandidates_cff import *
from RecoJets.Configuration.RecoPFJets_cff import *
from RecoJets.Configuration.RecoGenJets_cff import * 

def jetToolbox( proc, jetType, addGroomers = True, addNsub = True, addNsubUpTo5 = False, addQJets = True, minPt = 100. ):
	
	###############################################################################
	#######  Just defining simple variables
	###############################################################################
	supportedJetAlgos = { 'ak': 'AntiKt', 'ca' : 'CambridgeAachen', 'kt' : 'Kt' }
	algo = ''
	jetAlgo = ''
	size = ''
	for type, algorithm in supportedJetAlgos.iteritems(): 
		if type in jetType.lower():
			algo = type
			jetAlgo = algorithm
			size = jetType.replace( type, '' )
	if algo == '': print 'Unsupported jet algorithm. Please use something like: jetType = CA8'

	coneSize = 0.
	if int(size) in range(0, 20): coneSize = int(size)/10.
	else: print 'coneSize has not a valid value. Insert a number between 1 and 20 after algorithm, like: AK8'
	### Trick for uppercase/lowercase algo name
	ALGO = algo.upper()+size
	algo = algo.lower()+size


	#################################################################################
	####### Toolbox start 
	#################################################################################

	elemToKeep = []

	### For MiniAOD, we need to load packedPFCandidates
	setattr( proc, 'chs', cms.EDFilter('CandPtrSelector', src = cms.InputTag('packedPFCandidates'), cut = cms.string('fromPV')) )

	setattr( proc, algo+'PFJetsCHS', ak4PFJetsCHS.clone( src = 'chs', doAreaFastjet = True, rParam = coneSize, jetAlgorithm = jetAlgo,  jetPtMin = minPt )) 
	elemToKeep += [ 'keep *_'+algo+'PFJetsCHS_*_*' ]

	#ak4GenJets.src = 'packedGenParticles'
	setattr( proc, algo+'GenJets', ak4GenJets.clone( src = 'packedGenParticles', rParam = coneSize, jetAlgorithm = jetAlgo ) ) 
	#fixedGridRhoFastjetAll.pfCandidatesTag = 'packedPFCandidates'

	####  Creating PATjets
	from PhysicsTools.PatAlgos.tools.jetTools import addJetCollection
	proc.load('PhysicsTools.PatAlgos.slimming.unpackedTracksAndVertices_cfi')

	if( int(size) > 10 ): size = '10' 	### For JEC for jets larger than 1 
	addJetCollection(
			proc,
			labelName = ALGO+'PFCHS',
			jetSource = cms.InputTag( algo+'PFJetsCHS'),
			algo = algo,
			rParam = coneSize,
			jetCorrections = ( 'AK'+size+'PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None'),
			trackSource = cms.InputTag('unpackedTracksAndVertices'),
			pvSource = cms.InputTag('unpackedTracksAndVertices'),
			btagDiscriminators = ['combinedSecondaryVertexBJetTags'],
			genJetCollection = cms.InputTag( algo+'GenJets' ),
			getJetMCFlavour = False 
			) 

	getattr( proc, 'patJets'+ALGO+'PFCHS' ).addJetCharge = False 
	getattr( proc, 'patJets'+ALGO+'PFCHS' ).addBTagInfo  = True
	getattr( proc, 'patJets'+ALGO+'PFCHS' ).addAssociatedTracks = False 
 	getattr( proc, 'patJetPartonMatch'+ALGO+'PFCHS' ).matched = 'prunedGenParticles' 
	getattr( proc, 'patJetCorrFactors'+ALGO+'PFCHS' ).primaryVertices = 'offlineSlimmedPrimaryVertices' 
	elemToKeep += [ 'keep *_patJets'+ALGO+'PFCHS_*_*' ]

	#proc.load('RecoBTag.Configuration.RecoBTag_cff')
	#proc.load('RecoJets.Configuration.RecoJetAssociations_cff')
	#setattr( proc, algo+'JetTracksAssociatorAtVertexPF', algo+'JetTracksAssociatorAtVertexPF'.clone( jets = cms.InputTag( algo+'PFJetsCHS' ), tracks = cms.InputTag('unpackedTracksAndVertices'), coneSize = coneSize ) ) 
	#impactParameterTagInfos.primaryVertex = cms.InputTag('unpackedTracksAndVertices')
	#inclusiveSecondaryVertexFinderTagInfos.extSVCollection = cms.InputTag('unpackedTracksAndVertices','secondary','')
	#combinedSecondaryVertex.trackMultiplicityMin = 1 #silly sv, uses un filtered tracks.. i.e. any pt

	#### For big jets
	if ( coneSize > 0.7 ):

		######### Prunning, Trimming, Filtering, cmsTopTagger
		if addGroomers:
			setattr( proc, algo+'PFJetsCHSPruned', ak8PFJetsCHSPruned.clone( src = 'chs', rParam = coneSize, jetAlgorithm = jetAlgo ) )
			setattr( proc, algo+'PFJetsCHSPrunedLinks', ak8PFJetsCHSPrunedLinks.clone( src = cms.InputTag( algo+"PFJetsCHS"), 
				matched = cms.InputTag( algo+'PFJetsCHSPruned'), distMax = cms.double( coneSize ) ) )

			setattr( proc, algo+'PFJetsCHSTrimmed', ak8PFJetsCHSTrimmed.clone( src = 'chs', rParam = coneSize, jetAlgorithm = jetAlgo ) ) 
			setattr( proc, algo+'PFJetsCHSTrimmedLinks', ak8PFJetsCHSTrimmedLinks.clone( src = cms.InputTag( algo+"PFJetsCHS"), 
				matched = cms.InputTag( algo+'PFJetsCHSTrimmed'), distMax = cms.double( coneSize ) ) )

			setattr( proc, algo+'PFJetsCHSFiltered', ak8PFJetsCHSFiltered.clone( src = 'chs', rParam = coneSize, jetAlgorithm = jetAlgo ) ) 
			setattr( proc, algo+'PFJetsCHSFilteredLinks', ak8PFJetsCHSFilteredLinks.clone( src = cms.InputTag( algo+"PFJetsCHS"), 
				matched = cms.InputTag( algo+'PFJetsCHSFiltered'), distMax = cms.double( coneSize ) ) )

			setattr( proc, 'cmsTopTagPFJetsCHS', cmsTopTagPFJetsCHS.clone( src = 'chs', rParam = coneSize, jetAlgorithm = jetAlgo ) )
			setattr( proc, 'cmsTopTagPFJetsCHSLinks'+ALGO, ak8PFJetsCHSPrunedLinks.clone( src = cms.InputTag( algo+"PFJetsCHS"), 
				matched = cms.InputTag("cmsTopTagPFJetsCHS"), distMax = cms.double( coneSize ) ) )

			elemToKeep += [ 'keep *_'+algo+'PFJetsCHSPrunedLinks_*_*', 
					'keep *_'+algo+'PFJetsCHSTrimmedLinks_*_*', 
					'keep *_'+algo+'PFJetsCHSFilteredLinks_*_*', 
					'keep *_cmsTopTagPFJetsCHSLinks'+ALGO+'_*_*'
					]
			getattr( proc, 'patJets'+ALGO+'PFCHS').userData.userFloats.src += [ algo+'PFJetsCHSPrunedLinks',
											    algo+'PFJetsCHSTrimmedLinks',
											    algo+'PFJetsCHSFilteredLinks',
											    'cmsTopTagPFJetsCHSLinks'+ALGO]

			if 'CA' in ALGO:

				###### MassDrop
				setattr( proc, algo+'PFJetsCHSMassDropFiltered', ca15PFJetsCHSMassDropFiltered.clone( src = 'chs', rParam = coneSize ) )
				setattr( proc, algo+'PFJetsCHSMassDropFilteredLinks', ak8PFJetsCHSPrunedLinks.clone( src = cms.InputTag( algo+"PFJetsCHS"), 
					matched = cms.InputTag(algo+'PFJetsCHSMassDropFiltered'), distMax = cms.double( coneSize ) ) )
				elemToKeep += [ 'keep *_'+algo+'PFJetsCHSMassDropFilteredLinks_*_*' ]
				getattr( proc, 'patJets'+ALGO+'PFCHS').userData.userFloats.src += [ algo+'PFJetsCHSMassDropFilteredLinks' ]

				###### hepTopTagger
				if( coneSize > 1 ): 
					setattr( proc, 'hepTopTagPFJetsCHS', hepTopTagPFJetsCHS.clone( src = 'chs' ) )
					setattr( proc, 'hepTopTagPFJetsCHSLinks'+ALGO, ak8PFJetsCHSPrunedLinks.clone( src = cms.InputTag( algo+"PFJetsCHS"), 
						matched = cms.InputTag("hepTopTagPFJetsCHS"), distMax = cms.double( coneSize ) ) )
					elemToKeep += [ 'keep *_hepTopTagPFJetsCHSLinks'+ALGO+'_*_*' ]
					getattr( proc, 'patJets'+ALGO+'PFCHS').userData.userFloats.src += [ 'hepTopTagPFJetsCHSLinks'+ALGO ]

		####### Nsubjettiness
		if addNsub:
			from RecoJets.JetProducers.nJettinessAdder_cfi import Njettiness

			if addNsubUpTo5: setattr( proc, 'Njettiness'+ALGO, Njettiness.clone( src = cms.InputTag( algo+'PFJetsCHS'), cone = cms.double( coneSize ), Njets = cms.vuint32(1,2,3,4,5) ) )
			else: setattr( proc, 'Njettiness'+ALGO, Njettiness.clone( src = cms.InputTag( algo+'PFJetsCHS'), cone = cms.double( coneSize ) ) )
			elemToKeep += [ 'keep *_Njettiness'+ALGO+'_*_*' ]
			getattr( proc, 'patJets'+ALGO+'PFCHS').userData.userFloats.src += ['Njettiness'+ALGO+':tau1','Njettiness'+ALGO+':tau2','Njettiness'+ALGO+':tau3']  
			if addNsubUpTo5: getattr( proc, 'patJets'+ALGO+'PFCHS').userData.userFloats.src += [ 'Njettiness'+ALGO+':tau4','Njettiness'+ALGO+':tau5' ]

		###### QJetsAdder
		if addQJets:
			### there must be a better way to do this random number introduction
			setattr( proc, 'RandomNumberGeneratorService', cms.Service("RandomNumberGeneratorService", 
								QJetsAdderCA8 = cms.PSet(initialSeed = cms.untracked.uint32(7)),
								QJetsAdderAK8 = cms.PSet(initialSeed = cms.untracked.uint32(31)),
								QJetsAdderCA15 = cms.PSet(initialSeed = cms.untracked.uint32(76)), ) )

			from RecoJets.JetProducers.qjetsadder_cfi import QJetsAdder
			setattr( proc, 'QJetsAdder'+ALGO, QJetsAdder.clone( src = cms.InputTag(algo+'PFJetsCHS'), jetRad = cms.double( coneSize ), jetAlgo = cms.string( ALGO[0:2] )))
			elemToKeep += [ 'keep *_QJetsAdder'+ALGO+'_*_*' ]
			getattr( proc, 'patJets'+ALGO+'PFCHS').userData.userFloats.src += ['QJetsAdder'+ALGO+':QjetsVolatility']  


	### "return"
	if hasattr(proc, 'out'): getattr(proc, 'out').outputCommands += elemToKeep
	else: setattr( proc, 'out', cms.OutputModule('PoolOutputModule', 
							fileName = cms.untracked.string('newJettoolbox.root'), 
							outputCommands = cms.untracked.vstring( elemToKeep ) ) )

