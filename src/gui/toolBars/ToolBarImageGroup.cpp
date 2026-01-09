#include "gui/toolBars/ToolBarImageGroup.h"
#include "controller/controls/ControlIO.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataHD.h"

#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qtoolbutton.h>

#include "gui/texts/ImageSettings_txt.hpp"

#include "io/ImageTypes.h"

#include "models/graph/CameraNode.h"

/*
enum class PrintSize
{
	ISO_A4 = 0,
	ISO_A3,
	ISO_A2,
	ISO_A1,
	ISO_A0,
	ANSI_A,
	ANSI_B,
	ANSI_C,
	ANSI_D,
	ANSI_E,
	MAX_ENUM
};

const std::map<PrintSize, QString> g_printSizeDictio = {
	{ PrintSize::ISO_A4, "ISO A4" },
	{ PrintSize::ISO_A3, "ISO A3" },
	{ PrintSize::ISO_A2, "ISO A2" },
	{ PrintSize::ISO_A1, "ISO A1" },
	{ PrintSize::ISO_A0, "ISO A0" },
	{ PrintSize::ANSI_A, "ANSI A" },
	{ PrintSize::ANSI_B, "ANSI B" },
	{ PrintSize::ANSI_C, "ANSI C" },
	{ PrintSize::ANSI_D, "ANSI D" },
	{ PrintSize::ANSI_E, "ANSI E" }
};

const std::unordered_map<PrintSize, glm::ivec2> g_printSizeValues = {
	{ PrintSize::ISO_A4, { 297, 210 } },
	{ PrintSize::ISO_A3, { 420, 297 } },
	{ PrintSize::ISO_A2, { 594, 420 } },
	{ PrintSize::ISO_A1, { 841, 594 } },
	{ PrintSize::ISO_A0, { 1189, 841 } },
	{ PrintSize::ANSI_A, { 279, 216 } },
	{ PrintSize::ANSI_B, { 432, 279 } },
	{ PrintSize::ANSI_C, { 559, 432 } },
	{ PrintSize::ANSI_D, { 864, 559 } },
	{ PrintSize::ANSI_E, { 1118, 864 } },
};
*/

enum class RatioPrint
{
	ISO_A = 0,
	ANSI_ACE,
	ANSI_BD,
	MAX_ENUM
};

const std::map<RatioPrint, QString> g_ratioPrintDictio = {
	{ RatioPrint::ISO_A, "ISO A" },
	{ RatioPrint::ANSI_ACE, "ANSI A/C/E" },
	{ RatioPrint::ANSI_BD, "ANSI B/D " },
};

const std::unordered_map<RatioPrint, double> g_ratioPrintValues = {
	{ RatioPrint::ISO_A, 1.41421 },
	{ RatioPrint::ANSI_ACE, 1.2917 },
	{ RatioPrint::ANSI_BD, 1.5484 }
};

enum class RatioFractional
{
	N21_D9 = 0,
	N2_D1,
	N16_D9,
	N3_D2,
	N4_D3,
	N1_D1,
	MAX_ENUM
};

const std::map<RatioFractional, QString> g_ratioFractionalDictio = {
	{ RatioFractional::N21_D9, "21/9" },
	{ RatioFractional::N2_D1, "2/1" },
	{ RatioFractional::N16_D9, "16/9" },
	{ RatioFractional::N3_D2, "3/2" },
	{ RatioFractional::N4_D3, "4/3" },
	{ RatioFractional::N1_D1, "1/1" }
};

const std::unordered_map<RatioFractional, double> g_ratioFractionalValues = {
	{ RatioFractional::N21_D9, 21.0 / 9.0 },
	{ RatioFractional::N2_D1, 2.0 },
	{ RatioFractional::N16_D9, 16.0 / 9.0 },
	{ RatioFractional::N3_D2, 1.5 },
	{ RatioFractional::N4_D3, 4.0 / 3.0 },
	{ RatioFractional::N1_D1, 1.0 }
};

