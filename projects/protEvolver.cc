//*******************************************************************************************************
//*******************************************************************************************************
//***********************************                          ******************************************
//***********************************      protEvolver 4       ******************************************
//***********************************                          ******************************************
//*******************************************************************************************************
//*********  -Sequence Selective Evolutionary Algorithm of a Fold in Implicit Solvent -  ****************
//*******************************************************************************************************

/////// Just specify infile structure, active chains and residues indexes, and it will evolve a fold

//--Included files and functions-------------------------------------------------------------------------
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "PDBInterface.h"

vector <UInt> getChainSequence(protein* _prot, UInt _chainIndex);
vector <UInt> getMutationPosition(UIntVec &_activeChains, UIntVec &_activeResidues);
UInt getProbabilisticMutation(vector < vector < UInt > > &_sequencePool, vector < vector < UInt > > &_possibleMutants, UIntVec &_mutantPosition);
void createPossibleMutantsDatabase(protein* &_prot, UIntVec &_activeChains, UIntVec &_activeResidues, UIntVec &_allowedTypes, bool polarityAssignment);
UInt convertAAStringtoInt(string AA, string aminoAcidString[], UInt size);
vector < vector < UInt > > readSequencePool();
vector < vector < UInt > > readPossibleMutants();

enum aminoAcid {A,R,N,D,Dh,C,Cx,Cf,Q,E,Eh,Hd,He,Hp,I,L,K,M,F,P,O,S,T,W,Y,V,G,dA,dR,dN,dD,dDh,dC,dCx,dCf,dQ,dE,dEh,dHd,dHe,dHp,dI,dL,dK,dM,dF,dP,dO,dS,dT,dW,dY,dV,SF4,HEM,NI2,CLN,CO2,MG2,OH,OXY,CLD,HIS};
string aminoAcidString[] = {"A","R","N","D","D","C","C","C","Q","E","E","H","H","H","I","L","K","M","F","P","O","S","T","W","Y","V","G","A","R","N","D","D","C","C","C","Q","E","E","H","H","H","I","L","K","M","F","P","O","S","T","W","Y","V","A","R","N","D","D","C","C","C","Q","E","E","H","H","H","I","L","K","M","F","P","O","S","T","W","Y","V","G","A","R","N","D","D","C","C","C","Q","E","E","H","H","H","I","L","K","M","F","P","O","S","T","W","Y","V","A","R","N","D","D","C","C","C","Q","E","E","H","H","H","I","L","K","M","F","P","O","S","T","W","Y","V","G","A","R","N","D","D","C","C","C","Q","E","E","H","H","H","I","L","K","M","F","P","O","S","T","W","Y","V"};
UInt aaSize = sizeof(aminoAcidString)/sizeof(aminoAcidString[0]);
UInt populationBaseline = 1000;

