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

#include "io/imports/step-simplification/stepGraph/StepFile.h"

#include <filesystem>
#include <stdio.h>

/**
 * StepFile Constructor from a graph
 * @param s The step graph we want associated it with StepFile object
 */
StepFile::StepFile(const std::string _path)
    : header(), footer(), path(_path), stepGraph(new StepGraph()) { buildGraph(path); }

StepFile::StepFile(StepGraph* _stepGraph)
    : stepGraph(_stepGraph), header(), footer()
{}

StepFile::StepFile(StepGraph* _stepGraph, const std::string& _header, const std::string& _footer)
    : stepGraph(_stepGraph), header(_header), footer(_footer)
{}

StepFile::~StepFile() {
    delete stepGraph;
}

/******************************************************************************************
                                    GRAPH BUILDING
 ******************************************************************************************/

/**
 * Add dependencies for an entity and its children
 * @param entity  The entity from which we add dependencies
 */
void StepFile::addDependencies(Entity *entity) {
    std::string arguments(entity->getArguments());
    if (entity->getId() == 747) {
        std::cout << std::endl;
    }

    // We need to detect wrong references.
    // Example : #747=NEXT_ASSEMBLY_USAGE_OCCURRENCE('#1709# (558898032) .... );   =>  with '#1709#' which does not refer to any entity
   /* std::list<std::string> wrongReferences;
    std::regex wrongDep( "#([0-9]+)#" ); // To attempt to detect false references
    std::sregex_iterator next1 (arguments.begin(), arguments.end(), wrongDep);
    std::sregex_iterator end1;
    std::smatch matchWrongDep;
    while (next1 != end1) {
        matchWrongDep = *next1;
        wrongReferences.push_back(matchWrongDep[1]);
        next1++;
    }*/

    std::regex regexDep( "#([0-9]+)" );
    std::sregex_iterator next (arguments.begin(), arguments.end(), regexDep);
    std::sregex_iterator end;
    std::smatch matchDep;
    while (next != end) {
        matchDep = *next;
        /*if (wrongReferences.empty()) {*/
            int matchId = std::stoi(matchDep[1].str());
            Entity *entityDep = stepGraph->getNodes()[matchId];
            if (entityDep != NULL) {
                entity->addChild(entityDep);
                entityDep->addParent(entity);
            }
            else {
                if (matchDep[1].str() == "322519")
					std::cout << "#" << matchDep[1].str() << std::endl; // DEBUG LINE
            }
        /*} else {
            if (!isWrongReference(matchDep[1].str(), wrongReferences)) {
                int matchId = std::stoi(matchDep[1].str());
                Entity *entityDep = stepGraph->getNodes()[matchId];
                if (entityDep != NULL) {
                    entity->addChild(entityDep);
                    entityDep->addParent(entity);
                }
            }
        }*/
        next++;
    }
}

/**
 * Methode indicating if a reference is wrong (Example : #0087# is a wrong reference because a '#" is appears at the end
 * @param ref The entity to compare
 * @param wrongReferences the list with wrong detected entities
 * @return true if the reference is wrong, false instead
 */
bool StepFile::isWrongReference(std::string ref, std::list<std::string> wrongReferences) {
    // Create a list Iterator
    std::list<std::string>::iterator it;
    // Fetch the iterator of element with value 'the'
    it = std::find(wrongReferences.begin(), wrongReferences.end(), ref);
    // Check if iterator points to end or not
    if (it != wrongReferences.end()) return true;
    else return false;
}

/**
 * Method for building the graph of entities and their dependencies
 * @param path The input STEP file from which we build the graph
 * @return stepFile The object containing the STEP file information
 */