enum class ImageScale
{
	SCALE_100_1,
	SCALE_50_1,
	SCALE_20_1,
	SCALE_10_1,
	SCALE_8_1,
	SCALE_4_1,
	SCALE_2_1,
	SCALE_1_1,
	SCALE_1_2,
	SCALE_1_4,
	SCALE_1_8,
	SCALE_1_10,
	SCALE_1_20,
	SCALE_1_40,
	SCALE_1_50,
	SCALE_1_75,
	SCALE_1_100,
	SCALE_1_150,
	SCALE_1_200,
	SCALE_1_300,
	SCALE_1_400,
	SCALE_1_500,
	SCALE_1_1000,
	MAX_ENUM
};

const std::map<ImageScale, QString> g_imageScaleDictio = {
	/*{ImageScale::SCALE_100_1, "100/1"},
	{ ImageScale::SCALE_50_1, "50/1" },
	{ ImageScale::SCALE_20_1, "20/1" },
	{ ImageScale::SCALE_10_1, "10/1" },
	{ ImageScale::SCALE_8_1, "8/1" },
	{ ImageScale::SCALE_4_1, "4/1" },
	{ ImageScale::SCALE_2_1, "2/1" },*/
	{ ImageScale::SCALE_1_1, "1/1" },
	{ ImageScale::SCALE_1_2, "1/2" },
	{ ImageScale::SCALE_1_4, "1/4" },
	{ ImageScale::SCALE_1_8, "1/8" },
	{ ImageScale::SCALE_1_10, "1/10" },
	{ ImageScale::SCALE_1_20, "1/20" },
	{ ImageScale::SCALE_1_40, "1/40" },
	{ ImageScale::SCALE_1_50, "1/50" },
	{ ImageScale::SCALE_1_75, "1/75" },
	{ ImageScale::SCALE_1_100, "1/100" },
	{ ImageScale::SCALE_1_150, "1/150" },
	{ ImageScale::SCALE_1_200, "1/200" },
	{ ImageScale::SCALE_1_300, "1/300" },
	{ ImageScale::SCALE_1_400, "1/400" },
	{ ImageScale::SCALE_1_500, "1/500" },
	{ ImageScale::SCALE_1_1000, "1/1000" }
};

const std::unordered_map<ImageScale, double> g_imageScaleValues = {
	{ ImageScale::SCALE_100_1, 100. },
	{ ImageScale::SCALE_50_1, 50. },
	{ ImageScale::SCALE_20_1, 20. },
	{ ImageScale::SCALE_10_1, 10. },
	{ ImageScale::SCALE_8_1, 8. },
	{ ImageScale::SCALE_4_1, 4. },
	{ ImageScale::SCALE_2_1, 2. },
	{ ImageScale::SCALE_1_1, 1. },
	{ ImageScale::SCALE_1_2, 1./2. },
	{ ImageScale::SCALE_1_4, 1./4. },
	{ ImageScale::SCALE_1_8, 1./8. },
	{ ImageScale::SCALE_1_10, 1./10. },
	{ ImageScale::SCALE_1_20, 1./20. },
	{ ImageScale::SCALE_1_40, 1./40. },
	{ ImageScale::SCALE_1_50, 1./50. },
	{ ImageScale::SCALE_1_75, 1./75. },
	{ ImageScale::SCALE_1_100, 1./100. },
	{ ImageScale::SCALE_1_150, 1./150. },
	{ ImageScale::SCALE_1_200, 1./200. },
	{ ImageScale::SCALE_1_300, 1./300. },
	{ ImageScale::SCALE_1_400, 1./400. },
	{ ImageScale::SCALE_1_500, 1./500. },
	{ ImageScale::SCALE_1_1000, 1./1000. }
};

enum class ImageDPI
{
	DPI_72,
	DPI_96,
	DPI_150,
	DPI_200,
	DPI_300,
	DPI_400,
	DPI_600,
	DPI_1200,
	MAX_ENUM
};

