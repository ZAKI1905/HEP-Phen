#ifndef M2Cut_H
#define M2Cut_H

#include "Cut.h"

//--------------------------------------------------------------
namespace PHENO
{
  class Pheno;

//--------------------------------------------------------------
namespace CUTS
{
//==============================================================
class M2Cut : public Cut
{
  
  // Gives access to input, etc.
  friend class PHENO::Pheno ;   

  //--------------------------------------------------------------
  public:
    // Default constructor
    M2Cut() ;
    
    // Constructor takes a pointer to the event
    M2Cut(ExEvent*) ;
   
    // Copy Constructor
    M2Cut(const M2Cut&) ;

  //--------------------------------------------------------------
  private:

    // Virtual method from cut class
    void CutCond(ParticleLST&) override ;   
    void Input(std::string) override ; 
    virtual M2Cut* IClone() const override ; 
    
    // The cut on m2
    float M2_Cut_Value = 0 ;   
};
//=============================================================
} // CUTS namespace
//==============================================================
} // PHENO namespace
//=============================================================
#endif /*M2Cut_H*/
