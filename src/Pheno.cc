/*
  Pheno class

  Last Updated by Zaki on July 6, 2019

*/

#include <iostream>
#include <omp.h>
#include <chrono>
#include <thread>

#include <zaki/File/SaveVec.h>

#include "../include/Pheno.h"
// #include "../include/IdEff.h"
// #include "../include/Isolation.h"
// #include "../include/M2Cut.h"
// #include "../include/M4Cut.h"
// #include "../include/PtCut.h"
// #include "../include/PrapCut.h"
// #include "../include/OffZCut.h"
#include "../include/STBinner.h"

//==============================================================

//--------------------------------------------------------------
// Constructor
PHENO::Pheno::Pheno()
{
  start_time = omp_get_wtime() ; 
  // logger.SetUnit("Pheno"); 
  
  auto t_1 = std::chrono::system_clock::now();
  std::time_t start_t = std::chrono::system_clock::to_time_t(t_1);
  char tmp[150];
  std::strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", std::gmtime(&start_t)) ;

  input_list.push_back("#------------------------------------------------------------------");
  input_list.push_back("#               Input Command List (" + std::string(tmp) + ")");
  input_list.push_back("#------------------------------------------------------------------");

}

//--------------------------------------------------------------
// Destructor
PHENO::Pheno::~Pheno()
{
  char tmp[100];
  sprintf(tmp, "Number of threads used: %2d,  Processing Time:  %.2f s.",
          threads, omp_get_wtime()-start_time) ;
  Z_LOG_INFO(tmp) ;

}

//--------------------------------------------------------------
void PHENO::Pheno::Input(std::string command, bool strip_space)
/*
  Input method that initializes the Pheno class:
*/
{
  // "strip_space" option is true by default

  // Saving the command in a list
  input_list.push_back(command) ;

  // Stripping the command from any spaces:
  if ( strip_space )
    command = Zaki::String::Strip(command, ' ');
  

  // Parsing the command
  std::vector<std::string> inp = Zaki::String::Pars(command, "=") ;

  std::string prop  = inp[0] ;
  bool value                 ;

  if(inp[1] == "true" ) {value = true  ; }
  if(inp[1] == "false") {value = false ; }

  // Cases with report options:
  std::vector<std::string> parsed_prop = Zaki::String::Pars(prop, ":") ;

  if( parsed_prop[0] == "report" ) 
  {
    if ( inp[1] == "true" || inp[1] == "false" )
    { 
      if ( parsed_prop[1] == "cuts" )
        report_cuts_flag  = value ;
      if ( parsed_prop[1] == "jets" )
        report_jets_flag  = value ;
      if ( parsed_prop[1] == "taus" )
        report_taus_flag  = value ;
    }
    else 
    {
      if ( parsed_prop[1] == "cuts" )
        report_cuts_set  = Zaki::String::RangeParser(inp[1])  ;
      if ( parsed_prop[1] == "jets" )
        report_jets_set  = Zaki::String::RangeParser(inp[1])  ;
      if ( parsed_prop[1] == "taus" )
        report_taus_set  = Zaki::String::RangeParser(inp[1])  ;
    }
  
  }


  if(prop == "Threads"  ) { req_threads = std::stoi(inp[1]) ; }


  if(prop == "print_events") 
  {
    // Parsing the input event numbers
    print_ev_set = Zaki::String::RangeParser(inp[1]) ;
  }

  if( prop == "required_set" )
  {
    std::vector<std::string> tmp_req_lst ;
    Zaki::String::Str2Lst( tmp_req_lst, inp[1] )  ;

    StateDict(tmp_req_lst) ;
  }

  // if(prop == "cuts")
  // {
  //   std::vector<std::string> tmp_cut_lst = {Zaki::String::Pars(inp[1], ":")[0]} ;
  //   Zaki::String::Str2Lst( tmp_cut_lst, Zaki::String::Pars(inp[1], ":")[1] )  ;
  //   cut_list.push_back(tmp_cut_lst) ;

  //   //................Recording funcs & vars................
  //   func_int.funcs.push_back({}) ;
  //   func_double.funcs.push_back({}) ;
  //   func_vec_double.funcs.push_back({}) ;

  //   rec_int.vars.push_back({}) ;
  //   rec_double.vars.push_back({}) ;
  //   rec_vec_double.vars.push_back({}) ;
    
  //   //......................................................
  // }

  if ( prop == "fastjet" )
  {
    if ( inp[1] == "run" )
    {
      gen_jet_idx = cut_list.size() - 1 ;
      // Z_LOG_INFO((" gen_jet_idx is = " + std::to_string(gen_jet_idx) ).c_str() );
    }
    else
    {
      // fastjet options
      std::vector<std::string> tmp_fj_commands = {Zaki::String::Pars(inp[1], ":")[0]} ;
      Zaki::String::Str2Lst( tmp_fj_commands, Zaki::String::Pars(inp[1], ":")[1] )  ;

      fastjet_commands.push_back(tmp_fj_commands) ;
    }
    
  }



  // if(prop == "Bin")
  // {
  //   std::vector<std::string> tmp_bin_lst = {Zaki::String::Pars(inp[1], ":")[0]} ;
  //   Zaki::String::Str2Lst( tmp_bin_lst, Zaki::String::Pars(inp[1], ":")[1] ) ;
  //   bin_list.push_back(tmp_bin_lst) ;
  // }

  if( prop == "Events") { tot_num_events = std::stoi(inp[1]) ; }

  if(prop == "file:lhe"    )
  {
    // Saving the LHE file path:
    LHE_path = inp[1] ;

    // Parsing the lhe file path
    std::vector<std::string> parsed_lhe_path = Zaki::String::Pars(LHE_path, "/", -1) ;

    // Extracting the lhe file name
    inp[1] = parsed_lhe_path [ parsed_lhe_path.size() - 1] ;

    // Removing the ".lhe" extenstion from the LHE file:
    std::vector<std::string> lhe_file_string = Zaki::String::Pars(inp[1], ".") ;

    file_LHE     = lhe_file_string[0] ;

    // LHE file path minus the file name ( + 4 for '.lhe')
    size_t lhe_str_len = file_LHE.length() + 4 ;
    LHE_path = LHE_path.substr(0, LHE_path.length() - lhe_str_len);

  }

  // pythia options
    // pythia banner
    if(prop == "show_pythia_banner"    ) { show_pythia_banner  = value ; }
    // internal pythia options
    if(prop == "pythia"    )
    {
      pythia_commands.push_back(inp[1]) ;
    }

}

