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
// $Id: G4EmParametersMessenger.cc 66241 2012-12-13 18:34:42Z gunter $
//
// -------------------------------------------------------------------
//
// GEANT4 Class file
//
// File name:     G4EmParametersMessenger
//
// Author:        Vladimir Ivanchenko created from G4EnergyLossMessenger
//
// Creation date: 22-05-2013 
//
// Modifications:
//
// -------------------------------------------------------------------
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#include "G4EmParametersMessenger.hh"

#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4UIparameter.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UImanager.hh"
#include "G4MscStepLimitType.hh"
#include "G4EmParameters.hh"

#include <sstream>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4EmParametersMessenger::G4EmParametersMessenger(G4EmParameters* ptr) 
  : theParameters(ptr)
{
  eLossDirectory = new G4UIdirectory("/process/eLoss/");
  eLossDirectory->SetGuidance("Commands for EM processes.");
  mscDirectory = new G4UIdirectory("/process/msc/");
  mscDirectory->SetGuidance("Commands for EM scattering processes.");
  emDirectory = new G4UIdirectory("/process/em/");
  emDirectory->SetGuidance("General commands for EM processes.");

  flucCmd = new G4UIcmdWithABool("/process/eLoss/fluct",this);
  flucCmd->SetGuidance("Enable/disable energy loss fluctuations.");
  flucCmd->SetParameterName("choice",true);
  flucCmd->SetDefaultValue(true);
  flucCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  rangeCmd = new G4UIcmdWithABool("/process/eLoss/CSDARange",this);
  rangeCmd->SetGuidance("Enable/disable CSDA range calculation");
  rangeCmd->SetParameterName("range",true);
  rangeCmd->SetDefaultValue(false);
  rangeCmd->AvailableForStates(G4State_PreInit);

  lpmCmd = new G4UIcmdWithABool("/process/eLoss/LPM",this);
  lpmCmd->SetGuidance("Enable/disable LPM effect calculation");
  lpmCmd->SetParameterName("lpm",true);
  lpmCmd->SetDefaultValue(true);
  lpmCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  splCmd = new G4UIcmdWithABool("/process/em/spline",this);
  splCmd->SetGuidance("Enable/disable usage spline for Physics Vectors");
  splCmd->SetParameterName("spl",true);
  splCmd->SetDefaultValue(false);
  splCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  rsCmd = new G4UIcmdWithABool("/process/eLoss/useCutAsFinalRange",this);
  rsCmd->SetGuidance("Enable?disable use of cut in range as a final range");
  rsCmd->SetParameterName("choice",true);
  rsCmd->SetDefaultValue(false);
  rsCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  aplCmd = new G4UIcmdWithABool("/process/em/applyCuts",this);
  aplCmd->SetGuidance("Enable/disable applying cuts for gamma processes");
  aplCmd->SetParameterName("apl",true);
  aplCmd->SetDefaultValue(false);
  aplCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  deCmd = new G4UIcmdWithABool("/process/em/fluo",this);
  deCmd->SetGuidance("Enable/disable atomic deexcitation");
  deCmd->SetParameterName("fluoFlag",true);
  deCmd->SetDefaultValue(false);
  deCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  dirFluoCmd = new G4UIcmdWithABool("/process/em/fluoBearden",this);
  dirFluoCmd->SetGuidance("Enable/disable usage of Bearden fluorescence files");
  dirFluoCmd->SetParameterName("fluoBeardenFlag",true);
  dirFluoCmd->SetDefaultValue(false);
  dirFluoCmd->AvailableForStates(G4State_PreInit);

  auCmd = new G4UIcmdWithABool("/process/em/auger",this);
  auCmd->SetGuidance("Enable/disable Auger electrons production");
  auCmd->SetParameterName("augerFlag",true);
  auCmd->SetDefaultValue(false);
  auCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  auCascadeCmd = new G4UIcmdWithABool("/process/em/augerCascade",this);
  auCascadeCmd->SetGuidance("Enable/disable simulation of cascade of Auger electrons");
  auCascadeCmd->SetParameterName("augerCascadeFlag",true);
  auCascadeCmd->SetDefaultValue(false);
  auCascadeCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  pixeCmd = new G4UIcmdWithABool("/process/em/pixe",this);
  pixeCmd->SetGuidance("Enable/disable PIXE simulation");
  pixeCmd->SetParameterName("pixeFlag",true);
  pixeCmd->SetDefaultValue(false);
  pixeCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  dcutCmd = new G4UIcmdWithABool("/process/em/deexcitationIgnoreCut",this);
  dcutCmd->SetGuidance("Enable/Disable usage of cuts in de-excitation module");
  dcutCmd->SetParameterName("deexcut",true);
  dcutCmd->SetDefaultValue(false);
  dcutCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  latCmd = new G4UIcmdWithABool("/process/msc/LateralDisplacement",this);
  latCmd->SetGuidance("Enable/disable sampling of lateral displacement");
  latCmd->SetParameterName("lat",true);
  latCmd->SetDefaultValue(true);
  latCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  mulatCmd = new G4UIcmdWithABool("/process/msc/MuHadLateralDisplacement",this);
  mulatCmd->SetGuidance("Enable/disable sampling of lateral displacement for muons and hadrons");
  mulatCmd->SetParameterName("mulat",true);
  mulatCmd->SetDefaultValue(true);
  mulatCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  catCmd = new G4UIcmdWithABool("/process/msc/DisplacementBeyondSafety",this);
  catCmd->SetGuidance("Enable/disable displacement at geometry boundary");
  catCmd->SetParameterName("cat",true);
  catCmd->SetDefaultValue(false);
  catCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  delCmd = new G4UIcmdWithABool("/process/eLoss/UseAngularGenerator",this);
  delCmd->SetGuidance("Enable usage of angular generator");
  delCmd->SetParameterName("del",true);
  delCmd->SetDefaultValue(false);
  delCmd->AvailableForStates(G4State_PreInit);

  mottCmd = new G4UIcmdWithABool("/process/msc/UseMottCorrection",this);
  mottCmd->SetGuidance("Enable usage of Mott corrections for e- elastic scattering");
  mottCmd->SetParameterName("mott",true);
  mottCmd->SetDefaultValue(false);
  mottCmd->AvailableForStates(G4State_PreInit);

  minSubSecCmd = new G4UIcmdWithADouble("/process/eLoss/minsubsec",this);
  minSubSecCmd->SetGuidance("Set the ratio subcut/cut ");
  minSubSecCmd->SetParameterName("rcmin",true);
  minSubSecCmd->AvailableForStates(G4State_PreInit);

  minEnCmd = new G4UIcmdWithADoubleAndUnit("/process/eLoss/minKinEnergy",this);
  minEnCmd->SetGuidance("Set the min kinetic energy for EM tables");
  minEnCmd->SetParameterName("emin",true);
  minEnCmd->SetUnitCategory("Energy");
  minEnCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  maxEnCmd = new G4UIcmdWithADoubleAndUnit("/process/eLoss/maxKinEnergy",this);
  maxEnCmd->SetGuidance("Set the max kinetic energy for EM tables");
  maxEnCmd->SetParameterName("emax",true);
  maxEnCmd->SetUnitCategory("Energy");
  maxEnCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  cenCmd = new G4UIcmdWithADoubleAndUnit("/process/eLoss/maxKinEnergyCSDA",this);
  cenCmd->SetGuidance("Set the max kinetic energy for CSDA table");
  cenCmd->SetParameterName("emaxCSDA",true);
  cenCmd->SetUnitCategory("Energy");
  cenCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  lowEnCmd = new G4UIcmdWithADoubleAndUnit("/process/em/lowestElectronEnergy",this);
  lowEnCmd->SetGuidance("Set the lowest kinetic energy for e+-");
  lowEnCmd->SetParameterName("elow",true);
  lowEnCmd->SetUnitCategory("Energy");
  lowEnCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  lowhEnCmd = new G4UIcmdWithADoubleAndUnit("/process/em/lowestMuHadEnergy",this);
  lowhEnCmd->SetGuidance("Set the lowest kinetic energy for muons and hadrons");
  lowhEnCmd->SetParameterName("elowh",true);
  lowhEnCmd->SetUnitCategory("Energy");
  lowhEnCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  lllCmd = new G4UIcmdWithADouble("/process/eLoss/linLossLimit",this);
  lllCmd->SetGuidance("Set linearLossLimit parameter");
  lllCmd->SetParameterName("linlim",true);
  lllCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  brCmd = new G4UIcmdWithADoubleAndUnit("/process/eLoss/bremThreshold",this);
  brCmd->SetGuidance("Set bremsstrahlung energy threshold");
  brCmd->SetParameterName("emaxBrem",true);
  brCmd->SetUnitCategory("Energy");
  brCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  labCmd = new G4UIcmdWithADouble("/process/eLoss/LambdaFactor",this);
  labCmd->SetGuidance("Set lambdaFactor parameter for integral option");
  labCmd->SetParameterName("Fl",true);
  labCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  mscfCmd = new G4UIcmdWithADouble("/process/msc/FactorForAngleLimit",this);
  mscfCmd->SetGuidance("Set factor for computation of a limit for -t (invariant trasfer)");
  mscfCmd->SetParameterName("Fact",true);
  mscfCmd->SetRange("Fact>0");
  mscfCmd->SetDefaultValue(1.);
  mscfCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  angCmd = new G4UIcmdWithADoubleAndUnit("/process/msc/ThetaLimit",this);
  angCmd->SetGuidance("Set the limit on the polar angle for msc and single scattering");
  angCmd->SetParameterName("theta",true);
  angCmd->SetUnitCategory("Angle");
  angCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  frCmd = new G4UIcmdWithADouble("/process/msc/RangeFactor",this);
  frCmd->SetGuidance("Set RangeFactor for msc processes of e+-");
  frCmd->SetParameterName("Fr",true);
  frCmd->SetRange("Fr>0");
  frCmd->SetDefaultValue(0.04);
  frCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  fr1Cmd = new G4UIcmdWithADouble("/process/msc/RangeFactorMuHad",this);
  fr1Cmd->SetGuidance("Set RangeFactor for msc processes of muons/hadrons");
  fr1Cmd->SetParameterName("Fr1",true);
  fr1Cmd->SetRange("Fr>0");
  fr1Cmd->SetDefaultValue(0.2);
  fr1Cmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  fgCmd = new G4UIcmdWithADouble("/process/msc/GeomFactor",this);
  fgCmd->SetGuidance("Set GeomFactor parameter for msc processes");
  fgCmd->SetParameterName("Fg",true);
  fgCmd->SetRange("Fg>0");
  fgCmd->SetDefaultValue(3.5);
  fgCmd->AvailableForStates(G4State_PreInit,G4State_Idle);
 
  skinCmd = new G4UIcmdWithADouble("/process/msc/Skin",this);
  skinCmd->SetGuidance("Set skin parameter for msc processes");
  skinCmd->SetParameterName("skin",true);
  skinCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  dedxCmd = new G4UIcmdWithAnInteger("/process/eLoss/binsDEDX",this);
  dedxCmd->SetGuidance("Set number of bins for EM tables");
  dedxCmd->SetParameterName("binsDEDX",true);
  dedxCmd->SetDefaultValue(77);
  dedxCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  lamCmd = new G4UIcmdWithAnInteger("/process/eLoss/binsLambda",this);
  lamCmd->SetGuidance("Set number of bins for EM tables");
  lamCmd->SetParameterName("binsL",true);
  lamCmd->SetDefaultValue(77);
  lamCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  amCmd = new G4UIcmdWithAnInteger("/process/eLoss/binsPerDecade",this);
  amCmd->SetGuidance("Set number of bins per decade for EM tables");
  amCmd->SetParameterName("bins",true);
  amCmd->SetDefaultValue(7);
  amCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  verCmd = new G4UIcmdWithAnInteger("/process/eLoss/verbose",this);
  verCmd->SetGuidance("Set verbose level for EM physics");
  verCmd->SetParameterName("verb",true);
  verCmd->SetDefaultValue(1);
  verCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  ver1Cmd = new G4UIcmdWithAnInteger("/process/em/verbose",this);
  ver1Cmd->SetGuidance("Set verbose level for EM physics");
  ver1Cmd->SetParameterName("verb1",true);
  ver1Cmd->SetDefaultValue(1);
  ver1Cmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  ver2Cmd = new G4UIcmdWithAnInteger("/process/em/workerVerbose",this);
  ver2Cmd->SetGuidance("Set worker verbose level for EM physics");
  ver2Cmd->SetParameterName("verb2",true);
  ver2Cmd->SetDefaultValue(1);
  ver2Cmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  mscCmd = new G4UIcmdWithAString("/process/msc/StepLimit",this);
  mscCmd->SetGuidance("Set msc step limitation type");
  mscCmd->SetParameterName("StepLim",true);
  mscCmd->SetCandidates("Minimal UseSafety UseSafetyPlus UseDistanceToBoundary");
  mscCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  msc1Cmd = new G4UIcmdWithAString("/process/msc/StepLimitMuHad",this);
  msc1Cmd->SetGuidance("Set msc step limitation type for muons/hadrons");
  msc1Cmd->SetParameterName("StepLim1",true);
  msc1Cmd->SetCandidates("fMinimal fUseSafety fUseSafetyPlus fUseDistanceToBoundary");
  msc1Cmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  pixeXsCmd = new G4UIcmdWithAString("/process/em/pixeXSmodel",this);
  pixeXsCmd->SetGuidance("The name of PIXE cross section");
  pixeXsCmd->SetParameterName("pixeXS",true);
  pixeXsCmd->SetCandidates("ECPSSR_Analytical Empirical ECPSSR_FormFactor");
  pixeXsCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  pixeeXsCmd = new G4UIcmdWithAString("/process/em/pixeElecXSmodel",this);
  pixeeXsCmd->SetGuidance("The name of PIXE cross section for electron");
  pixeeXsCmd->SetParameterName("pixeEXS",true);
  pixeeXsCmd->SetCandidates("ECPSSR_Analytical Empirical Livermore Penelope");
  pixeeXsCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  paiCmd = new G4UIcommand("/process/em/AddPAIRegion",this);
  paiCmd->SetGuidance("Activate PAI in the G4Region.");
  paiCmd->SetGuidance("  partName  : particle name (default - all)");
  paiCmd->SetGuidance("  regName   : G4Region name");
  paiCmd->SetGuidance("  paiType   : PAI, PAIphoton");

  G4UIparameter* part = new G4UIparameter("partName",'s',false);
  paiCmd->SetParameter(part);

  G4UIparameter* pregName = new G4UIparameter("regName",'s',false);
  paiCmd->SetParameter(pregName);

  G4UIparameter* ptype = new G4UIparameter("type",'s',false);
  paiCmd->SetParameter(ptype);

  meCmd = new G4UIcmdWithAString("/process/em/AddMicroElecRegion",this);
  meCmd->SetGuidance("Activate MicroElec model in the G4Region");
  meCmd->SetParameterName("MicroElec",true);
  meCmd->AvailableForStates(G4State_PreInit);

  dnaCmd = new G4UIcommand("/process/em/AddDNARegion",this);
  dnaCmd->SetGuidance("Activate DNA in the G4Region.");
  dnaCmd->SetGuidance("  regName   : G4Region name");
  dnaCmd->SetGuidance("  dnaType   : opt0, opt1, opt2");

  G4UIparameter* regName = new G4UIparameter("regName",'s',false);
  dnaCmd->SetParameter(regName);

  G4UIparameter* type = new G4UIparameter("type",'s',false);
  dnaCmd->SetParameter(type);

  dumpCmd = new G4UIcommand("/process/em/printParameters",this);
  dumpCmd->SetGuidance("Print all EM parameters.");

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4EmParametersMessenger::~G4EmParametersMessenger()
{
  delete eLossDirectory;
  delete mscDirectory;
  delete emDirectory;

  delete flucCmd;
  delete rangeCmd;
  delete lpmCmd;
  delete splCmd;
  delete rsCmd;
  delete aplCmd;
  delete deCmd;
  delete dirFluoCmd;
  delete auCmd;
  delete auCascadeCmd;
  delete pixeCmd;
  delete dcutCmd;
  delete latCmd;
  delete mulatCmd;
  delete catCmd;
  delete delCmd;
  delete mottCmd;

  delete minSubSecCmd;
  delete minEnCmd;
  delete maxEnCmd;
  delete cenCmd;
  delete lowEnCmd;
  delete lowhEnCmd;
  delete lllCmd;
  delete brCmd;
  delete labCmd;
  delete mscfCmd;
  delete angCmd;
  delete frCmd;
  delete fr1Cmd;
  delete fgCmd;
  delete skinCmd;

  delete dedxCmd;
  delete lamCmd;
  delete amCmd;
  delete verCmd;
  delete ver1Cmd;
  delete ver2Cmd;

  delete mscCmd;
  delete msc1Cmd;

  delete pixeXsCmd;
  delete pixeeXsCmd;

  delete paiCmd;
  delete meCmd;
  delete dnaCmd;
  delete dumpCmd;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void G4EmParametersMessenger::SetNewValue(G4UIcommand* command, 
                                          G4String newValue)
{
  G4bool physicsModified = false;
  if (command == flucCmd) {
    theParameters->SetLossFluctuations(flucCmd->GetNewBoolValue(newValue));
    physicsModified = true;
  } else if (command == rangeCmd) {
    theParameters->SetBuildCSDARange(rangeCmd->GetNewBoolValue(newValue));
  } else if (command == lpmCmd) {
    theParameters->SetLPM(lpmCmd->GetNewBoolValue(newValue));
    physicsModified = true;
  } else if (command == splCmd) {
    theParameters->SetSpline(splCmd->GetNewBoolValue(newValue));
    physicsModified = true;
  } else if (command == rsCmd) {
    theParameters->SetUseCutAsFinalRange(rsCmd->GetNewBoolValue(newValue));
    physicsModified = true;
  } else if (command == aplCmd) {
    theParameters->SetApplyCuts(aplCmd->GetNewBoolValue(newValue));
    physicsModified = true;
  } else if (command == deCmd) {
    theParameters->SetFluo(deCmd->GetNewBoolValue(newValue));
    physicsModified = true;
  } else if (command == dirFluoCmd) {
    theParameters->SetBeardenFluoDir(dirFluoCmd->GetNewBoolValue(newValue));
  } else if (command == auCmd) {
    theParameters->SetAuger(auCmd->GetNewBoolValue(newValue));
    physicsModified = true;
  } else if (command == auCascadeCmd) {
    theParameters->SetAugerCascade(auCascadeCmd->GetNewBoolValue(newValue));
    physicsModified = true;
  } else if (command == pixeCmd) {
    theParameters->SetPixe(pixeCmd->GetNewBoolValue(newValue));
    physicsModified = true;
  } else if (command == dcutCmd) {
    theParameters->SetDeexcitationIgnoreCut(dcutCmd->GetNewBoolValue(newValue));
    physicsModified = true;
  } else if (command == latCmd) {
    theParameters->SetLateralDisplacement(latCmd->GetNewBoolValue(newValue));
    physicsModified = true;
  } else if (command == mulatCmd) {
    theParameters->SetMuHadLateralDisplacement(mulatCmd->GetNewBoolValue(newValue));
    physicsModified = true;
  } else if (command == catCmd) {
    theParameters->SetLatDisplacementBeyondSafety(catCmd->GetNewBoolValue(newValue));
    physicsModified = true;
  } else if (command == delCmd) {
    theParameters->ActivateAngularGeneratorForIonisation(delCmd->GetNewBoolValue(newValue));
  } else if (command == mottCmd) {
    theParameters->SetUseMottCorrection(mottCmd->GetNewBoolValue(newValue));

  } else if (command == minSubSecCmd) {
    theParameters->SetMinSubRange(minSubSecCmd->GetNewDoubleValue(newValue));
  } else if (command == minEnCmd) {
    theParameters->SetMinEnergy(minEnCmd->GetNewDoubleValue(newValue));
    physicsModified = true;
  } else if (command == maxEnCmd) { 
    theParameters->SetMaxEnergy(maxEnCmd->GetNewDoubleValue(newValue));
    physicsModified = true;
  } else if (command == cenCmd) { 
    theParameters->SetMaxEnergyForCSDARange(cenCmd->GetNewDoubleValue(newValue));
    physicsModified = true;
  } else if (command == lowEnCmd) { 
    theParameters->SetLowestElectronEnergy(lowEnCmd->GetNewDoubleValue(newValue));
    physicsModified = true;
  } else if (command == lowhEnCmd) { 
    theParameters->SetLowestMuHadEnergy(lowhEnCmd->GetNewDoubleValue(newValue));
    physicsModified = true;
  } else if (command == lllCmd) { 
    theParameters->SetLinearLossLimit(lllCmd->GetNewDoubleValue(newValue));
    physicsModified = true;
  } else if (command == brCmd) { 
    theParameters->SetBremsstrahlungTh(brCmd->GetNewDoubleValue(newValue));
    physicsModified = true;
  } else if (command == labCmd) {
    theParameters->SetLambdaFactor(labCmd->GetNewDoubleValue(newValue));
    physicsModified = true;
  } else if (command == mscfCmd) {
    theParameters->SetFactorForAngleLimit(mscfCmd->GetNewDoubleValue(newValue));
    physicsModified = true;
  } else if (command == angCmd) { 
    theParameters->SetMscThetaLimit(angCmd->GetNewDoubleValue(newValue));
    physicsModified = true;
  } else if (command == frCmd) {
    theParameters->SetMscRangeFactor(frCmd->GetNewDoubleValue(newValue));
    physicsModified = true;
  } else if (command == fr1Cmd) {
    theParameters->SetMscMuHadRangeFactor(fr1Cmd->GetNewDoubleValue(newValue));
    physicsModified = true;
  } else if (command == fgCmd) {
    theParameters->SetMscGeomFactor(fgCmd->GetNewDoubleValue(newValue));
    physicsModified = true;
  } else if (command == skinCmd) { 
    theParameters->SetMscSkin(skinCmd->GetNewDoubleValue(newValue));
    physicsModified = true;

  } else if (command == dedxCmd) { 
    theParameters->SetNumberOfBins(dedxCmd->GetNewIntValue(newValue));
    physicsModified = true;
  } else if (command == lamCmd) { 
    theParameters->SetNumberOfBins(lamCmd->GetNewIntValue(newValue));
    physicsModified = true;
  } else if (command == amCmd) { 
    theParameters->SetNumberOfBinsPerDecade(amCmd->GetNewIntValue(newValue));
    physicsModified = true;
  } else if (command == verCmd) {
    theParameters->SetVerbose(verCmd->GetNewIntValue(newValue));
  } else if (command == ver1Cmd) {
    theParameters->SetVerbose(ver1Cmd->GetNewIntValue(newValue));
  } else if (command == ver2Cmd) {
    theParameters->SetWorkerVerbose(ver2Cmd->GetNewIntValue(newValue));

  } else if (command == mscCmd || command == msc1Cmd) {
    G4MscStepLimitType msctype = fUseSafety;
    if(newValue == "Minimal") { 
      msctype = fMinimal;  
    } else if(newValue == "UseDistanceToBoundary") { 
      msctype = fUseDistanceToBoundary;
    } else if(newValue == "UseSafety") { 
      msctype = fUseSafety; 
    } else if(newValue == "UseSafetyPlus") {
      msctype = fUseSafetyPlus; 
    } else {
      G4cout << "### G4EmParametersMessenger WARNING: StepLimit type <" 
             << newValue << "> unknown!" << G4endl;
      return;
    }
    if (command == mscCmd) {
      theParameters->SetMscStepLimitType(msctype);
    } else {
      theParameters->SetMscMuHadStepLimitType(msctype);
    }
    physicsModified = true;
  } else if (command == pixeXsCmd) {
    theParameters->SetPIXECrossSectionModel(newValue);
    physicsModified = true;
  } else if (command == pixeeXsCmd) {
    theParameters->SetPIXEElectronCrossSectionModel(newValue);
    physicsModified = true;
  } else if (command == paiCmd) {
    G4String s1(""),s2(""),s3("");
    std::istringstream is(newValue);
    is >> s1 >> s2 >> s3;
    theParameters->AddPAIModel(s1, s2, s3);
  } else if (command == meCmd) {
    theParameters->AddMicroElec(newValue);
  } else if (command == dnaCmd) {
    G4String s1(""),s2("");
    std::istringstream is(newValue);
    is >> s1 >> s2;
    theParameters->AddDNA(s1, s2);
  } else if (command == dumpCmd) {
    theParameters->Dump();
  }
  if(physicsModified) {
    G4UImanager::GetUIpointer()->ApplyCommand("/run/physicsModified");
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
