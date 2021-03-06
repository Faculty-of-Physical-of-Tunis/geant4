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
//J.M. Quesada (August2008). Based on:
//
// Hadronic Process: Nuclear De-excitations
// by V. Lara (Oct 1998)
//
// Modif (03 September 2008) by J. M. Quesada for external choice of inverse 
// cross section option
// JMQ (06 September 2008) Also external choices have been added for 
// superimposed Coulomb barrier (if useSICB is set true, by default is false) 
//
// JMQ (14 february 2009) bug fixed in emission width: hbarc instead of 
//                        hbar_Planck in the denominator
//
#include "G4EvaporationProbability.hh"
#include "G4VCoulombBarrier.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4PairingCorrection.hh"
#include "G4NucleiProperties.hh"
#include "G4KalbachCrossSection.hh"
#include "G4ChatterjeeCrossSection.hh"
#include "Randomize.hh"
#include "G4Exp.hh"

using namespace std;

static const G4double explim = 350.;
static const G4double invmev = 1./CLHEP::MeV;
static const G4double ssqr3  = 1.5*std::sqrt(3.0);

G4EvaporationProbability::G4EvaporationProbability(G4int anA, G4int aZ, 
						   G4double aGamma, 
						   G4VCoulombBarrier*) 
  : theA(anA),
    theZ(aZ),
    Gamma(aGamma)
{
  resZ = resA = fragA = fragZ = nbins = 0;
  resA13 = muu = resMass = fragMass = U = delta0 = delta1 = a0 = 0.0;
  partMass = G4NucleiProperties::GetNuclearMass(theA, theZ);
  index = 0;
  if(1 == theZ) { index = theA; }
  else { index = theA + 1; }
  for(G4int i=0; i<11; ++i) { probability[i] = 0.0; }
}

G4EvaporationProbability::~G4EvaporationProbability() 
{}

// obsolete method  
G4double G4EvaporationProbability::EmissionProbability(
         const G4Fragment&, G4double)
{
  return 0.0;
}

G4double G4EvaporationProbability::TotalProbability(
         const G4Fragment & fragment, G4double minEnergy, G4double maxEnergy)
{
  if (maxEnergy <= minEnergy) { return 0.0; }

  fragA = fragment.GetA_asInt();
  fragZ = fragment.GetZ_asInt();
  resA  = fragA - theA;
  resZ  = fragZ - theZ;

  U = fragment.GetExcitationEnergy();
  delta0 = std::max(0.0, fPairCorr->GetPairingCorrection(fragA,fragZ));
  if(U < delta0) { return 0.0; }

  fragMass = fragment.GetGroundStateMass();
  delta1 = std::max(0.0, fPairCorr->GetPairingCorrection(resA,resZ));
  resMass = G4NucleiProperties::GetNuclearMass(resA, resZ);
  resA13 = fG4pow->Z13(resA);
   
  G4double Width = 0.0;
  if (OPTxs==0) {

    G4double SystemEntropy = 2.0*std::sqrt(
      theEvapLDPptr->LevelDensityParameter(fragA,fragZ,U)*(U-delta0));
								  
    static const G4double RN2 = 
      2.25*fermi*fermi/(twopi* hbar_Planck*hbar_Planck);

    G4double Alpha = CalcAlphaParam(fragment);
    G4double Beta = CalcBetaParam(fragment);
	
    a0 = theEvapLDPptr->LevelDensityParameter(resA,resZ,maxEnergy);
    G4double GlobalFactor = Gamma*Alpha*partMass*RN2*resA13*resA13/(a0*a0);
    
    G4double maxea = maxEnergy*a0;
    G4double Term1 = Beta*a0 - 1.5 + maxea;
    G4double Term2 = (2.0*Beta*a0-3.0)*std::sqrt(maxea) + 2*maxea;
	
    G4double ExpTerm1 = 0.0;
    if (SystemEntropy <= explim) { ExpTerm1 = G4Exp(-SystemEntropy); }
	
    G4double ExpTerm2 = 2.*std::sqrt(maxea) - SystemEntropy;
    ExpTerm2 = std::min(ExpTerm2, explim);
    ExpTerm2 = G4Exp(ExpTerm2);
	
    Width = GlobalFactor*(Term1*ExpTerm1 + Term2*ExpTerm2);
             
  } else {

    a0 = theEvapLDPptr->LevelDensityParameter(fragA,fragZ,U - delta0);
    // compute power once
    if(OPTxs <= 2) { 
      muu =  G4ChatterjeeCrossSection::ComputePowerParameter(resA, index);
    } else {
      muu = G4KalbachCrossSection::ComputePowerParameter(resA, index);
    }
    // if Coulomb barrier cutoff is superimposed for all cross sections 
    // then the limit is the Coulomb Barrier
    Width = IntegrateEmissionProbability(minEnergy, maxEnergy);
  }
  return Width;
}

