#ifndef BOX_NODE_H
#define BOX_NODE_H

#include "SimpleObjectNode.h"
#include "models/data/Grid/GridData.h"

class BoxNode : public SimpleObjectNode, public GridData
{
public:
	BoxNode(const BoxNode& object);
	BoxNode(bool isSimpleBox);
	~BoxNode();

	ElementType getType() const override;
	TreeType getDefaultTreeType() const override;

	virtual ClippingMode getClippingMode() const override;
	virtual void pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const override;
	virtual void pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const override;

	virtual void addGenericMeshInstance() override;

	std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const override;
	std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const override;

	void setIsSimpleBox(bool isSimpleBox);
	bool isSimpleBox() const;

	void setNeedUpdate(bool needUpdate);

	MeshDrawData getGridMeshDrawData(const glm::dmat4& gTransfo);

protected:
	void updateGrid();
	std::shared_ptr<MeshBuffer> getGridBuffer();

private:
	bool									m_gridNeedUpdate = true;
	// box buffer
	std::shared_ptr<MeshBuffer>             m_gridSBuf;
	bool									m_gridAllocationSucces = false;

	bool									m_isSimpleBox = false;
};

#endif // !CLIPPING_BOX_NODE_H_