void StepFile::buildGraph(std::string& path) {
    // Testing the opening of the file
    std::list<std::string> fileExtension = {".stp", ".STP", ".step", ".STEP"};
    std::stringstream pathToTry;

    if (!(std::ifstream{path})) {
        std::stringstream pathBeforeExtension;
        // Split the string then get the last part of the string (file extension)
        int start = 0;
        std::string del = ".";
        int end = (int)path.find(del);

        auto firstPass = true;
        while (end != -1) {
            if (firstPass) pathBeforeExtension << path.substr(start, end - start);
            else pathBeforeExtension << "." << path.substr(start, end - start);
            start = end + (int)del.size();
            end = (int)path.find(del, start);
            firstPass = false;
        }

        for (auto ext : fileExtension) {
            pathToTry.str("");
            pathToTry << pathBeforeExtension.str() << ext.substr();
            if (std::ifstream{pathToTry.str()}) break;
        }
    } else pathToTry << path.substr();

    std::cout << pathToTry.str() << std::endl;
    std::ifstream file(pathToTry.str());
    assert(("Opening file", file));

    // Update the path that will be used as a basis for defining the writing path if it is not specified
    path = pathToTry.str();

    // Regex defining the writing of an entity in the step file
    std::regex  regexId(R"(\#(\d+)\s*)");
    std::regex  regexName(R"(=\s*([A-Z0-9_]*)\s*\()");

    // Read the first line of the step file
    std::string line;
    std::getline(file,line,';');

    // Do not interchange the three operations below, they have been designed to work in this specific order
    setHeader(line, regexId, regexName, file);           // Fetch the header of the step file
    addAllEntities(line, regexId, regexName, file);      // Reading the data of the step file and adding each entity into the graph
    setFooter(line, file);                                      // Fetch the footer of the step file

    // For each entity we build dependencies
    buildAllDependencies();

    stepGraph->createStepPieces(nodesOfStepParts);
}

/**
 * Building the dependencies of each entity in the graph
 * @param g The graph in which the dependencies must be built
 */
void StepFile::buildAllDependencies() {
    // For each entity we build dependencies
    std::map<unsigned int, Entity *> nodes = stepGraph->getNodes();
    for(auto & node : nodes) addDependencies(node.second);
}

/**
 * Method reading the data part of the step file and adding each entity into the graph
 * @param g The graph to fill in
 * @param l The line that was read
 * @param r The regex defining the writing of an entity in the step file
 * @param f The input step file
 */
void StepFile::addAllEntities(std::string &l, std::regex &rId, std::regex &rName, std::ifstream &f) {
    l.erase(std::remove(l.begin(), l.end(), '\n'), l.end()); // We delete the line breaks for an entity

    // Reading each entity to create their respective objects
    std::smatch mId;
    std::smatch mName;

    while(std::regex_search(l, mId, rId) && std::regex_search(l,mName,rName)) {

        // We get the identifier, the name and the arguments of the entity
        std::string ID = mId[1].str();
        if (mId[1].str().empty()) std::cout << l << std::endl;
        unsigned int matchId = std::stoul(ID); // Conversion from string to unsigned int
        std::string matchName(mName[1].str());

        unsigned int beginArgPos = (unsigned int)l.find_first_of('(');
        unsigned int endArgPos = (unsigned int)l.find_last_of(')');
        // Keep only the arguments, which are between the first '(' and the last ')'
        std::string arguments =  l.substr(beginArgPos+1, endArgPos-beginArgPos-1);

        auto currentEntity = EntityFactory::createEntity(matchId, matchName, arguments);
        stepGraph->addNode(currentEntity); // Add entity to the hash map between id's and entities
        if (matchName == "MANIFOLD_SOLID_BREP") nodesOfStepParts.push_back(currentEntity); // Add entity to the list of Step parts (manifold)

        std::getline(f, l, ';');
        l.erase(std::remove(l.begin(), l.end(), '\n'), l.end()); // We remove line breaks for each entity
    }
}


/******************************************************************************************
                              WRITING OUTPUT STEP FILE
 ******************************************************************************************/

std::string getOutputPath(std::string inputPath);

/**
 * Writing the output file from the graph
 * @param _path The path of the output file (.step)
 */
void StepFile::write(const std::string& path) {
	std::remove(path.c_str());
    std::ofstream outputFile(path);
    assert(("Correct output file", outputFile));

    // Writing header
    outputFile.imbue(std::locale::classic());
    outputFile << header << std::endl;
    // Writing data into the output file (since the map of entities)
    for (auto node : stepGraph->getNodes()) if (node.second != NULL) outputFile << *(node.second);
    // Writing footer
    outputFile << footer << std::endl;
}

