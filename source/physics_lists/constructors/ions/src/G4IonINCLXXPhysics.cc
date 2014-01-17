//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: G4IonINCLXXPhysics.cc 71042 2013-06-10 09:28:44Z gcosmo $
//
//---------------------------------------------------------------------------
//
// ClassName:   G4IonINCLXXPhysics
//
// Author:      D. Mancusi
//
// Modified:
//
//----------------------------------------------------------------------------
//

#include "G4IonINCLXXPhysics.hh"

#include "G4SystemOfUnits.hh"
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"
#include "G4Deuteron.hh"
#include "G4Triton.hh"
#include "G4He3.hh"
#include "G4Alpha.hh"
#include "G4GenericIon.hh"
#include "G4IonConstructor.hh"

#include "G4HadronInelasticProcess.hh"
#include "G4ComponentGGNuclNuclXsc.hh"
#include "G4CrossSectionInelastic.hh"

#include "G4INCLXXInterface.hh"
#include "G4PreCompoundModel.hh"
#include "G4ExcitationHandler.hh"
#include "G4FTFBuilder.hh"
#include "G4HadronicInteraction.hh"
#include "G4BuilderType.hh"

// factory
#include "G4PhysicsConstructorFactory.hh"
//
G4_DECLARE_PHYSCONSTR_FACTORY(G4IonINCLXXPhysics);

G4ThreadLocal std::vector<G4HadronInelasticProcess*>* G4IonINCLXXPhysics::p_list = 0;
G4ThreadLocal std::vector<G4HadronicInteraction*>* G4IonINCLXXPhysics::model_list = 0;
G4ThreadLocal G4VCrossSectionDataSet* G4IonINCLXXPhysics::theNuclNuclData = 0; 
G4ThreadLocal G4VComponentCrossSection* G4IonINCLXXPhysics::theGGNuclNuclXS = 0;
G4ThreadLocal G4INCLXXInterface* G4IonINCLXXPhysics::theINCLXXIons = 0;
G4ThreadLocal G4HadronicInteraction* G4IonINCLXXPhysics::theFTFP = 0;
G4ThreadLocal G4FTFBuilder* G4IonINCLXXPhysics::theBuilder = 0;
G4ThreadLocal G4bool G4IonINCLXXPhysics::wasActivated = false;

G4IonINCLXXPhysics::G4IonINCLXXPhysics(G4int ver) :
  G4VPhysicsConstructor("IonINCLXX"),
  verbose(ver)
{
  // INCLXX light ion maximum energy is 3.0 GeV/nucleon
  emax_d     = 2 * 3.0 * GeV;
  emax_t     = 3 * 3.0 * GeV;
  emax_he3   = 3 * 3.0 * GeV;
  emax_alpha = 4 * 3.0 * GeV;
  emax       = 16 * 3.0 * GeV;
  emaxFTFP   = 1.*TeV;
  emin       = 0.*MeV;
  SetPhysicsType(bIons);
  if(verbose > 1) G4cout << "### G4IonINCLXXPhysics" << G4endl;
}

G4IonINCLXXPhysics::G4IonINCLXXPhysics(const G4String& name, 
						     G4int ver)
  :  G4VPhysicsConstructor(name),
  verbose(ver)
{
  // INCLXX light ion maximum energy is 3.0 GeV/nucleon
  emax_d     = 2 * 3.0 * GeV;
  emax_t     = 3 * 3.0 * GeV;
  emax_he3   = 3 * 3.0 * GeV;
  emax_alpha = 4 * 3.0 * GeV;
  emax       = 16 * 3.0 * GeV;
  emaxFTFP   = 1.*TeV;
  emin       = 0.*MeV;
  SetPhysicsType(bIons);
  if(verbose > 1) G4cout << "### G4IonINCLXXPhysics" << G4endl;
}

