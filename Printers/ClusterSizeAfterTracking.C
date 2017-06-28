//
//
// Original Author:  Sungwoong Cho (sungwoong.cho@cern.ch), Kwangrok Ryoo (kwangrok21@naver.com)
//         Rewrite:  Jaehoon Lim (jae.hoon.lim@cern.ch)
//                   588 R-012, +41 22 76 73278
//         Created:  2017 Jun 27
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <stdlib.h>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>

using namespace std;

int main(int argc, char **argv){

   // FIXME ////////////////
   int StreamerMode = 6;
   /////////////////////////

   if(argc != 2 && argc != 3) {
       cerr << endl << "\tERROR : Check Input arguments" << endl;
       cerr << "\t" << argv[0] << " LastHVPointTRKFile Detector" << endl << endl;
       cerr << "\tFor KODEL Chamber" << endl;
       cerr << "\t" << argv[0] << " LastHVPointTRKFile" << endl << endl;
       exit(EXIT_FAILURE);
   }

   string detector;
   if(argc == 3){
      detector = argv[2];
      for(string::size_type pos = 0; (pos = detector.find("t", pos)) != string::npos;){
         detector.replace(pos, 1, "T");
         pos += 1;
      }
      for(string::size_type pos = 0; (pos = detector.find("s", pos)) != string::npos;){
         detector.replace(pos, 1, "S");
         pos += 1;
      }
   } else {
      detector = "KODEL";
   }

   cout << endl << "Detector " << detector << endl;

   TFile *f_daq[20];
   TTree *t_daq[20];
   vector<string> *Cluster_DEC_Type = 0;
   vector<int>    *Cluster_NumHits = 0;
   vector<int>    *Cluster_TrackIndex = 0;
   vector<int>    *Track_Index = 0;
   vector<bool>   *Track_isMuonTrack = 0;
   Long64_t nentries = 0;
   double temp_size = 0;
   double Cluster_entries = 0;
   double Streamer_entries = 0;

   string last_hv_root = argv[1];
   int HV_point = (last_hv_root.find("HV"))+2;
   int daq_pos = (last_hv_root.find("DAQ"))-1;
   int num_HV = atoi(last_hv_root.substr(HV_point,daq_pos-HV_point).c_str());

   for(int i=0; i<num_HV; ++i){
      HV_point = (last_hv_root.find("HV"))+2;
      daq_pos = (last_hv_root.find("DAQ"))-1;
      f_daq[i] = new TFile(last_hv_root.replace(HV_point,daq_pos-HV_point,Form("%d",i+1)).c_str());
      if(!f_daq[i]->IsOpen()) {
         cerr << endl << "\tERROR : Check Input Root File : " << last_hv_root << endl << endl;
         exit(EXIT_FAILURE);
      }
      t_daq[i] = (TTree*)f_daq[i]->Get("KODEL_Tree");
      t_daq[i]->SetBranchAddress("Cluster_DEC_Type",&Cluster_DEC_Type);
      t_daq[i]->SetBranchAddress("Cluster_NumHits",&Cluster_NumHits);
      t_daq[i]->SetBranchAddress("Cluster_TrackIndex",&Cluster_TrackIndex);
      t_daq[i]->SetBranchAddress("Track_Index",&Track_Index);
      t_daq[i]->SetBranchAddress("Track_isMuonTrack",&Track_isMuonTrack);

      nentries = t_daq[i]->GetEntriesFast();
      TH1D* h_ClusterSize = new TH1D("h_ClusterSize","Cluster Size",200,0,200);
      TH1D* h_Streamer = new TH1D("h_Streamer","Streamer Mode",200,0,200);
      Cluster_entries = 0;
      Streamer_entries = 0;
      for (Long64_t jentry=0; jentry<nentries;jentry++) {
         t_daq[i]->GetEntry(jentry);
         for(int i_Track=0; i_Track<Track_Index->size(); ++i_Track){
            if(!Track_isMuonTrack->at(i_Track)) continue;
            for(int i_Cluster=0; i_Cluster<Cluster_NumHits->size(); ++i_Cluster){
               if(Track_Index->at(i_Track) != Cluster_TrackIndex->at(i_Cluster)) continue;
               if((Cluster_DEC_Type->at(i_Cluster)).find(detector) == std::string::npos) continue;
               temp_size = Cluster_NumHits->at(i_Cluster);
               h_ClusterSize->Fill(temp_size);
               if(temp_size > StreamerMode) h_Streamer->Fill(temp_size);
            }
         }
      }
      Cluster_entries = h_ClusterSize->GetEntries();
      Streamer_entries = h_Streamer->GetEntries();
     
      cout << last_hv_root << " - Entreis (Streamer) : " << Cluster_entries << " (" << Streamer_entries << ")" << endl; 

      for(int cout_i=0;cout_i<last_hv_root.length();++cout_i) cout << " ";
      cout << " - Mean Muon Cluster Size : ";
      if(Cluster_entries == 0) {cout << "No Cluster" << endl;}
      else {cout << h_ClusterSize->GetMean() << " +/- " << h_ClusterSize->GetRMS()/sqrt(1.*Cluster_entries) << endl;}

      for(int cout_i=0;cout_i<last_hv_root.length();++cout_i) cout << " ";
      cout << " - Streamer Probaility : ";
      if(Streamer_entries == 0) {cout << "No Streamer Mode" << endl;}
      else{cout << 100.*Streamer_entries/Cluster_entries << " +/- " << 100.*sqrt(Streamer_entries/Cluster_entries*(1.-(Streamer_entries/Cluster_entries))/Cluster_entries) << " [%]" << endl << endl;}

      delete h_ClusterSize;
      delete h_Streamer;
   }
}
