#ifndef _IMPORTSCAN_H_
#define _IMPORTSCAN_H_

#include "ui_convertion_options.h"
#include "gui/Dialog/ADialog.h"
#include "gui/UnitConverter.h"

class ConvertionOptionsBox : public ADialog
{
	Q_OBJECT

public:
	enum BoxOptions{
        FORMAT = 0x0001,
        PRECISION = 0x0002,
        OUTDIR = 0x0004,
        OUTFILE = 0x0008,
        CSV = 0x0010,
        IMPORT = 0x0020,
        USER_EDITS = 0x0040,
        ONLY_VISIBLES_POINTS = 0x0080,
        TRUNCATE_COORDINATES = 0x0100,
        ASK_COLOR_PRESENT = 0x0200,
		FORCE_FILE_OVERWRITE = 0x0400,
		AUTO_RENAMING_MULTISCAN = 0x0800
    };
public:
	ConvertionOptionsBox(IDataDispatcher& dataDispatcher, QWidget *parent = nullptr);
	~ConvertionOptionsBox();
	void informData(IGuiData *keyValue);
	void closeEvent(QCloseEvent *event);

private:
	void updateUI();

public slots:
	void sendConvertionInfo();
	void cancelConvertion();
	void selectOutDir();

private:
	bool					m_isBrowsingFolder;
	uint64_t				m_mask;

	glm::dvec3 m_storedTranslation;
	Ui::convertion_options m_ui;
	QWidget* m_fileInspector;
};

#endif //_IMPORTSCAN_H_
