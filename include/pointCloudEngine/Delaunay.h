#ifndef DELAUNAY_H
#define DELAUNAY_H

#define INIT_VERTICES_COUNT 6 /* count of vertices in the initial hull */
#define INIT_FACES_COUNT 8 /* count of faces in the initial hull */
#define VECTOR_LENGTH 1 /* radius of unit sphere the dots projected into */

#include <glm/glm.hpp>

class DelaunayTriangulation
{
private:
	Vector3D * _AuxiliaryDots[INIT_VERTICES_COUNT];
	std::vector<Vector3D>* _ProjectedDots;
	std::vector<Triangle*>* _Mesh;

	// 0: triangle search operations
	// 1: local optimizations
	// 2: start time; 3: end time;
	long _Statistics[4];

	void BuildInitialHull(std::vector<Vector3D*>* dots);
	void InsertDot(glm::dvec3* dot);
	void RemoveExtraTriangles();
	void SplitTriangle(Triangle* triangle, Vector3D* dot);
	void FixNeighborhood(Triangle* target, Triangle* oldNeighbor, Triangle* newNeighbor);
	void DoLocalOptimization(Triangle* t0, Triangle* t1);
	bool TrySwapDiagonal(Triangle* t0, Triangle* t1);
	bool IsMinimumValueInArray(double arr[], int length, int index);
	double GetDistance(Vector3D* v0, Vector3D* v1);
	double GetDeterminant(Vector3D* v0, Vector3D* v1, Vector3D* v2);
	double GetDeterminant(double matrix[]);

public:
	DelaunayTriangulation();
	~DelaunayTriangulation();

	std::vector<std::tuple<int, int, int>*> GetTriangulationResult(std::vector<Vector3D*> &dots);
	std::string GetStatistics();
};

class Vector3D
{
private:
	int GenerateRunningId();
public:
	int Id = 0;

	// coordinate
	double X, Y, Z;

	// color
	uint8_t R, G, B;

	bool IsVisited = false;
	bool IsAuxiliaryDot = false;

	Vector3D(double x, double y, double z, uint8_t r = 255, uint8_t g = 248, uint8_t b = 220);
	Vector3D(double x, double y, double z, bool isAuxiliaryDot, uint8_t r = 255, uint8_t g = 248, uint8_t b = 220);
	Vector3D(Vector3D* dot, double lengthAfterProjection);
	~Vector3D();

	bool IsCoincidentWith(Vector3D* dot);
	std::string ToString();
};

class Triangle
{
private:
	int GenerateRunningId();
public:
	int Id = 0;

	// pointers pointing to 3 vertices
	Vector3D* Vertex[3];

	// pointers pointing to 3 neighbors
	Triangle* Neighbor[3];

	Triangle(Vector3D* v0, Vector3D* v1, Vector3D* v2);
	~Triangle();

	bool HasVertexCoincidentWith(Vector3D* dot);
	void AssignNeighbors(Triangle* n0, Triangle* n1, Triangle* n2);
	std::string ToString();
};
#endif