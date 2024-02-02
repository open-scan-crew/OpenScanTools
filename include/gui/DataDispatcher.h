#ifndef DATA_DISPATCHER_H
#define DATA_DISPATCHER_H

#include "gui/IDataDispatcher.h"

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include <queue>
#include <map>
#include <set>
#include <mutex>

/*! \class DataDispatcher
 * \brief Implementation of the IDataDispacher
 */
class DataDispatcher : public QObject, public IDataDispatcher
{
    Q_OBJECT

public:
    DataDispatcher();
    ~DataDispatcher();

	/*! Envoie les informations IGuiData 
		
		Les classes héritant de IPanel et ayant le DataDispatcher peuvent accéder aux messages IGuiData.

		Si _owner_ est non-nul, le message IGuiData enregistré ne sera pas lu par le panel _owner_.
		(C'est pour permettre d'éviter des boucles infinis)

		*/
    void updateInformation(IGuiData *value, IPanel* owner=nullptr) override;

    void unregisterObserver(IPanel* panel) override;
	/*! Enregistre le IPanel _panel_ 
		pour écouter (via la méthode IPanel::informData) les messages IGuiData de type _type_*/
    void registerObserverOnKey(IPanel* panel, guiDType type) override;
    void unregisterObserverOnKey(IPanel* panel, guiDType type) override;

    void InitializeControlListener(IControlListener *listener) override;
    void sendControl(AControl *control) override;

    // Functions only visible in the implementation
	void setActive(bool state);



public slots:
    void update();

private:
    std::mutex m_eventMutex;
    std::queue<std::pair<IGuiData*,IPanel*>> m_eventQueue;

	bool m_stoped = false;

    IControlListener *m_controlListener;
	bool m_active = true;

    QTimer timer;

    std::mutex m_observerMutex;
    std::unordered_map<guiDType, std::set<IPanel*>> m_dataKeyObservers;
};

#endif // !DATA_DISPATCHER_H