//--Program setup----------------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
	//--Running parameters
	if (argc !=2)
	{
		cout << "Error: Required input file not given, should be run with input file." << endl;
		cout << "Command: protEvolver <inputfile>" << endl;
		cout << "Input file format as listed below:" << endl;
		cout << "Input PDB File,xyz.pdb," << endl;
		cout << "Active Chains,0,1,2," << endl;
		cout << "Active Positions,0,1,2,3,5,6,7,9,10," << endl;
		cout << "Random Positions,0,2,5,6,10," << endl;
		cout << "Frozen Positions,4,8," << endl;
		cout << "Amino Acids,A,R,N,D,C,Q,E,He,I,L,K,M,F,P,S,T,W,Y,V,G," << endl;
		cout << "Backbone Relaxation,false," << endl;
		cout << "Polarity Assignment,true," << endl;
		exit(1);
	}
	
	//--read input file
	UIntVec activeChains, allowedTypes, activeResidues, randomResidues, frozenResidues;
	bool backboneRelaxation, polarityAssignment;
	string inputfile = argv[1];
	string infile;
	ifstream file(inputfile);
	if (file){
		string item, line;
		UInt delimitercounter, linecounter = 0;
		while(getline(file,line))
		{
			delimitercounter = 0;
			stringstream stream(line);
			while(getline(stream,item,','))
			{
				if (delimitercounter > 0){
					if (linecounter == 0){
						infile = item;
					}
					if (linecounter == 1){
						stringstream inputString(item);
						UInt index;
						inputString >> index;
						activeChains.push_back(index);
					}
					if (linecounter == 2){
						stringstream inputString(item);
						UInt index;
						inputString >> index;
						activeResidues.push_back(index);
					}
					if (linecounter == 3){
						stringstream inputString(item);
						UInt index;
						inputString >> index;
						randomResidues.push_back(index);
					}
					if (linecounter == 4){
						stringstream inputString(item);
						UInt index;
						inputString >> index;
						frozenResidues.push_back(index);
					}
					if (linecounter == 5){
						UInt index = convertAAStringtoInt(item, aminoAcidString, aaSize);
						allowedTypes.push_back(index);
					}
					if (linecounter == 6){
						if (item.compare("false") == 0){
							backboneRelaxation = false;
						}
						else{backboneRelaxation = true;}
					}
					if (linecounter == 7){
						if (item.compare("false") == 0){
							polarityAssignment = false;
						}
						else{polarityAssignment = true;}
					}
				}
				delimitercounter++;
			}
			linecounter++;
		}
		file.close();
	}
	else{
		fstream inf;
		inf.open ("evolver.in", fstream::in | fstream::out | fstream::app);
		inf << "Input PDB File,xyz.pdb," << endl;
		inf << "Active Chains,0,1,2," << endl;
		inf << "Active Positions,0,1,2,3,5,6,7,9,10," << endl;
		inf << "Random Positions,0,2,5,6,10," << endl;
		inf << "Frozen Positions,4,8," << endl;
		inf << "Amino Acids,A,R,N,D,C,Q,E,He,I,L,K,M,F,P,S,T,W,Y,V,G," << endl;
		inf << "Backbone Relaxation,false," << endl;
		inf << "Polarity Assignment,true," << endl;
		cout << "Error: Required input file doesn't exist." << endl << "Template input file has been generated, please fill it out and rerun." << endl;
		exit(1);
	}
	
	//--Energy parameters
	residue::setElectroSolvationScaleFactor(1.0);
	residue::setHydroSolvationScaleFactor(1.0);
	residue::setPolarizableElec(true);
	amberElec::setScaleFactor(1.0);
	amberVDW::setScaleFactor(1.0);
	residue::setTemperature(300);

	//--set initial variables
	random_device rd; srand((int)rd());
	double startEnergy = 1E10, pastEnergy, Energy, deltaEnergy;
	UInt timeid, sec, mutant = 0, plateau = 15, nobetter = 0;
	vector < UInt > mutantPosition, chainSequence, randomPosition;
	vector < vector < UInt > > sequencePool, finalSequence, possibleMutants;
	stringstream convert; string startstr, outFile;
	UInt name = rand() % 100000000;
	convert << name, startstr = convert.str();
	string tempModel = startstr + "_temp.pdb";
	bool acceptance;

	//load pdb
	PDBInterface* thePDB = new PDBInterface(infile);
	ensemble* theEnsemble = thePDB->getEnsemblePointer();
	molecule* pMol = theEnsemble->getMoleculePointer(0);
	protein* prot = static_cast<protein*>(pMol);

	// set defaults if input file parameters are empty
	if (activeChains.size() < 1) 
	{
		for (UInt i = 0; i < prot->getNumChains(); i++)
		{
			UInt resN = prot->getNumResidues(i);
			if (resN < 2 && prot->isCofactor(i,0)){continue;}
			else{activeChains.push_back(i);}
		}
	}
	if (activeResidues.size() < 1) 
	{
		bool done = false;
		for (UInt i = 0; i < prot->getNumChains(); i++)
		{
			if (!done){
				UInt resN = prot->getNumResidues(i);
				if (resN < 2 && prot->isCofactor(i,0)){continue;}
				else{
					for (UInt j = 0; j < resN; j++)
					{
						activeResidues.push_back(j);
					}
					done = true;
				}
			}
		}
	}
	if (allowedTypes.size() < 1){
		allowedTypes.push_back(0); //A
		allowedTypes.push_back(1); //R
		allowedTypes.push_back(2); //N
		allowedTypes.push_back(3); //D
		allowedTypes.push_back(4); //C
		allowedTypes.push_back(8); //Q
		allowedTypes.push_back(9); //E
		allowedTypes.push_back(12);//H
		allowedTypes.push_back(14);//I
		allowedTypes.push_back(15);//L
		allowedTypes.push_back(16);//K
		allowedTypes.push_back(17);//M
		allowedTypes.push_back(18);//F
		allowedTypes.push_back(19);//P
		allowedTypes.push_back(21);//S
		allowedTypes.push_back(22);//T
		allowedTypes.push_back(23);//W
		allowedTypes.push_back(24);//Y
		allowedTypes.push_back(25);//V
		allowedTypes.push_back(26);//G
	}

	//-build possible sequence database per position
	possibleMutants = readPossibleMutants();
	if(possibleMutants.size() < activeResidues.size())
	{
		createPossibleMutantsDatabase(prot, activeChains, activeResidues, allowedTypes, polarityAssignment);
		possibleMutants = readPossibleMutants();
	}
	delete thePDB;
	
	//--Run multiple independent evolution cycles-----------------------------------------------------
	while(true)
	{
		thePDB = new PDBInterface(infile);
		theEnsemble = thePDB->getEnsemblePointer();
		pMol = theEnsemble->getMoleculePointer(0);
		prot = static_cast<protein*>(pMol);
		sequencePool = readSequencePool();

		//--load in initial pdb and mutate in random starting structure on active chains and random residues
		nobetter = 0;
		for (UInt i = 0; i < activeChains.size(); i++)
		{
			for (UInt j = 0; j < randomResidues.size(); j++)
			{
				randomPosition.push_back(activeChains[i]);
				randomPosition.push_back(randomResidues[j]);
				mutant = getProbabilisticMutation(sequencePool, possibleMutants, randomPosition);
				prot->mutateWBC(activeChains[i], randomResidues[j], mutant);
				randomPosition.clear();
			}
		}
		prot->protRelax(frozenResidues, activeChains);
		pdbWriter(prot, tempModel);
		Energy = prot->protEnergy();
		pastEnergy = Energy;

		//--Run through a single evolutionary path (ancestral line) till hitting plateau
		do
		{
			//--Make new mutation
			nobetter++;
			mutantPosition.clear();
			mutantPosition = getMutationPosition(activeChains, activeResidues);
			mutant = getProbabilisticMutation(sequencePool, possibleMutants, mutantPosition);
			prot->mutateWBC(mutantPosition[0], mutantPosition[1], mutant);

			//--Energy test
			prot->protRelax(frozenResidues, activeChains);
			Energy = prot->protEnergy();
			deltaEnergy = Energy-pastEnergy;
			acceptance = prot->boltzmannEnergyCriteria(deltaEnergy);
			if (acceptance){
				prot->saveCurrentState();
				pdbWriter(prot, tempModel);
				pastEnergy = Energy; 
				if (deltaEnergy < (residue::getKT()*-1)){nobetter = 0;}
			}
			
			//--Revert to previous state
			else{prot->undoState();}
			
		}while (nobetter < plateau);
		delete thePDB;

		//--Print final energy and write a pdb file----------------------------------------------------
		thePDB = new PDBInterface(tempModel);
		theEnsemble = thePDB->getEnsemblePointer();
		pMol = theEnsemble->getMoleculePointer(0);
		prot = static_cast<protein*>(pMol);
		
		//-Determine probability of being accepted into pool
		prot->protMin(backboneRelaxation, frozenResidues, activeChains);
		Energy = prot->protEnergy();
		deltaEnergy = Energy-startEnergy;
		acceptance = prot->boltzmannEnergyCriteria(deltaEnergy);
		startEnergy = Energy;
	
		// calculate binding energy
		double receptorE = prot->protEnergy(0);
		double ligandE = prot->protEnergy(1);
		double bindingEnergy = Energy-(ligandE+receptorE);
		
		//-generate pdb output
		sec = time(NULL); timeid = sec;
		stringstream convert; string countstr;
		convert << timeid, countstr = convert.str();
		outFile = countstr + "." + startstr + ".evo.pdb";
		pdbWriter(prot, outFile);
		
		//-write to data files
		finalSequence.clear(), chainSequence.clear();
		for (UInt i = 0; i < activeChains.size(); i++)
		{
			chainSequence = getChainSequence(prot, activeChains[i]);
			finalSequence.push_back(chainSequence);
		}
		fstream finalline; finalline.open ("results.out", fstream::in | fstream::out | fstream::app);
		finalline << timeid << " " << Energy << " " << bindingEnergy << endl;
		fstream fs; fs.open ("sequencepool.out", fstream::in | fstream::out | fstream::app);
		fstream population; population.open ("population.faa", fstream::in | fstream::out | fstream::app);
		if (acceptance){population << ">" << outFile << endl;}
		for (UInt i = 0; i < activeChains.size(); i++)
		{
			for (UInt j = 0; j < finalSequence[i].size(); j++)
			{
				if (acceptance){
					fs << finalSequence[i][j] << ",";
					population << aminoAcidString[finalSequence[i][j]];
				}
			}
		}
		if (acceptance){
			fs << endl;
			population << endl;
		}
		fs.close(); finalline.close(); population.close();
		
		//-clear variables for next iteration
		delete thePDB;
		sequencePool.clear(), mutantPosition.clear(), chainSequence.clear(), randomPosition.clear();
		sequencePool.resize(0),  mutantPosition.resize(0), chainSequence.resize(0), randomPosition.resize(0);
	}
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// functions //////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

