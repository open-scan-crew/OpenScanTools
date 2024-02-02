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

#include "io/imports/step-simplification/classification/ClassificationByComplexity.h"

ClassificationByComplexity::ClassificationByComplexity(StepGraph* _stepGraph, unsigned int _nbClasses)
        : ClassificationStrategy(_stepGraph), nbClasses(_nbClasses)
{}

ClassificationByComplexity::~ClassificationByComplexity() = default;

bool ClassificationByComplexity::moreComplex(std::list<StepPiece*> v1, std::list<StepPiece*> v2) {
    return v1.front()->getNbEntities() < v2.front()->getNbEntities();
}

/**
 * Method of classifying parts according to their complexity (the number of entities necessary for its representation)
 * The number of classes expected in the result is defined by the user when creating the object
 * The 'families' of piece are ordered from the least complex to the most complex
 *
 * @return The list of 'families' of pieces : a list of list of StepPiece*
 */
std::list<std::list<StepPiece*>> ClassificationByComplexity::classify() {
    std::list<std::list<StepPiece*>>  classesBySimilarity = QuickClassificationBySimilarity(getStepGraph()).classify();
    std::list<std::list<StepPiece*>> classesByComplexity;

    // Definition of the number of classes to be returned
    unsigned int nbCls = 0;
    if (classesBySimilarity.size() < nbClasses) 
        nbCls = (unsigned int)classesBySimilarity.size();
    else 
        nbCls = nbClasses;

    classesBySimilarity.sort(moreComplex);
    if (classesBySimilarity.size() == nbCls) 
        return classesBySimilarity;

    std::list<unsigned int> threshold;
    for (unsigned int i = 1; i <= nbCls; ++i) {
        threshold.push_back((i) * ((unsigned int)classesBySimilarity.size() / nbCls));
        auto v = std::list<StepPiece*>();
        classesByComplexity.push_back(v);
    }

    std::list<unsigned int>::iterator itThreshold; itThreshold = threshold.begin();
    unsigned int i = 0;
    for(auto & itClassBySimilarity : classesBySimilarity) {
        for(auto & itClassByComplexity : classesByComplexity) {
            if(i < *itThreshold) {
                for(auto & stepPiece : itClassBySimilarity) 
                    itClassByComplexity.push_back(stepPiece);
                break;
            } itThreshold++;
        } i++;
    }

    return classesByComplexity;
}


