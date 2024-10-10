// Tell emacs that this is a C++ source
//	-*- C++ -*-.
#ifndef LCRECO_H
#define LCRECO_H

#include <fun4all/SubsysReco.h>

#include <string>

class PHCompositeNode;

class LcReco : public SubsysReco
{
public:
	LcReco(std::string const& = "LcReco");
	virtual ~LcReco() override;

	/** Called during initialization.
			Typically this is where you can book histograms, and e.g.
			register them to Fun4AllServer (so they can be output to file
			using Fun4AllServer::dumpHistos() method).
	 */
	int Init(PHCompositeNode*) override;

	/** Called for first event when run number is known.
			Typically this is where you may want to fetch data from
			database, because you know the run number. A place
			to book histograms which have to know the run number.
	 */
	int InitRun(PHCompositeNode*) override;

	/** Called for each event.
			This is where you do the real work.
	 */
	int process_event(PHCompositeNode*) override;

	/// Clean up internals after each event.
	int ResetEvent(PHCompositeNode*) override;

	/// Called at the end of each run.
	int EndRun(int const) override;

	/// Called at the end of all processing.
	int End(PHCompositeNode*) override;

	/// Reset
	int Reset(PHCompositeNode*) override;

	void Print(std::string const& = "ALL") const override;

private:
	std::string m_track_node_name =  "reconstructedParticles_SvtxTrackMap";
	// std::string m_track_node_name =  "HFSelected_SvtxTrackMap";
	// std::string m_track_node_name =  "SvtxTrackMap";
	std::string m_vertex_node_name = "SvtxVertexMap";
	std::string m_truth_node_name =  "G4TruthInfo";
};

#endif // LCRECO_H