//--------------------------------------------------------------
template<class T>
void PHENO::Pheno::Input(std::string input, T (*func)(ParticleLST& parts) )
{
  // Parsing the command
  std::vector<std::string> inp = Zaki::String::Pars(input, "=") ;

  if( inp[0] == "record")
  {
    rec_fun<T> tmp_rec  ;
    tmp_rec.f = func ;
    tmp_rec.name = inp[1] ;

    func_double.funcs[cut_list.size()].push_back(tmp_rec) ;
  }
}

//--------------------------------------------------------------
// Specialization for int instances
template<>
void PHENO::Pheno::Input<int>(std::string input, int (*func)(ParticleLST& parts) )
{
  // Parsing the command
  std::vector<std::string> inp = Zaki::String::Pars(input, "=") ;

  if( inp[0] == "record")
  {
    rec_fun<int> tmp_rec  ;
    tmp_rec.f = func ;
    tmp_rec.name = inp[1] ;

    func_int.funcs[cut_list.size()].push_back(tmp_rec) ;
  }
}

//--------------------------------------------------------------
// Specialization for double vectors instances
template<>
void PHENO::Pheno::Input<std::vector<double> >(std::string input, std::vector<double> (*func)(ParticleLST& parts) )
{
  // Parsing the command
  std::vector<std::string> inp = Zaki::String::Pars(input, "=") ;

  if( inp[0] == "record")
  {
    rec_fun<std::vector<double> > tmp_rec  ;
    tmp_rec.f = func ;
    tmp_rec.name = inp[1] ;

    func_vec_double.funcs[cut_list.size()].push_back(tmp_rec) ;
  }
}

//--------------------------------------------------------------
int PHENO::Pheno::CompStr2Int(std::string input)
{
  if (input == ">=")
    return 1 ;
  else if (input == "<=")
    return 2 ;
  else if (input == ">")
    return 3 ;
  else if (input == "<")
    return 4 ;
  else if (input == "=")
    return 5 ;
  else if (input == "!=")
    return 6 ;
  else
    return 0 ;
}

