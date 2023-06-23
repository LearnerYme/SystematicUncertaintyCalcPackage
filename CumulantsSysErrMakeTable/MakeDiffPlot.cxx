#include <iostream>

#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TMath.h"
#include "TGraphErrors.h"

#include "Point.h"

#include "GraphLoader.h"

using std::cout;
using std::endl;

using ygse::Point;

bool IsPassedBarlow(Point pdef, Point pvrd);

int main(int argc, char** argv) {

    /*
        Args:
        :folder path: a string, the path to the root files (root path)
        :rapidity index: a string, like cum.cbwc.trad.y5
        :particle tag: Netp, Pro or Pbar
        :var idx: 0 to 3 for C1 ~ C4, 4 to 6 for R21 ~ R42, 7 to 10 for k1 ~ k4, 11 to 13 fork21 ~ k41, ...
        :cent tag: centrality tag 0 to 8
        :fig path: to save the plot
    */

    GraphLoader* gl = new GraphLoader(argv[1]);
    const char* rapidity_index = argv[2];
    const char* particle_tag = argv[3];
    const char* var_idx = argv[4];
    const char* cent_tag = argv[5];
    const char* fig_path = argv[6];

    int cent_idx = std::atoi(cent_tag);
    if (cent_idx < 0 || cent_idx > 8) {
        std::cout << "[ERROR] Centrality Index should be 0 to 8 but got " << cent_idx << ".\n";
        return -1;
    }

    const int nCent = 9;
    const int nSource = 6;
    const int nCut = 4;

    const char* cent_titles[nCent] = {
        "0~5%", 
        "5~10%", 
        "10~20%", 
        "20~30%", 
        "30~40%", 
        "40~50%", 
        "50~60%", 
        "60~70%", 
        "70~80%"
    };

    const char* var_tags[14] = {
        "C1", "C2", "C3", "C4", "R21", "R32", "R42", 
        "k1", "k2", "k3", "k4", "k21", "k31", "k41"
    };
    const char* var_titles[14] = {
        "C_{1}", "C_{2}", "C_{3}", "C_{4}", "C_{2}/C_{1}", "C_{3}/C_{2}", "C_{4}/C_{2}", 
        "#kappa_{1}", "#kappa_{2}", "#kappa_{3}", "#kappa_{4}", "#kappa_{2}/#kappa_{1}", "#kappa_{3}/#kappa_{1}", "#kappa_{4}/#kappa_{1}"
    };

    const char* var_tag = var_tags[std::atoi(var_idx)];
    const char* var_title = var_titles[std::atoi(var_idx)];

    const int nCuts = 20;
    const char* cutsFolderList[nCuts] = { // read root files
        "default.coll", 
        "dca0p8.coll", "dca0p9.coll", "dca1p1.coll", "dca1p2.coll", 
        "nhits15.coll", "nhits18.coll", "nhits22.coll", "nhits25.coll", 
        "nsig1p6.coll", "nsig1p8.coll", "nsig2p2.coll", "nsig2p5.coll", 
        "mass21.coll", "mass22.coll", "mass23.coll", "mass24.coll", 
        "eff1.coll", "eff2.coll", 
        "pidmode.coll"
    };

    const char* cutTags[nCuts] = { // x labels
        "Default", 
        "DCA < 0.8", "DCA < 0.9", "DCA < 1.1", "DCA < 1.2", 
        "nHitsFit > 15", "nHitsFit > 18", "nHistFit > 22", "nHisFit > 25", 
        "|n#sigma-#delta| < 1.6", "|n#sigma-#delta| < 1.8", "|n#sigma-#delta| < 2.2", "|n#sigma-#delta| < 2.5", 
        "m^{2}#in(0.50,1.10)", "m^{2}#in(0.55,1.15)", "m^{2}#in(0.65,1.25)", "m^{2}#in(0.70,1.30)", 
        "#epsilon#times1.02", "#epsilon#times0.98", 
        "#gamma_{PID} varied"
    };

    double values[nCuts];
    double errors[nCuts]; // statistic error for each cut
    bool pass_barlow[nCuts];
    
    const int ms_passed = 20;
    const int ms_failed = 24;
    const int mc_passed = 1;
    const int mc_failed = 2;

    for (int i=0; i<nCuts; i++) {
        values[i] = 0.0;
        errors[i] = 0.0;
        pass_barlow[i] = false;
    }

    Point pdef;
    Point pvrd;

    gl->GetPoint(cutsFolderList[0], rapidity_index, particle_tag, var_tag, cent_idx, &pdef);

    for (int i=0; i<nCuts; i++) { // get varied
        gl->GetPoint(cutsFolderList[i], rapidity_index, particle_tag, var_tag, cent_idx, &pvrd);
        values[i] = pvrd.GetValue();
        errors[i] = pvrd.GetError();
        if (i == 0) { // default can always pass the Barlow Check
            pass_barlow[i] = true;
        } else {
            pass_barlow[i] = IsPassedBarlow(pdef, pvrd);
        }
    }

    // now prepare the TGraph
    TGraphErrors* tgp = new TGraphErrors(); // passed
    TGraphErrors* tgf = new TGraphErrors(); // failed

    tgp->SetMarkerColor(mc_passed);
    tgf->SetMarkerColor(mc_failed);
    tgp->SetLineColor(mc_passed);
    tgf->SetLineColor(mc_failed);
    tgp->SetMarkerStyle(ms_passed);
    tgf->SetMarkerStyle(ms_failed);

    int n_passed = 0;
    int n_failed = 0;
    for (int i=0; i<nCuts; i++) {
        if (pass_barlow[i]) {
            n_passed ++;
        } else {
            n_failed ++;
        }
    }

    tgp->Set(n_passed);
    tgf->Set(n_failed);

    int i_passed = 0;
    int i_failed = 0;
    for (int i=0; i<nCuts; i++) {
        if (pass_barlow[i]) {
            tgp->SetPoint(i_passed, i, values[i]);
            tgp->SetPointError(i_passed, 0.0, errors[i]);
            i_passed ++;
        } else {
            tgf->SetPoint(i_failed, i, values[i]);
            tgf->SetPointError(i_failed, 0.0, errors[i]);
            i_failed ++;
        }
    }

    // get the limitation of y axis
    double vmax = -999;
    double vmin = 999;
    for (int i=0; i<nCuts; i++) {
        double vhigh = values[i] + errors[i];
        vmax = vhigh > vmax ? vhigh : vmax;
        double vlow = values[i] - errors[i];
        vmin = vlow < vmin ? vlow : vmin;
    }
    const double y_range_fac = 1.5;
    double dist = vmax - vmin;
    double vmean = 0.5 * (vmax + vmin);
    vmax = vmean + y_range_fac * dist;
    vmin = vmean - y_range_fac * dist;

    TCanvas* c = new TCanvas();
    c->cd();
    gPad->SetBottomMargin(0.2);
    TH1F* frame = (TH1F*)gPad->DrawFrame(-1, vmin, nCuts, vmax);

    // set x label
    for (int i=0; i<nCuts; i++) {
        frame->GetXaxis()->SetBinLabel(frame->FindBin(i), cutTags[i]);
    }

    TLatex* lat = new TLatex();
    lat->DrawLatexNDC(0.15, 0.85, var_title);
    lat->DrawLatexNDC(0.35, 0.85, cent_titles[cent_idx]);

    TLegend* leg = new TLegend(0.65, 0.75, 0.85, 0.9);
    leg->AddEntry(tgp, "Passed Barlow Check");
    leg->AddEntry(tgf, "Failed to pass Barlow Check");
    leg->Draw("same");

    tgp->Draw("epsame");
    tgf->Draw("epsame");
    c->Print(Form("%s/%s_%s.pdf", fig_path, var_tag, cent_tag));

    return 0;
}

bool IsPassedBarlow(Point pdef, Point pvrd) {
    double vdiff2 = TMath::Power(pdef.GetValue() - pvrd.GetValue(), 2);
    double ediff2 = fabs(TMath::Power(pdef.GetError(), 2) - TMath::Power(pvrd.GetError(), 2)); 
    return vdiff2 > ediff2;
}