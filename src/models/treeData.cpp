<<<<<<< HEAD
=======
#include "models/treeData.h"
#include "models/data/Scan.h"
#include "models/data/Cluster.h"
#include "models/data/Tag.h"
#include "models/data/SimpleMeasure.h"
#include "models/data/Clipping.h"
#include "utils/Logger.h"

treeData::treeData()
{
	tId = 0;
	dId = 0;
	dataName = "";
	type = TreeElementType::None;
	tlsPresent = false;
	showHide = false;
}

treeData::treeData(IData *element)
{
	tId = INVALID_TREE_ID;
	dId = element->getId();
	dataName = element->getName();
	type = element->getType();
	tlsPresent = false;
	showHide = element->isVisible();

	if (element->getType() == TreeElementType::Scan)
		tlsPresent = static_cast<ModelScan*>(element)->getTlsPresent();
	else if (element->getType() == TreeElementType::Cluster)
		dataName = static_cast<ModelCluster*>(element)->getComposedName();
	else if (element->getType() == TreeElementType::Tag)
		dataName = static_cast<Tag*>(element)->getComposedName();
	else if (element->getType() == TreeElementType::SimpleMeasure)
		dataName = static_cast<ModelSimpleMeasure*>(element)->getComposedName();
	else if (element->getType() == TreeElementType::Clipping)
		dataName = static_cast<Clipping*>(element)->getComposedName();
}

treeData::treeData(TreeElement<IData*> *treedata)
{
	tId = treedata->id();
	dId = (**treedata)->getId();
	dataName = (**treedata)->getName();
	type = (**treedata)->getType();
	tlsPresent = false;
	showHide = (**treedata)->isVisible();

	if ((**treedata)->getType() == TreeElementType::Scan)
		tlsPresent = static_cast<ModelScan*>(**treedata)->getTlsPresent();
	else if ((**treedata)->getType() == TreeElementType::Cluster)
		dataName = static_cast<ModelCluster*>(**treedata)->getComposedName();
	else if ((**treedata)->getType() == TreeElementType::Tag)
		dataName = static_cast<Tag*>(**treedata)->getComposedName();
	else if ((**treedata)->getType() == TreeElementType::SimpleMeasure)
		dataName = static_cast<ModelSimpleMeasure*>(**treedata)->getComposedName();
	else if ((**treedata)->getType() == TreeElementType::Clipping)
		dataName = static_cast<Clipping*>(**treedata)->getComposedName();
}
>>>>>>> develop