vector < UInt > getChainSequence(protein* _prot, UInt _chainIndex)
{
	UInt type, numResidues;
	vector < UInt > sequence;
	numResidues = _prot->getNumResidues(_chainIndex);
	for (UInt j = 0; j < numResidues; j++)
	{
		type = _prot->getTypeFromResNum(_chainIndex, j);
		sequence.push_back(type);
	}
	return sequence;
}

vector <UInt> getMutationPosition(UIntVec &_activeChains, UIntVec &_activeResidues)
{
	UInt randres, randchain;
	vector <UInt> _mutantPosition;
	randchain = _activeChains[rand() % _activeChains.size()];
	randres = _activeResidues[rand() % _activeResidues.size()];
	_mutantPosition.push_back(randchain);
	_mutantPosition.push_back(randres);
	return _mutantPosition;
}

UInt getProbabilisticMutation(vector < vector < UInt > > &_sequencePool, vector < vector < UInt > > &_possibleMutants, UIntVec &_mutantPosition)
{
	double Pi, entropy, poolSize = _sequencePool.size();
	vector <UInt> Freqs(57,1);
	UInt mutant, type;
	UInt positionPossibles = _possibleMutants[(_mutantPosition[0]+1)*_mutantPosition[1]].size();

	//--determine frequency based chance of mutation acceptance (statistical potential)
	do
	{
		entropy = rand() % 100+1;
		mutant = _possibleMutants[(_mutantPosition[0]+1)*_mutantPosition[1]][rand() % positionPossibles];
		if (poolSize >= ::populationBaseline){
			for (UInt i = 0; i < poolSize; i++)
			{
				type = _sequencePool[i][(_mutantPosition[0]+1)*_mutantPosition[1]];
				Freqs[type] += 1;
			}
			Pi = (Freqs[mutant]/(poolSize-1))*100;
		}
		else{Pi = 100;}  //random mutant
	}while (entropy > Pi);
	return mutant;
}

