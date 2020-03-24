/*
  Isolation class

  Last Updated by Zaki on July 6, 2019

*/

#include "../include/Isolation.h"

//==============================================================
/*

sum_ET takes an event, and sums over the E_T's of all particles 
in R<0.3 of each particle (except itself), and checks if they 
satisfy the cut condition.

*/

//--------------------------------------------------------------
// Default Constructor
Isolation::Isolation()
{
  logger.SetUnit("Isolation");
  SetName("Isolation") ;
}

// Constructor
Isolation::Isolation(ExEvent* ev) : Cut(ev) 
{
  logger.SetUnit("Isolation");
  SetName("Isolation") ;
}

//--------------------------------------------------------------
// Virtual method from cut class:
void Isolation::CutCond(std::vector<ExParticle>& in_parlst)
{
  PROFILE_SCOPE("Isolation::CutCond") ;

  for(size_t i=0 ; i < in_parlst.size() ; ++i )
  {
    sum_ET_value     = 0.0 ;
    sum_ET_tau_value = 0.0 ;

    ExParticle prt_i =  in_parlst[i];

    for(size_t j=0 ; j < evPtr->size() ; ++j )
    {
      ExParticle prt_j =  evPtr->FullEvent()[j] ;

        /* 
          Checking if part_j is final state, and not the same as
            the particle from input list.
        */
        if( prt_j.isVisible() && prt_j.isFinal() && prt_j.GetMom().eT() > 0.001 &&  
            !( prt_j == prt_i  ) )
        {   
            // Checking if they are within 0.3 cone
            if( R(prt_j, prt_i) < 0.3 )
            {

                /* 
                  In case of electron and muon check they are final states
                */
                if((prt_i.idAbs()==ID_ELECTRON || prt_i.idAbs()==ID_MUON) 
                    && prt_i.isFinal())
                { sum_ET_value += prt_j.GetMom().eT() ; }

                // Removing the inner 0.1 cone in case of tau
                else if(prt_i.idAbs()==ID_TAU && 0.1 < R(prt_j, prt_i))
                { sum_ET_tau_value += prt_j.GetMom().eT() ;  }
            }
        }
    }

    // Checking isolation cut for muons and electrons
    if((prt_i.idAbs()==ID_ELECTRON || prt_i.idAbs()==ID_MUON) &&
      prt_i.isFinal() && prt_i.GetMom().pT() != 0)
    {
      if((sum_ET_value /prt_i.GetMom().pT()) > 0.15)
        add_elem(rm_list, (int) i) ;
    }

    // Checking isolation cut for taus (both hadronic and leptonic)
    if(prt_i.idAbs()==ID_TAU)
    {
      if(sum_ET_tau_value  > 2)
        add_elem(rm_list, (int) i) ;
    }
  }

}

//--------------------------------------------------------------
// Overriding the clone method
Cut* Isolation::Clone() 
{
  return new Isolation(*this) ;
}

//==============================================================
