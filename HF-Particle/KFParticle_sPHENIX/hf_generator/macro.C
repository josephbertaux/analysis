#ifndef MACRO_C
#define MACRO_C

#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
R__LOAD_LIBRARY(libfun4all.so)

#include <lcreco/LcReco.h>
R__LOAD_LIBRARY(libLcReco.so)

R__LOAD_LIBRARY(libg4dst.so)

#include <cstddef>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

int
macro (
	std::vector<std::string> const& dst_files = {
		"dat/LcpKpi_DST_0.root",
		// "dat/LcpKpi_DST_1.root",
		// "dat/LcpKpi_DST_2.root",
		// "dat/MinBias_DST_0.root",
		// "dat/MinBias_DST_1.root",
		// "dat/MinBias_DST_2.root",
	}
) {
	// gSystem->Load("libg4dst.so");

	Fun4AllServer* se = Fun4AllServer::instance();
	se->Verbosity(0);

	for ( auto const& file_name : dst_files ) {
		Fun4AllInputManager* in = new Fun4AllDstInputManager("DSTin");
		in->AddFile(file_name);
		se->registerInputManager(in);
	}

	SubsysReco* lc_reco = new LcReco();
	se->registerSubsystem(lc_reco);

	se->run(2);
	se->End();

	return EXIT_SUCCESS;
}

#endif//MACRO_C
