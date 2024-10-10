#ifndef FIT_C
#define FIT_C

#include <sPhenixStyle.C>

#include <TCanvas.h>
#include <TTree.h>
#include <TFile.h>
#include <TTree.h>

#include <boost/format.hpp>

#include <cstddef>

#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <vector>

int
fit (
	std::vector<std::string> const& file_names = {
		// "dat/outputHFTrackEff_Lc_pKpi_.root",
		"dat/outputHFTrackEff_Lc_pKpi_0.root",
		"dat/outputHFTrackEff_Lc_pKpi_1.root",
		"dat/outputHFTrackEff_Lc_pKpi_2.root",
		// "dat/outputHFTrackEff_Lc_pKpi_3.root",
		// "dat/outputHFTrackEff_Lc_pKpi_4.root",
		// "dat/outputHFTrackEff_Lc_pKpi_5.root",
		// "dat/outputHFTrackEff_Lc_pKpi_6.root",
		// "dat/outputHFTrackEff_Lc_pKpi_7.root",
		// "dat/outputHFTrackEff_Lc_pKpi_8.root",
		// "dat/outputHFTrackEff_Lc_pKpi_9.root",
	},
	std::string const& tree_name = "HFTrackEfficiency"
) {
	SetsPhenixStyle();
	std::map<Float_t, Long64_t> reco_mass_map;
	Long64_t counts = 0;

	for ( auto const& file_name : file_names ) {
		TFile* file = TFile::Open(file_name.c_str(), "READ");
		if ( !file ) {
			std::cerr << "file" << std::endl;
			return EXIT_FAILURE;
		}
	
		TTree* tree = dynamic_cast<TTree*>(file->Get(tree_name.c_str()));
		if ( !tree ) {
			std::cerr << "tree" << std::endl;
			return EXIT_FAILURE;
		}
	
		Float_t reco_mother_mass;
		tree->SetBranchAddress("reco_mother_mass", &reco_mother_mass);
	
		for ( Int_t n = 0, N = tree->GetEntriesFast(); n < N; ++n ) {
			tree->GetEntry(n);
			if ( reco_mother_mass != reco_mother_mass) continue; // nan
			++reco_mass_map[reco_mother_mass];
			++counts;
		}

		file->Close();
	}

	Long64_t integrated = 0;
	Float_t quartile[5] = {};
	for (auto const& [mass, count] : reco_mass_map) {
		for (int i = 0; i < 5; ++i) {
			if (integrated < i * 0.2 * counts) {
				quartile[i] = mass;
			}
		}
		integrated += count;
	}
	for (int i = 0; i < 5; ++i) {
		std::cout << quartile[i] << std::endl;
	}

	// Scott / Freedman-Diaconis
	Float_t bin_width = pow( 24 * sqrt(3.14159265358979) / counts, 1.0 / 3.0 ) * (quartile[3] - quartile[1]);
	Float_t lower = quartile[2] - 12.0 * (quartile[3] - quartile[1]) / 2.0;
	Float_t upper = quartile[2] + 12.0 * (quartile[3] - quartile[1]) / 2.0;
	Int_t num_bins = (upper - lower) / bin_width;

	// Canvas
	TCanvas* cnvs = new TCanvas("L_C prompt mass", "L_C prompt mass", 800, 800);
	cnvs->SetFillStyle(4000);

	cnvs->cd();
	TPad* hist_pad = new TPad("hist_pad", "hist_pad", 0.0, 0.0, 1.0, 0.8);
	hist_pad->Range(0.0, 0.0, 1.0, 1.0);
	hist_pad->SetFillStyle(4000);
	hist_pad->Draw();
	hist_pad->cd();

	// Hist
	TH1F* hist = new TH1F (
		"L_c_reco_hist", "L_c_reco_hist",
		num_bins, lower, upper
	);
	hist->GetXaxis()->SetTitle("#Lambda_{C} #mbox{reco mass (GeV)}");
	hist->GetYaxis()->SetTitle("Counts");
	for ( auto const& [mass, count] : reco_mass_map ) {
		hist->Fill(mass, count);
	}
	hist->Draw();

	// Fit
	TF1* gaus = new TF1 (
		"L_c_reco_fit", "gaus", lower, upper
	);
	gaus->SetParameter(1, quartile[2]);
	gaus->SetParameter(2, (quartile[3] - quartile[1]) / 2.0);
	gaus->SetParLimits(2, 0, quartile[3] - quartile[1]);
	gaus->SetLineColor(kRed);
	hist->Fit(gaus); //, "L");

	gaus->Draw("same");

	cnvs->cd();
	TPad* label_pad = new TPad("label_pad", "label_pad", 0.0, 0.8, 1.0, 1.0);
	label_pad->Range(0.0, 0.0, 1.0, 1.0);
	label_pad->SetFillStyle(4000);
	label_pad->Draw();
	label_pad->cd();

	// Labels
	TLatex latex;
	latex.SetTextAlign(23);
	latex.SetTextSize(0.15);
	latex.DrawLatex(0.5, 0.75, "#Lambda_{C} Prompt Mass Distribution");

	latex.SetTextSize(0.10);
	latex.DrawLatex(0.5, 0.50, (boost::format("#mu = %.4f #pm %0.4f #mbox{GeV}") % gaus->GetParameter(1) % gaus->GetParameter(2)).str().c_str());
	latex.DrawLatex(0.5, 0.25, (boost::format("N = %d") % (int)counts).str().c_str());

	cnvs->SaveAs("L_c_reco_mass.png");
	// std::this_thread::sleep_for(std::chrono::seconds(10));
	cnvs->Close();

	return EXIT_SUCCESS;
}

#endif//FIT_C