const std::map<ImageDPI, QString> g_imageDPIDictio = {
	{ ImageDPI::DPI_72, "72" },
	{ ImageDPI::DPI_96, "96" },
	{ ImageDPI::DPI_150, "150" },
	{ ImageDPI::DPI_200, "200" },
	{ ImageDPI::DPI_300, "300" },
	{ ImageDPI::DPI_400, "400" },
	{ ImageDPI::DPI_600, "600" },
	{ ImageDPI::DPI_1200, "1200" }
};

const std::unordered_map<ImageDPI, double> g_imageDPIValues = {
	{ ImageDPI::DPI_72, 72 },
	{ ImageDPI::DPI_96, 96 },
	{ ImageDPI::DPI_150, 150 },
	{ ImageDPI::DPI_200, 200 },
	{ ImageDPI::DPI_300, 300 },
	{ ImageDPI::DPI_400, 400 },
	{ ImageDPI::DPI_600, 600 },
	{ ImageDPI::DPI_1200, 1200 }
};

enum class ImageAntialiasing
{
	Off = 0,
	Low,
	Mid,
	High,
	MAX_ENUM
};

const std::map<ImageAntialiasing, QString> g_imageAntialiasingDictio = {
	{ ImageAntialiasing::Off, "Off" },
	{ ImageAntialiasing::Low, "Low" },
	{ ImageAntialiasing::Mid, "Mid" },
	{ ImageAntialiasing::High, "High" }
};

const std::unordered_map<ImageAntialiasing, int> g_imageAntialiasingSamples = {
	{ ImageAntialiasing::Off, 1 },
	{ ImageAntialiasing::Low, 2 },
	{ ImageAntialiasing::Mid, 4 },
	{ ImageAntialiasing::High, 8 }
};

