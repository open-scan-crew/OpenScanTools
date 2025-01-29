#include "io/imports/step-simplification/deletePiece/SmartDeletion.h"
#include "io/imports/step-simplification/deletePiece/DeleteClass.h"
#include "io/imports/step-simplification/deletePiece/DeletePiece.h"

#include <cassert>
#include <sstream>

SmartDeletion::SmartDeletion(StepGraph *g) {
    std::list<std::list<StepPiece*>> emptyClassification;
    classificationBySimilarity = emptyClassification;
    classificationByVolumeMbb = emptyClassification;
    classificationByComplexity = emptyClassification;
    classification = emptyClassification;
    weightSimilarity = 1;
    weightVolume = 1;
    weightComplexity = 1;
    graph = g;
}

std::stringstream SmartDeletion::getClasses() {
    // Get number of piece in the model
    size_t numberOfPiece = 0;
	size_t numberOfPieceCounted = 0;
    bool showVolumeValues = false, showComplexityValues = false;

	std::stringstream ss;

    if (!classificationByVolumeMbb.empty()) showVolumeValues = true;
    if (!classificationByComplexity.empty()) showComplexityValues = true;

    if (!classificationBySimilarity.empty() && classificationByComplexity.empty() && classificationByVolumeMbb.empty()) ss << "Display of the classes with similarity criteria : \n" << std::endl;
    else if (classificationBySimilarity.empty() && !classificationByComplexity.empty() && classificationByVolumeMbb.empty()) ss << "Display of the classes with complexity criteria : \n" << std::endl;
    else if (classificationBySimilarity.empty() && classificationByComplexity.empty() && !classificationByVolumeMbb.empty()) ss << "Display of the classes with volume criteria : \n" << std::endl;


    for (auto cls : classification) numberOfPiece+= cls.size();
    unsigned int classId = 1;
    for (auto cls : classification) {
        numberOfPieceCounted += cls.size();
		ss << classId << ". " << cls.size() << " parts | " << numberOfPieceCounted * 100 / numberOfPiece << "%";
        if (showVolumeValues) ss << std::endl << "   Volume : " << cls.front()->getVolumeMBB() << " | X distance : " << cls.front()->getXAxisDistance() << " | Y distance : " << cls.front()->getYAxisDistance() << " | Z distance : " << cls.front()->getZAxisDistance() << std::endl;
        if (showComplexityValues) ss << "   Complexity : " << cls.front()->getNbEntities() << std::endl;
        for (auto ent : cls) ss << " #" << ent->getRoot()->getId();
		ss << std::endl;
        classId++;
    }

	return ss;
}

size_t SmartDeletion::performDeletion(double percentageOfPartsToKeep, double& newPercentToKeep) {
    if (percentageOfPartsToKeep < 0 || percentageOfPartsToKeep > 1) {
		return 0;
    }
	size_t nbOfClassesToDelete = findNumberOfClassesToDelete(1.0 - percentageOfPartsToKeep, classification, newPercentToKeep);
	size_t totalNbDeletedPiece = 0;

    // Deletion with classification by similarity
    if (nbOfClassesToDelete != 0) {
        for (const auto& cls : classification) {
            DeleteClass deleteClassObj = DeleteClass(cls, graph);
            totalNbDeletedPiece += deleteClassObj.deleteClass();
            if (totalNbDeletedPiece != 0) 
                nbOfClassesToDelete--;
            if (nbOfClassesToDelete == 0) 
                break;
        }
    }

    return totalNbDeletedPiece;
}

