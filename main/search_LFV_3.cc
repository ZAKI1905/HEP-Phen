/*

  This file is for proposing new searches for
  the LFV_3 model.

  - Last Updated by Zaki on September 3, 2019
*/

#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
#include <chrono>
#include <ctime>
#include <omp.h>

#include "../include/Pheno.h"

// Cut classes header files
#include "../include/IdEffCut.h"
#include "../include/IsolationCut.h"
#include "../include/M2Cut.h"
#include "../include/M4Cut.h"
#include "../include/PtCut.h"
#include "../include/PrapCut.h"
#include "../include/OffZCut.h"

std::vector<double> scaleTauE(std::vector<PHENO::ExParticle>&);

///////////////////////////////////////////////////
///////////////      main()     ///////////////////
///////////////////////////////////////////////////
int main(int argc,char *argv[])
{
  Zaki::Util::Instrumentor::BeginSession("search_LFV_3", "Profile_search_LFV_3.json") ;        // Begin session  

if(argc < 5) 
  { Z_LOG_ERROR("Filename, total events, number of threads & report bool are missing!") ;
   return 1;
  }

  std::string filename           = "" ;
  std::string Tot_Num_Events_str = "" ;
  std::string NUM_THREADS_str    = "" ;
  std::string report_input_str   = "" ;

  filename.assign(argv[1]) ;
  Tot_Num_Events_str.assign(argv[2]) ;
  NUM_THREADS_str.assign(argv[3]) ;
  report_input_str.assign(argv[4]) ;

  if( report_input_str == "1")
    report_input_str = "true" ;
  else
    report_input_str = "false" ;

  // Seeding the random function for ID_eff "rand01()"
  srand(time(NULL)) ;

  using namespace PHENO ;

  Pheno phen;

  //..................INPPUT.....................
  // 'input' method strips inputs from any spaces,
  // unless a "false" option is passed as a second argument.

  // Number of requested threads
  phen.Input("Threads=" + NUM_THREADS_str) ;

  // Total number of events
  phen.Input("Events=" + Tot_Num_Events_str) ;
  phen.Input("file:lhe=" + filename) ;

  // reporting options
  phen.Input("report:cuts=" + report_input_str) ;

  // This is an exception list that will always
  //  print reports for the range of events listed.
  // Use ',' for separatng event ranges, or single events.
  // Use '-' for ranges.
  // phen.Input("report:cuts=1-500") ;

  phen.Input("report:taus=" + report_input_str) ;
  // phen.Input("report:taus=5-17,155,122") ;
  // phen.Input("report:jets="+report_input_str) ;
  // phen.Input("report:jets=1-1000") ;

  // printing event records
  // phen.Input("print_events=1-10,50,300") ;


  // ----------------------pythia options-------------------------------------

  phen.Input( "pythia=Beams:frameType = 4" ) ;

  // "8" for CTEQ6L1 PDF set.
  // "False" option is for not stripping the space 
  // between 'pSet' & '8'.
  phen.Input( "pythia=PDF:pSet 8", false ) ;

  phen.Input( "pythia=Random:setSeed = on" ) ;

  // Use -1 for a time dep. random #
  phen.Input( "pythia=Random:seed = -1" ) ;

  // Show pythia banner or not?
  phen.Input( "show_pythia_banner=false" ) ;

  // Init settings:
  phen.Input( "pythia=Init:showProcesses  = off" ) ;
  phen.Input( "pythia=Init:showChangedParticleData  = off" ) ;
  phen.Input( "pythia=Init:showMultipartonInteractions  = off" ) ;
  phen.Input( "pythia=Init:showChangedSettings   = off" ) ;

  // Next settings:
  phen.Input( "pythia=Next:numberShowProcess  = 0" ) ;
  phen.Input( "pythia=Next:numberShowInfo  = 0" ) ;
  phen.Input( "pythia=Next:numberShowLHA = 0" ) ;
  phen.Input( "pythia=Next:numberShowEvent = 0" ) ;


  // Stat settings:
  phen.Input( "pythia=Stat:showProcessLevel = off" ) ;

  // ----------------------Cut options----------------------------------------

  // Required final states ( use e for both e+ and e-)
  phen.Input( "required_set=ta_h>=1, e>=1, mu+>=1, mu->=1" ) ; // LFV_3

  // ---------------------- Cuts ----------------------
  // You can move the recording function(s) between the cuts
  // it will record the values for particles passing
  // the cut prior to it. The functions defined here or
  // in included header files can be used.

  // ID_Eff cut
  phen.Input({new CUTS::IdEffCut, "drop_low_eff=true"}) ;

  // Cut on M_l+l-
  phen.Input({new CUTS::M2Cut, "M2_Cut_Value=12"}) ;

  /*  p_T Cut Conditions:
        e & mu: pt>= 7 GeV  (at least 1 > 20 GeV)
        t_h: pt>= 20 GeV
  */
  phen.Input({new CUTS::PtCut, "lead=20,sub_lead=7,extra=7,had_tau=20"}) ;

  /*  prap Cut Conditions:
        e & mu: |eta| < 2.4
        t_h: |eta| < 2.3
  */
  phen.Input({new CUTS::PrapCut, "e=2.4,mu=2.4,had_tau=2.3"}) ;

  // Isolation cut
  phen.Input(new CUTS::IsolationCut) ;

  //-------------------------
  // // Don't have to run fastjet in this case
  // // fastjet options
  // phen.Input("fastjet=Def:Algorithm=antikt_algorithm,R=0.5") ;
  // phen.Input("fastjet=Selector:EtaMax=2.5") ;
  // phen.Input("fastjet=Selector:PtMin=30") ;
  
  // // running fastjet
  // phen.Input("fastjet=run") ;
  //-------------------------

  // Demanding  m(l+l-) <= 75 GeV or 105 GeV <= m(l+l-)
  phen.Input({new CUTS::OffZCut, "OffZ_Cut_Min=75,OffZ_Cut_Max=105"}) ;

  // Demanding 80 GeV <= m(ta, e, mu+, mu-) <= 120 GeV
  phen.Input({new CUTS::M4Cut, "M4_Cut_Min=80,M4_Cut_Max=120"}) ;  // LFV_3
  // -------------------------------------------------------------------------

  //------------------------------------------------------
  // Recording invariant masses
  
  phen.Input("record=Tau_E_Scaled_5", scaleTauE) ;  // LFV_3_1
  //------------------------------------------------------
  

  phen.Run() ;

  Zaki::Util::Instrumentor::EndSession();  // End Session
  return 0 ;
}