ToolBarImageGroup::ToolBarImageGroup(IDataDispatcher& dataDispatcher, QWidget* parent, const float& guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_projection(ProjectionMode::Perspective)
	, m_scale(1.0)
	, m_dpmm(100)
	, m_cameraOrthoSize(10.0, 10.0)
	, m_viewportSize(10.0, 10.0)
	, m_focusCamera()
{
	m_ui.setupUi(this);
	setEnabled(false);

	QObject::connect(m_ui.toolButton_screenshot, &QPushButton::released, this, [this]() {quickScreenshot(""); });
	QObject::connect(m_ui.checkBox_frame, &QCheckBox::toggled, this, &ToolBarImageGroup::slotShowFrame);

	QObject::connect(m_ui.formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarImageGroup::imageFormat);
	QObject::connect(m_ui.checkBox_alphaImage, &QCheckBox::toggled, this, &ToolBarImageGroup::imageFormat);
	QObject::connect(m_ui.comboBox_print, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarImageGroup::slotRatioChanged);
	QObject::connect(m_ui.comboBox_ratioImage, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarImageGroup::slotRatioChanged);
	QObject::connect(m_ui.comboBox_scale, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarImageGroup::slotScaleChanged);
	QObject::connect(m_ui.comboBox_dpi, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarImageGroup::slotDPIChanged);
	QObject::connect(m_ui.toolButton_createImage, &QToolButton::released, this, [this]() {slotCreateImage("", true); });
	QObject::connect(m_ui.lineEdit_imageW, &QLineEdit::textChanged, this, &ToolBarImageGroup::refreshImageSize);
	QObject::connect(m_ui.lineEdit_imageH, &QLineEdit::textChanged, this, &ToolBarImageGroup::slotHeightChanged);
	// The two radio buttons (*_ratioImage and *_ratioPrint) are exclusive, only one connect is needed
	QObject::connect(m_ui.radioButton_ratioImage, &QRadioButton::toggled, this, &ToolBarImageGroup::slotRatio);
	// The two radio buttons are exclusive, we only need to connect one button
	QObject::connect(m_ui.radioButton_portrait, &QRadioButton::toggled, this, &ToolBarImageGroup::slotPortrait);

	m_ui.formatComboBox->clear();
	for (const auto& iterator : ImageFormatDictio)
		m_ui.formatComboBox->addItem(QString::fromStdString(iterator.second));
	m_ui.formatComboBox->setCurrentIndex(1);

	for (const auto& iterator : g_ratioPrintDictio)
	{
		m_ui.comboBox_print->addItem(iterator.second, QVariant((int)iterator.first));
	}

	for (const auto& iterator : g_ratioFractionalDictio)
	{
		m_ui.comboBox_ratioImage->addItem(iterator.second, QVariant((int)iterator.first));
	}
	m_ui.comboBox_ratioImage->setCurrentIndex((int)RatioFractional::N3_D2);
	setSilentWidthHeight(3000, 2000);
	m_storePerspImageSize = glm::ivec2(3000, 2000);

	for (const auto& iterator : g_imageScaleDictio)
	{
		m_ui.comboBox_scale->addItem(iterator.second, QVariant((int)iterator.first));
	}
	m_ui.comboBox_scale->setCurrentIndex((int)ImageScale::SCALE_1_50);

	for (const auto& iterator : g_imageDPIDictio)
	{
		m_ui.comboBox_dpi->addItem(iterator.second, QVariant((int)iterator.first));
	}
	m_ui.comboBox_dpi->setCurrentIndex((int)ImageDPI::DPI_150);

	for (const auto& iterator : g_imageAntialiasingDictio)
	{
		m_ui.comboBox_antialiasHD->addItem(iterator.second, QVariant((int)iterator.first));
	}
	m_ui.comboBox_antialiasHD->setCurrentIndex((int)ImageAntialiasing::Off);

	refreshShowUI();

	registerGuiDataFunction(guiDType::projectLoaded, &ToolBarImageGroup::onProjectLoad);
	registerGuiDataFunction(guiDType::focusViewport, &ToolBarImageGroup::onFocusViewport);
	registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarImageGroup::onActiveCamera);
	registerGuiDataFunction(guiDType::hdCall, &ToolBarImageGroup::onCallHD);
}

ToolBarImageGroup::~ToolBarImageGroup()
{
	m_dataDispatcher.unregisterObserver(this);
}


void ToolBarImageGroup::informData(IGuiData *data)
{
	if (m_guiDFunctions.find(data->getType()) != m_guiDFunctions.end())
	{
		GuiDataFunction fct = m_guiDFunctions.at(data->getType());
		(this->*fct)(data);
	}
}

void ToolBarImageGroup::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* cData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(cData->m_isProjectLoad);
}

void ToolBarImageGroup::onFocusViewport(IGuiData* data)
{
	GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
	if (castData->m_forceFocus)
		m_focusCamera = castData->m_camera;

	if (m_focusCamera == castData->m_camera)
		m_viewportSize = { castData->m_width, castData->m_height };

	refreshShowUI();
}

void ToolBarImageGroup::onActiveCamera(IGuiData* data)
{
	GuiDataCameraInfo* castData = static_cast<GuiDataCameraInfo*>(data);
	if (castData->m_camera && castData->m_camera != m_focusCamera)
		return;

	ReadPtr<CameraNode> vp = m_focusCamera.cget();
	if (!vp)
		return;

	if (m_projection == ProjectionMode::Perspective && vp->getProjectionMode() == ProjectionMode::Orthographic)
	{
		bool resW = false;
		bool resH = false;
		uint32_t width = m_ui.lineEdit_imageW->text().toUInt(&resW, 10);
		uint32_t height = m_ui.lineEdit_imageH->text().toUInt(&resH, 10);

		m_storePerspImageSize = glm::ivec2(width, height);
	}

	if (m_projection == ProjectionMode::Orthographic && vp->getProjectionMode() == ProjectionMode::Perspective)
		setSilentWidthHeight(m_storePerspImageSize.x, m_storePerspImageSize.y);

	m_projection = vp->getProjectionMode();

	if (m_projection == ProjectionMode::Orthographic)
	{
		m_cameraOrthoSize = glm::dvec2(vp->getHeightAt1m());
		m_cameraOrthoSize.x *= vp->getRatioW_H();
	}
	else
	{
		m_cameraOrthoSize = glm::dvec2(std::numeric_limits<double>::quiet_NaN());
	}
	refreshShowUI();
}

