//
//
// Original Author:  Sungwoong Cho (sungwoong.cho@cern.ch), Jaehoon Lim (jae.hoon.lim@cern.ch)
//                   588 R-012, +41 22 76 73278
//         Created:  2015 Oct 14
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <stdlib.h>
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

using namespace std;

class KODEL {
public :
   TTree          *fChain;
   Int_t          fCurrent;
   Int_t          EventNumber;
   Int_t          number_of_hits;
   vector<int>    *TDC_channel;
   vector<float>  *TDC_TimeStamp;
   TBranch        *b_EventNumber;
   TBranch        *b_number_of_hits;
   TBranch        *b_TDC_channel;
   TBranch        *b_TDC_TimeStamp;

   KODEL(TTree *tree=0);
   virtual ~KODEL();
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Input(string f_name, string cut_strip, string cut_partition, string cut_time);
   virtual void     ReadConfig(string mapconfig);
   virtual void     Loop();
   virtual float    DeltaStrip(int strip1, int strip2, int partition);
   virtual float    DeltaPartition(int strip1, int strip2, int partition);

   string             file_name;
   float              CUT_Delta_Strip;          // Strip Distance
   float              CUT_Delta_Partition;      // Partition Distance
   float              CUT_Cluster_time;         // Time Difference
   map<int,int>       TDCtoSTRIP;
   map<int,string>    TDCtoNAME;
   map<int,float>     TDCtoCAL;
   map<string,int>    StripPerPartition;
   map<string,int>    TotalStrip;
   map<string,string> PartitionCut;
};

KODEL::KODEL(TTree *tree) : fChain(0) 
{
   if (tree == 0) {
       cerr << endl << "\tERROR : Check Input Root File" << endl << endl;
       exit(EXIT_FAILURE);    
   }
   Init(tree);
   gROOT->ProcessLine("#include <vector>");
}

KODEL::~KODEL()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Long64_t KODEL::LoadTree(Long64_t entry)
{
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
   }
   return centry;
}


void KODEL::Init(TTree *tree)
{
   TDC_channel = 0;
   TDC_TimeStamp = 0;
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("EventNumber", &EventNumber, &b_EventNumber);
   fChain->SetBranchAddress("number_of_hits", &number_of_hits, &b_number_of_hits);
   fChain->SetBranchAddress("TDC_channel", &TDC_channel, &b_TDC_channel);
   fChain->SetBranchAddress("TDC_TimeStamp", &TDC_TimeStamp, &b_TDC_TimeStamp);
}

float KODEL::DeltaStrip(int strip1, int strip2, int partition)
{  
   int strip1x,strip1y,strip2x,strip2y;
   strip1x = (strip1-1)/partition;
   strip1y = (strip1-1)%partition;
   strip2x = (strip2-1)/partition;
   strip2y = (strip2-1)%partition;
   float deltac;
   deltac = sqrt((strip1x - strip2x)*(strip1x - strip2x)+(strip1y - strip2y)*(strip1y - strip2y));
   return deltac;
}

float KODEL::DeltaPartition(int strip1, int strip2, int partition)
{  
   int strip1x,strip2x;
   strip1x = (strip1-1)/partition;
   strip2x = (strip2-1)/partition;
   float deltap;
   deltap = strip1x - strip2x;
   if (deltap < 0) deltap = -1 * deltap;
   return deltap;
}

void KODEL::Input(string f_name, string cut_strip, string cut_partition, string cut_time)
{
   cout << "Input File : " << f_name << endl;
   f_name.replace(f_name.find(".root"),5,"-KODEL.root");
   file_name = f_name;
   cout << "output File : " << file_name << endl;
   CUT_Delta_Strip = atof(cut_strip.c_str());
   CUT_Delta_Partition = atof(cut_partition.c_str());
   CUT_Cluster_time = atof(cut_time.c_str());
   cout << "Cut Value for Clustering - Strip : " << CUT_Delta_Strip << " / Partition : " << CUT_Delta_Partition << " / Time : " << CUT_Cluster_time << endl;
}