/**
 * Writing the output file from the graph and place it in the same directory that the original model
 */
void StepFile::write() {
	std::string outPut = getOutputPath(path);
    std::ofstream outputFile(outPut, std::ofstream::out);
    assert(("Correct output file", outputFile));

    // Writing header
    outputFile << header << std::endl;
    // Writing data into the output file (since the map of entities)
    for (auto node : stepGraph->getNodes()) if (node.second != NULL) outputFile << *(node.second);
    // Writing footer
    outputFile << footer << std::endl;
}


/******************************************************************************************
                             STEP HEADER AND FOOTER SETTERS
 ******************************************************************************************/

/**
 * Method fetching the header from the STEP file and setting it as attribute
 * @param l The line that was read
 * @param r The regex defining the writing of an entity in the STEP file
 * @param f The input STEP file
 * @return The string containing the header
 */
void StepFile::setHeader(std::string &l, std::regex &rId, std::regex &rName, std::ifstream &f) {
    while ( !(std::regex_search(l, rId) && std::regex_search(l,rName)) ) {
        header.append(l+';');
        if (!std::getline(f,l,';')) throw std::runtime_error("No entities detected. The file is not a STEP file");
    }
}

/**
 * Method fetching the footer from the STEP file and setting it as attribute
 * @param l The line that was read
 * @param f The input STEP file
 * @return The string containing the footer
 */
void StepFile::setFooter(std::string &l, std::ifstream &f) {
    footer.append(l+';');
    while(std::getline(f,l,';')) footer.append(l+';');
    footer.pop_back(); // deletion of the extra semicolon
}

/**
 * Function that returns the output path and creates the output directories in the "out" directory in case the original file is in the 'in' directory
 * @param inputPath The path of the input step file
 * @return The path of the output step file
 */
std::string getOutputPath(std::string inputPath) {
    std::string pathOutFolder = inputPath; 
	unsigned int index;
    bool needToCreateDirectories = false;
    while((index = (unsigned int)pathOutFolder.find("/in/")) != std::string::npos) {
        pathOutFolder.replace(index, 4, "/out/"); //remove and replace from that position
        needToCreateDirectories = true;
    }

    std::stringstream pathBeforeFileName;

    unsigned int start = 0; std::string del = "/";
	unsigned int end = (unsigned int)pathOutFolder.find(del);
	unsigned int pos = end;

    std::list<std::string> nameRepositories;
    auto addNameRepo = false; auto firstPass = true;
    std::string firstDirectoryCreationPath;
    while (end != -1) {
        if (!addNameRepo) firstDirectoryCreationPath = pathBeforeFileName.str();
        pos = end; std::string nameCurrentFolder = pathOutFolder.substr(start, end - start);
        if (firstPass) pathBeforeFileName << nameCurrentFolder;
        else pathBeforeFileName << "/" << nameCurrentFolder;
        if (nameCurrentFolder == "out") addNameRepo = true;
        if (addNameRepo) nameRepositories.push_back(nameCurrentFolder);
        start = end + (unsigned int)del.size();
        end = (unsigned int)pathOutFolder.find(del, start);
        firstPass = false;
    }

    std::stringstream partialDirectoryCreationPath;
    partialDirectoryCreationPath << firstDirectoryCreationPath;
    if (needToCreateDirectories) { //if the output path is different from the input path ...
        for (auto dir : nameRepositories) {
            partialDirectoryCreationPath << "/" << dir;
            std::string stringToConvert = partialDirectoryCreationPath.str();
            const char* directoryCreationPath = stringToConvert.c_str();
            // Creating a directory
			std::filesystem::create_directory((directoryCreationPath));
        }
    }

    std::stringstream pathOutputFile;
    std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream replace; replace << "/" << std::ctime(&now_time) << "";
    if (needToCreateDirectories) 
		pathOutputFile << pathBeforeFileName.str() << "out.step";
    else 
		pathOutputFile << pathBeforeFileName.str() << "out.step";

    return pathOutputFile.str();
}