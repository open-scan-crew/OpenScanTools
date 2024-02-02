#ifndef CONTROL_WAVEFRONT_H_
#define CONTROL_WAVEFRONT_H_

#include "controller/controls/IControl.h"
#include "controller/messages/ImportMeshObjectMessage.h"
#include "models/OpenScanToolsModelEssentials.h"
#include <filesystem>

#include <unordered_set>

class MeshObjectNode;
class ClusterNode;

namespace control
{
	namespace meshObject
	{
		class CreateMeshObjectFromFile : public AControl
		{
		public:
			CreateMeshObjectFromFile(const FileInputData& data);
			~CreateMeshObjectFromFile();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;

		private:
				FileInputData m_data;
		};

		class StepSimplification : public AControl
		{
		public:
			StepSimplification(const FileInputData& data, const StepClassification& classification, const double& keepPercent, const std::filesystem::path& outputPath, bool importAfter);
			~StepSimplification();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			FileInputData m_data;
			StepClassification m_classification;
			double m_keepPercent;

			bool m_importAfter;
			std::filesystem::path m_outputPath;
		};

		class ActivateDuplicate : public AControl
		{
		public:
			ActivateDuplicate();
			~ActivateDuplicate();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};
	}
}

#endif // !CONTROL_WAVEFRONT_H_