//--------------------------------------------------------------
void PHENO::Pheno::StateDict( std::vector<std::string> lst_in)
{
  // The binary comparisons: {">=", "<=", ">", "<", "=", "!="}
  // Note that ">=" should precede "=" and ">" for pars to
  // work correctly. Otherwise, ">=" could be read as ">" or "="
  std::vector<std::string> binary_comp = {">=", "<=", ">", "<", "=", "!="} ;
  for (size_t i=0 ; i<lst_in.size() ; ++i)
  {
    std::vector<std::string> name_str = Zaki::String::Pars(lst_in[i], binary_comp) ;
    std::vector<std::vector<int> > tmp_lst = { { std::stoi(name_str[2]),
                                      CompStr2Int(name_str[1]) } } ;

        if ( name_str[0] == "e+"  )
        tmp_lst.push_back({-ID_ELECTRON}) ;

  else if ( name_str[0]  == "e-"  )
        tmp_lst.push_back({+ID_ELECTRON}) ;

  else if ( name_str[0]  == "e"  )
        tmp_lst.push_back({+ID_ELECTRON, -ID_ELECTRON}) ;

  else if ( name_str[0]  == "mu+" )
        tmp_lst.push_back({-ID_MUON}) ;

  else if ( name_str[0]  == "mu-" )
        tmp_lst.push_back({ID_MUON}) ;

  else if ( name_str[0]  == "mu" )
        tmp_lst.push_back({+ID_MUON, -ID_MUON}) ;

  else if ( name_str[0]  == "emu" )
      tmp_lst.push_back({+ID_ELECTRON, -ID_ELECTRON,
                         +ID_MUON, -ID_MUON}) ;

  else if ( name_str[0]  == "emuta_h" )
      tmp_lst.push_back({+ID_ELECTRON, -ID_ELECTRON,
                         +ID_MUON, -ID_MUON,
                         +ID_TAU, -ID_TAU}) ;

  else if ( name_str[0]  == "ta_h" )
      tmp_lst.push_back({+ID_TAU, -ID_TAU}) ;
  else if ( name_str[0]  == "ta+_h" )
      tmp_lst.push_back({-ID_TAU}) ;
  else if ( name_str[0]  == "ta-_h" )
      tmp_lst.push_back({ID_TAU}) ;

  else
    Z_LOG_ERROR(("No matching definition for '" + name_str[0] + "' !").c_str()) ;

  req_states.push_back(tmp_lst) ;
  }

}

//--------------------------------------------------------------
void PHENO::Pheno::Run()
{
  PROFILE_FUNCTION() ;
  omp_set_num_threads(req_threads) ;
  Z_LOG_INFO( ("Threads requested: " + std::to_string(req_threads)).c_str()) ;

  num_cuts_passed.resize(tot_num_events) ;
  
  // Initializing the binner classes
  // InitBinner() ;

  #pragma omp parallel for firstprivate(show_pythia_banner, cut_list, report_cuts_flag, report_taus_flag, report_jets_flag, print_ev_set, file_LHE, pythia_commands, fastjet_commands) shared(bin_list)
  for (int j=0 ; j<req_threads ; ++j)
  {
    // Asking one of the threads to save the available threads
    if( j==0 ) threads = omp_get_num_threads() ;

    RunPythia(j) ;

  }

  FinalReport() ;
}

