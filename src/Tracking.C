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
#include <TInterpreter.h>
#include <algorithm>

using namespace std;

class Tracking {

public :
   TTree           *fChain;
   Int_t           fCurrent;
   Int_t           Info_Event_Number;
   Int_t           Info_Number_of_Hits;
   vector<int>     *TDC_Channel;
   vector<float>   *TDC_Time;
   vector<float>   *TDC_Time_Cal;
   vector<string>  *Strip_DEC_Type;
   vector<int>     *Strip_Number;
   vector<int>     *Strip_ClusterIndex;
   vector<string>  *DEC_Type;
   vector<int>     *DEC_NumHits;
   vector<int>     *DEC_NumCluster;
   vector<string>  *Cluster_DEC_Type;
   vector<int>     *Cluster_Index;
   vector<int>     *Cluster_NumHits;
   vector<float>   *Cluster_Time_F;
   vector<float>   *Cluster_Time_L;
   vector<float>   *Cluster_Time_M;
   vector<float>   *Cluster_X;
   vector<float>   *Cluster_Y;
   vector<int>     *Cluster_Strip_F;
   TBranch         *b_Info_Event_Number;
   TBranch         *b_Info_Number_of_Hits;
   TBranch         *b_TDC_Channel;
   TBranch         *b_TDC_Time;
   TBranch         *b_TDC_Time_Cal;
   TBranch         *b_Strip_DEC_Type;
   TBranch         *b_Strip_Number;
   TBranch         *b_Strip_ClusterIndex;
   TBranch         *b_DEC_Type;
   TBranch         *b_DEC_NumHits;
   TBranch         *b_DEC_NumCluster;
   TBranch         *b_Cluster_DEC_Type;
   TBranch         *b_Cluster_Index;
   TBranch         *b_Cluster_NumHits;
   TBranch         *b_Cluster_Time_F;
   TBranch         *b_Cluster_Time_L;
   TBranch         *b_Cluster_Time_M;
   TBranch         *b_Cluster_X;
   TBranch         *b_Cluster_Y;
   TBranch         *b_Cluster_Strip_F;

   Tracking(TTree *tree=0);
   virtual ~Tracking();
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual void     Input(string f_name, string cut_x, string cut_y, string cut_time);
   virtual void     ReadConfig(string mapconfig, string setconfig);

   string             file_name;
   float              CUT_Delta_X;       // X-axis Distance
   float              CUT_Delta_Y;       // Y-axis Distance
   float              CUT_Delta_Time;    // Time Difference
   vector<string>     DEC_name;
   map<string,int>    DEC_rotation;
   map<string,int>    DEC_sizeX;
   map<string,int>    DEC_sizeY;
   map<string,int>    DEC_positionX;
   map<string,int>    DEC_positionY;
   map<string,int>    DEC_total;
   map<string,int>    DEC_partition;

};

Tracking::Tracking(TTree *tree) : fChain(0)
{
   if (tree == 0) {
       cerr << endl << "\tERROR : Check Input Root File" << endl << endl;
       exit(EXIT_FAILURE);
   }
   Init(tree);
   gROOT->ProcessLine("#include <vector>");

}

Tracking::~Tracking()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Long64_t Tracking::LoadTree(Long64_t entry)
{
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
   }
   return centry;
}