void ToolBarImageGroup::onCallHD(IGuiData* data)
{
	GuiDataCallImage* castData = static_cast<GuiDataCallImage*>(data);

	if (castData->m_callHDImage)
	{
		const bool showProgressBar = castData->m_filepath.empty();
		slotCreateImage(castData->m_filepath, showProgressBar);
	}
	else
		quickScreenshot(castData->m_filepath);
}

void ToolBarImageGroup::quickScreenshot(std::filesystem::path filepath)
{
	m_dataDispatcher.sendControl(new control::io::QuickScreenshot((ImageFormat)m_ui.formatComboBox->currentIndex(), filepath, isAlphaChannelEnabled()));
}

void ToolBarImageGroup::imageFormat()
{
	m_dataDispatcher.updateInformation(new GuiDataRenderImagesFormat((ImageFormat)m_ui.formatComboBox->currentIndex(), isAlphaChannelEnabled()), this);
}

bool ToolBarImageGroup::isAlphaChannelEnabled() const
{
	ImageFormat format = static_cast<ImageFormat>(m_ui.formatComboBox->currentIndex());
	return hasAlphaSupport(format) && m_ui.checkBox_alphaImage->isChecked();
}

bool ToolBarImageGroup::hasAlphaSupport(ImageFormat format) const
{
	return format == ImageFormat::PNG || format == ImageFormat::PNG16 || format == ImageFormat::TIFF;
}

void ToolBarImageGroup::refreshShowUI()
{
	m_ui.widget_ratios->setEnabled(m_ui.checkBox_frame->isChecked());
	m_ui.widget_layout->setEnabled(m_ui.checkBox_frame->isChecked());
	m_ui.toolButton_createImage->setText(m_projection == ProjectionMode::Perspective ? TEXT_CREATE_HD_IMAGE : TEXT_CREATE_ORTHOIMAGE);
	m_ui.lineEdit_imageW->setEnabled(m_projection == ProjectionMode::Perspective);
	m_ui.lineEdit_imageH->setEnabled(m_projection == ProjectionMode::Perspective);
	m_ui.comboBox_scale->setEnabled(m_projection == ProjectionMode::Orthographic);
	m_ui.comboBox_dpi->setEnabled(m_projection == ProjectionMode::Orthographic);

	refreshImageSize();
}

