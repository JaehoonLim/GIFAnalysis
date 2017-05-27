//
//
// Original Author:  Sungwoong Cho (sungwoong.cho@cern.ch), Jaehoon Lim (jae.hoon.lim@cern.ch)
//                   588 R-012, +41 22 76 73278
//         Created:  2017 May 24
//

void TrackPlot(string rootfile_name, string mapfile_name, string setfile_name, string type_name, string event_number){

   int canvas_width  = 1000;
   int canvas_height = 700;
   int for_fast_draw = 10;
   int label_problem = 10; // test 1 or 10

   gROOT->SetStyle("Plain");
   gROOT->ProcessLine("#include <map>");
   gStyle->SetOptStat(0);
   gStyle->SetMarkerSize(0.8);
   gStyle->SetPalette(1);
   gStyle->SetCanvasPreferGL(true);

   if(type_name == "all" || type_name == "All" || type_name == "ALL"){
      type_name = "All";
   } else if(type_name == "Muon" || type_name == "MUON" || type_name == "muon" || type_name == "Muons" || type_name == "MUONS" || type_name == "muons"){
      type_name = "Muon";
   } else {
      cerr << endl << "\tERROR : Check Plot variable : " << type_name << endl << endl;
      exit(EXIT_FAILURE);
   }

   if( !isdigit(*(event_number.begin())) ) {
      cerr << endl << "\tERROR : Check Event Number : " << event_number << endl << endl;
      exit(EXIT_FAILURE);
   }

   ifstream mapfile( mapfile_name.c_str() );
   if(!mapfile.is_open()) {
       cerr << endl << "\tERROR : Check MAP config File" << endl << endl;
       exit(EXIT_FAILURE);
   }

   ifstream setfile( setfile_name.c_str() );
   if(!setfile.is_open())  {
       cerr << endl << "\tERROR : Check SET config File" << endl << endl;
       exit(EXIT_FAILURE);
   }

   string temp_string;
   string s_line;
   vector<string> temp_input;

   vector<int> TDC_STRIP;
   vector<string> NAME_PARTITION;
   map<string,int> DEC_total;
   map<string,int> DEC_partition;

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
      } else if(!(s_line.empty()) && s_line.find("/") != 0) {
         NAME_PARTITION.clear();
         while (ss_line){
            if (!getline( ss_line, temp_string, '\t' )) break;
            NAME_PARTITION.push_back(temp_string);
         }
         DEC_partition[NAME_PARTITION.at(0)] = atoi((NAME_PARTITION.at(1)).c_str());
         DEC_total[NAME_PARTITION.at(0)] = 0;
      }
   }

   vector<string>     DEC_name;
   map<string,int>    DEC_order;
   map<string,int>    DEC_sizeX;
   map<string,int>    DEC_sizeYW;
   map<string,int>    DEC_sizeYN;
   map<string,int>    DEC_positionX;
   map<string,int>    DEC_positionY;
   map<string,int>    DEC_rotation;
   vector<float>      BEAM_position;

   DEC_name.clear();
   DEC_order.clear();
   DEC_sizeX.clear();
   DEC_sizeYW.clear();
   DEC_sizeYN.clear();
   DEC_positionX.clear();
   DEC_positionY.clear();
   DEC_rotation.clear();
   BEAM_position.clear();

   while (setfile) {
      if (!getline( setfile, s_line )) break;
      istringstream ss_line( s_line );

      temp_input.clear();

      while (ss_line) {
         if (s_line.find("/") == 0) break;
         if (!getline( ss_line, temp_string, '\t' )) break;
            temp_input.push_back(temp_string);
      }

      if (temp_input.size()>1){
         if(temp_input.at(0) != "BEAM") {
            DEC_name.push_back(temp_input.at(0));
            DEC_order[temp_input.at(0)]     = atof((temp_input.at(1)).c_str());
            DEC_sizeX[temp_input.at(0)]     = atof((temp_input.at(2)).c_str());
            DEC_sizeYW[temp_input.at(0)]    = atof((temp_input.at(3)).c_str());
            DEC_sizeYN[temp_input.at(0)]    = atof((temp_input.at(4)).c_str());
            DEC_positionX[temp_input.at(0)] = atof((temp_input.at(5)).c_str());
            DEC_positionY[temp_input.at(0)] = atof((temp_input.at(6)).c_str());
            DEC_rotation[temp_input.at(0)]  = atoi((temp_input.at(7)).c_str());
         } else {
            for(int beam_i = 1; beam_i < 7; ++beam_i){
               BEAM_position.push_back(atof((temp_input.at(beam_i)).c_str()));
            }
         }
      }
   }


   int xmax = 0;
   int ymax = 0;
   int zmax = 0;

   for(int Check_DEC = 0; Check_DEC<DEC_name.size(); ++Check_DEC){
      string DEC_name_ = DEC_name.at(Check_DEC);
      if(zmax < DEC_order[DEC_name_]) zmax = DEC_order[DEC_name_];
      if(DEC_rotation[DEC_name_] == 0 || DEC_rotation[DEC_name_] == 180){
         if(xmax < DEC_sizeX[DEC_name_] + DEC_positionX[DEC_name_]) xmax = DEC_sizeX[DEC_name_] + DEC_positionX[DEC_name_];
         if(ymax < DEC_sizeYW[DEC_name_] + DEC_positionY[DEC_name_]) ymax = DEC_sizeYW[DEC_name_] + DEC_positionY[DEC_name_];
      } else {
         if(xmax < DEC_sizeYW[DEC_name_] + DEC_positionX[DEC_name_]) xmax = DEC_sizeYW[DEC_name_] + DEC_positionX[DEC_name_];
         if(ymax < DEC_sizeX[DEC_name_] + DEC_positionY[DEC_name_]) ymax = DEC_sizeX[DEC_name_] + DEC_positionY[DEC_name_];
      }
   }
   int blank = 200;

   TH3D* h_test = new TH3D("h_test", "h_test", (xmax+(2*blank))/for_fast_draw, 0, xmax+(2*blank), 10*(zmax+1), 0, zmax+1, (ymax+(2*blank))/for_fast_draw, 0, ymax+(2*blank));
   h_test->SetMinimum(0);
   h_test->SetMaximum(zmax+1);

   TF1 * tf = new TF1("TransferFunction", "x", 0., (float)(zmax+1));
   TList * lof = h_test->GetListOfFunctions();
   if (lof) lof->Add(tf);

   // Draw Beam // 
   for(int Beam_x = 0; Beam_x<2*BEAM_position.at(1);++Beam_x){
      for(int Beam_y = 0; Beam_y<2*BEAM_position.at(3);++Beam_y){
         if( ((double)(Beam_x*Beam_x)/(double)(BEAM_position.at(1)*BEAM_position.at(1))) +
             ((double)(Beam_y*Beam_y)/(double)(BEAM_position.at(3)*BEAM_position.at(3))) < 1){
            for(int Beam_z = 0; Beam_z<5*(zmax+1);++Beam_z){
                 h_test->SetBinContent((blank+BEAM_position.at(0)+Beam_x)/for_fast_draw, 2*Beam_z, (blank+BEAM_position.at(2)+Beam_y)/for_fast_draw, 0.05);
                 h_test->SetBinContent((blank+BEAM_position.at(0)-Beam_x)/for_fast_draw, 2*Beam_z, (blank+BEAM_position.at(2)+Beam_y)/for_fast_draw, 0.05);
                 h_test->SetBinContent((blank+BEAM_position.at(0)+Beam_x)/for_fast_draw, 2*Beam_z, (blank+BEAM_position.at(2)-Beam_y)/for_fast_draw, 0.05);
                 h_test->SetBinContent((blank+BEAM_position.at(0)-Beam_x)/for_fast_draw, 2*Beam_z, (blank+BEAM_position.at(2)-Beam_y)/for_fast_draw, 0.05);
            }
         }
      }
   }

   // Draw Chamber //
   for(int Loop_DEC = 0; Loop_DEC<DEC_name.size(); ++Loop_DEC){
      int flip         = 1;
      string DEC_name_ = DEC_name.at(Loop_DEC);
      int dec_center   = blank;
      int dec_x_start  = blank;
      int dec_x_end    = blank;
      int dec_y_wide   = DEC_sizeYW[DEC_name_]/2;
      int dec_y_narrow = DEC_sizeYN[DEC_name_]/2;
      if(DEC_rotation[DEC_name_] == 0 || DEC_rotation[DEC_name_] == 180){
         if(DEC_rotation[DEC_name_] == 0){
            dec_x_start  += DEC_positionX[DEC_name_];
            dec_x_end    += DEC_positionX[DEC_name_]+DEC_sizeX[DEC_name_];
            dec_center   += DEC_positionY[DEC_name_]+dec_y_wide;
         } else {
            dec_x_start  += DEC_positionX[DEC_name_]+DEC_sizeX[DEC_name_];
            dec_x_end    += DEC_positionX[DEC_name_];
            dec_center   += DEC_positionY[DEC_name_]+dec_y_narrow;
            flip          = -1;
         }
         for(int hist_x = 0; hist_x<abs(dec_x_end-dec_x_start); ++hist_x){
              int dec_y = ((hist_x*dec_y_narrow) + (abs(dec_x_end-dec_x_start)-hist_x)*dec_y_wide) / abs(dec_x_end-dec_x_start);
              h_test->SetBinContent((dec_x_start+(flip*hist_x))/for_fast_draw, 10*DEC_order[DEC_name_], (dec_center+dec_y)/for_fast_draw, zmax+1);
              h_test->SetBinContent((dec_x_start+(flip*hist_x))/for_fast_draw, 10*DEC_order[DEC_name_], (dec_center-dec_y)/for_fast_draw, zmax+1);
         }
         for(int hist_y = 0; hist_y<dec_y_wide; ++hist_y){
              h_test->SetBinContent(dec_x_start/for_fast_draw, 10*DEC_order[DEC_name_], (dec_center+hist_y)/for_fast_draw, zmax+1);
              h_test->SetBinContent(dec_x_start/for_fast_draw, 10*DEC_order[DEC_name_], (dec_center-hist_y)/for_fast_draw, zmax+1);
              if(hist_y<dec_y_narrow){
                 h_test->SetBinContent(dec_x_end/for_fast_draw, 10*DEC_order[DEC_name_], (dec_center+hist_y)/for_fast_draw, zmax+1);
                 h_test->SetBinContent(dec_x_end/for_fast_draw, 10*DEC_order[DEC_name_], (dec_center-hist_y)/for_fast_draw, zmax+1);
              }
         }
      } else if(DEC_rotation[DEC_name_] == 90 || DEC_rotation[DEC_name_] == 270){
         if(DEC_rotation[DEC_name_] == 90){
            dec_x_start  += DEC_positionY[DEC_name_]+DEC_sizeX[DEC_name_];
            dec_x_end    += DEC_positionY[DEC_name_];
            dec_center   += DEC_positionX[DEC_name_]+dec_y_narrow;
            flip          = -1;
         } else {
            dec_x_start  += DEC_positionY[DEC_name_];
            dec_x_end    += DEC_positionY[DEC_name_]+DEC_sizeX[DEC_name_];
            dec_center   += DEC_positionX[DEC_name_]+dec_y_wide;
         }
         for(int hist_x = 0; hist_x<abs(dec_x_end-dec_x_start); ++hist_x){
              int dec_y = ((hist_x*dec_y_narrow) + (abs(dec_x_end-dec_x_start)-hist_x)*dec_y_wide) / abs(dec_x_end-dec_x_start);
              h_test->SetBinContent((dec_center+dec_y)/for_fast_draw, 10*DEC_order[DEC_name_], (dec_x_start+(flip*hist_x))/for_fast_draw, zmax+1);
              h_test->SetBinContent((dec_center-dec_y)/for_fast_draw, 10*DEC_order[DEC_name_], (dec_x_start+(flip*hist_x))/for_fast_draw, zmax+1);
         }
         for(int hist_y = 0; hist_y<dec_y_wide; ++hist_y){
              h_test->SetBinContent((dec_center+hist_y)/for_fast_draw, 10*DEC_order[DEC_name_], dec_x_start/for_fast_draw, zmax+1);
              h_test->SetBinContent((dec_center-hist_y)/for_fast_draw, 10*DEC_order[DEC_name_], dec_x_start/for_fast_draw, zmax+1);
              if(hist_y<dec_y_narrow){
                 h_test->SetBinContent((dec_center+hist_y)/for_fast_draw, 10*DEC_order[DEC_name_], dec_x_end/for_fast_draw, zmax+1);
                 h_test->SetBinContent((dec_center-hist_y)/for_fast_draw, 10*DEC_order[DEC_name_], dec_x_end/for_fast_draw, zmax+1);
              }
         }
      } else {
         cerr << "ERROR : Ratation Angle" << endl;
         exit(EXIT_FAILURE);
      }
   }

   TFile* f_input = new TFile(Form("%s",rootfile_name.c_str()));
   TTree* t_input = (TTree*)f_input->Get("KODEL_Tree");

   vector<int> *Strip_Number, *Strip_ClusterIndex, *Cluster_TrackIndex, *Cluster_Index, *Track_Index;
   vector<string> *Strip_DEC_Type, *Cluster_DEC_Type;
   vector<bool> *Track_isMuonTrack;

   t_input->SetBranchAddress("Strip_Number",       &Strip_Number);
   t_input->SetBranchAddress("Strip_ClusterIndex", &Strip_ClusterIndex);
   t_input->SetBranchAddress("Strip_DEC_Type",     &Strip_DEC_Type);
   t_input->SetBranchAddress("Cluster_Index",      &Cluster_Index);
   t_input->SetBranchAddress("Cluster_DEC_Type",   &Cluster_DEC_Type);
   t_input->SetBranchAddress("Cluster_TrackIndex", &Cluster_TrackIndex);
   t_input->SetBranchAddress("Track_Index",        &Track_Index);
   t_input->SetBranchAddress("Track_isMuonTrack",  &Track_isMuonTrack);

   t_input->GetEntry(atoi(event_number.c_str()));

   // Draw Strip
   for(int Track_i=0;Track_i<Track_Index->size();++Track_i){
      if(!Track_isMuonTrack->at(Track_i) && type_name == "Muon") continue;
      for(int Cluster_i=0;Cluster_i<Cluster_Index->size();++Cluster_i){
         if(type_name == "All" || Track_Index->at(Track_i) == Cluster_TrackIndex->at(Cluster_i)){
            for(int Strip_i=0;Strip_i<Strip_ClusterIndex->size();++Strip_i){
               if(type_name == "All" || (Cluster_Index->at(Cluster_i) == Strip_ClusterIndex->at(Strip_i) && Cluster_DEC_Type->at(Cluster_i) == Strip_DEC_Type->at(Strip_i))){
                  string DEC_name_  = Strip_DEC_Type->at(Strip_i);
                  int num_strip     = DEC_partition[DEC_name_];
                  int num_partition = DEC_total[DEC_name_] / num_strip;
                  int strip_x       = Strip_Number->at(Strip_i) / num_strip;
                  int strip_size_x  = DEC_sizeX[DEC_name_] / num_partition; 
                  int strip_size_y  = (((1+strip_x)*DEC_sizeYN[DEC_name_] - ((1+strip_x)*DEC_sizeYW[DEC_name_])) / num_partition) + DEC_sizeYW[DEC_name_]; 
                  int strip_y       = (DEC_sizeYW[DEC_name_] - strip_size_y)/2;
                      strip_size_y /= num_strip;
                      strip_x       = strip_x * strip_size_x;
                      strip_y      += (Strip_Number->at(Strip_i) % num_strip) * strip_size_y;
                  int x_start       = blank;
                  int x_end         = blank;
                  int y_start       = blank;
                  int y_end         = blank;
                  if(DEC_rotation[DEC_name_] == 0){
                     x_start += DEC_positionX[DEC_name_]+strip_x;
                     x_end   += DEC_positionX[DEC_name_]+strip_x+strip_size_x;
                     y_start += DEC_positionY[DEC_name_]+strip_y;
                     y_end   += DEC_positionY[DEC_name_]+strip_y+strip_size_y;
                  } else if(DEC_rotation[DEC_name_] == 90){
                     x_start += DEC_positionX[DEC_name_]+(DEC_sizeYN[DEC_name_]-DEC_sizeYW[DEC_name_])/2+strip_y;
                     x_end   += DEC_positionX[DEC_name_]+(DEC_sizeYN[DEC_name_]-DEC_sizeYW[DEC_name_])/2+strip_y+strip_size_y;
                     y_start += DEC_positionY[DEC_name_]+DEC_sizeX[DEC_name_]-strip_x-strip_size_x;
                     y_end   += DEC_positionY[DEC_name_]+DEC_sizeX[DEC_name_]-strip_x;
                  } else if(DEC_rotation[DEC_name_] == 180){
                     x_start += DEC_positionX[DEC_name_]+DEC_sizeX[DEC_name_]-strip_x-strip_size_x;
                     x_end   += DEC_positionX[DEC_name_]+DEC_sizeX[DEC_name_]-strip_x;
                     y_start += DEC_positionY[DEC_name_]+(DEC_sizeYN[DEC_name_]+DEC_sizeYW[DEC_name_])/2-strip_y-strip_size_y;
                     y_end   += DEC_positionY[DEC_name_]+(DEC_sizeYN[DEC_name_]+DEC_sizeYW[DEC_name_])/2-strip_y;
                  } else if(DEC_rotation[DEC_name_] == 270){
                     x_start += DEC_positionX[DEC_name_]+DEC_sizeYW[DEC_name_]-strip_y-strip_size_y;
                     x_end   += DEC_positionX[DEC_name_]+DEC_sizeYW[DEC_name_]-strip_y;
                     y_start += DEC_positionY[DEC_name_]+strip_x; 
                     y_end   += DEC_positionY[DEC_name_]+strip_x+strip_size_x;
                  } else {
                     cerr << "ERROR : Ratation Angle" << endl;
                     exit(EXIT_FAILURE);
                  } 
                  for(int draw_x = x_start; draw_x < x_end; ++draw_x){
                     for(int draw_y = y_start; draw_y < y_end; ++draw_y){
                        h_test->SetBinContent(draw_x/for_fast_draw, 10*DEC_order[DEC_name_], draw_y/for_fast_draw, DEC_order[DEC_name_]);
                     }
                  }
               }
            }
            if(type_name == "All"){Cluster_i = Cluster_Index->size(); Track_i = Track_Index->size();}
         }
      }
   }

   TCanvas* c_test = new TCanvas("c_test","Canvas",canvas_width,canvas_height);

   h_test->Draw("GLCOL FB"); // FB BB A
   h_test->GetXaxis()->SetBinLabel(1,"");
   h_test->GetZaxis()->SetBinLabel(1,"");
   h_test->GetYaxis()->SetLabelSize(0.01);
   h_test->GetYaxis()->SetLabelOffset(0.1);
   for(int Draw_Label=0;Draw_Label<DEC_name.size();++Draw_Label){
      h_test->GetYaxis()->SetBinLabel(label_problem*(DEC_order[DEC_name.at(Draw_Label)]+1),(DEC_name.at(Draw_Label)).c_str());
   }
   TPaveText* tx_test = new TPaveText(0.15, 0.85, 0.95, 0.95);
   tx_test->SetFillColor(0);
   tx_test->SetTextSize(0.03);
   string hist_title = rootfile_name;
   hist_title.erase(0,hist_title.find("Scan"));
   hist_title.erase(hist_title.find("_DAQ"),hist_title.size()-1);
   hist_title.replace(hist_title.find("_"),1," ");
   if(type_name == "All") tx_test->AddText(Form("%s %sth event (All hits)",hist_title.c_str(),event_number.c_str()));
   else tx_test->AddText(Form("%s %sth event (Muon Beam)",hist_title.c_str(),event_number.c_str()));
   tx_test->Draw();

   gPad->SetTheta(45); // default is 30
   gPad->SetPhi(45); // default is 30
   gPad->Update();

   rootfile_name.replace(rootfile_name.find(".root"),5,Form("-%s-%s.root",event_number.c_str(),type_name.c_str()));
   TFile *OutputFile = new TFile(Form("%s", rootfile_name.c_str()),"RECREATE");
   c_test->Write();
   OutputFile->Close();
}