void Tracking::Init(TTree *tree)
{
   TDC_Channel = 0;
   TDC_Time = 0;
   TDC_Time_Cal = 0;
   Strip_DEC_Type = 0;
   Strip_Number = 0;
   Strip_ClusterIndex = 0;
   DEC_Type = 0;
   DEC_NumHits = 0;
   DEC_NumCluster = 0;
   Cluster_DEC_Type = 0;
   Cluster_Index = 0;
   Cluster_NumHits = 0;
   Cluster_Time_F = 0;
   Cluster_Time_L = 0;
   Cluster_Time_M = 0;
   Cluster_X = 0;
   Cluster_Y = 0;
   Cluster_Strip_F = 0;
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("Info_Event_Number", &Info_Event_Number, &b_Info_Event_Number);
   fChain->SetBranchAddress("Info_Number_of_Hits", &Info_Number_of_Hits, &b_Info_Number_of_Hits);
   fChain->SetBranchAddress("TDC_Channel", &TDC_Channel, &b_TDC_Channel);
   fChain->SetBranchAddress("TDC_Time", &TDC_Time, &b_TDC_Time);
   fChain->SetBranchAddress("TDC_Time_Cal", &TDC_Time_Cal, &b_TDC_Time_Cal);
   fChain->SetBranchAddress("Strip_DEC_Type", &Strip_DEC_Type, &b_Strip_DEC_Type);
   fChain->SetBranchAddress("Strip_Number", &Strip_Number, &b_Strip_Number);
   fChain->SetBranchAddress("Strip_ClusterIndex", &Strip_ClusterIndex, &b_Strip_ClusterIndex);
   fChain->SetBranchAddress("DEC_Type", &DEC_Type, &b_DEC_Type);
   fChain->SetBranchAddress("DEC_NumHits", &DEC_NumHits, &b_DEC_NumHits);
   fChain->SetBranchAddress("DEC_NumCluster", &DEC_NumCluster, &b_DEC_NumCluster);
   fChain->SetBranchAddress("Cluster_DEC_Type", &Cluster_DEC_Type, &b_Cluster_DEC_Type);
   fChain->SetBranchAddress("Cluster_Index", &Cluster_Index, &b_Cluster_Index);
   fChain->SetBranchAddress("Cluster_NumHits", &Cluster_NumHits, &b_Cluster_NumHits);
   fChain->SetBranchAddress("Cluster_Time_F", &Cluster_Time_F, &b_Cluster_Time_F);
   fChain->SetBranchAddress("Cluster_Time_L", &Cluster_Time_L, &b_Cluster_Time_L);
   fChain->SetBranchAddress("Cluster_Time_M", &Cluster_Time_M, &b_Cluster_Time_M);
   fChain->SetBranchAddress("Cluster_X", &Cluster_X, &b_Cluster_X);
   fChain->SetBranchAddress("Cluster_Y", &Cluster_Y, &b_Cluster_Y);
   fChain->SetBranchAddress("Cluster_Strip_F", &Cluster_Strip_F, &b_Cluster_Strip_F);
}

void Tracking::Input(string f_name, string cut_x, string cut_y, string cut_time)
{
   cout << "Input File : " << f_name << endl;
   f_name.replace(f_name.find("KODEL"),5,"Trk");
   file_name = f_name;
   cout << "output File : " << file_name << endl;
   CUT_Delta_X = atof(cut_x.c_str());
   CUT_Delta_Y = atof(cut_y.c_str());
   CUT_Delta_Time = atof(cut_time.c_str());
   cout << "Cut Value for Tracking - X : " << CUT_Delta_X << " / Y : " << CUT_Delta_Y << " / Time : " << CUT_Delta_Time << endl;
}

