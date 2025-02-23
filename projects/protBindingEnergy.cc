//*******************************************************************************************************
//*******************************************************************************************************
//*****************************                         *************************************************
//*****************************    protBindingEnergy    *************************************************
//*****************************                         *************************************************
//*******************************************************************************************************
//*******************************************************************************************************

#include <iostream>
#include <string>
#include "ensemble.h"
#include "PDBInterface.h"

//--Program setup-------------------------------------------------------------
int main (int argc, char* argv[])
{	
	if (argc !=4)
	{
		cout << "protBindingEnergy <inFile.pdb> <receptor chain ID> <ligand chain ID>" << endl;
		exit(1);
	}
	string infile = argv[1];
	UInt ligandChainID, receptorChainID;
	sscanf(argv[2], "%d", &receptorChainID);
	sscanf(argv[3], "%d", &ligandChainID);
	PDBInterface* thePDB = new PDBInterface(infile);
	ensemble* theEnsemble = thePDB->getEnsemblePointer();
	molecule* pMol = theEnsemble->getMoleculePointer(0);
	protein* prot = static_cast<protein*>(pMol);

	residue::setElectroSolvationScaleFactor(1.0);
	residue::setHydroSolvationScaleFactor(1.0);
	residue::setPolarizableElec(true);
	amberElec::setScaleFactor(1.0);
	amberVDW::setScaleFactor(1.0);
	residue::setEntropyFactor(1.0);
	residue::setTemperature(293);

	double complexE = prot->protEnergy();
	double receptorE = prot->protEnergy(receptorChainID);
	residue::setEntropyFactor(0.0); //assuming free ligand if molecule, peptide or small unfolded protein, is not conformationally restrained, if protein this should be commented out
	double ligandE = prot->protEnergy(ligandChainID);
	
	// calculate binding energys
	double bindingEnergy = complexE-(ligandE+receptorE);
	cout << infile << " " << complexE << " " << bindingEnergy << endl;
	pdbWriter(prot,infile);
	return 0;
}
