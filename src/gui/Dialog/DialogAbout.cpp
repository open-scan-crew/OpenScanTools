#include "gui/Dialog/DialogAbout.h"
#include "utils/OpenScanToolsVersion.h"
#include "gui/texts/AboutTexts.hpp"

#include <QtGui/qdesktopservices.h>
#include <QtCore/qurl.h>

DialogAbout::DialogAbout(IDataDispatcher& dataDispatcher, QWidget* parent)
    : ADialog(dataDispatcher, parent)
{
    m_ui.setupUi(this);

    setModal(true);

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::MSWindowsFixedSizeDialogHint;
    flags ^= Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    connect(m_ui.contactButton, &QPushButton::released, []() {QDesktopServices::openUrl(QUrl(OPENSCANTOOLS_CONTACT_URL)); });
    connect(m_ui.okButton, &QPushButton::released, [this]() {this->hide(); });
    
    adjustSize();
}

DialogAbout::~DialogAbout()
{}

void DialogAbout::informData(IGuiData * keyValue)
{}

void DialogAbout::setDialog(float guiScale)
{
    m_ui.label->setTextFormat(Qt::RichText);
    m_ui.label->setText(
        TEXT_ABOUT_VERSION + "<br>" +
        TEXT_ABOUT_COPYRIGHT.arg(OPENSCANTOOLS_YEAR) + "<br>" +
        TEXT_ABOUT_WEBSITE + "<br>" +
        TEXT_ABOUT_EMAIL + "<br>" +
        TEXT_ABOUT_CREDITS +
        "<ul><li>" + TEXT_ABOUT_QT +
        "</li><li>" + TEXT_ABOUT_ICONS +
        "</li><li>" + TEXT_ABOUT_IMGUI +
        "</li><li>" + TEXT_ABOUT_OPENE57 +
        "</li><li>" + TEXT_ABOUT_VULKAN +
        "</li><li>" + TEXT_ABOUT_VMA +
        "</li><li>" + TEXT_ABOUT_BADUIT +
        "</li><li>" + TEXT_ABOUT_OCCT +
        "</li><li>" + TEXT_ABOUT_FBXSDK +
        "</li><li>" + TEXT_ABOUT_IFCPP +
        "</li><li>" + TEXT_ABOUT_DXFLIB +
        "</li><li>" + "FaroSDK" +
        "</li><li>" + "ReCapSDK" +
        "</li></ul><br>Build: " + OPENSCANTOOLS_VERSION);
    QObject::connect(m_ui.label, &QLabel::linkActivated, [](const QString& link) { QDesktopServices::openUrl(link); });
    adjustSize();
}