G4double 
G4EvaporationProbability::IntegrateEmissionProbability(G4double low, G4double up)
{
  G4double sum = 0.0;
  G4double del = up - low;
  nbins = std::min(10, G4int(del*invmev));
  nbins = std::max(nbins, 2);
  del /= G4double(nbins);
  G4double T = low - del*0.5;
  for(G4int i=1; i<=nbins; ++i) {
    T += del;
    sum +=  ProbabilityDistributionFunction(T);
    probability[i] = sum;
  }
  sum *= del;
  return sum;
}

G4double G4EvaporationProbability::ProbabilityDistributionFunction(G4double K)
{ 
  //G4cout << "### G4EvaporationProbability::ProbabilityDistributionFunction" 
  //  << G4endl;

  G4double E0 = U - delta0;
  G4double E1 = U + fragMass - partMass - resMass - delta1 - K;
  /*
  G4cout << "PDF: FragZ= " << fragment.GetZ_asInt() << " FragA= " 
	 << fragment.GetA_asInt()
  	 << " Z= " << theZ << "  A= " << theA 
	 << " K= " << K << " E0= " << E0 << " E1= " << E1 << G4endl;
  */
  if(E1 < 0.0) { return 0.0; }

  G4double a1 = theEvapLDPptr->LevelDensityParameter(resA, resZ, E1);

  //JMQ 14/02/09 BUG fixed: hbarc should be in the denominator instead 
  //             of hbar_Planck; without 1/hbar_Panck remains as a width

  static const G4double pcoeff = millibarn/((pi*hbarc)*(pi*hbarc)); 

  // Fixed numerical problem
  G4double Prob = pcoeff*Gamma*partMass
    *G4Exp(2*(std::sqrt(a1*E1) - std::sqrt(a0*E0)))*K*CrossSection(K);

  return Prob;
}

G4double 
G4EvaporationProbability::CrossSection(G4double K)
{
  G4double res;
  if(OPTxs <= 2) { 
    res = G4ChatterjeeCrossSection::ComputeCrossSection(K, resA13, muu,
							index, theZ, 
							resZ, resA); 
  } else { 
    res = G4KalbachCrossSection::ComputeCrossSection(K, resA13, muu,
						     index, theZ, theA, 
						     resZ, resA);
  }
  return res;
}  

G4double 
G4EvaporationProbability::SampleKineticEnergy(G4double minKinEnergy, 
					      G4double maxKinEnergy)
{
  if(maxKinEnergy <= minKinEnergy) { return 0.0; }

  G4double T = 0.0;
  if (OPTxs==0) {
    // JMQ:
    // It uses Dostrovsky's approximation for the inverse reaction cross
    // in the probability for fragment emission
    // MaximalKineticEnergy energy in the original version (V.Lara) was 
    // calculated at the Coulomb barrier.
    
    G4double Rb = 4.0*a0*maxKinEnergy;
    G4double RbSqrt = std::sqrt(Rb);
    G4double PEX1 = 0.0;
    if (RbSqrt < 160.0) PEX1 = G4Exp(-RbSqrt);
    G4double Rk = 0.0;
    G4double FRk = 0.0;
    do {
      G4double RandNumber = G4UniformRand();
      Rk = 1.0 + (1./RbSqrt)*G4Log(RandNumber + (1.0-RandNumber)*PEX1);
      G4double Q1 = 1.0;
      G4double Q2 = 1.0;
      if (theZ == 0) { // for emitted neutron
        G4double Beta = (2.12/(resA*resA) - 0.05)*MeV/(0.76 + 2.2/resA13);
        Q1 = 1.0 + Beta/maxKinEnergy;
        Q2 = Q1*std::sqrt(Q1);
      } 
      
      FRk = ssqr3 * Rk * (Q1 - Rk*Rk)/Q2;
      
      // Loop checking, 05-Aug-2015, Vladimir Ivanchenko
    } while (FRk < G4UniformRand());
    
    T = maxKinEnergy * (1.0-Rk*Rk) + minKinEnergy;

  } else { 

    G4double p =  probability[nbins]*G4UniformRand();
    G4int i = 0;
    for(; i<nbins; ++i) { if(p <= probability[i+1]) { break; } }
    G4double delta = (maxKinEnergy - minKinEnergy)/G4double(nbins);
    T = minKinEnergy + delta*i 
      + delta*(p - probability[i])/(probability[i+1] - probability[i]);

  }
  //G4cout << " T= " << T << G4endl;
  return T;
}