//-----------------------------------
void PHENO::Pheno::RunPythia(int pr_id)
/*
  Runs pythia, input the events in EV's and apply cuts
  and make reports, along with binning.
*/
{
  PROFILE_FUNCTION() ;

  //Generator.
    Pythia8::Pythia pythia("", show_pythia_banner) ;

  // Making a shared character for naming all the files
    char shared_file_char[150] ;
    sprintf(shared_file_char, "%s_%d", file_LHE.c_str(), pr_id) ;

  // Inputing the LHE file in pythia
    std::string tmp_pyth_lhe(shared_file_char);
    tmp_pyth_lhe = "Beams:LHEF = "+ LHE_path + tmp_pyth_lhe + ".lhe";

    pythia_commands.push_back(tmp_pyth_lhe) ;

  // Initialize pythia from pythia_commands:
    for (size_t i=0 ;  i<pythia_commands.size() ; ++i)
    {
      pythia.readString(pythia_commands[i]) ;
    }

   // Initialize fastjet from fastjet_commands:
   // For now I don't know how to implement this (May-12-2019)
   // for (size_t i=0 ;  i<fastjet_commands.size() ; ++i)
   //       {
   //         GenJet.Input(fastjet_commands[i]) ;
   //       }

  pythia.init() ;

  // Allow for possibility of a few faulty events.
  int nAbort      = 10 ;
  int iAbort      = 0  ;
  int iEvent_Glob = 1  ;
  
  // ======================= BEGIN EVENT LOOP =======================

  // Begin event loop; generate until none left in input file.
  for (int iEvent=0  ; ; ++iEvent)
  {
    PROFILE_SCOPE("Event Loop") ;

    {
      PROFILE_SCOPE("Pythia.next") ;
      // Generate events, and check whether generation failed.
      if (!pythia.next())
      {
        // If failure because reached end of file then exit event loop.
        if (pythia.info.atEndOfFile()) break ;

        // First few failures write off as "acceptable" errors, then quit.
        if (++iAbort < nAbort) continue ;
        break ;
      }

    }
    // ------------------------------Event info--------------------------
    int num_proc = req_threads ;
    iEvent_Glob = pr_id*(tot_num_events / num_proc) + iEvent + 1 ;

    // if ( pr_id == 0 && iEvent % 100 == 0)
    //   {
    //     double progress = (double) iEvent / ( (double) Tot_Num_Events / num_proc) ;
    //     showProgress(progress) ;
    //   }

    // Creating the ev object, by taking a pythia event and it's number
    ExEvent ev(iEvent_Glob, pythia.event) ;

    // Reporting events
    if( Zaki::Vector::Exists(iEvent_Glob, print_ev_set) )
    {
      std::string event_rep_str(shared_file_char) ;
      event_rep_str = "Event_Report_"+event_rep_str + "_" +
                      std::to_string(iEvent_Glob)+".txt" ;
      ev.Print(event_rep_str) ;
    }

    //If Report_Taus (private) is true, it will report them
    std::string tau_rep_str(shared_file_char) ;
    tau_rep_str = "Tau_Report_" + tau_rep_str + ".txt" ;

    bool tau_report_bool = ( report_taus_flag || Zaki::Vector::Exists(ev.i(), report_taus_set) ) ;
    ev.Input("Report_Taus", tau_report_bool) ;

    // Find hadronic taus
    ev.FindHadTaus(tau_rep_str) ;

    // Leptons are the only particles to loop over.
    ParticleLST PartList = ev(lept_id_list) ;

    // //---------------------Setting the cut report filename------------------
    // std::string cut_file_str(shared_file_char) ;
    // cut_file_str = "Pythia_Cut_Report_" + cut_file_str + ".txt" ;

    // ------------------------------Applying Cuts--------------------------
    // There is a life-time issue with genjet, so we define it here. 
    // This way the jet information is accessible all the way through "run_cut"
    // i.e. can be accessed via recording functions.
    // As for the cut information, I'm not sure now. There might be an
    // issue there too. Since they access EV via a reference to EV 
    // inside the base class "Cut".   
    GenJet genjet(ev) ; //GenJet constructor

    // Number of cuts passed by an event
    int cut_counter  = RunCuts(ev, PartList, cut_list, shared_file_char) ;

    // +1 for the intial N_l check
    int tot_num_cuts = cut_list.size() + 1 ;

    // #pragma omp critical
    num_cuts_passed[iEvent_Glob-1] = cut_counter ;

    if (  cut_counter != tot_num_cuts ) continue ;

    // -------------------------------Binning------------------------------

    // Update the list of hadronic taus (keep the ones that passed all the cuts)
    // Needed for the binning procedure
    ev.UpdateHadTaus(PartList) ;

    #pragma omp critical
    BinEvents(ev, shared_file_char) ;

    }
  // ======================= END EVENT LOOP =======================
  // endwin();

}

