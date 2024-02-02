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

#include "io/imports/step-simplification/classification/QuickClassificationBySimilarity.h"

/**
 * Constructor that allocates memory for the list of list of StepPiece obtained with the result of the classification
 */
QuickClassificationBySimilarity::QuickClassificationBySimilarity(StepGraph* _stepGraph)
    : ClassificationStrategy(_stepGraph)
{};

/**
 * Method of classifying parts according to their similarity to each other
 * @param stepPieces The list of pieces to compare
 * @return The list of 'families' of pieces : a list of list of StepPiece*
 *
 * Explanation :
 * The similarity criteria used is an equivalence relation. It is a binary relation that is reflexive, symmetric and transitive.
 * This equivalence relation provides the partition of the underlying list into disjoint equivalence classes.
 * Two elements in the given list are equivalent to each other, if and only if they belong to the same equivalence class.
 * Here, the similarity criteria is not 100% reliable as it is only based on both number of entities and depth of the pieces.
 */
std::list<std::list<StepPiece*>> QuickClassificationBySimilarity::classify() {
    auto quickClasses = std::list<std::list<StepPiece *>>();

    for (auto stepPiece : getStepGraph()->getStepPieces()) {
        bool stepPieceNotInserted = true;

        for (auto& quickClass : quickClasses) {
            auto pieceOfClass = quickClass.front();
            if (stepPiece->getDepth() == pieceOfClass->getDepth() &&
                stepPiece->getNbEntities() == pieceOfClass->getNbEntities() ) 
            {
                quickClass.push_back(stepPiece);
                stepPieceNotInserted = false;
                break;
            }
        }
        if (stepPieceNotInserted) {
            auto newClass = std::list<StepPiece*>();
            newClass.push_back(stepPiece);
            quickClasses.push_back(newClass);
        }
    }

    return quickClasses;
}
