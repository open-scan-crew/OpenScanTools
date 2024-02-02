#ifndef CONTROL_CLUSTER_H
#define CONTROL_CLUSTER_H

#include "controller/controls/AEditionControl.h"
#include "models/ScansapModelEssentials.h"
#include "utils/Color32.hpp"
#include "utils/tree/TreeTypes.hpp"

class ModelCluster;

namespace control
{
	namespace cluster
	{
		class CreateCluster : public AEditionControl
		{
		public:
			CreateCluster(const std::string& name, const TreeType& type, const Color32& newColor = Color32(rand() % 255, rand() % 255, rand() % 255, 255));
			~CreateCluster();
			void doFunction(Controller& controller) override;
			bool canUndo() override;
			void undoFunction(Controller& controller) override;
			UIControl getType() override;
		private:
			const std::string m_name;
			const TreeType m_type;
			const Color32 m_color;
			ModelCluster* m_cluster;
			dataId		m_clusterId;
		};
	}
}

#endif