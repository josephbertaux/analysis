#ifndef TRAIN_C
#define TRAIN_C

#include <boost/format.hpp>

int
train (
	std::vector<std::string> const& signal_file_names = {
		"dat/outputKFParticle_Lc_pKpi_0.root",
	},
	std::vector<std::string> const& background_file_names = {
		"dat/outputMinBiasKFParticle_Lc_pKpi_0.root",
	},
	std::string const& factory_file_name = "dat/factory_output.root",
	std::string const& branch_file_name = "branches.txt",
	std::string const& mass_branch_name = "Lambda_cplus_mass",
	float mean =  2.27977e+00, float sigma = 1.30624e-02
) {
	TFile* factory_file = TFile::Open(factory_file_name.c_str(), "RECREATE");
	if (!factory_file) {
		std::cerr << "file: " << factory_file_name << std::endl;
		return EXIT_FAILURE;
	}

	TMVA::Factory* factory = new TMVA::Factory(
		"factory", factory_file,
		"!V:!Silent:AnalysisType=Classification"
	);
	TMVA::DataLoader* dataloader = new TMVA::DataLoader("dataloader");

	for (auto const& signal_file_name : signal_file_names) {
		TFile* signal_file = TFile::Open(signal_file_name.c_str(), "READ");
		if (!signal_file) {
			std::cerr << "file: " << signal_file_name << std::endl;
			return EXIT_FAILURE;
		}
		TTree* signal_tree = (TTree*)signal_file->Get("DecayTree");
		if (!signal_tree) {
			std::cerr << "tree: " << signal_file_name << std::endl;
			return EXIT_FAILURE;
		}
		dataloader->AddSignalTree(signal_tree);
	}

	for (auto const& background_file_name : background_file_names) {
		TFile* background_file = TFile::Open(background_file_name.c_str(), "READ");
		if (!background_file) {
			std::cerr << "file: " << background_file_name << std::endl;
			return EXIT_FAILURE;
		}
		TTree* background_tree = (TTree*)background_file->Get("DecayTree");
		if (!background_tree) {
			std::cerr << "tree: " << background_file_name << std::endl;
			return EXIT_FAILURE;
		}
		dataloader->AddBackgroundTree(background_tree);
	}

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
		variables[line] = 0;
		dataloader->AddVariable(line.c_str());
	}

	// sideband cut for background
	boost::format cut = boost::format("(%f < %s && %s < %f) || (%f < %s && %s < %f)")
		% (mean - 6.0 * sigma) % (mass_branch_name) % (mass_branch_name) % (mean - 3.0 * sigma)
		% (mean + 3.0 * sigma) % (mass_branch_name) % (mass_branch_name) % (mean + 6.0 * sigma);
	TCut sideband = cut.str().c_str();
	dataloader->AddCut(sideband, "Background");

	for (auto const& [name, val] : variables) {
		cut = boost::format("!(%s != %s)")
			% name % name;
		TCut no_nan = cut.str().c_str();
		dataloader->AddCut(no_nan, "Signal");
		dataloader->AddCut(no_nan, "Background");
	}

	factory->BookMethod (
		// dataloader, TMVA::Types::kBDT, "BDT",
		// "!H:!V:NTrees=400:MinNodeSize=5%:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20"
		dataloader, TMVA::Types::kMLP, "MLP",
		"H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:!UseRegulator"
	);

	factory->TrainAllMethods();
	factory->TestAllMethods();
	factory->EvaluateAllMethods();

	factory_file->Close();
	delete dataloader;
	delete factory;

	TMVA::TMVAGui(factory_file_name.c_str());

	return EXIT_SUCCESS;
}

#endif//TRAIN_C
