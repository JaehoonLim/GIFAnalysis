//
//
// Original Author:  Sungwoong Cho (sungwoong.cho@cern.ch), Jaehoon Lim (jae.hoon.lim@cern.ch)
//                   588 R-012, +41 22 76 73278
//         Created:  2015 Oct 14
//

void KODELPlot(string rootfile_name, string mapfile_name, string setfile_name, string dec_name, string type_name, string event_number){

   string title_name = "";

   gROOT->SetStyle("Plain");
   gROOT->ProcessLine("#include <map>");
   gStyle->SetOptStat(0);
   gStyle->SetMarkerSize(0.8);
   gStyle->SetPalette(1);

   if(type_name != "Clusters" && type_name != "Hits"){
      if(type_name == "cluster" || type_name == "clusters" || type_name == "Cluster" || type_name == "CLUSTER" || type_name == "CLUSTERS"){
         type_name = "Clusters";
      } else if(type_name == "hit" || type_name == "hits" || type_name == "Hit" || type_name == "HIT" || type_name == "HITS"){
         type_name = "Hits";
      } else {
         cerr << endl << "\tERROR : Check Plot variable : " << type_name << endl << endl;
         exit(EXIT_FAILURE);
      }
   }
   if(event_number == "all" || event_number == "All" || event_number == "ALL"){
      event_number = "All";
   } else if( !isdigit(*(event_number.begin())) ) {
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
   map<string,int> DEC_rotation;

   DEC_total.clear();
   DEC_partition.clear();
   DEC_rotation.clear();

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
         DEC_rotation[temp_input.at(0)]  = atoi((temp_input.at(6)).c_str());
      }
   }

   bool find_dec = false;
   for(map<string,int>::iterator DEC_NAME = DEC_partition.begin(); DEC_NAME != DEC_partition.end(); ++DEC_NAME){
      if( (DEC_NAME->first).find(dec_name) != string::npos ) {
         dec_name = DEC_NAME->first;
         find_dec = true;
      }
   } 
   if(!find_dec) {
       cerr << endl << "\tERROR : Check Chamber Name : " << dec_name << endl << endl;
       exit(EXIT_FAILURE);
   }

   int canvas_width  = 1000;
   int canvas_height = 700;

   int num_partition  = DEC_total[dec_name] / DEC_partition[dec_name];
   int num_strip      = DEC_partition[dec_name];
   int histo_width    = num_partition;
   int histo_height   = num_strip; 
   int histo_x_sign   = -1;
   int histo_y_sign   = 1;
   string x_title     = "Partiton";
   char partition_end = 'A'+num_partition-1;
   string x_bin_start = "Partiton A";
   string x_bin_end(&partition_end);
   x_bin_end = "Partition " + x_bin_end;
   string y_title     = "Strip Number";
   char strip_end[3];
   sprintf(strip_end,"%d",num_strip);
   string y_bin_start = "1";
   string y_bin_end   = strip_end;

   if(DEC_rotation[dec_name] == 90 || DEC_rotation[dec_name] == 270) {
      histo_width   = num_strip;
      histo_height  = num_partition;
      canvas_width  = 700;
      canvas_height = 1000;
      x_bin_start   = "1";
      x_bin_end     = strip_end;
      y_bin_start   = "A";
      y_bin_end     = partition_end;
      x_title       = "Strip Number";
      y_title       = "Partiton";
      if(DEC_rotation[dec_name] == 90) {
         histo_x_sign = -1;
         histo_y_sign = -1;
      } else if(DEC_rotation[dec_name] == 270) {
         histo_x_sign = 1;
         histo_y_sign = 1;
      }
   } else if(DEC_rotation[dec_name] == 180) {
      histo_x_sign = 1;
      histo_y_sign = -1;
   }

   int histo_x_start = 0;
   int histo_x_end   = histo_width;
   int histo_y_start = 0;
   int histo_y_end   = histo_height;
   string temp_swap;


   if(histo_x_sign == -1) {
      histo_x_start = -1 * histo_width;
      histo_x_end   = 0;
      temp_swap     = x_bin_start;
      x_bin_start   = x_bin_end;
      x_bin_end     = temp_swap;
   }
   if(histo_y_sign == -1) {
      histo_y_start = -1 * histo_height;
      histo_y_end   = 0;
      temp_swap     = y_bin_start;
      y_bin_start   = y_bin_end;
      y_bin_end     = temp_swap;
   }

   int test_x_start = -100;
   int test_x_end   = 700;
   if(type_name == "Clusters") {
      test_x_start = 0;
      test_x_end   = 20;
   }

   TH2F* h_test     = new TH2F("h_test", "h_test", histo_width, histo_x_start, histo_x_end, histo_height, histo_y_start, histo_y_end);
   TH1F* h_TimeCal  = new TH1F("h_TimeCal", "h_TimeCal", 800, -400, 400);
   TH1F* h_NHits    = new TH1F("h_NHits", "h_NHits", DEC_total[dec_name] , 0, DEC_total[dec_name] );
   TH1F* h_test2    = new TH1F("h_test2", "h_test2", test_x_end-test_x_start, test_x_start, test_x_end);

   TFile* f_input = new TFile(Form("%s",rootfile_name.c_str()));
   TTree* t_input = (TTree*)f_input->Get("KODEL_Tree");

   vector<int> *Strip_Number, *Strip_ClusterIndex, *DEC_NumHits, *DEC_NumCluster, *Cluster_NumHits, *Cluster_Index;
   vector<string> *Strip_DEC_Type, *DEC_Type, *Cluster_DEC_Type;
   vector<float> *TDC_Time, *TDC_Time_Cal, *Cluster_Time_F, *Cluster_X, *Cluster_Y;

   t_input->SetBranchAddress("TDC_Time",&TDC_Time);
   t_input->SetBranchAddress("TDC_Time_Cal",&TDC_Time_Cal);
   t_input->SetBranchAddress("Strip_Number",&Strip_Number);
   t_input->SetBranchAddress("Strip_ClusterIndex",&Strip_ClusterIndex);
   t_input->SetBranchAddress("Strip_DEC_Type",&Strip_DEC_Type);
   t_input->SetBranchAddress("DEC_Type",&DEC_Type);
   t_input->SetBranchAddress("DEC_NumHits",&DEC_NumHits);
   t_input->SetBranchAddress("DEC_NumCluster",&DEC_NumCluster);
   t_input->SetBranchAddress("Cluster_Index",&Cluster_Index);
   t_input->SetBranchAddress("Cluster_DEC_Type",&Cluster_DEC_Type);
   t_input->SetBranchAddress("Cluster_NumHits",&Cluster_NumHits);
   t_input->SetBranchAddress("Cluster_X",&Cluster_X);
   t_input->SetBranchAddress("Cluster_Y",&Cluster_Y);
   t_input->SetBranchAddress("Cluster_Time_F",&Cluster_Time_F);

   for (int k = 0; k < t_input->GetEntries();++k) {
      t_input->GetEntry(k);
      if(event_number == "All" || k == atoi(event_number.c_str()) ){
         if(type_name == "Hits"){
            for(int i = 0; i < Strip_Number->size(); ++i){
               if(Strip_DEC_Type->at(i) == dec_name){ 
                  h_TimeCal->Fill(TDC_Time_Cal->at(i));
                  h_test2->Fill(TDC_Time->at(i));
                  if(event_number == "All") {
                     h_test->Fill( histo_x_sign*(((Strip_Number->at(i)-1)/num_strip)+0.5) ,  histo_y_sign*(((Strip_Number->at(i)-1)%num_strip)+0.5));
                  } else {
                     h_test->Fill( histo_x_sign*(((Strip_Number->at(i)-1)/num_strip)+0.5) ,  histo_y_sign*(((Strip_Number->at(i)-1)%num_strip)+0.5),Strip_ClusterIndex->at(i));
                  }
               }
            }
            for(int j = 0; j < DEC_Type->size(); ++j){
               if(DEC_Type->at(j) == dec_name){ 
                  h_NHits->Fill(DEC_NumHits->at(j));
               }
            }
         } else if(type_name == "Clusters"){
            for(int i = 0; i < Cluster_DEC_Type->size(); ++i){
               if(Cluster_DEC_Type->at(i) == dec_name){
                  h_TimeCal->Fill(Cluster_Time_F->at(i));
                  h_NHits->Fill(Cluster_NumHits->at(i));
                  if(event_number == "All") {
                     h_test->Fill( histo_x_sign*(Cluster_X->at(i)+0.5), histo_y_sign*(Cluster_Y->at(i)+0.5));
                  } else {
                     h_test->Fill( histo_x_sign*(Cluster_X->at(i)+0.5), histo_y_sign*(Cluster_Y->at(i)+0.5), Cluster_Index->at(i));
                  }
               }
            }
            for(int j = 0; j < DEC_Type->size(); ++j){
               if(DEC_Type->at(j) == dec_name){ 
                  h_test2->Fill(DEC_NumCluster->at(j));
               }
            }
         }
      }
   }

   TCanvas* c_test = new TCanvas("c_test", "c_test", canvas_width, canvas_height);
   c_test->SetFillColor(0);
   c_test->SetFillStyle(4000);
   c_test->SetGridx();
   c_test->SetGridy();

   h_test->GetXaxis()->SetNdivisions(histo_width);
   h_test->GetYaxis()->SetNdivisions(histo_height);
   h_test->GetXaxis()->SetBinLabel(1,x_bin_start.c_str());
   h_test->GetXaxis()->SetBinLabel(histo_width,x_bin_end.c_str());
   h_test->GetYaxis()->SetBinLabel(1,y_bin_start.c_str());
   h_test->GetYaxis()->SetBinLabel(histo_height,y_bin_end.c_str());

   if(title_name == ""){
      if(event_number == "All") title_name = dec_name + " / " + type_name + " / All Events";
      else if ((atoi(event_number.c_str()))%10 == 1) title_name = dec_name + " / " + type_name + " / " + event_number + "st Event";
      else if ((atoi(event_number.c_str()))%10 == 2) title_name = dec_name + " / " + type_name + " / " + event_number + "nd Event";
      else title_name = dec_name + " / " + type_name + " / " + event_number + "th Event";
   }

   h_test->SetTitle(Form("%s;%s;%s",title_name.c_str(),x_title.c_str(),y_title.c_str()));

   c_test->cd();
   if(DEC_rotation[dec_name] == 0 || DEC_rotation[dec_name] == 90) {
      h_test->Draw("text col y+");
   } else {
      h_test->Draw("text col");
   }
   c_test->SaveAs(Form("%s_%s_%s_%s_2D.png",rootfile_name.c_str(),dec_name.c_str(),type_name.c_str(),event_number.c_str()));

   TCanvas* c_NHits = new TCanvas("c_NHits", "c_NHits", 1000, 700);
   c_NHits->SetFillColor(0);
   c_NHits->SetFillStyle(4000);
   c_NHits->SetLogy();

   h_NHits->GetYaxis()->SetTitleOffset(1);
   h_NHits->GetYaxis()->SetLabelSize(0.025);
   h_NHits->GetXaxis()->SetTitleOffset(1.1);

   TLegend* l_NHits = new TLegend(0.75, 0.7, 0.95, 0.95);
   l_NHits->SetFillColor(0);
   l_NHits->SetTextSize(0.025);
   l_NHits->AddEntry(h_NHits, Form("Entries : \t%.0f",h_NHits->GetEntries()), "l");
   l_NHits->AddEntry(h_NHits, Form("Mean : \t%.3f",h_NHits->GetMean()), "");
   l_NHits->AddEntry(h_NHits, Form("RMS  : \t%.3f",h_NHits->GetRMS()), "");

   string NHits_xtitle = "Hit Multiplicity";
   if(type_name == "Clusters") NHits_xtitle = "Cluster Size";
   h_NHits->SetTitle(Form("%s;%s;%s",title_name.c_str(),NHits_xtitle.c_str(),"Entries"));
   h_NHits->Draw("");
   l_NHits->Draw("same");

   c_NHits->SaveAs(Form("%s_%s_%s_%s_%s.png",rootfile_name.c_str(),dec_name.c_str(),type_name.c_str(),event_number.c_str(),NHits_xtitle.c_str()));

   TCanvas* c_TimeCal = new TCanvas("c_TimeCal", "c_TimeCal", 1000, 700);
   c_TimeCal->SetFillColor(0);
   c_TimeCal->SetFillStyle(4000);

   h_TimeCal->GetYaxis()->SetTitleOffset(1);
   h_TimeCal->GetYaxis()->SetLabelSize(0.025);
   h_TimeCal->GetXaxis()->SetTitleOffset(1.1);

   TLegend* l_TimeCal = new TLegend(0.75, 0.7, 0.95, 0.95);
   l_TimeCal->SetFillColor(0);
   l_TimeCal->SetTextSize(0.025);
   l_TimeCal->AddEntry(h_TimeCal, Form("Entries : \t%.0f",h_TimeCal->GetEntries()), "l");
   l_TimeCal->AddEntry(h_TimeCal, Form("Mean : \t%.3f",h_TimeCal->GetMean()), "");
   l_TimeCal->AddEntry(h_TimeCal, Form("RMS  : \t%.3f",h_TimeCal->GetRMS()), "");

   string TimeCal_xtitle = "Calibrated TDC Hit Time";
   if(type_name == "Clusters") TimeCal_xtitle = "Cluster Fastest Hit Time";
   h_TimeCal->SetTitle(Form("%s;%s [nsec];%s",title_name.c_str(),TimeCal_xtitle.c_str(),"Entries"));
   h_TimeCal->Draw("");
   l_TimeCal->Draw("same");

   c_TimeCal->SaveAs(Form("%s_%s_%s_%s_%s.png",rootfile_name.c_str(),dec_name.c_str(),type_name.c_str(),event_number.c_str(),TimeCal_xtitle.c_str()));

   TCanvas* c_test2 = new TCanvas("c_test2", "c_test2", 1000, 700);
   c_test2->SetFillColor(0);
   c_test2->SetFillStyle(4000);

   h_test2->GetYaxis()->SetTitleOffset(1);
   h_test2->GetYaxis()->SetLabelSize(0.025);
   h_test2->GetXaxis()->SetTitleOffset(1.1);

   TLegend* l_test2 = new TLegend(0.75, 0.7, 0.95, 0.95);
   l_test2->SetFillColor(0);
   l_test2->SetTextSize(0.025);
   l_test2->AddEntry(h_test2, Form("Entries : \t%.0f",h_test2->GetEntries()), "l");
   l_test2->AddEntry(h_test2, Form("Mean : \t%.3f",h_test2->GetMean()), "");
   l_test2->AddEntry(h_test2, Form("RMS  : \t%.3f",h_test2->GetRMS()), "");

   string test2_xtitle = "TDC Hit Time [nsec]";
   if(type_name == "Clusters") test2_xtitle = "Cluster Multiplicity";
   h_test2->SetTitle(Form("%s;%s;%s",title_name.c_str(),test2_xtitle.c_str(),"Entries"));
   h_test2->Draw("");
   l_test2->Draw("same");
   test2_xtitle.replace(12,7,"");

   c_test2->SaveAs(Form("%s_%s_%s_%s_%s.png",rootfile_name.c_str(),dec_name.c_str(),type_name.c_str(),event_number.c_str(),test2_xtitle.c_str()));

}