//--------------------------------------------------------------
// The conditions is true when we want to continue the event
// loop to the next iteration, i.e. cut is not passed.
// So the conditions here are the logical negation of :
//       {">=", "<=", ">", "<", "=", "!="}
bool PHENO::Pheno::break_ev_loop(ParticleLST& prt_lst)
{

  for ( size_t i=0 ; i < req_states.size() ; ++i)
  {
    if ( req_states[i][0][1] == 1 )
      {if ( count(prt_lst, req_states[i][1]) <  req_states[i][0][0] )
        return true ;}

    else if ( req_states[i][0][1] == 2 )
      {if ( count(prt_lst, req_states[i][1]) >  req_states[i][0][0] )
        return true ;}

    else if ( req_states[i][0][1] == 3 )
      {if ( count(prt_lst, req_states[i][1]) <=  req_states[i][0][0] )
        return true ;}

    else if ( req_states[i][0][1] == 4 )
      {if ( count(prt_lst, req_states[i][1]) >=  req_states[i][0][0] )
        return true ;}

    else if ( req_states[i][0][1] == 5 )
      {if ( count(prt_lst, req_states[i][1]) !=  req_states[i][0][0] )
        return true ;}

    else if ( req_states[i][0][1] == 6 )
      {if ( count(prt_lst, req_states[i][1]) ==  req_states[i][0][0] )
        return true ;}
  }

  return false ;
}

//-------------------------------------------------------
// Cuts
int PHENO::Pheno::RunCuts(ExEvent& ev, ParticleLST& part_lst,
 std::vector<PHENO::CUTS::CutOptions> cut_lst, char* shared_file_char)
{
  PROFILE_SCOPE("Run Cuts") ;

  //------Setting the cut report filename--------
  std::string cut_file_str(shared_file_char) ;
  cut_file_str = "Pythia_Cut_Report_" + cut_file_str + ".txt" ;

  // std::string cut_name ;
  std::string cut_rep_title = "Report_Cut:" ;

  // Number of cuts that are passed by the event
  int cut_count = 0 ;

  // N_l cut: It checks how many of the input events pass
  // the required final state conditions
  if ( break_ev_loop(part_lst) ) return cut_count ;
    cut_count++ ;

  // Recording var's before any cuts
  Record(-1, part_lst) ;

  for( size_t i=0 ; i<cut_lst.size() ; ++i)
  {
    // the cut name:
    // cut_name = cut_lst[i][0] ;
    cut_rep_title += cut_lst[i].cutPtr->GetName() ;

    // std::shared_ptr<Cut> c1 = CutDict(&ev, cut_name) ;
    cut_lst[i].cutPtr->SetEventPtr(&ev) ;
    // std::shared_ptr<Cut> c1(cut_lst[i].cut->Clone()) ;
    // std::shared_ptr<Cut> c1(cut_lst[i].cut) ;


    // Checking if c1 is NULL pointer
    if ( !cut_lst[i].cutPtr )
    {
      if ( ev.i() == 1 )
        std::cerr<<"\n\n WARNING: Invalid cut input ignored! \n"<<std::flush ;
      cut_rep_title += "(ignored) > " ;
      cut_count++ ;
      continue ;
    }

    // Saving the pointer to current cut to the event
    ev.AddCutPtr(cut_lst[i].cutPtr) ;

    // Saving the name of the cut in the cut instance
    // c1->Cut::Input("Name:"+cut_name) ;

    bool cut_report_bool = ( report_cuts_flag || Zaki::Vector::Exists(ev.i(), report_cuts_set) ) ;
    if(cut_report_bool) cut_lst[i].cutPtr->Cut::Input(cut_rep_title+ ":" + cut_file_str) ;

    // // Read the input for each cut
    // for( size_t j=1 ; j<cut_lst[i].size() ; ++j)
    // {
    //   c1->Input(cut_lst[i][j]) ;
    // }

    // Applying cuts
    cut_lst[i].cutPtr->Apply(part_lst) ;

    cut_rep_title += " > " ;

    if ( break_ev_loop(part_lst) ) return cut_count ;
    cut_count++ ;

    // Update the list of particles that passed the cuts so far
    ev.UpdatePassedLepts(part_lst) ;

    // run fastjet
    if ( i == gen_jet_idx ) RunGenJet(ev, shared_file_char) ;

    Record(i, part_lst) ;

  }


  return cut_count ;
}