vector < vector < UInt > > readSequencePool()
{
	ifstream file("sequencepool.out");
	string item, line;
	vector < UInt > sequence;
	vector < vector < UInt > > sequencePool;

	while(getline(file,line))
	{
		stringstream stream(line);
		while(getline(stream,item,','))
		{
			stringstream seqString(item);
			int seqIndex;
			seqString >> seqIndex;
			sequence.push_back(seqIndex);
		}
		sequencePool.push_back(sequence);
		sequence.clear();
	}
	file.close();
	if (sequencePool.size() > ::populationBaseline){
		sequencePool.erase(sequencePool.begin(),sequencePool.end()-::populationBaseline);
	}
	return sequencePool;
}

vector < vector < UInt > > readPossibleMutants()
{
	ifstream file("possiblemutants.out");
	string item, line;
	vector < UInt > _position;
	vector < vector < UInt > > _possibleMutants;
	while(getline(file,line))
	{
		stringstream stream(line);
		while(getline(stream,item,','))
		{
			stringstream seqString(item);
			int seqIndex;
			seqString >> seqIndex;
			_position.push_back(seqIndex);
		}
		_possibleMutants.push_back(_position);
		_position.clear();
	}
	file.close();
	return _possibleMutants;
}

void createPossibleMutantsDatabase(protein* &_prot, UIntVec &_activeChains, UIntVec &_activeResidues, UIntVec &_allowedTypes, bool polarityAssignment)
{
	double aveDielectric, sumDielectric=0.0;
	UInt counter = 0;
	if (polarityAssignment){
		_prot->updateDielectrics();
		for (UInt i = 0; i < _prot->getNumChains(); i++)
		{
			for (UInt j = 0; j < _prot->getNumResidues(i); j++)
			{
				sumDielectric += _prot->getDielectric(i,j);
				counter++;
			}
		}
		aveDielectric = sumDielectric/counter;
	}
	fstream pm; double phi = -100.0; bool active;
	pm.open ("possiblemutants.out", fstream::in | fstream::out | fstream::app);
	
	for (UInt i = 0; i < _activeChains.size(); i++)
	{
		for (UInt j = 0; j <_prot->getNumResidues(i); j++)
		{
			active = false;
			for (UInt k = 0; k < _activeResidues.size(); k++)
			{
				if(j == _activeResidues[k]){active = true;}
			}
			if (active){
				for (UInt l = 0; l <_allowedTypes.size(); l++)
				{
					if (polarityAssignment){
						double dielectric = _prot->getDielectric(_activeChains[i],j);
						if (dielectric < aveDielectric){
							if (_allowedTypes[l] != A && _allowedTypes[l] != I && _allowedTypes[l] != L && _allowedTypes[l] != M && _allowedTypes[l] != F && _allowedTypes[l] != W && _allowedTypes[l] != Y && _allowedTypes[l] != V && _allowedTypes[l] != G &&
								_allowedTypes[l] != dA && _allowedTypes[l] != dI && _allowedTypes[l] != dL && _allowedTypes[l] != dM && _allowedTypes[l] != dF && _allowedTypes[l] != dW && _allowedTypes[l] != dY && _allowedTypes[l] != dV){
									continue;
							}
						} 
						else{
							if (_allowedTypes[l] == A || _allowedTypes[l] == I || _allowedTypes[l] == L || _allowedTypes[l] == M || _allowedTypes[l] == F || _allowedTypes[l] == W || _allowedTypes[l] == Y || _allowedTypes[l] == V ||
								_allowedTypes[l] == dA || _allowedTypes[l] == dI || _allowedTypes[l] == dL || _allowedTypes[l] == dM || _allowedTypes[l] == dF || _allowedTypes[l] == dW || _allowedTypes[l] == dY || _allowedTypes[l] == dV){
									continue;
							}
						}
					}
					if(j > 0){
						phi = _prot->getPhi(_activeChains[i], j);
					}
					if ((phi < 0 && _allowedTypes[l] < dA) || (phi > 0 && _allowedTypes[l] > V)){
						pm << _allowedTypes[l] << ",";
					}
				}
			}
			else{pm << _prot->getTypeFromResNum(_activeChains[i],j) << ",";}
			pm << endl;
		}
	}
}

UInt convertAAStringtoInt(string AA, string aminoAcidString[], UInt size)
{
	UInt index;
	for (UInt i = 0; i < size; i++)
	{
		if (AA.compare(aminoAcidString[i]) == 0){index = i;}
	}
	return index;
}