size_t SmartDeletion::findNumberOfClassesToDelete(double deletionCoefficient, std::list<std::list<StepPiece*>> classification, double& newPercent) {
    // Count the number of parts
    size_t numberOfPiece = 0;
    for (auto cls : classification) numberOfPiece+= cls.size();

    // Processing of requests at the limits of the deletion coefficient
    if (deletionCoefficient >= 1.0) return numberOfPiece;
    if (deletionCoefficient == 0.0) return 0;

	size_t numberOfPieceToDelete = (size_t)(deletionCoefficient * numberOfPiece);
    size_t numberOfDeletedPieceBefore = 0, numberOfDeletedPieceAfter = 0;
    size_t numberOfClassesToDelete = 0;

    for (auto cls : classification) {
        numberOfDeletedPieceAfter += cls.size();

        if (numberOfDeletedPieceAfter > numberOfPieceToDelete) {
			/*double percentageOfPieceToDelete = 0;
			size_t numberOfPartToDeleteProposal = 0;
            if (((numberOfPieceToDelete - numberOfDeletedPieceBefore) > (numberOfDeletedPieceAfter - numberOfPieceToDelete))
                && deletionCoefficient != numberOfDeletedPieceBefore * 100 / numberOfPiece) 
            {
                percentageOfPieceToDelete = (double) (numberOfDeletedPieceAfter * 100 / numberOfPiece);
                numberOfPartToDeleteProposal = numberOfDeletedPieceAfter;
            } else {
                percentageOfPieceToDelete = (double) (numberOfDeletedPieceBefore * 100 / numberOfPiece);
                numberOfPartToDeleteProposal = numberOfDeletedPieceBefore;
            }

            // No interaction if the request is perfectly satisfied
            bool severalPossibilities = false;
            if (numberOfPieceToDelete == numberOfPartToDeleteProposal) {
				size_t numberOfClassesToDeleteAfter = 1;
                for (auto classes : classification) {
                    if (numberOfClassesToDelete <= numberOfClassesToDeleteAfter) {
                        if (percentageOfPieceToDelete != percentageOfPieceToDelete + classes.size() * 100 / numberOfPiece)
							return numberOfClassesToDelete;
                        else
						{ 
							severalPossibilities = true; 
							break;
						}
                    }
                    numberOfClassesToDeleteAfter++;
                }
            }

            if(!severalPossibilities) {
                // Otherwise, we must consult the user
                char answerFirstChoice; bool help = false;
                do {
                    std::cout << "We suggest to delete " << percentageOfPieceToDelete << "% of the model instead of " << deletionCoefficient * 100 << "% involving the removal of " << numberOfPartToDeleteProposal << " from the orginal " << numberOfPiece << " pieces.\n";
                    if (percentageOfPieceToDelete == 0) {
                        if (!help) std::cout << "Accept the suggestion (Y/N) ? Your rejection will not result in the removal of any part : ";
                        else std::cout << "Accept the suggestion (Y/N) ? Your rejection will not result in the removal of any part. You must type a 'Y' or an 'N' : ";
                    }
                    else {
                        if (!help) std::cout << "Accept the suggestion (Y/N) : ";
                        else std::cout << "Accept the suggestion (Y/N) ? You must type a 'Y' or an 'N' : ";
                    }

					newPercent = percentageOfPieceToDelete;
                    //std::cin >> answerFirstChoice;
					answerFirstChoice = 'y';
                    help = true;
                }
                while((answerFirstChoice !='Y') && (answerFirstChoice !='N') && (answerFirstChoice !='y') && (answerFirstChoice !='n'));

                if ((answerFirstChoice == 'y' || answerFirstChoice == 'Y')) return numberOfClassesToDelete;

                // Update the proposal if the user refuse the first
                if (percentageOfPieceToDelete == numberOfDeletedPieceBefore * 100 / numberOfPiece) {
                    percentageOfPieceToDelete = (double) (numberOfDeletedPieceAfter * 100 / numberOfPiece);
                    numberOfPartToDeleteProposal = numberOfDeletedPieceAfter;
                    numberOfClassesToDelete++;
                } else {
                    percentageOfPieceToDelete = (double) (numberOfDeletedPieceBefore * 100 / numberOfPiece);
                    numberOfPartToDeleteProposal = numberOfDeletedPieceBefore;
                    numberOfClassesToDelete--;
                }

                // The response of the previous question is no
                if (numberOfClassesToDelete != 0) {
                    char answerSecondChoice; help = false;
                    do {
                        std::cout << "We suggest to delete " << percentageOfPieceToDelete << "% of the model involving the removal of " << numberOfPartToDeleteProposal << " from the orginal " << numberOfPiece << " parts.\n";
                        if (!help) std::cout << "Accept the suggestion ? No other suggestions will be made [Y/N] : ";
                        else std::cout << "Accept the suggestion (Y/N) ? No other suggestions will be made. You must type a 'Y' or an 'N' : ";

                        //std::cin >> answerSecondChoice;
						answerSecondChoice = 'y';
                        help = true;
                    }
                    while((answerSecondChoice !='Y') && (answerSecondChoice !='N') && (answerSecondChoice !='y') && (answerSecondChoice !='n'));

                    if ((answerSecondChoice == 'y' || answerSecondChoice == 'Y'))  return numberOfClassesToDelete;
                }
                std::cout << "No part will be deleted " << std::endl;
                return 0;
            }*/
            if ((numberOfDeletedPieceAfter - numberOfPieceToDelete) > (numberOfPieceToDelete - numberOfDeletedPieceBefore))
            {
                newPercent = ((double)numberOfDeletedPieceBefore / (double)numberOfPiece) * 100;
                return numberOfClassesToDelete;
            }
            else
            {
                newPercent = ((double)numberOfDeletedPieceAfter / (double)numberOfPiece) * 100;
                return numberOfClassesToDelete + 1;
            }
        }
        numberOfDeletedPieceBefore = numberOfDeletedPieceAfter;
        numberOfClassesToDelete++;
    }
    assert("Unable to find the number of classes to delete, so no part will be deleted ");
    return 0;
}

