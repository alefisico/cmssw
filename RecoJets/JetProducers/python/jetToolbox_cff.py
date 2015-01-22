import FWCore.ParameterSet.Config as cms

from PhysicsTools.PatAlgos.producersLayer1.patCandidates_cff import *
from RecoJets.Configuration.RecoPFJets_cff import *
from RecoJets.Configuration.RecoGenJets_cff import * 
from PhysicsTools.PatAlgos.tools.jetTools import addJetCollection
from PhysicsTools.PatAlgos.tools.jetTools import switchJetCollection

#process.load('PhysicsTools.PatAlgos.selectionLayer1.selectedPatCandidates_cff')


def jetToolbox( proc, jetType, jetSequence, outputFile, addSubstructure=True, addGroomers=True, addNsub=True, addNsubUpTo5=False, addQJets=True, addSubjets=False, minPt=100., miniAOD=False ):
	
	###############################################################################
	#######  Just defining simple variables
	###############################################################################
	supportedJetAlgos = { 'ak': 'AntiKt', 'ca' : 'CambridgeAachen', 'kt' : 'Kt' }
	jetAlgo = ''
	algorithm = ''
	size = ''
	for type, tmpAlgo in supportedJetAlgos.iteritems(): 
		if type in jetType.lower():
			jetAlgo = type
			algorithm = tmpAlgo
			size = jetType.replace( type, '' )
	if jetAlgo == '': print 'Unsupported jet algorithm. Please use something like: jetType = CA8'

	jetSize = 0.
	if int(size) in range(0, 20): jetSize = int(size)/10.
	else: print 'jetSize has not a valid value. Insert a number between 1 and 20 after algorithm, like: AK8'
	### Trick for uppercase/lowercase algo name
	jetALGO = jetAlgo.upper()+size
	jetalgo = jetAlgo.lower()+size
	if( int(size) > 10 ): size = '10' 	### For JEC for jets larger than 1 


	#################################################################################
	####### Toolbox start 
	#################################################################################

	elemToKeep = []
	jetSeq = cms.Sequence()

	#print proc
	#if hasattr( cms.InputTag(''), 'ak4PFJetsCHS' ):
	#if hasattr( proc, 'packedPFCandidates' ):

	#try:
	#### For AOD
	'''
	if not miniAOD:
		print 'YES'
		proc.load('RecoJets.Configuration.GenJetParticles_cff')
		proc.load('RecoJets.Configuration.RecoPFJets_cff')
		setattr( proc, jetalgo+'GenJets', ak4GenJets.clone( src = 'genParticlesForJetsNoNu', rParam = jetSize, jetAlgorithm = algorithm ) ) 
		jetSeq += getattr(proc, jetalgo+'GenJets' )

		addJetCollection(
				proc,
				labelName = jetALGO+'PFCHS',
				jetSource = cms.InputTag( jetalgo+'PFJetsCHS'),
				algo = jetalgo,
				rParam = jetSize,
				jetCorrections = ( 'AK'+size+'PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None'),
				outputModules = ['outputFile'],
				genJetCollection = cms.InputTag( jetalgo+'GenJets' ),
				) 
		elemToKeep += [ 'keep *_patJets'+jetALGO+'PFCHS_*_*' ]

	else:
	#setattr( proc, jetalgo+'PFJetsCHSPruned', ak8PFJetsCHSPruned.clone( src = 'patJets'+jetALGO+'PFCHS', rParam = jetSize, jetAlgorithm = algorithm ) )
	#setattr( proc, jetalgo+'PFJetsCHSPrunedLinks', ak8PFJetsCHSPrunedLinks.clone( src = cms.InputTag( jetalgo+"PFJetsCHS"), 
#		matched = cms.InputTag( jetalgo+'PFJetsCHSPruned'), distMax = cms.double( jetSize ) ) )
	elemToKeep += [ 'keep *_'+jetalgo+'PFJetsCHSPrunedLinks_*_*'] 
	jetSeq += getattr(proc, jetalgo+'PFJetsCHSPruned' )
	jetSeq += getattr(proc, jetalgo+'PFJetsCHSPrunedLinks' )
	getattr( proc, 'patJets'+jetALGO+'PFCHS').userData.userFloats.src += [ jetalgo+'PFJetsCHSPrunedLinks']
	'''
	setattr( proc, 'chs', cms.EDFilter('CandPtrSelector', src = cms.InputTag('packedPFCandidates'), cut = cms.string('fromPV')) )
	jetSeq += getattr(proc, 'chs')

	setattr( proc, jetalgo+'PFJetsCHS', ak4PFJetsCHS.clone( src = 'chs', doAreaFastjet = True, rParam = jetSize, jetAlgorithm = algorithm,  jetPtMin = minPt )) 
	jetSeq += getattr(proc, jetalgo+'PFJetsCHS' )
	elemToKeep += [ 'keep *_'+jetalgo+'PFJetsCHS_*_*' ]

	#ak4GenJets.src = 'packedGenParticles'
	setattr( proc, jetalgo+'GenJets', ak4GenJets.clone( src = 'packedGenParticles', rParam = jetSize, jetAlgorithm = algorithm ) ) 
	jetSeq += getattr(proc, jetalgo+'GenJets' )
	#fixedGridRhoFastjetAll.pfCandidatesTag = 'packedPFCandidates'

	####  Creating PATjets
	proc.load('PhysicsTools.PatAlgos.slimming.unpackedTracksAndVertices_cfi')

	addJetCollection(
			proc,
			labelName = jetALGO+'PFCHS',
			jetSource = cms.InputTag( jetalgo+'PFJetsCHS'),
			algo = jetalgo,
			rParam = jetSize,
			jetCorrections = ( 'AK'+size+'PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None'),
			trackSource = cms.InputTag('unpackedTracksAndVertices'),
			pvSource = cms.InputTag('unpackedTracksAndVertices'),
			btagDiscriminators = ['combinedSecondaryVertexBJetTags'],
			genJetCollection = cms.InputTag( jetalgo+'GenJets' ),
			getJetMCFlavour = False,
			outputModules = ['outputFile']
			) 

	getattr( proc, 'patJets'+jetALGO+'PFCHS' ).addJetCharge = False 
	getattr( proc, 'patJets'+jetALGO+'PFCHS' ).addBTagInfo  = False #True
	getattr( proc, 'patJets'+jetALGO+'PFCHS' ).addAssociatedTracks = False 
	getattr( proc, 'patJetPartonMatch'+jetALGO+'PFCHS' ).matched = 'prunedGenParticles' 
	getattr( proc, 'patJetCorrFactors'+jetALGO+'PFCHS' ).primaryVertices = 'offlineSlimmedPrimaryVertices' 
	elemToKeep += [ 'keep *_patJets'+jetALGO+'PFCHS_*_*' ]
	jetSeq += getattr(proc, 'patJetGenJetMatch'+jetALGO+'PFCHS' )
	jetSeq += getattr(proc, 'patJetPartonMatch'+jetALGO+'PFCHS' )
	jetSeq += getattr(proc, 'patJetCorrFactors'+jetALGO+'PFCHS' )

	#proc.load('RecoBTag.Configuration.RecoBTag_cff')
	#proc.load('RecoJets.Configuration.RecoJetAssociations_cff')
	from RecoJets.Configuration.RecoJetAssociations_cff import ak4JetTracksAssociatorAtVertexPF
	setattr( proc, jetalgo+'JetTracksAssociatorAtVertexPF', ak4JetTracksAssociatorAtVertexPF.clone( jets = cms.InputTag( jetalgo+'PFJetsCHS' ), tracks = cms.InputTag('unpackedTracksAndVertices'), coneSize = jetSize ) )  
	getattr( proc, 'impactParameterTagInfos' ).primaryVertex = cms.InputTag('unpackedTracksAndVertices')
	getattr( proc, 'inclusiveSecondaryVertexFinderTagInfos' ).extSVCollection = cms.InputTag('unpackedTracksAndVertices','secondary','')
	getattr( proc, 'combinedSecondaryVertex').trackMultiplicityMin = 1 #silly sv, uses un filtered tracks.. i.e. any pt


	if ( addSubstructure ):

		######### Prunning, Trimming, Filtering, cmsTopTagger
		if addGroomers:
			setattr( proc, jetalgo+'PFJetsCHSPruned', ak8PFJetsCHSPruned.clone( src = 'chs', rParam = jetSize, jetAlgorithm = algorithm ) )
			setattr( proc, jetalgo+'PFJetsCHSPrunedLinks', ak8PFJetsCHSPrunedLinks.clone( src = cms.InputTag( jetalgo+"PFJetsCHS"), 
				matched = cms.InputTag( jetalgo+'PFJetsCHSPruned'), distMax = cms.double( jetSize ) ) )

			setattr( proc, jetalgo+'PFJetsCHSTrimmed', ak8PFJetsCHSTrimmed.clone( src = 'chs', rParam = jetSize, jetAlgorithm = algorithm ) ) 
			setattr( proc, jetalgo+'PFJetsCHSTrimmedLinks', ak8PFJetsCHSTrimmedLinks.clone( src = cms.InputTag( jetalgo+"PFJetsCHS"), 
				matched = cms.InputTag( jetalgo+'PFJetsCHSTrimmed'), distMax = cms.double( jetSize ) ) )

			setattr( proc, jetalgo+'PFJetsCHSFiltered', ak8PFJetsCHSFiltered.clone( src = 'chs', rParam = jetSize, jetAlgorithm = algorithm ) ) 
			setattr( proc, jetalgo+'PFJetsCHSFilteredLinks', ak8PFJetsCHSFilteredLinks.clone( src = cms.InputTag( jetalgo+"PFJetsCHS"), 
				matched = cms.InputTag( jetalgo+'PFJetsCHSFiltered'), distMax = cms.double( jetSize ) ) )

			setattr( proc, 'cmsTopTagPFJetsCHS', cmsTopTagPFJetsCHS.clone( src = 'chs' ) ) #, rParam = jetSize ) )
			setattr( proc, 'cmsTopTagPFJetsCHSLinks'+jetALGO, ak8PFJetsCHSPrunedLinks.clone( src = cms.InputTag( jetalgo+"PFJetsCHS"), 
				matched = cms.InputTag("cmsTopTagPFJetsCHS"), distMax = cms.double( jetSize ) ) )

			elemToKeep += [ 'keep *_'+jetalgo+'PFJetsCHSPrunedLinks_*_*', 
					'keep *_'+jetalgo+'PFJetsCHSTrimmedLinks_*_*', 
					'keep *_'+jetalgo+'PFJetsCHSFilteredLinks_*_*', 
					'keep *_cmsTopTagPFJetsCHSLinks'+jetALGO+'_*_*'
					]
			jetSeq += getattr(proc, jetalgo+'PFJetsCHSPruned' )
			jetSeq += getattr(proc, jetalgo+'PFJetsCHSPrunedLinks' )
			jetSeq += getattr(proc, jetalgo+'PFJetsCHSTrimmed' )
			jetSeq += getattr(proc, jetalgo+'PFJetsCHSTrimmedLinks' )
			jetSeq += getattr(proc, jetalgo+'PFJetsCHSFiltered' )
			jetSeq += getattr(proc, jetalgo+'PFJetsCHSFilteredLinks' )
			jetSeq += getattr(proc, 'cmsTopTagPFJetsCHS' )
			jetSeq += getattr(proc, 'cmsTopTagPFJetsCHSLinks'+jetALGO )
			getattr( proc, 'patJets'+jetALGO+'PFCHS').userData.userFloats.src += [ jetalgo+'PFJetsCHSPrunedLinks',
											    jetalgo+'PFJetsCHSTrimmedLinks',
											    jetalgo+'PFJetsCHSFilteredLinks',
											    'cmsTopTagPFJetsCHSLinks'+jetALGO]

			if 'CA' in jetALGO:

				###### MassDrop
				setattr( proc, jetalgo+'PFJetsCHSMassDropFiltered', ca15PFJetsCHSMassDropFiltered.clone( src = 'chs', rParam = jetSize ) )
				setattr( proc, jetalgo+'PFJetsCHSMassDropFilteredLinks', ak8PFJetsCHSPrunedLinks.clone( src = cms.InputTag( jetalgo+"PFJetsCHS"), 
					matched = cms.InputTag(jetalgo+'PFJetsCHSMassDropFiltered'), distMax = cms.double( jetSize ) ) )
				elemToKeep += [ 'keep *_'+jetalgo+'PFJetsCHSMassDropFilteredLinks_*_*' ]
				getattr( proc, 'patJets'+jetALGO+'PFCHS').userData.userFloats.src += [ jetalgo+'PFJetsCHSMassDropFilteredLinks' ]
				jetSeq += getattr(proc, jetalgo+'PFJetsCHSMassDropFiltered' )
				jetSeq += getattr(proc, jetalgo+'PFJetsCHSMassDropFilteredLinks' )

				###### hepTopTagger
				if( jetSize > 1 ): 
					setattr( proc, 'hepTopTagPFJetsCHS', hepTopTagPFJetsCHS.clone( src = 'chs' ) )
					setattr( proc, 'hepTopTagPFJetsCHSLinks'+jetALGO, ak8PFJetsCHSPrunedLinks.clone( src = cms.InputTag( jetalgo+"PFJetsCHS"), 
						matched = cms.InputTag("hepTopTagPFJetsCHS"), distMax = cms.double( jetSize ) ) )
					elemToKeep += [ 'keep *_hepTopTagPFJetsCHSLinks'+jetALGO+'_*_*' ]
					getattr( proc, 'patJets'+jetALGO+'PFCHS').userData.userFloats.src += [ 'hepTopTagPFJetsCHSLinks'+jetALGO ]
					jetSeq += getattr(proc, 'hepTopTagPFJetsCHS' )
					jetSeq += getattr(proc, 'hepTopTagPFJetsCHSLinks'+jetALGO )

		####### Nsubjettiness
		if addNsub:
			from RecoJets.JetProducers.nJettinessAdder_cfi import Njettiness

			if addNsubUpTo5: setattr( proc, 'Njettiness'+jetALGO, Njettiness.clone( src = cms.InputTag( jetalgo+'PFJetsCHS'), cone = cms.double( jetSize ), Njets = cms.vuint32(1,2,3,4,5) ) )
			else: setattr( proc, 'Njettiness'+jetALGO, Njettiness.clone( src = cms.InputTag( jetalgo+'PFJetsCHS'), cone = cms.double( jetSize ) ) )
			elemToKeep += [ 'keep *_Njettiness'+jetALGO+'_*_*' ]
			getattr( proc, 'patJets'+jetALGO+'PFCHS').userData.userFloats.src += ['Njettiness'+jetALGO+':tau1','Njettiness'+jetALGO+':tau2','Njettiness'+jetALGO+':tau3']  
			if addNsubUpTo5: getattr( proc, 'patJets'+jetALGO+'PFCHS').userData.userFloats.src += [ 'Njettiness'+jetALGO+':tau4','Njettiness'+jetALGO+':tau5' ]
			jetSeq += getattr(proc, 'Njettiness'+jetALGO )

		###### QJetsAdder
		if addQJets:
			### there must be a better way to do this random number introduction
			setattr( proc, 'RandomNumberGeneratorService', cms.Service("RandomNumberGeneratorService", 
								QJetsAdderCA8 = cms.PSet(initialSeed = cms.untracked.uint32(7)),
								QJetsAdderAK8 = cms.PSet(initialSeed = cms.untracked.uint32(31)),
								QJetsAdderCA15 = cms.PSet(initialSeed = cms.untracked.uint32(76)), ) )

			from RecoJets.JetProducers.qjetsadder_cfi import QJetsAdder
			setattr( proc, 'QJetsAdder'+jetALGO, QJetsAdder.clone( src = cms.InputTag(jetalgo+'PFJetsCHS'), jetRad = cms.double( jetSize ), jetAlgo = cms.string( jetALGO[0:2] )))
			elemToKeep += [ 'keep *_QJetsAdder'+jetALGO+'_*_*' ]
			getattr( proc, 'patJets'+jetALGO+'PFCHS').userData.userFloats.src += ['QJetsAdder'+jetALGO+':QjetsVolatility']  
			jetSeq += getattr(proc, 'QJetsAdder'+jetALGO )

		####### Adding subjets
		if( addSubjets ): 
			jetSeq += getattr(proc, 'patJets'+jetALGO+'PFCHS' )
			setattr( proc, 'patJets'+jetALGO+'withSubjets', cms.EDProducer('addSubjetProducer', jets = cms.InputTag(jetalgo+'PFJetsCHS'), patjets = cms.InputTag('patJets'+jetALGO+'PFCHS') ) )
			elemToKeep += [ 'keep *_patJets'+jetALGO+'withSubjets_*_*' ]
			jetSeq += getattr(proc, 'patJets'+jetALGO+'withSubjets' )
		else: jetSeq += getattr(proc, 'patJets'+jetALGO+'PFCHS' )
	

	### "return"
	setattr(proc, jetSequence, jetSeq)
	if hasattr(proc, outputFile): getattr(proc, outputFile).outputCommands += elemToKeep
	else: setattr( proc, outputFile, cms.OutputModule('PoolOutputModule', 
							fileName = cms.untracked.string('jettoolbox.root'), 
							outputCommands = cms.untracked.vstring( elemToKeep ) ) )


