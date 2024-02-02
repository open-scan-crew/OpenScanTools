#include <QtWidgets/QWidget>
#include "ui_property_tagInfo.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "models/project/Tag.h"
#include "gui/widgets/ColorPicker.h"
#include "gui/CursorCalculator.h"
#include <utility>
#include <vector>

#ifndef PROPERTY_TAGINFO_H_
#define PROPERTY_TAGINFO_H_

class FocusWatcher : public QObject
{
	Q_OBJECT
public:
	FocusWatcher(QObject* parent = nullptr);
	virtual ~FocusWatcher();
	bool eventFilter(QObject *obj, QEvent *event) override;

signals:
	void focusOut();

protected:
	bool m_blocking;
};


class PropertyTagInfo : public QWidget, public IPanel, public CursorCalculator
{
	Q_OBJECT

public:
	explicit PropertyTagInfo(IDataDispatcher &dataDispatcher, QWidget *parent = nullptr, float guiScale = 1.f);
	~PropertyTagInfo();

private:
	void informData(IGuiData *keyValue);
	std::string getName() const override;

	void changeTagColor(Color32 color);
	void setComboIndex(QComboBox *combo, std::vector<std::string>& values, const std::string& valueToSet);

public slots:

	void createEditNameControl();
	void createEditDescControl();
	void changeCatCombo(int);
	void changeActionCombo(int);
	void changePrefixCombo(int);
	void changeStatusCombo(int);
	void changeUserIndex();
	void changeHyperLink();
	void changeHyperLinkIntern(const QString&);
	void addAsAnimationKeyPoint();
    //void catchDescUndo(bool undoAvailable);

private:
	Ui::property_tagInfo *ui;
    ColorPicker* m_colorPicker;
	IDataDispatcher &m_dataDispatcher;

    // Deep copy of the tag
	smp::Tag storedTag;

	std::vector<std::string> ActValues;
	std::vector<std::string> CatValues;
	std::vector<std::string> PrefixValues;
	std::vector<std::string> StatusValues;
};

#endif