void Tracking::ReadConfig(string mapconfig, string setconfig){

   ifstream mapfile( mapconfig );
   if(!mapfile.is_open()) {
       cerr << endl << "\tERROR : Check MAP config File" << endl << endl;
       exit(EXIT_FAILURE);
   }

   ifstream setfile( setconfig );
   if(!setfile.is_open())  {
       cerr << endl << "\tERROR : Check SET config File" << endl << endl;
       exit(EXIT_FAILURE);
   }

   string temp_string;
   string s_line;
   vector<string> temp_input;
   vector<int> TDC_STRIP;
   vector<string> NAME_PARTITION;

   DEC_name.clear();
   DEC_sizeX.clear();
   DEC_sizeY.clear();
   DEC_positionX.clear();
   DEC_positionY.clear();
   DEC_rotation.clear();
   DEC_total.clear();
   DEC_partition.clear();

   while (mapfile) {
      if (!getline( mapfile, s_line )) break;
      istringstream ss_line( s_line );

      if(isdigit(*(s_line.begin()))) {
         TDC_STRIP.clear();
         while (ss_line) {
            if (!getline( ss_line, temp_string, '\t' )) break;
            TDC_STRIP.push_back(atoi(temp_string.c_str()));
         }
         if(DEC_total[NAME_PARTITION.at(0)] < TDC_STRIP.at(2)) DEC_total[NAME_PARTITION.at(0)] = TDC_STRIP.at(2);
         if(DEC_total[NAME_PARTITION.at(0)] < TDC_STRIP.at(3)) DEC_total[NAME_PARTITION.at(0)] = TDC_STRIP.at(3);
      } else if (!(s_line.empty()) && s_line.find("/") != 0) {
         NAME_PARTITION.clear();
         while (ss_line){
            if (!getline( ss_line, temp_string, '\t' )) break;
            NAME_PARTITION.push_back(temp_string);
         }
         DEC_partition[NAME_PARTITION.at(0)] = atoi((NAME_PARTITION.at(1)).c_str());
         DEC_total[NAME_PARTITION.at(0)] = 0;
      }
   }

   while (setfile) {
      if (!getline( setfile, s_line )) break;
      istringstream ss_line( s_line );

      temp_input.clear();

      while (ss_line) {
         if (s_line.find("/") == 0) break;
         if (!getline( ss_line, temp_string, '\t' )) break;
            temp_input.push_back(temp_string);
      }

      if (temp_input.size()>1) {
         DEC_name.push_back(temp_input.at(0));
         DEC_sizeX[temp_input.at(0)]     = atof((temp_input.at(2)).c_str());
         DEC_sizeY[temp_input.at(0)]     = atof((temp_input.at(3)).c_str());
         DEC_positionX[temp_input.at(0)] = atof((temp_input.at(4)).c_str());
         DEC_positionY[temp_input.at(0)] = atof((temp_input.at(5)).c_str());
         DEC_rotation[temp_input.at(0)]  = atoi((temp_input.at(6)).c_str());
      }
   }

}

