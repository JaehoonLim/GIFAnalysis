//
//
// Original Author:  Sungwoong Cho (sungwoong.cho@cern.ch), Jaehoon Lim (jae.hoon.lim@cern.ch)
//                   588 R-012, +41 22 76 73278
//         Created:  2015 Oct 14
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <map>
#include <iterator>
#include <TROOT.h>
#include <TH1.h>
#include <TFile.h>
#include <TTree.h>

using namespace std;

int main(int argc, char **argv){

    if(argc != 6) {
        cerr << endl << "\tERROR : Check Input arguments" << endl;
        cerr << "\t" << argv[0] << " InputRootFile MAPConfigFile Cut_PercentageAfterCut Cut_TimeWindow Cut_RMSAfterCut" << endl << endl;
        exit(EXIT_FAILURE);
    }
    
    TFile *inputfile = new TFile( argv[1] );
    if(!inputfile->IsOpen()) {
        cerr << endl << "\tERROR : Check Input Root File" << endl << endl;
        exit(EXIT_FAILURE);
    }
    ifstream infile( argv[2] );
    if(!infile.is_open()) {
        cerr << endl << "\tERROR : Check MAP config File" << endl << endl;
        exit(EXIT_FAILURE);
    }

    float cut_entry = atof(argv[3]);
    float cut_timewindow = atof(argv[4]);
    float cut_rms = atof(argv[5]);

    string outputname = argv[2];
    outputname.replace(outputname.find("MAP"),3,"CAL");
    ofstream outfile( outputname );
    TTree *inputtree = (TTree*)inputfile->Get("RAWData");

    typedef vector<int>               vec_i;
    typedef vector<string>            vec_s;
    typedef map<int,string>           map_is;
    typedef map<int,float>            map_if;
    typedef map<int,string>::iterator it_is;
    typedef map<int,float>::iterator  it_if;

    vec_i TDC_STRIP;
    vec_s NAME_PARTITION;
    map_is TDCtoNAME;
    map_if TDCtoCAL;

    string temp_string;
    int channel_start = 0;
    int channel_end   = 0;
    float Total_time = 0;
    int Total_strip = 0;
    float strip_mean = 0;
    float chamber_mean = 0;

    TH1F* Chamber_mean = new TH1F("Chamber_mean","Chamber_mean",600,0,600);
    TH1F* Strip_mean = new TH1F("Strip_mean","Strip_mean",600,0,600);
    TH1F* BackSubMean = new TH1F("BackSubMean","BackSubMean",600,0,600);

    outfile << "// CALIBRATION config file" << endl << "//" << endl << "// TDCchannel'\\t'" << '\t' << "CalibratedTime" << endl << "//" << endl;

    while (infile) {
        string s_line;
        if (!getline( infile, s_line )) break;
        istringstream ss_line( s_line );

        if(isdigit(*(s_line.begin()))) {
            TDC_STRIP.clear();
            while (ss_line) {
                if (!getline( ss_line, temp_string, '\t' )) break;
                TDC_STRIP.push_back(atoi(temp_string.c_str()));
            }
            TDCtoNAME[TDC_STRIP.at(1)] = NAME_PARTITION.at(0);
            while(TDC_STRIP.at(0) != TDC_STRIP.at(1)){
                TDCtoNAME[TDC_STRIP.at(0)] = NAME_PARTITION.at(0);
                (TDC_STRIP.at(0)>TDC_STRIP.at(1)?--(TDC_STRIP.at(0)):++(TDC_STRIP.at(0)));
            }
        } else if(!(s_line.empty()) && s_line.find("/") != 0) {
            NAME_PARTITION.clear();
            while (ss_line){
                if (!getline( ss_line, temp_string, '\t' )) break;
                NAME_PARTITION.push_back(temp_string);
                strip_mean = 0;
                chamber_mean = 0;
            }
        } else if (s_line.find("/") != 0) {

            Total_time = 0;
            Total_strip = 0;        
            channel_start = (TDCtoNAME.begin())->first;
            channel_end   = (TDCtoNAME.rbegin())->first;
            inputtree->Draw("TDC_TimeStamp>>Chamber_mean",Form("TDC_channel>=%d && TDC_channel<=%d",channel_start,channel_end));
            strip_mean = (float)Chamber_mean->GetEntries()/300;
            for(int bin_index = 0;bin_index<=600;++bin_index){
                if(Chamber_mean->GetBinContent(bin_index) > strip_mean){
                    BackSubMean->SetBinContent(bin_index,Chamber_mean->GetBinContent(bin_index) - strip_mean);
                } else {
                    BackSubMean->SetBinContent(bin_index,0);
                }
            }
            chamber_mean = BackSubMean->GetMean();
            cout << endl << (TDCtoNAME.begin())->second << " : " << Chamber_mean->GetMean() << " -> " << chamber_mean << "(" << BackSubMean->GetRMS() << ")" << endl;

            for(it_is it_TDC = TDCtoNAME.begin();it_TDC != TDCtoNAME.end(); ++it_TDC){
                inputtree->Draw("TDC_TimeStamp>>Strip_mean",Form("TDC_channel==%d",it_TDC->first));
                cout << it_TDC->first << " : " << Strip_mean->GetMean();
                if(Strip_mean->GetEntries()>0){
                    strip_mean = (float)Strip_mean->GetEntries()/300;
                    for(int bin_index = 0;bin_index<=600;++bin_index){
                        if(Strip_mean->GetBinContent(bin_index) > strip_mean){
                            BackSubMean->SetBinContent(bin_index,Strip_mean->GetBinContent(bin_index) - strip_mean);
                        } else {
                            BackSubMean->SetBinContent(bin_index,0);
                        }
                    }
                    if(BackSubMean->Integral() > cut_entry/100*Strip_mean->GetEntries()){ 
                        if(BackSubMean->GetMean() > chamber_mean-cut_timewindow && BackSubMean->GetMean() < chamber_mean+cut_timewindow) {
                            if(BackSubMean->GetRMS() < cut_rms) {
                                TDCtoCAL[it_TDC->first] = BackSubMean->GetMean();
                                Total_time += BackSubMean->GetMean();
                                ++Total_strip;
                                cout << "\t -> " << BackSubMean->GetMean() << "(" << BackSubMean->GetRMS() << ")";
                            } else {
                                cout << "\t - Broad Peak - " << cut_rms << "ns (RMS : " << BackSubMean->GetRMS() << ")";
                            }
                        } else {
                            cout << "\t - Out of Beam Window - +/-" << cut_timewindow << " ns (Mean : " << BackSubMean->GetMean() << ")";
                        }
                    } else {
                        cout << "\t - Not Enought Entries - below " << cut_entry << "% (" << BackSubMean->Integral() << "/" << Strip_mean->GetEntries() << ")";
                    }
                } else {
                    cout << "\t - No Entry";
                }
                cout << endl;
            }
            cout << "Calibration Failed Channel (" << Total_time/Total_strip << ") : ";
            for(it_is it_TDC = TDCtoNAME.begin();it_TDC != TDCtoNAME.end(); ++it_TDC){
                if(TDCtoCAL.find(it_TDC->first) == TDCtoCAL.end()){
                    TDCtoCAL[it_TDC->first] = Total_time/Total_strip;
                    cout << it_TDC->first << " ";
                }
            }
            cout << endl;
            for(it_if it_CAL = TDCtoCAL.begin();it_CAL != TDCtoCAL.end(); ++it_CAL){
                outfile << it_CAL->first << '\t' << it_CAL->second << endl;
            }
            TDCtoNAME.clear();
            TDCtoCAL.clear();
        }
    }
}
