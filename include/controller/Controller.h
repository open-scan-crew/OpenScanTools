#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "models/OpenScanToolsModelEssentials.h"
#include "utils/Color32.hpp"
#include "models/Types.hpp"
#include "models/LicenceTypes.h"

#include <unordered_set>

class Controller_p;
class IControlListener;
class FunctionManager;
class ControllerContext;
class FilterSystem;
class IDataDispatcher;
class IGuiData;
class CameraInfo;
class AControl;

class GraphManager;
class AGraphNode;
class CameraNode;

/*! Classe qui s'occupe des fonctions de gestion des IControls et de raffraîchissement 

	Elle lance (notamment) le thread de dépilement des AControl,
	et fournie les fonctions d'actualisation SceneGraph et Gui,
	et se charge des undo-redo via la pile historique des AControl.

	
	*/

class Controller
{
public:
	Controller(IDataDispatcher& dataDispatcher, GraphManager& graphManager);
	~Controller();

    void changeSelection(const std::unordered_set<SafePtr<AGraphNode>> & newSelection, bool updateTree = true);

	//void actualizeClippingObjectWithUserValue(const double& value, const std::unordered_set<ElementType>& elementsType);
    /*! Permet d'envoyer les messages GuiData de rafraîchissement de l'affichage
     * de l'UI (si _updateGui_ est vraie) et du SceneGraph suite à la modification
     * d'un objet de xg::Guid _id_
	 */
    void actualizeTreeView(const std::unordered_set<SafePtr<AGraphNode>>& toActualizeDatas);
	void actualizeTreeView(SafePtr<AGraphNode> toActualizeData);

	void undoLastAction(); //historic class
	void redoLastAction(); //historic class
	void resetHistoric();

    void saveCurrentProject(const SafePtr<CameraNode>& camera);
	//void autosaveCurrentProject(const SafePtr<CameraNode>& camera);

	/*! Rajoute dans la file du DataDispatcher le message IGuiData _data_ 
		
		Il n'y a qu'un seul DataDispatcher.

		(plus d'info dans DataDispatcher::updateInformation)
		*/
	void updateInfo(IGuiData *data);

	IControlListener* getControlListener();
	IDataDispatcher& getDataDispatcher() const;
	FunctionManager& getFunctionManager();
	ControllerContext& getContext();
	FilterSystem& getFilterSystem();
	GraphManager& getGraphManager();

	const GraphManager& cgetGraphManager() const;
	const ControllerContext& cgetContext() const;

	void startMetaControl();
	void stopMetaControl();
	void abortMetaControl();

	void activateAutosave(uint64_t period_min);
	void deactivateAutosave();

	void startScantraInterface();
	void stopScantraInterface();
	
	uint32_t getNextUserId(ElementType type) const;
	std::vector<uint32_t> getMultipleUserId(ElementType type, int indexAmount) const;

	void setDefaultAuthor();

private:
    Controller_p* m_p;
};

#endif // !_CONTROLLER_H_