void KODEL::ReadConfig(string mapconfig){

   TDCtoSTRIP.clear();
   TDCtoNAME.clear();
   TDCtoCAL.clear();
   StripPerPartition.clear();
   TotalStrip.clear();
   PartitionCut.clear();

   ifstream mapfile( mapconfig );
   if(!mapfile.is_open()) {
       cerr << endl << "\tERROR : Check MAP config File" << endl << endl;
       exit(EXIT_FAILURE);
   }

   mapconfig.replace(mapconfig.find("MAP"),3,"CAL");
   ifstream calfile( mapconfig );
   if(!calfile.is_open())  {
       cerr << endl << "\tERROR : Check CAL config File" << endl << endl;
       exit(EXIT_FAILURE);
   }

   string temp_string;
   string s_line;

   vector<int> TDC_STRIP;
   vector<string> NAME_PARTITION;
   vector<float> TDC_CAL;

   while (calfile) {
      if (!getline( calfile, s_line )) break;
      istringstream ss_line( s_line );
      TDC_CAL.clear();
      while (ss_line) {
         if (s_line.find("/") == 0) break;
         if (!getline( ss_line, temp_string, '\t' )) break;
            TDC_CAL.push_back(atof(temp_string.c_str()));
      }
      if(TDC_CAL.size()>1) TDCtoCAL[(int)TDC_CAL.at(0)] = TDC_CAL.at(1);
   }

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
         if(TotalStrip[NAME_PARTITION.at(0)] < TDC_STRIP.at(2)) TotalStrip[NAME_PARTITION.at(0)] = TDC_STRIP.at(2);
         if(TotalStrip[NAME_PARTITION.at(0)] < TDC_STRIP.at(3)) TotalStrip[NAME_PARTITION.at(0)] = TDC_STRIP.at(3);
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
         PartitionCut[NAME_PARTITION.at(0)]      = NAME_PARTITION.at(2);
         TotalStrip[NAME_PARTITION.at(0)] = 0;
      }
   }
   //cout << "Number of Chamber : " << StripPerPartition.size() << " / " << PartitionCut.size() << endl;
   //cout << "Number of Strip   : " << TDCtoCAL.size() << " / " << TDCtoSTRIP.size() << " / " << TDCtoNAME.size() << endl;
}

