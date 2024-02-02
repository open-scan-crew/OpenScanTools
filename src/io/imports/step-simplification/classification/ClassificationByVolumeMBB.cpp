/* Copyright (C) 2020 Antoine Garnier <antoine.garnier1@etu.univ-nantes.fr>
 * Copyright (C) 2020 Evan Gisdal <evan.gisdal@etu.univ-nantes.fr>
 * Copyright (C) 2020 Antoine Humbert <antoine.humbert@etu.univ-nantes.fr>
 *
 * This file is part of a transversal project made by students from the computer science department
 * of the Graduate School of Engineering of the University of Nantes. It was realized as an attempt
 * to solve the problem of the simplification of 3D models with the STEP format.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "io/imports/step-simplification/classification/ClassificationByVolumeMBB.h"

ClassificationByVolumeMBB::ClassificationByVolumeMBB(StepGraph* _stepGraph, unsigned int _nbClasses)
        : ClassificationStrategy(_stepGraph), nbClasses(_nbClasses)
{}

/**
 * Useful method for comparing the size between two pieces
 * @param v1 The first volume to compare
 * @param v2 The second volume to compare
 * @return True if the second piece is larger than the first, False instead
 */
bool ClassificationByVolumeMBB::larger(std::list<StepPiece*> v1, std::list<StepPiece*> v2) {
    return v1.front()->getVolumeMBB() < v2.front()->getVolumeMBB();
}

/**
 * A method of going through all the parts of the model to group those with the same volumes together.
 * @param stepPieces The list containing all the pieces of the model
 * @return The resulting classes of classification performed (each class groups only parts of the same volume)
 */
std::list<std::list<StepPiece*>> ClassificationByVolumeMBB::classifyOneVolumeOneClass(std::list<StepPiece*> stepPieces) {
    std::list<std::list<StepPiece*>> classificationOneVolumeOneClass;
    for (auto & stepPiece : stepPieces) {
        bool volumeFound = false;
        for (auto & classOneVolumeOneClass : classificationOneVolumeOneClass) {
            // Due to the precision of the dual type, we need to establish a tolerance to avoid identical volumes being distributed in different classes.
            auto plus = classOneVolumeOneClass.front()->getVolumeMBB() + (classOneVolumeOneClass.front()->getVolumeMBB() * 0.001);
            auto minus = classOneVolumeOneClass.front()->getVolumeMBB() - (classOneVolumeOneClass.front()->getVolumeMBB() * 0.001);
            if ((plus > stepPiece->getVolumeMBB() && minus < stepPiece->getVolumeMBB()) ||
                stepPiece->getVolumeMBB() == classOneVolumeOneClass.front()->getVolumeMBB()) {
                classOneVolumeOneClass.push_back(stepPiece);
                volumeFound = true;
            }
        }
        if (!volumeFound) {
            std::list<StepPiece*> newClass;
            newClass.push_back(stepPiece);
            classificationOneVolumeOneClass.push_back(newClass);
        }
    }
    classificationOneVolumeOneClass.sort(larger);
    return classificationOneVolumeOneClass;
}

/**
 * Method of classifying parts according to their volume (when it is possible to calculate it)
 * The number of classes expected in the result is defined by the user when creating the object
 * The 'families' of piece are ordered from the smallest to the largest
 * If there are some pieces with no volume, these pieces will be grouped in the same class (the last of the returned list)
 *
 * @return The list of 'families' of pieces : a list of list of StepPiece*
 */
std::list<std::list<StepPiece*>> ClassificationByVolumeMBB::classify() {
    std::list<std::list<StepPiece*>> classesOneVolumeOneClass = classifyOneVolumeOneClass(getStepGraph()->getStepPieces());
    std::list<std::list<StepPiece*>> classesByVolume;

    // Definition of the number of classes to be returned
    unsigned int nbCls;
    if (classesOneVolumeOneClass.size() < nbClasses) 
        nbCls = (unsigned int)classesOneVolumeOneClass.size();
    else
        nbCls = nbClasses;

    if (nbCls != classesOneVolumeOneClass.size()) {
        std::list<unsigned int> threshold;
        for (unsigned int i = 1; i <= nbCls; ++i) {
            if (classesOneVolumeOneClass.front().front()->getVolumeMBB() == 0) 
                threshold.push_back((i) * ((unsigned int)classesOneVolumeOneClass.size()-1) / nbCls);
            else 
                threshold.push_back((i) * ((unsigned int)classesOneVolumeOneClass.size() / nbCls));
            auto v = std::list<StepPiece*>();
            classesByVolume.push_back(v);
        }

        std::list<unsigned int>::iterator itThreshold; itThreshold = threshold.begin();
        unsigned int i = 0;
        for(auto & itClassOneVolumeOneClass : classesOneVolumeOneClass) {
            if (itClassOneVolumeOneClass.front()->getVolumeMBB() != 0) {
                for(auto & itClassByVolume : classesByVolume) {
                    if(i < *itThreshold) {
                        for(auto & stepPiece : itClassOneVolumeOneClass) 
                            itClassByVolume.push_back(stepPiece);
                        break;
                    } itThreshold++;
                } i++;
            }
        }

        // The last class is the one with parts of volumes equal to zero (because they could not be calculated)
        if (classesOneVolumeOneClass.front().front()->getVolumeMBB() == 0) 
            classesByVolume.push_back(classesOneVolumeOneClass.front());
    }
    else {
        // Swap the first class (zero volumes) with the last one in the list
        std::list<StepPiece*> nullVolume = classesOneVolumeOneClass.front();
        if (nullVolume.front()->getVolumeMBB() == 0)
        {
            classesOneVolumeOneClass.pop_front();
            classesOneVolumeOneClass.push_back(nullVolume);
        }
        return classesOneVolumeOneClass;
    }

    return classesByVolume;
}
