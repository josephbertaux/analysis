#include "LcReco.h"

// simulation/g4simulation
#include <g4main/PHG4Hit.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Particle.h>
#include <g4main/PHG4TruthInfoContainer.h>

// offline/packages
#include <fun4all/Fun4AllReturnCodes.h>

#include <globalvertex/SvtxVertexMap.h>

#include <trackbase_historic/SvtxTrackMap.h>
#include <trackbase_historic/SvtxPHG4ParticleMap.h>
#include <trackbase_historic/PHG4ParticleSvtxMap.h>

// offline/framework
#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>
#include <phool/phool.h>

#include <iostream>

LcReco::LcReco(std::string const& name)
	: SubsysReco(name)
{
}

LcReco::~LcReco()
{
}

int LcReco::Init(PHCompositeNode*)
{
	return Fun4AllReturnCodes::EVENT_OK;
}

int LcReco::InitRun(PHCompositeNode*)
{
	return Fun4AllReturnCodes::EVENT_OK;
}

int LcReco::process_event(PHCompositeNode* top_node)
{
	auto track_map = findNode::getClass<SvtxTrackMap>(top_node, m_track_node_name);
	if ( !track_map ) {
		std::cerr << PHWHERE << "\n"
		          << "\t" << m_track_node_name << std::endl;
		return Fun4AllReturnCodes::ABORTEVENT;
	}

	auto vertex_map = findNode::getClass<SvtxVertexMap>(top_node, m_vertex_node_name);
	if ( !vertex_map ) {
		std::cerr << PHWHERE << "\n"
		          << "\t" << m_vertex_node_name << std::endl;
		return Fun4AllReturnCodes::ABORTEVENT;
	}

	auto truth_map = findNode::getClass<PHG4TruthInfoContainer>(top_node, m_truth_node_name);
	if ( !truth_map ) {
		std::cerr << PHWHERE << "\n"
		          << "\t" << m_truth_node_name << std::endl;
		return Fun4AllReturnCodes::ABORTEVENT;
	}

	float px = 0.0, py = 0.0, pz = 0.0;
	for ( SvtxTrackMap::ConstIter itr = track_map->begin(); itr != track_map->end(); ++itr ) {
		// itr->second is the SvtxTrack*
		std::cout << "track: " << itr->first << "\n"
		          << "\tpx: " << itr->second->get_px() << "\n"
		          << "\tpy: " << itr->second->get_py() << "\n"
		          << "\tpz: " << itr->second->get_pz() << "\n"

		          << "\tdca:          " << itr->second->get_dca() << "\n"
		          << "\tdca_err:      " << itr->second->get_dca_error() << "\n"

		          << "\tdca2d:        " << itr->second->get_dca2d() << "\n"
		          << "\tdca2d_err:    " << itr->second->get_dca2d_error() << "\n"

		          << "\tdca3d_xy:     " << itr->second->get_dca3d_xy() << "\n"
		          << "\tdca3d_xy_err: " << itr->second->get_dca3d_xy_error() << "\n"

		          << "\tdca3d_z:      " << itr->second->get_dca3d_z() << "\n"
		          << "\tdca3d_z_err:  " << itr->second->get_dca3d_z_error() << "\n"

		          << std::endl;

		px += itr->second->get_px();
		py += itr->second->get_py();
		pz += itr->second->get_pz();
	}

	std::cout << "total:\n"
	          << "\tpx: " << px << "\n"
	          << "\tpy: " << py << "\n"
	          << "\tpz: " << pz << "\n"
	          << std::endl;

	for ( PHG4TruthInfoContainer::ConstIterator itr = truth_map->GetParticleRange().first; itr != truth_map->GetParticleRange().second; ++itr) {
		// itr->second is the PHG4Partile*
		auto pid = abs(itr->second->get_pid());
		if ( pid != 2212 && pid != 321 && pid != 211 ) continue;
		// std::cout << "paticle: " << pid << " track id: " << itr->second->get_track_id() << "\n"
		//           << "\tpx: " << itr->second->get_px() << "\n"
		//           << "\tpy: " << itr->second->get_py() << "\n"
		//           << "\tpz: " << itr->second->get_pz() << "\n"
		//           << std::endl;
	}

	return Fun4AllReturnCodes::EVENT_OK;
}

int LcReco::ResetEvent(PHCompositeNode*)
{
	return Fun4AllReturnCodes::EVENT_OK;
}

int LcReco::EndRun(int const)
{
	return Fun4AllReturnCodes::EVENT_OK;
}

int LcReco::End(PHCompositeNode*)
{
	return Fun4AllReturnCodes::EVENT_OK;
}

int LcReco::Reset(PHCompositeNode*)
{
	return Fun4AllReturnCodes::EVENT_OK;
}

void LcReco::Print(std::string const&) const
{
}
