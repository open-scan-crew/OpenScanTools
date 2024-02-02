#ifndef INSULATION_DATA_H_
#define INSULATION_DATA_H_

class InsulationData
{
public:
	InsulationData();
	~InsulationData();

	void setInsulationRadius(const double& radius);
	double getInsulationRadius() const;

protected:
	double m_insulationRadius;
};

#endif // !STANDARD_RADIUS_DATA_H_ 