//==============================================================
// Electron & tau momentum scaling ( for LFV_3_1 )
std::vector<double> scaleTauE(std::vector<PHENO::ExParticle>& parts)
{
  if (parts.size() != 4)  return {-1, -1, -1, -1, -1, -1, -1} ;

  std::vector<PHENO::ExParticle> sel_emu ;
  std::vector<PHENO::ExParticle> sel_taumu ;
  int tau_sign;

  // Adding tau and electron
  for (size_t i=0 ; i<parts.size() ; ++i)
  {
    if ( parts[i].idAbs() == ID_TAU)
    {
      tau_sign = (parts[i].id() / parts[i].idAbs()) ;
      sel_taumu.push_back(parts[i]) ;
    }
    if ( parts[i].idAbs() == ID_ELECTRON)
      sel_emu.push_back(parts[i]) ;

  }

  // Finding muon
 for (size_t i=0 ; i<parts.size() ; ++i)
  {
    if ( parts[i].idAbs() == ID_MUON )
      {
        if( (parts[i].id() / parts[i].idAbs()) == tau_sign )
          sel_emu.push_back(parts[i]) ;
        else
          sel_taumu.push_back(parts[i]) ;
      }
  }

  if (sel_emu.size() != 2 || sel_taumu.size() != 2)  return {-2, -2, -2, -2, -2, -2, -2} ;

  Pythia8::Vec4 p_ta = sel_taumu[0].GetVisMom();
  Pythia8::Vec4 p_e = sel_emu[0].GetVisMom();
  Pythia8::Vec4 p_mu_sum = sel_taumu[1].GetVisMom() + sel_emu[1].GetVisMom();

  double m_ta = 1.777 ; // GeV
  double m_H  = 125   ; // GeV

  double M4 = (p_ta + sel_taumu[1].GetVisMom() + p_e + sel_emu[1].GetVisMom() ).mCalc() ;
  double tauMuInv  = (p_ta + sel_taumu[1].GetVisMom()).mCalc() ;
  double eMuInv    = (p_e + sel_emu[1].GetVisMom()).mCalc() ;


  // ..........................................
  // Fifth Scaling
  double a = ( 2*p_ta*p_e / (pow(eMuInv, 2)*pow(tauMuInv, 2)) + pow(m_ta,2) / pow(tauMuInv, 4) ); 
  double b = 2*(p_mu_sum) * ( p_ta/pow(tauMuInv, 2) + p_e/pow(eMuInv, 2) );  
  double c = p_mu_sum.m2Calc() - pow(m_H,2);  
  // ..........................................

  double mfi_sq_p = ( -b + sqrt( pow(b,2) - 4*a*c ) ) / ( 2*a ) ; 
  // double mfi_sq_m = ( -b - sqrt( pow(b,2) - 4*a*c ) ) / ( 2*a ) ; 

  // ..........................................

  double alpha_ta = mfi_sq_p / pow(tauMuInv, 2);
  double alpha_e = mfi_sq_p / pow(eMuInv, 2);

  if(alpha_ta < 1 || alpha_e < 1) { alpha_e = 1; alpha_ta = 1; }

  Pythia8::Vec4 scaled_p_ta = alpha_ta*p_ta ;
  Pythia8::Vec4 scaled_p_e = alpha_e*p_e ;
  // ..........................................


  double tauMuInv_scaled  = (scaled_p_ta + sel_taumu[1].GetVisMom()).mCalc() ;
  double eMuInv_scaled    = (scaled_p_e + sel_emu[1].GetVisMom()).mCalc() ;

  double M4_scaled = (scaled_p_ta + sel_taumu[1].GetVisMom() + scaled_p_e + sel_emu[1].GetVisMom() ).mCalc() ;

  std::vector<double> out = {tauMuInv, eMuInv, M4, mfi_sq_p, 
                        tauMuInv_scaled, eMuInv_scaled, M4_scaled} ;

  return out;
}

//==============================================================