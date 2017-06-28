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
   double KODEL_Partition_Area[3] = {4668.68, 3420.00, 2984.19}; // A, B, C
   double KODEL_Partition_Area_All = 11072.87;
   double Trigger_Window = 10000; // 600 for Efficiency Scan, 10000 for Rate Scan
   /////////////////////////

   if(argc != 3 && argc != 5) {
       cerr << endl << "\tERROR : Check Input arguments" << endl;
       cerr << "\t" << argv[0] << " LastHVPointKODELFile Detector Partition PartitionArea" << endl << endl;
       cerr << "\tFor KODEL Chamber" << endl;
       cerr << "\t" << argv[0] << " LastHVPointKODELFile Partition" << endl << endl;
       exit(EXIT_FAILURE);
   }

   string detector;
   char* partition;
   double pararea;
   if(argc == 5){
      detector = argv[2];
      partition = argv[3];
      pararea = atof(argv[4]);
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
      partition = argv[2];
   }

   bool AllPartition = false;
   if(int(*partition) > 96) *partition &= ~' '; // Capitalize char
   if(!strcmp(partition,"All") || !strcmp(partition,"ALL")) AllPartition = true;
   if(argc == 3) {
      if(AllPartition) {
         pararea = KODEL_Partition_Area_All;
      } else {
         pararea = KODEL_Partition_Area[int(*partition-'A')];
      }    
   }

   if(pararea <= 0){
       cerr << endl << "\tERROR : Partition Area" << endl << endl;
       exit(EXIT_FAILURE);
   }

   Trigger_Window /= pow(10,9);
   cout << endl << "Detector " << detector << " / Partition " << partition << " / Area " << pararea << " / TimeWindow " << Trigger_Window << endl;

   TFile *f_daq[20];
   TTree *t_daq[20];
   vector<string> *Cluster_DEC_Type = 0;
   vector<int>    *Cluster_NumHits = 0;
   vector<double> *Cluster_X = 0;
   Long64_t nentries = 0;
   double temp_x = 0;
   double NClusters = 0;

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
      t_daq[i]->SetBranchAddress("Cluster_X",&Cluster_X);

      nentries = t_daq[i]->GetEntriesFast();
      NClusters = 0;
      TH1D* h_ClusterSize = new TH1D("h_ClusterSize","Cluster Size",200,0,200);
      for (Long64_t jentry=0; jentry<nentries;jentry++) {
         t_daq[i]->GetEntry(jentry);
         for(int i_Cluster=0; i_Cluster<Cluster_NumHits->size(); ++i_Cluster){
            temp_x = Cluster_X->at(i_Cluster);
            if((Cluster_DEC_Type->at(i_Cluster)).find(detector) == std::string::npos) continue;
            if(!AllPartition && (temp_x > double(*partition)-64.5 || temp_x < double(*partition)-65.5)) continue;
            ++NClusters;
            h_ClusterSize->Fill(Cluster_NumHits->at(i_Cluster));
         }
      }
      cout << last_hv_root << " - Cluster Rate : " << 1.*NClusters/(pararea*Trigger_Window*nentries) << " +/- " << 1.*sqrt(NClusters)/(pararea*Trigger_Window*nentries) << " [Hz/cm^2]" << endl;
      for(int cout_i=0;cout_i<last_hv_root.length();++cout_i) cout << " ";
      cout << " - Mean Cluster Size : " << h_ClusterSize->GetMean() << " +/- " << h_ClusterSize->GetRMS()/sqrt(1.*h_ClusterSize->GetEntries()) << endl << endl; 
      delete h_ClusterSize;
   }
}