void Tracking::Loop()
{
   if (fChain == 0) return;
   TFile *OutputFile = new TFile(Form("%s", file_name.c_str()),"RECREATE");
   TTree *KODEL_Tree   = new TTree("KODEL_Tree","KODEL_Tree");

   int NumTrack, stripX, stripY, strip_max, strip_min, temp_strip;
   float size_partitionX, center_partitionX, size_stripY, center_stripY, temp_x, temp_y, size_clusterY, temp_dx, temp_dy, delta_clusterX, delta_clusterY;

   vector<string> Cluster_DEC;
   vector<float> Cluster_positionX, Cluster_positionY, Cluster_sizeX, Cluster_sizeY, Cluster_time;
   vector<int> Cluster_index;

   vector<int> Track_index, Track_NClusters, Track_NDetectors;
   vector<float> Track_positionX, Track_positionY, Track_time;
   vector<bool> Track_CheckDetectorS1;
   vector<bool> Track_CheckDetectorS2;
   vector<bool> Track_CheckDetectorS3;
   vector<bool> Track_CheckDetectorS4;
   vector<bool> Track_CheckDetectorS5;
   vector<bool> Track_CheckDetectorS6;

   KODEL_Tree->Branch("Info_NumTrack",           &NumTrack);

   KODEL_Tree->Branch("Cluster_DEC_Type",        &Cluster_DEC);
   KODEL_Tree->Branch("Cluster_PositionX",       &Cluster_positionX);
   KODEL_Tree->Branch("Cluster_PositionY",       &Cluster_positionY);
   KODEL_Tree->Branch("Cluster_SizeX",           &Cluster_sizeX);
   KODEL_Tree->Branch("Cluster_SizeY",           &Cluster_sizeY);
   KODEL_Tree->Branch("Cluster_MeanTime",        &Cluster_time);
   KODEL_Tree->Branch("Cluster_TrackIndex",      &Cluster_index);

   KODEL_Tree->Branch("Track_Index",             &Track_index);                      // Track Index
   KODEL_Tree->Branch("Track_NumClusters",       &Track_NClusters);                  // Number of Clusters for Track
   KODEL_Tree->Branch("Track_NumDetectors",      &Track_NDetectors);                 // Number of Detector for Track
   KODEL_Tree->Branch("Track_PositionX",         &Track_positionX);                  // Track X Position
   KODEL_Tree->Branch("Track_PositionY",         &Track_positionY);                  // Track Y Position
   KODEL_Tree->Branch("Track_MeanTime",          &Track_time);                       // Mean Time of Track (Calibrated)
   KODEL_Tree->Branch("Track_ClusterOnS1",       &Track_CheckDetectorS1);            // Check Cluster on each Detector
   KODEL_Tree->Branch("Track_ClusterOnS2",       &Track_CheckDetectorS2);            // Check Cluster on each Detector
   KODEL_Tree->Branch("Track_ClusterOnS3",       &Track_CheckDetectorS3);            // Check Cluster on each Detector
   KODEL_Tree->Branch("Track_ClusterOnS4",       &Track_CheckDetectorS4);            // Check Cluster on each Detector
   KODEL_Tree->Branch("Track_ClusterOnS5",       &Track_CheckDetectorS5);            // Check Cluster on each Detector
   KODEL_Tree->Branch("Track_ClusterOnS6",       &Track_CheckDetectorS6);            // Check Cluster on each Detector

   Long64_t nentries = fChain->GetEntriesFast();
   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;

      if (jentry%(nentries/10) == 0 && jentry != 0) cout << "running " << jentry << "th event (" << jentry/(nentries/100) << "%)" << endl;
      NumTrack = 0;

      Cluster_DEC.clear();
      Cluster_positionX.clear();
      Cluster_positionY.clear();
      Cluster_sizeX.clear();
      Cluster_sizeY.clear();
      Cluster_time.clear();
      Cluster_index.clear();

      Track_index.clear();
      Track_NClusters.clear();
      Track_NDetectors.clear();
      Track_positionX.clear();
      Track_positionY.clear();
      Track_time.clear();
      Track_CheckDetectorS1.clear();
      Track_CheckDetectorS2.clear();
      Track_CheckDetectorS3.clear();
      Track_CheckDetectorS4.clear();
      Track_CheckDetectorS5.clear();
      Track_CheckDetectorS6.clear();

      for(int cluster_i=0;cluster_i<Cluster_DEC_Type->size();++cluster_i){

         Cluster_DEC.push_back(Cluster_DEC_Type->at(cluster_i));
         Cluster_time.push_back(Cluster_Time_M->at(cluster_i));
         Cluster_index.push_back(99);

         size_partitionX = DEC_sizeX[Cluster_DEC_Type->at(cluster_i)] / (DEC_total[Cluster_DEC_Type->at(cluster_i)] / DEC_partition[Cluster_DEC_Type->at(cluster_i)]);
         center_partitionX = size_partitionX * (Cluster_X->at(cluster_i) + 0.5);

         size_stripY =  DEC_sizeY[Cluster_DEC_Type->at(cluster_i)] / DEC_partition[Cluster_DEC_Type->at(cluster_i)];
         center_stripY = size_stripY * (Cluster_Y->at(cluster_i) + 0.5);

         strip_max = -1000;
         strip_min = 1000;
         for(int strip_i=0;strip_i<Strip_Number->size();++strip_i){
             if(Strip_ClusterIndex->at(strip_i) == Cluster_Index->at(cluster_i)){
                 temp_strip = (Strip_Number->at(strip_i)-1)%DEC_partition[Cluster_DEC_Type->at(cluster_i)];
                 if (strip_max < temp_y) strip_max = temp_strip;
                 if (strip_min > temp_y) strip_min = temp_strip;
             } 
         }
         if (strip_max == -1000 && strip_min == 1000) {
            cerr << "\t" << Cluster_Index->at(cluster_i) << "th Cluster in " << Cluster_DEC_Type->at(cluster_i) << " has no hit?" << endl;
            exit(EXIT_FAILURE);
         } else if(fabs(strip_max - Cluster_Y->at(cluster_i)) > fabs(Cluster_Y->at(cluster_i) - strip_min)) {
            size_clusterY = size_stripY * fabs(strip_max - Cluster_Y->at(cluster_i));
         } else {
            size_clusterY = size_stripY * fabs(Cluster_Y->at(cluster_i) - strip_min);
         }

         if(DEC_rotation[Cluster_DEC_Type->at(cluster_i)] == 0){
            temp_x  = center_partitionX;
            temp_y  = center_stripY;
            temp_dx = size_partitionX;
            temp_dy = size_clusterY;
         } else if(DEC_rotation[Cluster_DEC_Type->at(cluster_i)] == 90){
            temp_x  = center_stripY;
            temp_y  = DEC_sizeY[Cluster_DEC_Type->at(cluster_i)] - center_partitionX; 
            temp_dx = size_clusterY;
            temp_dy = size_partitionX;
         } else if(DEC_rotation[Cluster_DEC_Type->at(cluster_i)] == 180){
            temp_x  = DEC_sizeX[Cluster_DEC_Type->at(cluster_i)] - center_partitionX;
            temp_y  = DEC_sizeY[Cluster_DEC_Type->at(cluster_i)] - center_stripY;
            temp_dx = size_partitionX;
            temp_dy = size_clusterY;
         } else if(DEC_rotation[Cluster_DEC_Type->at(cluster_i)] == 270){
            temp_x  = DEC_sizeX[Cluster_DEC_Type->at(cluster_i)] - center_stripY; 
            temp_y  = center_partitionX;
            temp_dx = size_clusterY;
            temp_dy = size_partitionX;
         } else {
            cerr << endl << "\tERROR : Check SET config File";
            cerr << endl << "\tRotation Angle : " << DEC_rotation[Cluster_DEC_Type->at(cluster_i)] << endl << endl;
            exit(EXIT_FAILURE);
         }
         Cluster_positionX.push_back(temp_x + DEC_positionX[Cluster_DEC_Type->at(cluster_i)]);
         Cluster_positionY.push_back(temp_y + DEC_positionY[Cluster_DEC_Type->at(cluster_i)]);
         Cluster_sizeX.push_back(temp_dx);
         Cluster_sizeY.push_back(temp_dy);
      }

      bool check_init = false;
      bool track_change = true;
      bool cluster_change = false;
      int track_temp_index = 0;

      while(track_change){
         track_change = false;
         for(int i_f=0;i_f<Cluster_DEC.size();++i_f){
            cluster_change = false;
            check_init = false;
            if(Cluster_index.at(i_f) == 99){
               ++track_temp_index;
               Cluster_index.at(i_f) = track_temp_index;
               check_init = true;
            }
            for(int i_s=i_f;i_s<Cluster_DEC.size();++i_s){
               Cluster_sizeX.at(i_s)>Cluster_sizeX.at(i_f)?delta_clusterX=Cluster_sizeX.at(i_s):delta_clusterX=Cluster_sizeX.at(i_f);
               Cluster_sizeY.at(i_s)>Cluster_sizeY.at(i_f)?delta_clusterY=Cluster_sizeY.at(i_s):delta_clusterY=Cluster_sizeY.at(i_f);
               if( (fabs(Cluster_positionX.at(i_f) - Cluster_positionX.at(i_s)) < CUT_Delta_X + delta_clusterX) && 
                   (fabs(Cluster_positionY.at(i_f) - Cluster_positionY.at(i_s)) < CUT_Delta_Y + delta_clusterY) && 
                   (fabs(Cluster_time.at(i_f) - Cluster_time.at(i_s))           < CUT_Delta_Time) ) {
                  if(Cluster_index.at(i_f) > Cluster_index.at(i_s)){
                     Cluster_index.at(i_f) = Cluster_index.at(i_s);
                     cluster_change = true;
                     track_change = true;
                  } else if(Cluster_index.at(i_f) < Cluster_index.at(i_s)){
                     Cluster_index.at(i_s) = Cluster_index.at(i_f);
                     cluster_change = true;
                     track_change = true;
                  }
               }
            }
               if(check_init && !cluster_change) {Cluster_index.at(i_f) = 99; --track_temp_index;}
         }
      }

      map<int,int> sort_track;
      sort_track.clear();
      for(int remove_i=0;remove_i<Cluster_index.size();++remove_i){
         if(Cluster_index.at(remove_i) != 99) sort_track[Cluster_index.at(remove_i)] = 1;
      }
      NumTrack = sort_track.size(); 
      map<int,int>::iterator it_t = sort_track.begin();
      for(int index_track = 1; index_track <= sort_track.size(); ++index_track, ++it_t){
         for(int sort_i=0;sort_i<Cluster_index.size();++sort_i){
            if(Cluster_index.at(sort_i) == it_t->first) {
               Cluster_index.at(sort_i) = index_track;
            }
         }
      }

      map<string,bool> check_track;
      float mean_X, mean_Y, mean_T;
      int num_cluster, num_detector;
      for(int trk=1;trk<=NumTrack;++trk){
         mean_X = 0;
         mean_Y = 0;
         mean_T = 0;
         num_cluster = 0;
         num_detector = 0;
         check_track.clear();
         for(int dec=0;dec<DEC_name.size();++dec){
            check_track[DEC_name.at(dec)] = false;
         }
         for(int clu=0;clu<Cluster_index.size();++clu){
            if(Cluster_index.at(clu) == trk){
                mean_X += Cluster_positionX.at(clu);
                mean_Y += Cluster_positionY.at(clu);
                mean_T += Cluster_time.at(clu);
                check_track[Cluster_DEC.at(clu)] = true;
                ++num_cluster;
            }
         }
         Track_index.push_back(trk);
         Track_NClusters.push_back(num_cluster);
         Track_positionX.push_back(mean_X / num_cluster);
         Track_positionY.push_back(mean_Y / num_cluster);
         Track_time.push_back(mean_T / num_cluster);
         for(map<string,bool>::iterator it = check_track.begin();it != check_track.end();++it) { 
            if((it->first).find("S1") != std::string::npos) Track_CheckDetectorS1.push_back(it->second); 
            if((it->first).find("S2") != std::string::npos) Track_CheckDetectorS2.push_back(it->second); 
            if((it->first).find("S3") != std::string::npos) Track_CheckDetectorS3.push_back(it->second); 
            if((it->first).find("S4") != std::string::npos) Track_CheckDetectorS4.push_back(it->second); 
            if((it->first).find("S5") != std::string::npos) Track_CheckDetectorS5.push_back(it->second); 
            if((it->first).find("S6") != std::string::npos) Track_CheckDetectorS6.push_back(it->second); 
            if(check_track[it->first]) ++num_detector;
         }
         Track_NDetectors.push_back(num_detector);
      }

      KODEL_Tree->Fill();

   } // jentry

   KODEL_Tree->Write();
   OutputFile->Close();

} // Loop

int main(int argc, char **argv){

   if(argc != 7) {
      cerr << endl << "\tERROR : Check Input arguments" << endl;
      cerr << "\t" << argv[0] << " InputRootFile MAPConfigFile SETConfigFile Cut_X Cut_Y Cut_Time" << endl << endl;
      exit(EXIT_FAILURE);
   }
   TFile *inputfile = new TFile( argv[1] );
   if(!inputfile->IsOpen()) {
      cerr << endl << "\tERROR : Check Input Root File" << endl << endl;
      exit(EXIT_FAILURE);
   }
   TTree *inputtree = (TTree*)inputfile->Get("KODEL_Tree");
   Tracking KODELTracking(inputtree);
   KODELTracking.Input(argv[1],argv[4],argv[5],argv[6]);
   KODELTracking.ReadConfig(argv[2], argv[3]);
   KODELTracking.Loop();

}