void ToolBarImageGroup::refreshImageSize()
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t area_width = 0;
	uint32_t area_height = 0;
	bool useFrame = m_ui.checkBox_frame->isChecked();
	glm::dvec2 frameOrthoSize = glm::dvec2(1.0, 1.0);
	double ratio = getRatioWH();
	double frameFactor = useFrame ? 0.9 : 1.0;

	// En ortho, les dimensions en pixel sont fonction du scale, dpi et taille réelle du viewport
	if (m_projection == ProjectionMode::Orthographic)
	{
		if (m_cameraOrthoSize.x / m_cameraOrthoSize.y > ratio)
		{
			frameOrthoSize.y = m_cameraOrthoSize.y * frameFactor;
			frameOrthoSize.x = frameOrthoSize.y * ratio;
		}
		else
		{
			frameOrthoSize.x = m_cameraOrthoSize.x * frameFactor;
			frameOrthoSize.y = frameOrthoSize.x / ratio;
		}
		width = frameOrthoSize.x * 1000.0 * m_scale * m_dpmm;
		height = frameOrthoSize.y * 1000.0 * m_scale * m_dpmm;
	}
	// En perspective, les dimensions sont libres (avec ratio imposE
	else
	{
		bool res = false;
		width = m_ui.lineEdit_imageW->text().toUInt(&res, 10);
		if (!res)
			width = resetWidth();
		height = round(width / ratio);

	}

	if(m_viewportSize.y != 0. && m_viewportSize.x / m_viewportSize.y > ratio)
	{
		area_height = m_viewportSize.y * frameFactor;
		area_width = area_height * ratio;
	}
	else
	{
		area_width = m_viewportSize.x * frameFactor;
		area_height = area_width / ratio;
	}

	setAreaWidthHeight(area_width, area_height);
	setSilentWidthHeight(width, height);

	// Indique au viewport les dimmensions du 
	m_dataDispatcher.updateInformation(new GuiDataPrepareHDImage(useFrame, ratio, m_focusCamera), this);
}

double ToolBarImageGroup::getRatioWH()
{
	double ratio = 1.0;
	if (m_ui.checkBox_frame->isChecked())
	{
		if (m_ui.radioButton_ratioImage->isChecked())
		{
			ratio = g_ratioFractionalValues.at((RatioFractional)m_ui.comboBox_ratioImage->currentData().toInt());
		}
		else
		{
			ratio = g_ratioPrintValues.at((RatioPrint)m_ui.comboBox_print->currentData().toInt());
		}
	}
	else
	{
		uint32_t width = m_viewportSize.x;
		uint32_t height = m_viewportSize.y;
		if (height > 0.0)
			ratio = (double)width / (double)height;
	}

	if (m_ui.radioButton_portrait->isChecked())
		ratio = 1 / ratio;
	return ratio;
}

void ToolBarImageGroup::setAreaWidthHeight(uint32_t area_w, uint32_t area_h)
{
	m_ui.lineEdit_areaH->blockSignals(true);
	m_ui.lineEdit_areaW->blockSignals(true);

	m_ui.lineEdit_areaH->setText(QString::number(area_h));
	m_ui.lineEdit_areaW->setText(QString::number(area_w));

	m_ui.lineEdit_areaH->blockSignals(false);
	m_ui.lineEdit_areaW->blockSignals(false);
}

void ToolBarImageGroup::setSilentWidthHeight(uint32_t _w, uint32_t _h)
{
	m_ui.lineEdit_imageW->blockSignals(true);
	m_ui.lineEdit_imageH->blockSignals(true);

	m_ui.lineEdit_imageW->setText(QString::number(_w));
	m_ui.lineEdit_imageH->setText(QString::number(_h));

	m_ui.lineEdit_imageW->blockSignals(false);
	m_ui.lineEdit_imageH->blockSignals(false);
}

uint32_t ToolBarImageGroup::resetWidth()
{
	m_ui.lineEdit_imageW->setText("0");
	return 0;
}

uint32_t ToolBarImageGroup::resetHeight()
{
	m_ui.lineEdit_imageH->setText("0");
	return 0;
}

