#include "gui/widgets/CustomWidgets/GenericPropertiesHead.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/controls/ControlDataEdition.h"
#include "gui/widgets/FocusWatcher.h"

#include "models/application/Author.h"
#include "models/application/Ids.hpp"
#include "gui/texts/DefaultUserLists.hpp"

GenericPropertiesHead::GenericPropertiesHead(QWidget* parent, float pixelRatio)
    : QWidget(parent)
    , m_dataDispatcher(nullptr)
    , m_controllerContext(nullptr)
    , m_object()
{
    m_ui.setupUi(this);
    m_ui.IndexInfield->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);

    connect(m_ui.colorPicker, &ColorPicker::pickedColor, this, &GenericPropertiesHead::onColorChange);
    // Link action
    connect(m_ui.IndexInfield, &QLineEdit::editingFinished, this, &GenericPropertiesHead::onUserIndexEdit);
    connect(m_ui.IdentifierInfield, &QLineEdit::editingFinished, this, &GenericPropertiesHead::onIdentifierEdit);
    connect(m_ui.NameInfield, &QLineEdit::editingFinished, this, &GenericPropertiesHead::onNameEdit);
    connect(new FocusWatcher(m_ui.DescInfield), &FocusWatcher::focusOut, this, &GenericPropertiesHead::onDescEdit);
    connect(m_ui.comboDiscipline, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GenericPropertiesHead::onDisciplineChange);
    connect(m_ui.comboPhase, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GenericPropertiesHead::onPhaseChange);
}

GenericPropertiesHead::~GenericPropertiesHead()
{
}

void GenericPropertiesHead::hideEvent(QHideEvent* event)
{
    /*m_ui.IndexInfield->blockSignals(true);
    m_ui.NameInfield->blockSignals(true);
    m_ui.IdentifierInfield->blockSignals(true);
    m_ui.AuthorInfield->blockSignals(true);
    m_ui.DescInfield->blockSignals(true);*/
}

void GenericPropertiesHead::setControllerInfo(const Controller& controller)
{
    m_dataDispatcher = &controller.getDataDispatcher();
    m_controllerContext = &controller.cgetContext();
}

void GenericPropertiesHead::setObject(SafePtr<AObjectNode> object)
{
    m_object = object;
    update();
}

void GenericPropertiesHead::onUserIndexEdit()
{
    if (!m_dataDispatcher)
        return;
    m_dataDispatcher->sendControl(new control::dataEdition::SetUserId(m_object, m_ui.IndexInfield->text().toInt()));
}

void GenericPropertiesHead::onIdentifierEdit()
{
    if (!m_dataDispatcher)
        return;
    m_dataDispatcher->sendControl(new control::dataEdition::SetIdentifier(m_object, m_ui.IdentifierInfield->text().toStdWString()));
}

void GenericPropertiesHead::onDisciplineChange()
{
    if (!m_dataDispatcher)
        return;
    m_dataDispatcher->sendControl(new control::dataEdition::SetDiscipline(m_object, m_ui.comboDiscipline->currentText().toStdWString()));
}

void GenericPropertiesHead::onPhaseChange()
{
    if (!m_dataDispatcher)
        return;
    m_dataDispatcher->sendControl(new control::dataEdition::SetPhase(m_object, m_ui.comboPhase->currentText().toStdWString()));
}

void GenericPropertiesHead::onColorChange(const Color32& color)
{
    if (!m_dataDispatcher)
        return;
    m_dataDispatcher->sendControl(new control::dataEdition::SetColor(m_object, color));
}

void GenericPropertiesHead::onNameEdit()
{
    if (!m_dataDispatcher)
        return;
    m_dataDispatcher->sendControl(new control::dataEdition::SetName(m_object, m_ui.NameInfield->text().toStdWString()));
}

void GenericPropertiesHead::onDescEdit()
{
    if (!m_dataDispatcher)
        return;
    m_dataDispatcher->sendControl(new control::dataEdition::SetDescription(m_object, m_ui.DescInfield->toPlainText().toStdWString()));
}

