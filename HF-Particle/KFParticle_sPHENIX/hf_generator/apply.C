#ifndef APPLY_C
#define APPLY_C

int
apply (
	std::string const& test_file_name = "dat/outputMinBiasKFParticle_Lc_pKpi_1.root",
	std::string const& branch_file_name = "branches.txt",
	std::string const& mass_branch_name = "Lambda_cplus_mass"
) {
	TMVA::Reader* reader = new TMVA::Reader("!Color:!Silent");

	// Get training variables from text file
	std::map<std::string, Float_t> variables;
	std::ifstream branch_file(branch_file_name, std::ios_base::in);
	for (std::string line; std::getline(branch_file, line);) {
		// whitespace
		line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
		line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());

		// comments
		for(std::size_t pos; (pos = line.find("#")) != std::string::npos;) {
			line = line.substr(0, pos);
		}

		// type must be float
		if (line[line.find("/") + 1] != 'F') {
			continue;
		};
		// and not an array
		if (line.find("[") != std::string::npos) {
			continue;
		};

		line = line.substr(0, line.find("/"));
		reader->AddVariable(line.c_str(), &(variables[line]));
	}
	// reader->BookMVA("BDT", "dataloader/weights/factory_BDT.weights.xml");
	reader->BookMVA("MLP", "dataloader/weights/factory_MLP.weights.xml");

	TFile* test_file = TFile::Open(test_file_name.c_str(), "READ");
	if (!test_file) {
		std::cerr << "file: " << test_file_name << std::endl;
		return EXIT_FAILURE;
	}
	TTree* test_tree = (TTree*)test_file->Get("DecayTree");
	if (!test_tree) {
		std::cerr << "tree: " << test_file_name << std::endl;
		return EXIT_FAILURE;
	}
	Float_t lc_mass;
	test_tree->SetBranchAddress(mass_branch_name.c_str(), &lc_mass);

	for (auto& [name, val] : variables) {
		if (!test_tree->GetBranch(name.c_str())) {
			std::cerr << "branch: " << name << std::endl;
			return EXIT_FAILURE;
		}
		test_tree->SetBranchAddress(name.c_str(), &val);
	}

	Long64_t counts = 0;
	std::map<Float_t, Long64_t> mass_pdf;
	for (Long64_t n = 0, N = test_tree->GetEntriesFast(); n < N; ++n) {
		test_tree->GetEntry(n);

		 // NaN filtering
		bool skip = false;
		for (auto const& [name, val] : variables) {
			if (val != val) skip = true;
		}
		if (skip) {
			// std::cout << "skipped nan" << std::endl;
			// for (auto const& [name, val] : variables) {
			// 	std::cout << name << ": " << val << std::endl;
			// }
			continue;
		}

		float bdt_val = reader->EvaluateMVA("MLP");
		// if (bdt_val < 0.11) continue; // -0.2249
		++mass_pdf[lc_mass];
		++counts;
	}
	std::cout << counts << "/" << test_tree->GetEntriesFast() << std::endl;

	Long64_t integrated = 0;
	Float_t quartile[5] = {};
	for (auto const& [mass, count] : mass_pdf) {
		for (int i = 0; i < 5; ++i) {
			if (integrated <= i * 0.2 * counts) {
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
	Float_t lower = 1.90; //quartile[2] - 12.0 * (quartile[3] - quartile[1]) / 2.0;
	Float_t upper = 2.50; //quartile[2] + 12.0 * (quartile[3] - quartile[1]) / 2.0;
	Int_t num_bins = 30; // (upper - lower) / bin_width;

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
	for ( auto const& [mass, count] : mass_pdf ) {
		hist->Fill(mass, count);
	}
	hist->Draw();

	return EXIT_SUCCESS;
}

#endif//APPLY_C