void KODEL::Loop()
{

   /////////////////////////////
   // KODEL Cluster Variables //
   /////////////////////////////

   bool debug_cout = false;
   //    CUT_Delta_Strip;                // Strip Distance (strip)
   //    CUT_Delta_Partition;            // Partition Distance
   //    CUT_Cluster_time;               // Time Difference (ns)
   float CUT_Cluster_Mean_time = 9999.0; // Time Difference from Mean Time (ns)
   float CUT_Time_Start = -9999.0;       // for Test (We are using T_fastest Cut)
   float CUT_Time_End = 9999.0;

   if (fChain == 0) return;
   TFile *OutputFile = new TFile(Form("%s", file_name.c_str()),"RECREATE");
   // Output file name
   TTree *KODEL_Tree   = new TTree("KODEL_Tree","KODEL_Tree");
   // KODEL tree

   int Info_Event_Number, Info_Number_of_Hits;

   vector<int> TDC_Channel, Strip_Number, Strip_ClusterIndex;
   vector<float> TDC_Time, TDC_Time_Cal;
   vector<string> Strip_DEC_Type;

   vector<int> DEC_NumHits, DEC_NumCluster;
   vector<string> DEC_Type;

   vector<int> Cluster_Index, Cluster_NumHits, Cluster_Strip_F;
   vector<float> Cluster_Time_F, Cluster_Time_L, Cluster_Time_M, Cluster_X, Cluster_Y;
   vector<string> Cluster_DEC_Type;

   KODEL_Tree->Branch("Info_Event_Number",   &Info_Event_Number,        "Info_Event_Number/I");
   KODEL_Tree->Branch("Info_Number_of_Hits", &Info_Number_of_Hits,      "Info_Number_of_Hits/I");

   // Each events
   KODEL_Tree->Branch("TDC_Channel",         &TDC_Channel);
   KODEL_Tree->Branch("TDC_Time",            &TDC_Time);
   KODEL_Tree->Branch("TDC_Time_Cal",        &TDC_Time_Cal);
   KODEL_Tree->Branch("Strip_DEC_Type",      &Strip_DEC_Type);
   KODEL_Tree->Branch("Strip_Number",        &Strip_Number);
   KODEL_Tree->Branch("Strip_ClusterIndex",  &Strip_ClusterIndex);

   // Each Detector
   KODEL_Tree->Branch("DEC_Type",            &DEC_Type);
   KODEL_Tree->Branch("DEC_NumHits",         &DEC_NumHits);
   KODEL_Tree->Branch("DEC_NumCluster",      &DEC_NumCluster);

   // Each Cluster
   KODEL_Tree->Branch("Cluster_DEC_Type",    &Cluster_DEC_Type);                        // Detector Name
   KODEL_Tree->Branch("Cluster_Index",       &Cluster_Index);                           // Cluster Index
   KODEL_Tree->Branch("Cluster_NumHits",     &Cluster_NumHits);                         // Number of Hits for Cluster
   KODEL_Tree->Branch("Cluster_Time_F",      &Cluster_Time_F);                          // First Hit Time of Cluster (Calibrated)
   KODEL_Tree->Branch("Cluster_Time_L",      &Cluster_Time_L);                          // Last Hit Time of Cluster (Calibrated)
   KODEL_Tree->Branch("Cluster_Time_M",      &Cluster_Time_M);                          // Mean Time of Cluster (Calibrated)
   KODEL_Tree->Branch("Cluster_X",           &Cluster_X);                               // Cluster X Position (Section A = 0)
   KODEL_Tree->Branch("Cluster_Y",           &Cluster_Y);                               // Cluster Y Position (Strip 1 = 0)
   KODEL_Tree->Branch("Cluster_Strip_F",     &Cluster_Strip_F);                         // First Hit Strip of Cluster

   map<float, int> sort_time;   // time                / event index           : time sorting
   map<int, int> sort_cluster;  // space cluster index / event index           : checking number of cluster
   map<int, int> sort_strip;    // strip               / event index           : space clustering
   map<int, int> time_cluster;  // event index         / time cluster index    : saving time cluster
   map<int, int> space_cluster; // event index         / space cluster index   : saving space cluster (for each event)
   map<int, int> sort_space;    // event index         / space cluster index   : saving space cluster (for each time cluster)
   map<int, int> all_cluster;   // event index         / space cluster index   : saving space cluster
   bool use_ParA, use_ParB, use_ParC, use_ParD, DuplicatedStrip;
   int N_partition, N_time_cluster, N_time_cluster_size, cluster_change, MAX_space_cluster, clustersize, cluster_strip_f, NumHits_temp;
   float time_cluster_mean, cluster_time_f, cluster_time_l, cluster_time_m, cluster_x, cluster_y, time_cal_temp, temp_time;
   string CutPartition;
   map<float, int>::iterator it_time_temp;

   Long64_t nentries = fChain->GetEntriesFast();
   Long64_t nbytes = 0, nb = 0;

   for (Long64_t jentry=0; jentry<nentries;jentry++) { // XXX:Entry Loop Start
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;

      ////////////////////////////////
      // KODEL Tree main code start //
      ////////////////////////////////
      if (jentry%(nentries/10) == 0 && jentry != 0) cout << "running " << jentry << "th event (" << jentry/(nentries/100) << "%)" << endl;
      if (debug_cout) cout << endl << endl << "running " << jentry << "th event" << endl;

      Info_Event_Number = EventNumber;
      Info_Number_of_Hits = number_of_hits; 
      TDC_Channel.clear();
      TDC_Time.clear();
      TDC_Time_Cal.clear();
      Strip_DEC_Type.clear();
      Strip_Number.clear();
      Strip_ClusterIndex.clear();
      all_cluster.clear();
      time_cal_temp = 0;

      ///////////////////////////////////////////
      // Time Calibration && Cut on each strip //
      ///////////////////////////////////////////
      if (debug_cout) cout << "Time Calibration Start" << endl;
      for(int i_Channel=0; i_Channel<TDC_channel->size(); ++i_Channel){
         if (TDC_channel->at(i_Channel) == 0) continue;
         if (TDCtoCAL.find( TDC_channel->at(i_Channel) ) == TDCtoCAL.end()) {
            //cerr << endl << "\tERROR : Check CAL config File";
            //cerr << endl << "\tTDC channel : " << TDC_channel->at(i_Channel) << endl << endl;
            //exit(EXIT_FAILURE);
            continue;
         }
         time_cal_temp = TDC_TimeStamp->at(i_Channel) - TDCtoCAL[TDC_channel->at(i_Channel)];
         if (time_cal_temp > CUT_Time_Start && time_cal_temp < CUT_Time_End) { // Timing Cut

            N_partition = (TDCtoSTRIP[TDC_channel->at(i_Channel)]-1)/StripPerPartition[TDCtoNAME[TDC_channel->at(i_Channel)]]; // Partiton A = 0
            CutPartition = PartitionCut[TDCtoNAME[TDC_channel->at(i_Channel)]];
            CutPartition.find("A") != std::string::npos ? use_ParA = true : use_ParA = false; 
            CutPartition.find("B") != std::string::npos ? use_ParB = true : use_ParB = false; 
            CutPartition.find("C") != std::string::npos ? use_ParC = true : use_ParC = false; 
            CutPartition.find("D") != std::string::npos ? use_ParD = true : use_ParD = false; 
            if ((N_partition == 0 && use_ParA == true) || (N_partition == 1 && use_ParB == true) ||
                (N_partition == 2 && use_ParC == true) || (N_partition == 3 && use_ParD == true) ) { // Partition Cut

               TDC_Channel.push_back(    TDC_channel->at(i_Channel)             );
               TDC_Time.push_back(       TDC_TimeStamp->at(i_Channel)           );
               TDC_Time_Cal.push_back(   time_cal_temp                          );
               Strip_DEC_Type.push_back( TDCtoNAME[TDC_channel->at(i_Channel)]  );
               Strip_Number.push_back(   TDCtoSTRIP[TDC_channel->at(i_Channel)] );

               if (debug_cout) cout << "TDC         : " << TDC_channel->at(i_Channel) << endl;
               if (debug_cout) cout << "STRIP       : " << TDCtoSTRIP[TDC_channel->at(i_Channel)] << "-" << TDCtoNAME[TDC_channel->at(i_Channel)] << endl;
               if (debug_cout) cout << "N_partition : " << N_partition << endl;
               if (debug_cout) cout << "Time        : " << TDC_TimeStamp->at(i_Channel) << " / " << time_cal_temp << endl;

            }
         }    
      }
      if (debug_cout) cout << "Time Calibration End" << endl;

      ///////////////////
      // KODEL Cluster //
      ///////////////////
      if (debug_cout) cout << "KODEL Cluster Start" << endl;
      DEC_Type.clear();
      DEC_NumHits.clear();
      DEC_NumCluster.clear();
      Cluster_DEC_Type.clear();
      Cluster_Index.clear();
      Cluster_NumHits.clear();
      Cluster_Time_F.clear();
      Cluster_Time_L.clear();
      Cluster_Time_M.clear();
      Cluster_X.clear();
      Cluster_Y.clear();
      Cluster_Strip_F.clear();

     if (debug_cout) cout << "DEC Loop Start" << endl;
      map<string,int>::iterator DEC_NAME = StripPerPartition.begin();
      for (int DEC_TYPE_LOOP = 0; DEC_TYPE_LOOP < StripPerPartition.size(); ++DEC_TYPE_LOOP, ++DEC_NAME) { // XXX:DEC TYPE Loop Start
         if (debug_cout) cout << "DEC TYPE : " << DEC_NAME->first << endl;
         NumHits_temp = 0;

         ///////////////////////////////////////
         // Calibrated Time in Acending Order //
         ///////////////////////////////////////
         if (debug_cout) cout << "Time Map Start" << endl;
         sort_time.clear();
         for(int i_time=0; i_time<TDC_Time.size(); i_time++){
            if (Strip_DEC_Type[i_time] == DEC_NAME->first) {
               temp_time = TDC_Time_Cal[i_time];
               while (sort_time.find( temp_time ) != sort_time.end()) {
                  temp_time+=0.001;
                  if(debug_cout) cout << "While : " << temp_time << endl;
               }
               sort_time[temp_time] = i_time;
               if (debug_cout) cout << i_time << " - " << TDC_Time_Cal[i_time] << "(" << temp_time << ")" << endl;
               ++NumHits_temp;
            }
         } // sort_time[Calibrated Time] = Index
         if (debug_cout) cout << "Time Map End" << endl;

         //////////////////
         // Time Cluster //
         //////////////////
         if (debug_cout) cout << "Time Cluster Start" << endl;
         N_time_cluster = 0;
         N_time_cluster_size = 0;
         time_cluster_mean = 0.0;
         time_cluster.clear();
         for (map<float,int>::iterator it_t=sort_time.begin(); it_t != sort_time.end(); ++it_t){
            if (it_t == sort_time.begin()) {
               ++N_time_cluster;
               N_time_cluster_size = 1;
               time_cluster_mean = it_t->first;
            } else {
               it_time_temp = it_t;
               --it_time_temp;
               if (it_time_temp->first + CUT_Cluster_time < it_t->first
                   || time_cluster_mean/N_time_cluster_size + CUT_Cluster_Mean_time < it_t->first) {
                  ++N_time_cluster;
                  N_time_cluster_size = 1;
                  time_cluster_mean = it_t->first;
               } else {
                   ++N_time_cluster_size;
                   time_cluster_mean += it_t->first;
               }
            }
            if (debug_cout) cout << "TIME : " << it_t->first << " | " << N_time_cluster << endl;
            time_cluster[it_t->second] = N_time_cluster;
         } // time_cluster[Index] = Index of Time Cluster
         if (debug_cout) cout << "Time Cluster End" << endl;

         ///////////////////
         // Space Cluster //
         ///////////////////
         if (debug_cout) cout << "Space Cluster Start" << endl;
         MAX_space_cluster = 0;
         space_cluster.clear();
         DuplicatedStrip = false;
         for (int t=1; t<=N_time_cluster; ++t){ // XXX:Time Cluster Loop Start

            //////////////////////////////
            // Space Cluster Initialize // 
            //////////////////////////////
            sort_space.clear();
            sort_strip.clear();
            for (map<int,int>::iterator it_tc=time_cluster.begin(); it_tc != time_cluster.end(); ++it_tc){
               if (it_tc->second == t) {
                  if (sort_strip.find( Strip_Number[it_tc->first] ) == sort_strip.end()) {
                      sort_strip[Strip_Number[it_tc->first]] = it_tc->first;
                  } else {
                     DuplicatedStrip = true;
                  }
                  sort_space[it_tc->first] = 999;
               }
            } // sort_strip[Strip Number] = Index
              // sort_space[Index] = 999

            cluster_change = 1;
            while (cluster_change != 0) { // XXX:Space Cluster Loop Start
               cluster_change = 0;

               /////////////////////////////
               // Space Clustering Upward //
               /////////////////////////////
               if (debug_cout) cout << "Space Cluster Upward Start" << endl;
               for (map<int,int>::iterator it_ss1 = sort_strip.begin(); it_ss1 != sort_strip.end(); ++it_ss1){
                  if (sort_space[it_ss1->second] == 999) {
                     ++MAX_space_cluster;
                     sort_space[it_ss1->second] = MAX_space_cluster;
                  }
                  for (map<int,int>::iterator it_ss2=it_ss1; it_ss2 != sort_strip.end(); ++it_ss2){
                     if (DeltaStrip(it_ss1->first,it_ss2->first,DEC_NAME->second) <= CUT_Delta_Strip &&
                         DeltaPartition(it_ss1->first,it_ss2->first,DEC_NAME->second) <= CUT_Delta_Partition) {
                        if (sort_space[it_ss1->second] < sort_space[it_ss2->second]) {
                           sort_space[it_ss2->second] = sort_space[it_ss1->second];
                           ++cluster_change;
                        } else if (sort_space[it_ss1->second] > sort_space[it_ss2->second]) {
                           sort_space[it_ss1->second] = sort_space[it_ss2->second];
                           ++cluster_change;
                        }
                     }
                  }
               }
               if (debug_cout) cout << "Space Cluster Upward End" << endl;

               ///////////////////////////////
               // Space Clustering Downward //
               ///////////////////////////////
               if (debug_cout) cout << "Space Cluster Downward Start" << endl;
               for (map<int,int>::reverse_iterator it_ss3 = sort_strip.rbegin(); it_ss3 != sort_strip.rend(); ++it_ss3){
                  if (sort_space[it_ss3->second] == 999) {
                     ++MAX_space_cluster;
                     sort_space[it_ss3->second] = MAX_space_cluster;
                  }
                  for (map<int,int>::reverse_iterator it_ss4=it_ss3; it_ss4 != sort_strip.rend(); ++it_ss4){
                     if (DeltaStrip(it_ss3->first,it_ss4->first,DEC_NAME->second) <= CUT_Delta_Strip &&
                         DeltaPartition(it_ss3->first,it_ss4->first,DEC_NAME->second) <= CUT_Delta_Partition) {
                        if (sort_space[it_ss3->second] < sort_space[it_ss4->second]) {
                           sort_space[it_ss4->second] = sort_space[it_ss3->second];
                           ++cluster_change;
                        } else if (sort_space[it_ss3->second] > sort_space[it_ss4->second]) {
                           sort_space[it_ss3->second] = sort_space[it_ss4->second];
                           ++cluster_change;
                        }
                     }
                  }
               }
               if (debug_cout) cout << "Space Cluster Downward End" << endl;
            } // XXX:Space Cluster Loop End
              // sort_space[Index] = Index of Space Cluster
            if (debug_cout) cout << "Space Cluster End" << endl;

            ////////////////////////////
            // Check Duplicated Strip //
            ////////////////////////////
            if (debug_cout) cout << "Check Duplicated Strip Start" << endl;
            for (map<int,int>::iterator it_sc1 = sort_space.begin(); it_sc1 != sort_space.end(); ++it_sc1){
               if (DuplicatedStrip) {
                  if (it_sc1->second == 999) {
                     for (map<int,int>::iterator it_sc2 = sort_space.begin(); it_sc2 != sort_space.end(); ++it_sc2){
                        if ((TDC_channel->at(it_sc1->first) == TDC_channel->at(it_sc2->first)) && (it_sc2->second != 999)) {
                           it_sc1->second = it_sc2->second;
                        }
                     }
                  }
               }
               space_cluster[it_sc1->first] = it_sc1->second;
            } // space_cluster[Index] = Index of Space Cluster
            if (debug_cout) cout << "Check Duplicated Strip End" << endl;

         } // XXX:Time Cluster Loop End
         if (debug_cout) cout << "Time Cluster End" << endl;

         ////////////////////////////////
         // Remove Empty Cluster Index //
         ////////////////////////////////
         if (debug_cout) cout << "Remove Empty Start" << endl;
         sort_cluster.clear();
         for(map<int,int>::iterator it_sc3 = space_cluster.begin(); it_sc3 != space_cluster.end(); ++it_sc3){
            sort_cluster[it_sc3->second] = it_sc3->first;
         } // sort_cluster[Index of Space Cluster] = Index
         map<int,int>::iterator it_c = sort_cluster.begin();
         for(int index_cluster = 1; index_cluster <= sort_cluster.size(); ++index_cluster, ++it_c){
            for(map<int,int>::iterator it_sc4 = space_cluster.begin(); it_sc4 != space_cluster.end(); ++it_sc4){
               if(it_sc4->second == it_c->first) {
                  it_sc4->second = index_cluster;
               }
            }
         }
         if (debug_cout) cout << "Remove Empty End" << endl;

         ////////////////////////
         // Input Cluster Info //
         ////////////////////////
         if (debug_cout) cout << "Input Start" << endl;
         for (int cluster_input = 1; cluster_input<=sort_cluster.size(); ++cluster_input) { // XXX:Cluster Input Loop Start
            clustersize = 0;
            cluster_time_f = 9999.0;
            cluster_time_l = -9999.0;
            cluster_time_m = 0.0;
            cluster_x = 0.0;
            cluster_y = 0.0;
            cluster_strip_f = 0;
            for(map<int,int>::iterator it_input = space_cluster.begin(); it_input != space_cluster.end(); ++it_input){
               if (cluster_input == it_input->second) {
                  all_cluster[it_input->first] = it_input->second;
                  ++clustersize;
                  cluster_time_m += TDC_Time_Cal[it_input->first];
                  if (cluster_time_f > TDC_Time_Cal[it_input->first]) { cluster_time_f  = TDC_Time_Cal[it_input->first];
                                                                        cluster_strip_f = Strip_Number[it_input->first]; }
                  if (cluster_time_l < TDC_Time_Cal[it_input->first]) { cluster_time_l  = TDC_Time_Cal[it_input->first]; }
                  cluster_x += (Strip_Number[it_input->first]-1)/DEC_NAME->second;
                  cluster_y += (Strip_Number[it_input->first]-1)%DEC_NAME->second;
                  if (debug_cout) cout << "Index : " << it_input->first << " / Time : " << TDC_Time_Cal[it_input->first] <<  " / Strip : " << Strip_Number[it_input->first] << endl;
               }
            }
            if (clustersize != 0) {
               cluster_time_m /= clustersize;
               cluster_x /= clustersize;
               cluster_y /= clustersize;
               Cluster_DEC_Type.push_back(DEC_NAME->first);
               Cluster_Index.push_back(cluster_input);
               Cluster_NumHits.push_back(clustersize);
               Cluster_Time_F.push_back(cluster_time_f);
               Cluster_Time_L.push_back(cluster_time_l);
               Cluster_Time_M.push_back(cluster_time_m);
               Cluster_X.push_back(cluster_x);
               Cluster_Y.push_back(cluster_y);
               Cluster_Strip_F.push_back(cluster_strip_f);
            } else {
               cout << DEC_NAME->first << " : " << cluster_input << "th Cluster Size : 0 ERROR" << endl;
            }
         } // XXX:Cluster Input Loop End     
         if (debug_cout) cout << "Input End" << endl;

         DEC_Type.push_back(DEC_NAME->first);
         DEC_NumHits.push_back(NumHits_temp);
         DEC_NumCluster.push_back(sort_cluster.size());

      } // XXX:DEC TYPE Loop end
      if (debug_cout) cout << "DEC Loop End" << endl;

      /////////////////////////
      // Input Cluster Index //
      /////////////////////////
      if (all_cluster.size() != TDC_Channel.size()) {
         cout << jentry << "th ClusterIndex ERROR : " << all_cluster.size() << " | " << TDC_Channel.size() << endl;
      } else {
         for (int index_input = 0; index_input<all_cluster.size(); ++index_input) {
            Strip_ClusterIndex.push_back(all_cluster[index_input]);
         }
      }

      ///////////////
      // Tree Fill //
      ///////////////
      KODEL_Tree->Fill();

   } // XXX:Entry Loop end
   
   ///////////////////////
   // Write Output File //
   ///////////////////////
   KODEL_Tree->Write();
   OutputFile->Close();

}


int main(int argc, char **argv){

   if(argc != 6) {
       cerr << endl << "\tERROR : Check Input arguments" << endl;
       cerr << "\t" << argv[0] << " InputRootFile MAPConfigFile Delta_Strip Delta_Partition Delta_Time" << endl << endl; 
       exit(EXIT_FAILURE);
   }
   TFile *inputfile = new TFile(Form("%s",argv[1]));
   if(!inputfile->IsOpen()) {
       cerr << endl << "\tERROR : Check Input Root File" << endl << endl;
       exit(EXIT_FAILURE);
   }
   TTree *inputtree = (TTree*)inputfile->Get("RAWData");
   KODEL KODELCluster(inputtree);
   KODELCluster.Input(argv[1],argv[3],argv[4],argv[5]);
   KODELCluster.ReadConfig(argv[2]);
   cout << "Total Entries : " << inputtree->GetEntriesFast() << endl;
   KODELCluster.Loop();

}
