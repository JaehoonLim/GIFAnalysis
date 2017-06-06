//
//
// Original Author:  Sungwoong Cho (sungwoong.cho@cern.ch), Jaehoon Lim (jae.hoon.lim@cern.ch)
//                   588 R-012, +41 22 76 73278
//         Created:  2015 Oct 14
//

void TrackEff(string last_hv_root, string trigger_condition, string eff_dec){

   if(eff_dec.size() == 0){
      cerr << "ERROR : No Target Chamber" << endl;
      exit(EXIT_FAILURE);
   }

   gROOT->Reset();
   gROOT->SetStyle("Plain");
   gStyle->SetOptStat(0);	
   gStyle->SetMarkerSize(0.8);
   gStyle->SetPalette(1);

   // FIXME ////////////////
   string hist_title = "";
   Float_t HV[20]      = {0.0};
   Float_t HV_Err[20]  = {0.0};
   /////////////////////////

   TFile *f_trk[20];
   TTree *t_trk[20];
   Float_t Eff[20];
   Float_t Eff_Err[20];

   for(string::size_type pos = 0; (pos = trigger_condition.find("t", pos)) != string::npos;){
      trigger_condition.replace(pos, 1, "T");
      pos += 1;
   }
   for(string::size_type pos = 0; (pos = trigger_condition.find("s", pos)) != string::npos;){
      trigger_condition.replace(pos, 1, "S");
      pos += 1;
   }
   for(string::size_type pos = 0; (pos = trigger_condition.find("||", pos)) != string::npos;){
      trigger_condition.replace(pos, 2, " || ");
      pos += 4;
   }
   for(string::size_type pos = 0; (pos = trigger_condition.find("&&", pos)) != string::npos;){
      trigger_condition.replace(pos, 2, " && ");
      pos += 4;
   }
   string s_den = trigger_condition; 

   for(string::size_type pos = 0; (pos = eff_dec.find("t", pos)) != string::npos;){
      eff_dec.replace(pos, 1, "T");
      pos += 1;
   }
   for(string::size_type pos = 0; (pos = eff_dec.find("s", pos)) != string::npos;){
      eff_dec.replace(pos, 1, "S");
      pos += 1;
   }
   for(string::size_type pos = 0; (pos = eff_dec.find("||", pos)) != string::npos;){
      eff_dec.replace(pos, 2, " || ");
      pos += 4;
   }
   for(string::size_type pos = 0; (pos = eff_dec.find("&&", pos)) != string::npos;){
      eff_dec.replace(pos, 2, " && ");
      pos += 4;
   }
   string s_num = eff_dec;

   if(s_den == "Simple"){
      for(string::size_type pos = 0; (pos = s_num.find("T", pos)) != string::npos;){
         s_num.replace(pos, 1, "Info_HasMuonHitOnT");
         pos += 18;
      }
      s_den = "Info_NumMuonTrack>0";
   } else {
      for(string::size_type pos = 0; (pos = s_den.find("T", pos)) != string::npos;){
         s_den.replace(pos, 1, "Track_ClusterOnT");
         pos += 16;
      }
      for(string::size_type pos = 0; (pos = s_num.find("T", pos)) != string::npos;){
         s_num.replace(pos, 1, "Track_ClusterOnT");
         pos += 16;
      }

      if(s_den.size()>0){
         s_den = "Track_isMuonTrack && (" + s_den + ")";
      } else {
         s_den = "Track_isMuonTrack";
      }
   }

   TCut cut_num = Form("%s && (%s)", s_den.c_str(), s_num.c_str());
   TCut cut_den = Form("%s", s_den.c_str());

   int HV_point = (last_hv_root.find("HV"))+2;
   int daq_pos = (last_hv_root.find("DAQ"))-1;
   int num_HV = atoi(last_hv_root.substr(HV_point,daq_pos-HV_point).c_str());

   TCanvas* c_test = new TCanvas("c_test","c_test",1,1);
   float EffMax = 0.0; 

   for(int i=0; i<num_HV; ++i){
      HV_point = (last_hv_root.find("HV"))+2;
      daq_pos = (last_hv_root.find("DAQ"))-1;
      f_trk[i] = new TFile(last_hv_root.replace(HV_point,daq_pos-HV_point,Form("%d",i+1)).c_str());
      t_trk[i] = (TTree*)f_trk[i]->Get("KODEL_Tree");
      TH1F* h_num = new TH1F("h_num", "h_num", 2, 0, 2);
      TH1F* h_den = new TH1F("h_den", "h_den", 2, 0, 2);
      h_num->Sumw2();
      h_den->Sumw2();
      t_trk[i]->Draw(Form("%s >> h_num",s_num.c_str()), cut_num);
      t_trk[i]->Draw(Form("%s >> h_den",s_num.c_str()), cut_den);
      Eff[i] = 100. * h_num->GetEntries() / h_den->GetEntries();
      if(EffMax < Eff[i]) EffMax = Eff[i];
      if(h_num->GetEntries() > 0 && h_den->GetEntries() > 0){
   //      Eff_Err[i] = 100. * h_num->GetEntries()/h_den->GetEntries() * sqrt(1./h_num->GetEntries()+1./h_den->GetEntries());
         Eff_Err[i] = 100. * 
                      sqrt( h_num->GetEntries() / h_den->GetEntries() 
                            * (1.- ( h_num->GetEntries() / h_den->GetEntries() ) ) / 5000.0 );
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
   if(hist_title == "") {
      if(trigger_condition == ""){
         hist_title = Form("%s Efficiency (Default Trigger Only)", eff_dec.c_str());
      } else if(trigger_condition == "Simple"){
         hist_title = Form("%s Efficiency (Hit Based Simple Calculation)", eff_dec.c_str());
      } else {
         hist_title = Form("%s Efficiency (Trigger : %s)", eff_dec.c_str(), trigger_condition.c_str());
      }
   }
   Frame_eff->SetTitle(hist_title.c_str());
    
   TGraphErrors* graph_Plot = new TGraphErrors(num_HV, HV, Eff, HV_Err, Eff_Err);
   graph_Plot->SetMarkerStyle(20);
   graph_Plot->SetMarkerSize(0.8);
   graph_Plot->SetMarkerColor(1);
   graph_Plot->SetLineStyle(2);
   graph_Plot->SetLineColor(1);
   graph_Plot->SetLineWidth(1);
   graph_Plot->Draw("lpe");
    
   last_hv_root.erase(last_hv_root.find("HV"),last_hv_root.size()-1);
   c_eff->SaveAs(Form("%sEff_%s.png",last_hv_root.c_str(),eff_dec.c_str()));

   double sigmoid(double *x, double *par){
      return par[0]/(1+exp(-1*par[1]*(x[0]-par[2])));
   }
   TF1 *fitfunc = new TF1("fitfunc",sigmoid,HV[0]-HV[num_HV-1],2*HV[num_HV-1],3);
   fitfunc->SetLineStyle(2);
   fitfunc->SetLineWidth(3);
   fitfunc->SetLineColor(kRed);
   fitfunc->SetParameter(0,90);
   fitfunc->SetParLimits(0,0,100);
   fitfunc->SetParameter(2,HV[0]);

   graph_Plot->SetLineStyle(1);
   graph_Plot->SetLineWidth(3);
   c_eff->Update();
   graph_Plot->Fit("fitfunc","","L",0,2*HV[num_HV-1]);

   float HVWP = fitfunc->GetX(0.95*fitfunc->GetParameter(0),HV[0],HV[num_HV-1],0.01) + 100;
   float EFWP = fitfunc->Eval(HVWP);
   TLine* wpline = new TLine();
   wpline->SetLineStyle(2);
   wpline->SetLineColor(kBlue);
   wpline->DrawLine(Frame_x_start,EFWP,Frame_x_end,EFWP);
   wpline->DrawLine(HVWP,0,HVWP,110);

   TLegend* sigmoid_legend = new TLegend(0.58, 0.15, 0.89, 0.37);
   sigmoid_legend->SetFillColor(0);
   sigmoid_legend->SetTextSize(0.03);
   sigmoid_legend->SetNColumns(2);
   sigmoid_legend->AddEntry((TObject*)0,"#chi^{2} / NDF","");
   sigmoid_legend->AddEntry((TObject*)0,Form("%0.4f / %d",fitfunc->GetChisquare(), fitfunc->GetNDF()),"");
   sigmoid_legend->AddEntry((TObject*)0,"#epsilon_{MAX}","");
   sigmoid_legend->AddEntry((TObject*)0,Form("%0.2f #pm %0.2f",fitfunc->GetParameter(0),fitfunc->GetParError(0)),"");
   sigmoid_legend->AddEntry((TObject*)0,"#lambda","");
   sigmoid_legend->AddEntry((TObject*)0,Form("%0.4f #pm %0.4f",fitfunc->GetParameter(1),fitfunc->GetParError(1)),"");
   sigmoid_legend->AddEntry((TObject*)0,"HV_{50}","");
   sigmoid_legend->AddEntry((TObject*)0,Form("%0.2f #pm %0.2f",fitfunc->GetParameter(2),fitfunc->GetParError(2)),"");
   sigmoid_legend->AddEntry((TObject*)0,"HV_{WP}","");
   sigmoid_legend->AddEntry((TObject*)0,Form("%0.2f",HVWP),"");
   sigmoid_legend->AddEntry((TObject*)0,"Eff.@WP","");
   sigmoid_legend->AddEntry((TObject*)0,Form("%0.2f",EFWP),"");
   sigmoid_legend->Draw();

   c_eff->SaveAs(Form("%sEff_%s_sigmoid.png",last_hv_root.c_str(),eff_dec.c_str()));

}
