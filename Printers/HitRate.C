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

using namespace std;

int main(int argc, char **argv){

   // FIXME ////////////////
   double KODEL_Partition_Area[3] = {4668.68, 3420.00, 2984.19}; // A, B, C
   double KODEL_Partition_Area_All = 11072.87;
   double Trigger_Window = 10000; // 600 for Efficiency Scan, 10000 for Rate Scan
   /////////////////////////

   if(argc != 4 && argc != 6) {
       cerr << endl << "\tERROR : Check Input arguments" << endl;
       cerr << "\t" << argv[0] << " LastHVPointDAQFile MAPConfigFile Detector Partition PartitionArea" << endl << endl;
       cerr << "\tFor KODEL Chamber" << endl;
       cerr << "\t" << argv[0] << " LastHVPointDAQFile MAPConfigFile Partition" << endl << endl;
       exit(EXIT_FAILURE);
   }

   ifstream mapfile( argv[2] );
   if(!mapfile.is_open()) {
       cerr << endl << "\tERROR : Check MAP config File" << endl << endl;
       exit(EXIT_FAILURE);
   }

   string detector;
   char* partition;
   double pararea;
   if(argc == 6){
      detector = argv[3];
      partition = argv[4];
      pararea = atof(argv[5]);
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
      partition = argv[3];
   }

   bool AllPartition = false;
   if(int(*partition) > 96) *partition &= ~' '; // Capitalize char
   if(!strcmp(partition,"All") || !strcmp(partition,"ALL")) AllPartition = true;
   if(argc == 4) {
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

   map<int,int>       TDCtoSTRIP;
   map<int,string>    TDCtoNAME;
   map<string,int>    StripPerPartition;
   TDCtoSTRIP.clear();
   TDCtoNAME.clear();
   StripPerPartition.clear();

   string temp_string;
   string s_line;

   vector<int> TDC_STRIP;
   vector<string> NAME_PARTITION;

   while (mapfile) {
      if (!getline( mapfile, s_line )) break;
      istringstream ss_line( s_line );

      if(isdigit(*(s_line.begin()))) {
         TDC_STRIP.clear();
         while (ss_line) {
            if (!getline( ss_line, temp_string, '\t' )) break;
            TDC_STRIP.push_back(atoi(temp_string.c_str()));
         }
         TDCtoSTRIP[TDC_STRIP.at(1)] = TDC_STRIP.at(3);
         TDCtoNAME[TDC_STRIP.at(1)] = NAME_PARTITION.at(0);
         while(TDC_STRIP.at(2) != TDC_STRIP.at(3)){
            if(TDCtoSTRIP.find(TDC_STRIP.at(0)) == TDCtoSTRIP.end()){
               TDCtoSTRIP[TDC_STRIP.at(0)] = TDC_STRIP.at(2);
               TDCtoNAME[TDC_STRIP.at(0)] = NAME_PARTITION.at(0);
            } else {
               cerr << "\tERROR : " << TDC_STRIP.at(0) << "/" << TDCtoSTRIP[TDC_STRIP.at(0)] << " vs " << TDC_STRIP.at(2) << endl;
            }
            (TDC_STRIP.at(0)>TDC_STRIP.at(1)?--(TDC_STRIP.at(0)):++(TDC_STRIP.at(0)));
            (TDC_STRIP.at(2)>TDC_STRIP.at(3)?--(TDC_STRIP.at(2)):++(TDC_STRIP.at(2)));
         }
      } else if(!(s_line.empty()) && s_line.find("/") != 0) {
         NAME_PARTITION.clear();
         while (ss_line){
            if (!getline( ss_line, temp_string, '\t' )) break;
            NAME_PARTITION.push_back(temp_string);
         }
         StripPerPartition[NAME_PARTITION.at(0)] = atoi((NAME_PARTITION.at(1)).c_str());
      }  
   }  

   TFile *f_daq[20];
   TTree *t_daq[20];
   vector<int> *TDC_channel = 0;
   vector<double> *TDC_TimeStamp = 0;
   Long64_t nentries = 0;
   int temp_ch = 0;
   int N_partition = 0;
   double NHits = 0;

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
      t_daq[i] = (TTree*)f_daq[i]->Get("RAWData");
      t_daq[i]->SetBranchAddress("TDC_channel",&TDC_channel);
      t_daq[i]->SetBranchAddress("TDC_TimeStamp",&TDC_TimeStamp);

      nentries = t_daq[i]->GetEntriesFast();
      NHits = 0;
      for (Long64_t jentry=0; jentry<nentries;jentry++) {
         t_daq[i]->GetEntry(jentry);
         for(int i_Channel=0; i_Channel<TDC_channel->size(); ++i_Channel){
            temp_ch = TDC_channel->at(i_Channel);
            if (temp_ch == 0) continue;
            if(TDCtoNAME[temp_ch].find(detector) == std::string::npos) continue;
            N_partition = (TDCtoSTRIP[temp_ch]-1)/StripPerPartition[TDCtoNAME[temp_ch]];
            if(!AllPartition && N_partition+65 != int(*partition)) continue;
            ++NHits;
         }
      }
      cout << last_hv_root << " - Hit Rate : " << 1.*NHits/(pararea*Trigger_Window*nentries) << " +/- " << 1.*sqrt(NHits)/(pararea*Trigger_Window*nentries) << " [Hz/cm^2]" << endl;
   }
}
