#ifndef TREE_TYPE_H
#define TREE_TYPE_H

/*! Type d'un TreeSystem

	Notre arborescence est constituée de sous-arbres ayant chacun un unique type TreeType.
	(Scan,TagTree,PointTree....)

	*/
enum class TreeType
{
	RawData = 0,
	Scan,
	User,
	Hierarchy,
	Measures,
	Boxes,
	Tags,
	MeshObjects,
	Pco,
	Pipe,
	Point,
	Sphere,
	Piping,
	ViewPoint,
	MAXENUM
};

#endif
