#ifndef SMARTDELETION_H
#define SMARTDELETION_H

#include "io/imports/step-simplification/classification/ReliableClassificationBySimilarity.h"
#include "io/imports/step-simplification/classification/ClassificationByVolumeMBB.h"
#include "io/imports/step-simplification/classification/ClassificationByComplexity.h"
#include "io/imports/step-simplification/stepGraph/StepPiece.h"

#include <list>

class SmartDeletion {

private:
    StepGraph *graph{};
    std::list<std::list<StepPiece*>> classificationBySimilarity;
    std::list<std::list<StepPiece*>> classificationByComplexity;
    std::list<std::list<StepPiece*>> classificationByVolumeMbb;
    std::list<std::list<StepPiece*>> classification;
    int weightSimilarity, weightVolume, weightComplexity = 1;

    std::list<std::list<StepPiece*>> performClassification();
	size_t findNumberOfClassesToDelete(double percentageOfPartsToKeep, std::list<std::list<StepPiece*>> classification, double& newPercent);
    static std::list<std::list<StepPiece*>> descSortByClassSize(std::list<std::list<StepPiece*>>);

public:
    explicit SmartDeletion(StepGraph*);
    inline void add(ReliableClassificationBySimilarity c) { classificationBySimilarity = c.classify(); classification = performClassification(); }
    inline void add(ClassificationByComplexity c) { classificationByComplexity = c.classify(); classification = performClassification(); }
    inline void add(ClassificationByVolumeMBB c) { classificationByVolumeMbb = c.classify(); classification = performClassification(); }
    inline void setWeightSimilarity(int weight) { weightSimilarity = weight; classification = performClassification(); }
    inline void setWeightVolume(int weight) { weightVolume = weight; classification = performClassification();}
    inline void setWeightComplexity(int weight) { weightComplexity = weight; classification = performClassification();}
    inline std::list<std::list<StepPiece*>> getClassification() { return classification; }

	size_t performDeletion(double percentageOfPartsToKeep, double& newPercentToKeep);
    void exportOneClass(std::list<StepPiece*> classToExport);
    std::stringstream getClasses();
};

#endif // SMARTDELETION_H