//-------------------------------------------------------
// Cut dictionary
// std::shared_ptr<Cut> PHENO::Pheno::CutDict(ExEvent* ev, std::string input)
// {

//   std::shared_ptr<Cut> pCut ;

//         if (input == "ID_Eff"  ) { pCut.reset( new IdEff(ev) )   ; }
//   else  if (input == "M2"      ) { pCut.reset( new M2Cut(ev) )   ; }
//   else  if (input == "PT"      ) { pCut.reset( new PtCut(ev) )   ; }
//   else  if (input == "PRap"    ) { pCut.reset( new PrapCut(ev))  ; }
//   else  if (input == "ISO"     ) { pCut.reset( new Isolation(ev)); }
//   else  if (input == "M4"      ) { pCut.reset( new M4Cut(ev) )   ; }
//   else  if (input == "OffZ"    ) { pCut.reset( new OffZCut(ev) ) ; }

//   return pCut ;

// }

//-------------------------------------------------------
// Runs fastjet and saves a list of jets, and reports if Report_Jets is true.
void PHENO::Pheno::RunGenJet(ExEvent& ev, char* shared_file_char)
{   
  PROFILE_FUNCTION() ;
  std::string jet_file_str(shared_file_char) ;

  bool jet_report_bool = ( report_jets_flag || Zaki::Vector::Exists(ev.i(), report_jets_set) ) ;
  if(jet_report_bool) { ev.GenJetPtr->Input( {"Report_Jets", jet_file_str} ) ; }

  for (size_t i=0 ; i<fastjet_commands.size() ; ++i)
    ev.GenJetPtr->Input(fastjet_commands[i]) ;

  ev.GenJetPtr->GenerateJets() ;
}
  
//-------------------------------------------------------
// Recording Values
void PHENO::Pheno::Record(size_t cut_idx, ParticleLST& part_lst)
{
  PROFILE_FUNCTION() ;
  // #pragma omp single
  rec_double.vars[cut_idx+1].resize( func_double.funcs[cut_idx+1].size() ) ;
  rec_vec_double.vars[cut_idx+1].resize( func_vec_double.funcs[cut_idx+1].size() ) ;
  rec_int.vars[cut_idx+1].resize( func_int.funcs[cut_idx+1].size() ) ;

  #pragma omp critical
  {
    // For double variables
    for (size_t j=0 ; j<func_double.funcs[cut_idx+1].size() ; ++j)
    {
      rec_var<double> tmp_var ;
      tmp_var.val = (*func_double.funcs[cut_idx+1][j].f)(part_lst) ;
      tmp_var.name = "_rec_" + func_double.funcs[cut_idx+1][j].name + "_" + file_LHE ;

      rec_double.vars[cut_idx+1][j].push_back( tmp_var ) ;
    }

    // For double vector variables
    for (size_t j=0 ; j<func_vec_double.funcs[cut_idx+1].size() ; ++j)
    {
      rec_var<std::vector<double> > tmp_var ;
      tmp_var.val = (*func_vec_double.funcs[cut_idx+1][j].f)(part_lst) ;
      tmp_var.name = "_rec_" + func_vec_double.funcs[cut_idx+1][j].name + "_" + file_LHE ;

      rec_vec_double.vars[cut_idx+1][j].push_back( tmp_var ) ;
    }

    // For int variables
    for (size_t j=0 ; j<func_int.funcs[cut_idx+1].size() ; ++j)
    {
      rec_var<int> tmp_var ;
      tmp_var.val = (*func_int.funcs[cut_idx+1][j].f)(part_lst) ;
      tmp_var.name = "_rec_" + func_int.funcs[cut_idx+1][j].name + "_" + file_LHE ;

      rec_int.vars[cut_idx+1][j].push_back( tmp_var ) ;
    }
  }

}

//-------------------------------------------------------
// Binner dictionary
// std::shared_ptr<Binner> PHENO::Pheno::bin_dict(std::string input)
// {
//   std::shared_ptr<Binner> pBinner ;

//     if (input == "STBinner"  ) { pBinner.reset( new STBinner() ) ; }