void ToolBarImageGroup::slotCreateImage(std::filesystem::path filepath, bool showProgressBar)
{
	ImageFormat format = (ImageFormat)m_ui.formatComboBox->currentIndex();
	bool resW = false;
	bool resH = false;
	uint32_t width = m_ui.lineEdit_imageW->text().toUInt(&resW, 10);
	uint32_t height = m_ui.lineEdit_imageH->text().toUInt(&resH, 10);
	int multisample = g_imageAntialiasingSamples.at((ImageAntialiasing)m_ui.comboBox_antialiasHD->currentData().toInt());

	ImageHDMetadata metadata;
	metadata.saveTextFile = true;
	metadata.ortho = m_projection == ProjectionMode::Orthographic;
	metadata.dpi = g_imageDPIValues.at((ImageDPI)(m_ui.comboBox_dpi->currentData().toInt()));
	metadata.scaleInv = 1/m_scale;
	metadata.includeAlpha = isAlphaChannelEnabled();

	bool useFrame = m_ui.checkBox_frame->isChecked();
	if (useFrame)
	{
		if (m_ui.radioButton_ratioImage->isChecked())
			metadata.imageRatioLabel = g_ratioFractionalDictio.at((RatioFractional)m_ui.comboBox_ratioImage->currentData().toInt()).toStdString();
		else
			metadata.imageRatioLabel = g_ratioPrintDictio.at((RatioPrint)m_ui.comboBox_print->currentData().toInt()).toStdString();
	}
	else
		metadata.imageRatioLabel = "Viewport";

	glm::dvec2 frameOrthoSize = glm::dvec2(1.0, 1.0);
	double ratio = getRatioWH();
	double frameFactor = useFrame ? 0.9 : 1.0;
	if (m_cameraOrthoSize.x / m_cameraOrthoSize.y > ratio)
	{
		frameOrthoSize.y = m_cameraOrthoSize.y * frameFactor;
		frameOrthoSize.x = frameOrthoSize.y * ratio;
	}
	else
	{
		frameOrthoSize.x = m_cameraOrthoSize.x * frameFactor;
		frameOrthoSize.y = frameOrthoSize.x / ratio;
	}
	metadata.orthoSize = frameOrthoSize;

	std::time_t t = std::time(nullptr);
	char timeStr[100];
	std::strftime(timeStr, sizeof(timeStr), "%Y/%m/%d %H:%M:%S", std::localtime(&t));
	metadata.date = std::string(timeStr);
	// Note (Yan): as the former comboBox_hdtilesize is not needed anymore, we set a fixed tile size
	constexpr int hdtilesize = 256;

	if (resW && resH && width > 0 && height > 0)
	{
		const bool fullResolutionTraversal = true;
		m_dataDispatcher.sendControl(new control::io::SetupImageHD(m_focusCamera, glm::ivec2(width, height), multisample, format, metadata, filepath, showProgressBar, hdtilesize, fullResolutionTraversal));
	}
	// else 
	//    return a error message
}

void ToolBarImageGroup::slotShowFrame()
{
	refreshShowUI();
}

void ToolBarImageGroup::slotRatio()
{
	refreshImageSize();
}

void ToolBarImageGroup::slotRatioChanged(int)
{
	refreshImageSize();
}

void ToolBarImageGroup::slotScaleChanged(int)
{
	m_scale = g_imageScaleValues.at((ImageScale)(m_ui.comboBox_scale->currentData().toInt()));
	refreshImageSize();
}

void ToolBarImageGroup::slotDPIChanged(int)
{
	// Convert to dots per millimeter
	m_dpmm = g_imageDPIValues.at((ImageDPI)(m_ui.comboBox_dpi->currentData().toInt())) / 25.4;
	refreshImageSize();
}

void ToolBarImageGroup::slotPortrait(bool checked)
{
	if (m_projection == ProjectionMode::Perspective)
	{
		bool resW = false;
		bool resH = false;
		uint32_t width = m_ui.lineEdit_imageW->text().toUInt(&resW, 10);
		uint32_t height = m_ui.lineEdit_imageH->text().toUInt(&resH, 10);

		// Exchange width and height
		if ((checked && width > height) || (!checked && width < height))
		{
			uint32_t w = width;
			width = height;
			height = w;
			setSilentWidthHeight(width, height);
		}
	}

	refreshImageSize();
}

void ToolBarImageGroup::slotHeightChanged()
{
	double ratio = getRatioWH();

	bool res = false;
	uint32_t height = m_ui.lineEdit_imageH->text().toUInt(&res, 10);
	if (!res)
		height = resetHeight();

	uint32_t width = round(height * ratio);

	setSilentWidthHeight(width, height);
}
