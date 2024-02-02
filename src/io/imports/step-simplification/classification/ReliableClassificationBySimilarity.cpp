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

#include "io/imports/step-simplification/classification/ReliableClassificationBySimilarity.h"
#include "io/imports/step-simplification/classification/QuickClassificationBySimilarity.h"

/**
 * Constructor that allocates memory for the list of list of StepPiece obtained with the result of the classification
 */
ReliableClassificationBySimilarity::ReliableClassificationBySimilarity(StepGraph* _stepGraph)
    : ClassificationStrategy(_stepGraph)
{}

/**
 * Destructor cleaning the memory allocated for the classes list
 */
ReliableClassificationBySimilarity::~ReliableClassificationBySimilarity() = default;


/**
 * Reliable method comparing each piece with all the other pieces.
 * @param stepPieces The list of pieces to compare
 * @return The list of 'families' of pieces : a list of list of StepPiece*
 *
 * Explanation :
 * The similarity criteria used is an equivalence relation. It is a binary relation that is reflexive, symmetric and transitive.
 * This equivalence relation provides the partition of the underlying list into disjoint equivalence classes.
 * Two elements in the given list are equivalent to each other, if and only if they belong to the same equivalence class.
 * Here, the similarity criteria is quite reliable as it is based on number of entities, depth of the pieces, but also the exact
 * occurrences of each entity.
 */
std::list<std::list<StepPiece *>> ReliableClassificationBySimilarity::classify() {
    auto quickClassification = QuickClassificationBySimilarity(getStepGraph());
    std::list<std::list<StepPiece *>> quickClasses = quickClassification.classify();

    auto reliableClasses = std::list<std::list<StepPiece*>>();

    for (auto quickClass : quickClasses) {
        if (quickClass.size() == 1)
            reliableClasses.push_back(quickClass);
        else {
            std::list<std::list<StepPiece*>> mapClasses = classifyByMap(quickClass);
            reliableClasses.insert(reliableClasses.end(), mapClasses.begin(), mapClasses.end());
        }
    }

    reliableClasses.sort([](std::list<StepPiece*> v1, std::list<StepPiece*> v2) { return v1.size() > v2.size(); });

    return reliableClasses;
}

std::list<std::list<StepPiece*>> ReliableClassificationBySimilarity::classifyByMap(std::list<StepPiece*> stepPieces) {
    auto mapClasses = std::list<std::list<StepPiece*>>();

    for (auto stepPiece : stepPieces) {
        bool stepPieceNotInserted = true;

        for (auto& mapClass : mapClasses) {
            auto pieceOfClass = mapClass.front();
            if (stepPiece->getOccurrences() == pieceOfClass->getOccurrences()) {
                mapClass.push_back(stepPiece);
                stepPieceNotInserted = false;
            }
        }
        if (stepPieceNotInserted) {
            auto newClass = std::list<StepPiece*>();
            newClass.push_back(stepPiece);
            mapClasses.push_back(newClass);
        }
    }

    return mapClasses;
}