//   return pBinner ;

// }

//-------------------------------------------------------
// Initializing the binner classes
// void PHENO::Pheno::InitBinner()
// {

//   for( size_t i=0 ; i<bin_list.size() ; ++i)
//   {
//     // the binner name:
//     std::string binner_name = bin_list[i][0] ;

//     std::shared_ptr<Binner> bi = bin_dict(binner_name) ;

//     // Read the input for each binner
//     for( size_t j=1 ; j<bin_list[i].size() ; ++j)
//     {
//       bi->Input(bin_list[i][j]) ;
//     }

//     binner_ptr_lst.push_back(bi) ;
//   }
// }

//-------------------------------------------------------
// Binning the event
// Has to be within critical statement in parallel region!
void PHENO::Pheno::BinEvents(ExEvent& ev, char* shared_file_char)
{
  PROFILE_FUNCTION() ;

  std::string cut_file_str(shared_file_char) ;
  cut_file_str = "Pythia_Cut_Report_" + cut_file_str + ".txt" ;

  bool cut_report_bool = ( report_cuts_flag || Zaki::Vector::Exists(ev.i(), report_cuts_set) ) ;

 // Read the input for each binner
  for( size_t i=0 ; i<bin_list.size() ; ++i)
  {
    bin_list[i].binnerPtr->Input(ev) ;
    bin_list[i].binnerPtr->BinIt(cut_report_bool, cut_file_str) ;
  }

  num_ev_passed += ev.Weight() ;
}

//-------------------------------------------------------
// Reporting
void PHENO::Pheno::FinalReport()
{

  char tmp[100] ;
  sprintf(tmp, "Total number of events passing the cuts: %f.", num_ev_passed) ;
  Z_LOG_INFO(tmp) ;


  // .....................................
  // Output the recorded values
  // .....................................
  // Double values
  for (size_t i=0 ; i<rec_double.vars.size() ; ++i)
    {
      for (size_t j=0 ; j<rec_double.vars[i].size() ; ++j)
      {
        SaveVec(rec_double.vars[i][j]) ;
      }
    }
  // Double vector values
  for (size_t i=0 ; i<rec_vec_double.vars.size() ; ++i)
    {
      for (size_t j=0 ; j<rec_vec_double.vars[i].size() ; ++j)
      {
        SaveVec(rec_vec_double.vars[i][j]) ;
      }
    }
  // int values
  for (size_t i=0 ; i<rec_int.vars.size() ; ++i)
  {
    for (size_t j=0 ; j<rec_int.vars[i].size() ; ++j)
    {
      SaveVec(rec_int.vars[i][j]) ;
    }
  }
  // .....................................

  if(rep_num_cut_flag)
    Zaki::File::SaveVec(num_cuts_passed, "_rec_Num_Cuts_" + file_LHE);


  input_list.push_back("#------------------------------------------------------------------");
  if(rep_input_commands_flag)
    Zaki::File::SaveVec(input_list,  "_main_inputs_" + file_LHE);

  // Output for each binner
  for( size_t i=0 ; i<bin_list.size() ; ++i)
   { bin_list[i].binnerPtr->Report(file_LHE) ; }
}

//-------------------------------------------------------
// Adding user defined cuts
void PHENO::Pheno::Input(CUTS::CutOptions in_options) 
{
  // Saving the command in a list
  input_list.push_back(in_options.GetStrForm()) ;

  cut_list.push_back(in_options) ;

  //................Recording funcs & vars................
  func_int.funcs.push_back({}) ;
  func_double.funcs.push_back({}) ;
  func_vec_double.funcs.push_back({}) ;

  rec_int.vars.push_back({}) ;
  rec_double.vars.push_back({}) ;
  rec_vec_double.vars.push_back({}) ;
  //......................................................
}

//-------------------------------------------------------
// Adding user defined binners
void PHENO::Pheno::Input(BinnerOptions in_options) 
{
  // Saving the command in a list
  input_list.push_back(in_options.GetStrForm()) ;

  bin_list.push_back(in_options) ;
}

//==============================================================

//==============================================================
//                    explicit instantiations
//==============================================================
template void PHENO::Pheno::Input<double>(std::string, double (*func)(ParticleLST&) ) ;

//==============================================================