#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "models/OpenScanToolsModelEssentials.h"
#include "utils/Color32.hpp"
#include "models/Types.hpp"
#include "models/LicenceTypes.h"
#include "controller/ActualizeOptions.h"

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

class OpenScanToolsGraphManager;
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
	Controller(IDataDispatcher& dataDispatcher, OpenScanToolsGraphManager& graphManager);
	~Controller();

    /*! \brief this function launch a thread that run the update() at a fixed refresh rate
     * Can only be called once per Controller instance.
     * The function stop() must be called before the Controller is destroyed.
     */
    void run(int refreshPerSecond);

    /*!
     * \brief this function stop the thread launched by run().
     * If there is no thread running, does nothing.
     */
    void stop();

    void changeSelection(const std::unordered_set<SafePtr<AGraphNode>> & newSelection, bool updateTree = true);

	//void actualizeClippingObjectWithUserValue(const double& value, const std::unordered_set<ElementType>& elementsType);
	/*! Permet d'envoyer les messages GuiData de rafraîchissement de l'affichage de l'UI (si _updateGui_ est vraie)
							et du SceneGraph suite à la modification d'un objet de xg::Guid  _id_ */
    void actualizeNodes(const ActualizeOptions& options, const std::unordered_set<SafePtr<AGraphNode>>& toActualizeDatas);
	void actualizeNodes(const ActualizeOptions& options, SafePtr<AGraphNode> toActualizeData);
	void applyWaitingActualize();

	void undoLastAction(); //historic class
	void redoLastAction(); //historic class

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
	OpenScanToolsGraphManager& getOpenScanToolsGraphManager();

	const OpenScanToolsGraphManager& cgetOpenScanToolsGraphManager() const;
	const ControllerContext& cgetContext() const;

	void startMetaControl();
	void stopMetaControl();
	void abortMetaControl();
	void cleanHistory(); //historic class

	bool isUndoPossible(); //historic class
	bool isRedoPossible(); //historic class

	bool startAutosaveThread(const uint64_t& timing);
	bool stopAutosaveThread();

	uint32_t getNextUserId(ElementType type) const;
	std::vector<uint32_t> getMultipleUserId(ElementType type, int indexAmount) const;

	void setDefaultAuthor();
private:
    /*!
    ** \brief this method is the method that will allow all the subsystem of the controller to be updated
    ** This method must be called every frame.
    */
    void update();

	void logHistoric(); //historic class

	bool updateControls();

	void cleanRedoStack();

private:
    Controller_p* m_p;
	bool m_metaContralCreation;
	std::list<AControl*> m_metaToUndo;
	std::unordered_map<ActualizeOptions, std::unordered_set<SafePtr<AGraphNode>>> m_waitingActualizeNodes;
};

#endif // !_CONTROLLER_H_
