#ifndef I_PANEL_H
#define I_PANEL_H

class IGuiData;

class IPanel
{
public:
	/*! S'exécute à chaque fois qu'un message IGuiData d'un type écouté par le IPanel (via la méthode DataDispatcher::registerObserverOnKey)
		est sortie de la file de message IGuiData du DataDispatcher.
		
		Il permet donc de faire une opération en fonction du IGuiData _keyValue_ reçu*/
    virtual void informData(IGuiData *keyValue) = 0;
};

#endif