G4IonINCLXXPhysics::~G4IonINCLXXPhysics()
{
  //For MT need to explicitly set back pointers to zero:
  //variables are static and if new threads are created we can have problems
  //since variable is still pointing old value
  if(wasActivated) {
    delete theBuilder; theBuilder=0;
    delete theGGNuclNuclXS; theGGNuclNuclXS=0;
    delete theNuclNuclData; theGGNuclNuclXS=0;
    G4int i;
    if ( p_list ) {
      G4int n = p_list->size();
      for(i=0; i<n; i++) {delete (*p_list)[i];}
      delete p_list; p_list = 0;
    }
    if ( model_list) { 
      G4int n = model_list->size();
      for(i=0; i<n; i++) { delete (*model_list)[i];}
      delete model_list; model_list = 0;
    }
  }
}

void G4IonINCLXXPhysics::ConstructProcess()
{
  if(wasActivated) return;
  wasActivated = true;

  theINCLXXIons= new G4INCLXXInterface();
  if ( model_list == 0 ) model_list = new std::vector<G4HadronicInteraction*>;
  model_list->push_back(theINCLXXIons);

  G4ExcitationHandler* handler = new G4ExcitationHandler();
  G4PreCompoundModel* thePreCompound = new G4PreCompoundModel(handler);

  theBuilder = new G4FTFBuilder("FTFP",thePreCompound);
  theFTFP = theBuilder->GetModel();
  model_list->push_back(theFTFP);

  theNuclNuclData = new G4CrossSectionInelastic( theGGNuclNuclXS = new G4ComponentGGNuclNuclXsc() );

  AddProcess("dInelastic", G4Deuteron::Deuteron(), theINCLXXIons, theFTFP, emax_d);
  AddProcess("tInelastic", G4Triton::Triton(), theINCLXXIons, theFTFP, emax_t);
  AddProcess("He3Inelastic", G4He3::He3(), theINCLXXIons, theFTFP, emax_he3);
  AddProcess("alphaInelastic", G4Alpha::Alpha(), theINCLXXIons, theFTFP, emax_alpha);
  AddProcess("ionInelastic", G4GenericIon::GenericIon(), theINCLXXIons, theFTFP, emax);
}

void G4IonINCLXXPhysics::AddProcess(const G4String& name,
				    G4ParticleDefinition* p, 
				    G4HadronicInteraction* hmodel,
				    G4HadronicInteraction* lmodel,
				    const G4double inclxxEnergyUpperLimit = 3.0 * GeV)
{
  G4HadronInelasticProcess* hadi = new G4HadronInelasticProcess(name, p);
  if ( p_list == 0 ) p_list = new  std::vector<G4HadronInelasticProcess*>;
  p_list->push_back(hadi);
  G4ProcessManager* pManager = p->GetProcessManager();
  pManager->AddDiscreteProcess(hadi);
  hadi->AddDataSet(theNuclNuclData);    
  hmodel->SetMinEnergy(emin);
  hmodel->SetMaxEnergy(inclxxEnergyUpperLimit);
  hadi->RegisterMe(hmodel);
  if(lmodel) {
    lmodel->SetMinEnergy(inclxxEnergyUpperLimit - MeV);
    lmodel->SetMaxEnergy(emaxFTFP);
    hadi->RegisterMe(lmodel);
  }
  if(verbose > 1) {
    G4cout << "Register " << hadi->GetProcessName()
	   << " for " << p->GetParticleName()
	   << " INCLXX/G4DeexcitationHandler for E(MeV)= " << emin << " - " << inclxxEnergyUpperLimit;
    if(lmodel) {
      G4cout  << " FTFP for E(MeV)= " << inclxxEnergyUpperLimit-MeV << " - " << emaxFTFP;
    }
    G4cout << G4endl;
  }
}

void G4IonINCLXXPhysics::ConstructParticle()
{
  //  Construct light ions
  G4IonConstructor pConstructor;
  pConstructor.ConstructParticle();  
}