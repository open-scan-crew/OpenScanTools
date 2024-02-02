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

#ifndef STEPFILE_H
#define STEPFILE_H

#include <map>
#include <list>
#include <iostream>
#include <fstream>
#include <regex>
#include <utility>
#include <cassert>
#include <chrono>
#include <ctime>
#include <sys/stat.h>

#include "StepGraph.h"
#include "EntityFactory.h"
#include "../classification/ClassificationStrategy.h"

/**
 * Representation of STEP file
 *
 * @param stepGraph    Dependency graph of entities from STEP file
 * @param header       STEP file header
 * @param footer       STEP file footer
 */
class StepFile {
private:
    // Attributes
    StepGraph* stepGraph;
    std::string header;
    std::string footer;
    std::string path;

private:
    void buildGraph(std::string& path);
    // Entities considered as start of pieces
    std::list<Entity*> nodesOfStepParts;

    // Graph Building
    void addAllEntities(std::string &l, std::regex &rId, std::regex &rName, std::ifstream &f);
    static bool isWrongReference(std::string ref, std::list<std::string> wrongReferences);
    void buildAllDependencies();

    // Step header and footer setters
    void setHeader(std::string &l, std::regex &rId, std::regex &rName, std::ifstream &f);
    void setFooter(std::string &l, std::ifstream &f);

public:
    StepFile(std::string path);
    StepFile(StepGraph* _stepGraph);
    StepFile(StepGraph* _stepGraph, const std::string& _header, const std::string& _footer);
    ~StepFile();

    // Attribute getters
    inline StepGraph* getGraph() { return stepGraph; }
    inline const std::string getHeader() const { return header; }
    inline const std::string getFooter() const { return footer; }
    inline const std::list<StepPiece*>& getPieces() { return stepGraph->getStepPieces(); }
    inline const std::map<unsigned int, Entity*>& getEntities() { return stepGraph->getNodes(); }

    // Graph Building
    void addDependencies(Entity *entity);

    // Writing output step file
    void write(const std::string& path);
    void write();
};

#endif //STEPFILE_H
