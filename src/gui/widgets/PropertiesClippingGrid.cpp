#include "gui/widgets/PropertiesClippingGrid.h"
#include  <QMessageBox>

#define _USE_MATH_DEFINES
#include <math.h>

#define MAX_BOXES 0xFFFF

PropertiesClippingGrid::PropertiesClippingGrid(QWidget *parent, const float& guiScale)
	: QWidget(parent)
	, ui(new Ui::property_grid())
{
	ui->setupUi(this);
	resetUI();
	ui->label_image->setPixmap(QPixmap("Grid_Readme.png"));

	connect(ui->okButton, &QAbstractButton::clicked, this, [this]() { updateGrid();});
	connect(ui->cancelButton, &QAbstractButton::clicked, this, [this]() { this->hide(); this->resetUI(); });
}

PropertiesClippingGrid::~PropertiesClippingGrid()
{};

const std::pair<QString, std::deque<GridBox>>& PropertiesClippingGrid::getGrid() const
{
	return m_grid;
}

std::pair<QString, std::deque<GridBox>> PropertiesClippingGrid::getGrid()
{
	return m_grid;
}

void PropertiesClippingGrid::resetUI()
{
	ui->lineEdit_originX->setText("0");
	ui->lineEdit_originY->setText("0");
	ui->lineEdit_finalX->setText("0");
	ui->lineEdit_finalY->setText("0");
	ui->lineEdit_maxZ->setText("0");
	ui->lineEdit_minZ->setText("0");
	ui->lineEdit_high->setText("0");
	ui->lineEdit_sizeXYZ->setText("0");
	ui->lineEdit_name->setText("");
	ui->checkBox_global->setChecked(true);
}

void PropertiesClippingGrid::updateGrid()
{
	m_grid.second.clear();
	m_grid.first = ui->lineEdit_name->text();
	
	glm::vec2 origin = { ui->lineEdit_originX->text().toFloat() ,ui->lineEdit_originY->text().toFloat()};
	glm::vec2 finale = { ui->lineEdit_finalX->text().toFloat() ,ui->lineEdit_finalY->text().toFloat()};

	glm::vec2 Z = { ui->lineEdit_minZ->text().toFloat() ,ui->lineEdit_maxZ->text().toFloat() };
    float size(ui->lineEdit_sizeXYZ->text().toFloat());
    float sizeX(glm::distance(origin, finale));
    float sizeY(ui->lineEdit_high->text().toFloat());

    // New base for the grid
	glm::vec2 gridX(glm::normalize(finale - origin));
    glm::vec2 gridY = sizeY > 0 ? glm::vec2(-gridX.y, gridX.x) : glm::vec2(gridX.y, -gridX.x);

	float alpha = std::atan2f(gridX.y, gridX.x);
	if (isnan(alpha))
	{
		QMessageBox errorbox(QMessageBox::Icon::Critical, tr("Informations invalides."), tr("Les vecteurs 2D sont identiques."),
			QMessageBox::StandardButton::Ok);
		errorbox.exec();
		return;
	}

    uint64_t maxXIteration(ceil(sizeX / size));
    uint64_t maxYIteration(ceil(abs(sizeY / size)));
    uint64_t maxZIteration(ceil((Z.y - Z.x) / size));
	if (maxXIteration > MAX_BOXES || maxYIteration > MAX_BOXES || maxZIteration > MAX_BOXES)
	{
		QMessageBox errorbox(QMessageBox::Icon::Critical, tr("Informations invalides."), tr("La grille est trop grande, le maximum est de 65 535 boxes par côté."),
			QMessageBox::StandardButton::Ok);
		errorbox.exec();
		return;
	}

	for (uint64_t iteratorZ(0); iteratorZ < maxZIteration; iteratorZ++)
	{
		for (uint64_t iteratorY(0); iteratorY < maxYIteration; iteratorY++)
		{
			for (uint64_t iteratorX(0); iteratorX < maxXIteration; iteratorX++)
			{
				GridBox box;
				box.posX = origin.x + size * gridX.x * (iteratorX + 0.5f) + size * gridY.x * (iteratorY + 0.5f);
				box.posY = origin.y + size * gridX.y * (iteratorX + 0.5f) + size * gridY.y * (iteratorY + 0.5f);
				box.posZ = Z.x + size * (iteratorZ + 0.5f);;
				box.sizeX = box.sizeY = box.sizeZ = size;
				box.rotX = 0;
                box.rotY = 0;
                box.rotZ = alpha;
				box.position.x = (uint8_t)iteratorX;
				box.position.y = (uint8_t)iteratorY;
				box.position.z = (uint8_t)iteratorZ;
				m_grid.second.push_back(box);
			}
		}
	}
	hide();
	resetUI();
	emit onGridChanged(m_grid, ui->checkBox_global->isChecked());
}