std::list<std::list<StepPiece *>> SmartDeletion::descSortByClassSize(std::list<std::list<StepPiece *>> classes) {
    auto descSortClasses = std::list<std::list<StepPiece *>>();
    while (!classes.empty()) {
        auto posMaxClassSize = 0, cpt = 0;
        auto maxClassSize = classes.front();
        for (auto cls : classes) {
            if (cls.size() > maxClassSize.size()) { maxClassSize = cls; posMaxClassSize = cpt; }
            cpt++;
        }
        descSortClasses.push_back(maxClassSize);
        cpt = 0;
        // Deletion of the class with the largest number of pieces
        for(auto it=classes.begin(); it!=classes.end(); ++it) {
            if (cpt == posMaxClassSize) {
                classes.erase(it); break;
            } cpt++;
        }
    }

    return descSortClasses;
}

void SmartDeletion::exportOneClass(std::list<StepPiece *> classToExport) {
    for (auto stepPiece : graph->getStepPieces()) {
        auto toDelete = true;
        for (auto stepPieceToKeep : classToExport)  if (stepPieceToKeep == stepPiece) toDelete = false;
        if (toDelete) {
            DeletePiece delPiece = DeletePiece(stepPiece->getRoot(), graph);
            delPiece.deletePiece();
        }
    }
}

/**
 * Method merging the available classifications into one prepared specifically for the deletion
 * @return the resulting classification
 */
std::list<std::list<StepPiece *>> SmartDeletion::performClassification() {
    // No need to merge classification if only one has been added for the current deletion
    if (!classificationBySimilarity.empty() && classificationByVolumeMbb.empty() && classificationByComplexity.empty()) return classificationBySimilarity;
    if (classificationBySimilarity.empty() && !classificationByVolumeMbb.empty() && classificationByComplexity.empty()) return classificationByVolumeMbb;
    if (classificationBySimilarity.empty() && classificationByVolumeMbb.empty() && !classificationByComplexity.empty()) return classificationByComplexity;

    std::list<std::list<StepPiece *>> result;
    std::list<StepPiece*> sortedByScore;

    // For each piece
    for (auto part : graph->getStepPieces()) {
        // We have to find the position of the piece in all the classification vectors
        int positionSimilarity = 0, positionVolume = 0, positionComplexity = 0;
        auto hasBeenFound = false;
        if (!classificationBySimilarity.empty()) {
            for (const auto& clsSimilarity : classificationBySimilarity) {
                positionSimilarity++;
                for (auto stepPiece : clsSimilarity) {
                    if (stepPiece == part) {
                        hasBeenFound = true;
                        break;
                    }
                }
                if (hasBeenFound) 
                    break;
            }
        }
        if (!classificationByVolumeMbb.empty()) {
            hasBeenFound = false;
            for (const auto& clsVolumeMbb : classificationByVolumeMbb) {
                positionVolume++;
                for (auto stepPiece : clsVolumeMbb) {
                    if (stepPiece == part) {
                        hasBeenFound = true;
                        break;
                    }
                }
                if (hasBeenFound) 
                    break;
            }
        }
        if (!classificationByComplexity.empty()) {
            hasBeenFound = false;
            for (const auto& clsComplexity : classificationByComplexity) {
                positionComplexity++;
                for (auto stepPiece : clsComplexity) {
                    if (stepPiece == part) {
                        hasBeenFound = true;
                        break;
                    }
                }
                if (hasBeenFound) 
                    break;
            }
        }

        double scoreIncr = 0.0;
        if (!classificationBySimilarity.empty()) scoreIncr += positionSimilarity * 100. / classificationBySimilarity.size() * weightSimilarity;
        if (!classificationByVolumeMbb.empty()) scoreIncr += positionVolume * 100. / classificationByVolumeMbb.size() * weightVolume;
        if (!classificationByComplexity.empty()) scoreIncr += positionComplexity * 100. / classificationByComplexity.size() * weightComplexity;
        part->setScore(scoreIncr);
        sortedByScore.push_back(part);
    }
    sortedByScore.sort([](StepPiece* a, StepPiece* b) { return a->getScore() < b->getScore(); });

    
    for (auto piece = sortedByScore.begin(); piece != sortedByScore.end(); piece++) {
        double minScore = (*piece)->getScore();
        std::list<StepPiece*> piecesWithLowestScore;
        
        for (; piece != sortedByScore.end(); piece++)
            if ((*piece)->getScore() == minScore)
                piecesWithLowestScore.push_back(*piece);
            else
                break;

        result.push_back(piecesWithLowestScore);
    }

    return result;
}
