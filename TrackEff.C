//
//
// Original Author:  Sungwoong Cho (sungwoong.cho@cern.ch), Jaehoon Lim (jae.hoon.lim@cern.ch)
//                   588 R-012, +41 22 76 73278
//         Created:  2015 Oct 14
//

void TrackEff(string last_hv_root, string trigger_condition, string eff_dec){

   gROOT->Reset();
   gROOT->SetStyle("Plain");
   gStyle->SetOptStat(0);	
   gStyle->SetMarkerSize(0.8);
   gStyle->SetPalette(1);

   // FIXME ////////////////
   string hist_title = "";
   Float_t HV[9]      = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
   Float_t HV_Err[9]  = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
   /////////////////////////

   TFile *f_trk[9];
   TTree *t_trk[9];
   Float_t Eff[9];
   Float_t Eff_Err[9];

   for(string::size_type pos = 0; (pos = trigger_condition.find("s", pos)) != string::npos;){
      trigger_condition.replace(pos, 1, "S");
      pos += 1;
   }
   for(string::size_type pos = 0; (pos = eff_dec.find("s", pos)) != string::npos;){
      eff_dec.replace(pos, 1, "S");
      pos += 1;
   }

   string s_num = eff_dec;
   string s_den = trigger_condition; 
   for(string::size_type pos = 0; (pos = s_num.find("S", pos)) != string::npos;){
      s_num.replace(pos, 1, "Track_ClusterOnS");
      pos += 16;
   }
   for(string::size_type pos = 0; (pos = s_den.find("S", pos)) != string::npos;){
      s_den.replace(pos, 1, "Track_ClusterOnS");
      pos += 16;
   }

   TCut cut_num = Form("Track_isMuonTrack && (%s) && (%s)", s_den.c_str(), s_num.c_str());
   TCut cut_den = Form("Track_isMuonTrack && (%s)", s_den.c_str());

   int HV_point = (last_hv_root.find("HV"))+2;
   int num_HV = atoi(last_hv_root.substr(HV_point,1).c_str());

   TCanvas* c_test = new TCanvas("c_test","c_test",1,1);

   for(int i=0; i<num_HV; ++i){
      f_trk[i] = new TFile(last_hv_root.replace(HV_point,1,Form("%d",i+1)).c_str());
      t_trk[i] = (TTree*)f_trk[i]->Get("KODEL_Tree");
      TH1F* h_num = new TH1F("h_num", "h_num", 2, 0, 2);
      TH1F* h_den = new TH1F("h_den", "h_den", 2, 0, 2);
      h_num->Sumw2();
      h_den->Sumw2();
      t_trk[i]->Draw(Form("%s >> h_num",s_num.c_str()), cut_num);
      t_trk[i]->Draw(Form("%s >> h_den",s_num.c_str()), cut_den);
      Eff[i] = 100. * h_num->GetEntries() / h_den->GetEntries();
      if(h_num->GetEntries() > 0 && h_den->GetEntries() > 0){
         Eff_Err[i] = 100. * h_num->GetEntries()/h_den->GetEntries() * sqrt(1./h_num->GetEntries()+1./h_den->GetEntries());
      } else {
         Eff_Err[i] = 0.0;
      }
      cout << "HV" << i+1 << " Eff[%]: " << Eff[i] << " +/- " << Eff_Err[i] << endl;

      if(HV[i] == 0) HV[i] = i+1;

      delete h_num;
      delete h_den;
   }
   delete t_trk;
   delete f_trk;
   delete c_test;

   TCanvas* c_eff = new TCanvas("c_eff", "c_eff", 1000, 700);
   c_eff->SetGrid();
   c_eff->SetFillColor(0);
   c_eff->SetFillStyle(4000);

   c_eff->cd();
   float Frame_x_start = 0;
   float Frame_x_end   = num_HV+1;
   if(HV[num_HV-1] != num_HV){
      Frame_x_start = HV[0] - 100;
      Frame_x_end   = HV[num_HV-1] + 100;
   } 
   TH1F* Frame_eff = (TH1F*)c_eff->DrawFrame(Frame_x_start, 0, Frame_x_end, 110);
   Frame_eff->GetYaxis()->SetTitle("Efficiency [%]");
   Frame_eff->GetYaxis()->SetTitleOffset(1.1);
   Frame_eff->GetYaxis()->CenterTitle(true);
   Frame_eff->GetYaxis()->SetLabelSize(0.037);
   Frame_eff->GetXaxis()->SetTitle("HV_{eff.} [V]");
   Frame_eff->GetXaxis()->SetTitleOffset(1.1);
   Frame_eff->GetXaxis()->CenterTitle(true);
   Frame_eff->GetXaxis()->SetLabelSize(0.037);
   if(hist_title == "") hist_title = Form("%s Efficiency (Trigger : %s)", eff_dec.c_str(), trigger_condition.c_str());
   Frame_eff->SetTitle(hist_title.c_str());
    
   TGraphErrors* graph_Plot = new TGraphErrors(num_HV, HV, Eff, HV_Err, Eff_Err);
   graph_Plot->SetMarkerStyle(20);
   graph_Plot->SetMarkerSize(0.8);
   graph_Plot->SetMarkerColor(1);
   graph_Plot->SetLineStyle(2);
   graph_Plot->SetLineColor(1);
   graph_Plot->SetLineWidth(1);
   graph_Plot->Draw("lpe");
    
   c_eff->SaveAs(Form("Eff_%s.png",eff_dec.c_str()));
}