void GenericPropertiesHead::update()
{
    blockWidgets(true);
    SafePtr<Author> author;

    //Object update
    {
        ReadPtr<AObjectNode> rObject = m_object.cget();
        if (!rObject)
        {
            blockSignals(false);
            return;
        }
        author = rObject->getAuthor();

        m_ui.NameInfield->setText(QString::fromStdWString(rObject->getName()));
        m_ui.IndexInfield->setText(QString::number(rObject->getUserIndex()));
        m_ui.DescInfield->setPlainText(QString::fromStdWString(rObject->getDescription()));
        m_ui.IdentifierInfield->setText(QString::fromStdWString(rObject->getIdentifier()));
        m_ui.colorPicker->setColorChecked(rObject->getColor());

        updatePhaseDiscipline(rObject);

        updateType(rObject->getType());
    }
    

    //Author update
    {
        std::wstring name = L"NO_AUTHOR";
        {
            ReadPtr<Author> rAuth = author.cget();
            if (rAuth)
                name = rAuth->getName();
        }

        m_ui.AuthorInfield->setText(QString::fromStdWString(name));
    }

    blockWidgets(false);

}

void GenericPropertiesHead::updatePhaseDiscipline(ReadPtr<AObjectNode>& rObject)
{
    if (!m_controllerContext)
        return;

    SafePtr<UserList> disciplineList = m_controllerContext->getUserList(listId(LIST_DISCIPLINE_ID));
    SafePtr<UserList> phaseList = m_controllerContext->getUserList(listId(LIST_PHASE_ID));

    std::wstring disciplineDefault = NA_FIELD_NAME.toStdWString();
    std::wstring phaseDefault = NA_FIELD_NAME.toStdWString();

    disciplineDefault = rObject->getDiscipline();
    phaseDefault = rObject->getPhase();

    {
        m_ui.comboDiscipline->clear();
        int i = -1;
        ReadPtr<UserList> rDisci = disciplineList.cget();
        if (rDisci)
        {
            int storedId = rDisci->clist().empty() ? -1 : 1;

            for (std::wstring disci : rDisci->clist())
            {
                m_ui.comboDiscipline->addItem(QString::fromStdWString(disci));
                ++i;
                if (disciplineDefault == disci)
                    storedId = i;
            }
            if (storedId >= 0)
                m_ui.comboDiscipline->setCurrentIndex(storedId);
        }
    }

    {
        m_ui.comboPhase->clear();
        int i = -1;
        ReadPtr<UserList> rPhase = phaseList.cget();
        if (rPhase)
        {
            int storedId = rPhase->clist().empty() ? -1 : 1;

            for (std::wstring phase : rPhase->clist())
            {
                m_ui.comboPhase->addItem(QString::fromStdWString(phase));
                ++i;
                if (phaseDefault == phase)
                    storedId = i;
            }
            if (storedId >= 0)
                m_ui.comboPhase->setCurrentIndex(storedId);
        }
    }
}

void GenericPropertiesHead::updateType(ElementType objectType)
{
    bool show_color = true;
    bool show_identifier = true;
    bool show_index = true;
    bool show_author = true;

    switch (objectType)
    {
    case ElementType::Cluster:
    {
        show_color = false;
        show_identifier = false;
        show_index = false;
        break;
    }
    case ElementType::Scan:
    {
        show_color = false;
        show_identifier = false;
        show_index = false;
        show_author = false;
        break;
    }
    }
    m_ui.AuthorLabel->setVisible(show_author);
    m_ui.AuthorInfield->setVisible(show_author);

    m_ui.IndexLabel->setVisible(show_index);
    m_ui.IndexInfield->setVisible(show_index);

    m_ui.NameLabel->setVisible(true);
    m_ui.NameInfield->setVisible(true);
    m_ui.NameInfield->setReadOnly(objectType == ElementType::Scan);

    m_ui.DescriptionLabel->setVisible(true);
    m_ui.DescInfield->setVisible(true);

    m_ui.IdentifierLabel->setVisible(show_identifier);
    m_ui.IdentifierInfield->setVisible(show_identifier);

    m_ui.colorLabel->setVisible(show_color);
    m_ui.colorPicker->setVisible(show_color);

    m_ui.labelDiscipline->setVisible(true);
    m_ui.comboDiscipline->setVisible(true);

    m_ui.labelPhase->setVisible(true);
    m_ui.comboPhase->setVisible(true);
}

void GenericPropertiesHead::blockWidgets(bool block)
{
    m_ui.AuthorInfield->blockSignals(block);
    m_ui.IndexInfield->blockSignals(block);
    m_ui.NameInfield->blockSignals(block);
    m_ui.DescInfield->blockSignals(block);
    m_ui.IdentifierInfield->blockSignals(block);
    m_ui.colorPicker->blockSignals(block);
    m_ui.comboDiscipline->blockSignals(block);
    m_ui.comboPhase->blockSignals(block);
}