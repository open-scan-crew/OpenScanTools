#include "io/imports/step-simplification/step-simplification.h"

#include "io/imports/step-simplification/deletePiece/SmartDeletion.h"
#include "io/imports/step-simplification/stepGraph/StepFile.h"

#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/SplashScreenTexts.hpp"
#include "utils/Logger.h"
#include "utils/Utils.h"

// Useful uses for calculating execution time
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;
auto t0 = high_resolution_clock::now();
auto t1 = t0;

#define IOLOG Logger::log(LoggerMode::IOLog)

void StepSimplification::startTimer(std::string display, std::stringstream& ss) {
    ss << std::endl;
	ss << display << std::endl;
    t1 = high_resolution_clock::now();
}

void StepSimplification::stopTimer(std::stringstream& ss) {
    auto t2 = high_resolution_clock::now();
    duration<double, std::milli> runningTime = t2 - t1;
    if (runningTime.count() < 1000) ss << runningTime.count() << " ms" << std::endl;
    else if(runningTime.count() < 60000) ss << runningTime.count()/1000 << " s" << std::endl;
    else ss << runningTime.count()/60000 << " min" << std::endl;
}

void StepSimplification::totalRunningTime() {
    std::cout << std::endl;
    auto tEnd = high_resolution_clock::now();
    duration<double, std::milli> totalTime = tEnd - t0;
    if (totalTime.count() < 1000) std::cout << "Total processing time : " << totalTime.count() << " ms" << std::endl;
    else if(totalTime.count() < 60000) std::cout << "Total processing time : " << totalTime.count()/1000 << " s" << std::endl;
    else std::cout << "Total processing time : " << totalTime.count()/60000 << " min" << std::endl;
    std::cout << std::endl;
}

std::ifstream::pos_type StepSimplification::filesize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

ObjectAllocation::ReturnCode StepSimplification::modelSimplification(const std::filesystem::path& inputFilePath, const std::filesystem::path& outputFilePath, const StepClassification& classification, const double& keepPercent, Controller& controller) {
    
	std::stringstream info;

	// Loading the STEP file
    startTimer("Collecting information and building graph...", info);
    auto stepFile = new StepFile(inputFilePath.string());
    stopTimer(info);
	controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(info.str().c_str())));
	controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SPLASH_SCREEN_SIMPLIFY_STEP_DATA_PROCESSING, 1));
	info.clear();
    //auto cls = ClassificationByVolumeMBB(stepFile->getGraph(), 1000).classify();

    //do {

        // Deletion of some pieces
        startTimer("Deleting pieces in the model...", info);
        SmartDeletion smartDeletion = SmartDeletion(stepFile->getGraph());
		controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SPLASH_SCREEN_SIMPLIFY_STEP_DATA_PROCESSING, 2));
		controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(info.str().c_str())));
		info.clear();

        /*auto cpt = 1;
        for (auto cls : ClassificationByVolumeMBB(stepFile->getGraph(), 1000).classify()){
            smartDeletion.exportOneClass(cls);
            std::stringstream entitiesId;
            for (auto piece : cls) entitiesId << "_#" << piece->getRoot()->getId();
            std::stringstream outputPath;
            outputPath << "/home/antoine/Documents/Polytech/S8/Stage/step-simplification/step-models/out/Antoine/" << cpt << "_901006134_30_TL00650KCKL" << entitiesId.str() << ".STEP";
            stepFile->write(outputPath.str());
            stepFile->getGraph()->restoreInitialModel();
            cpt++;
        }*/

        //smartDeletion.setWeightVolume(3);
        switch (classification)  {
			case StepClassification::Similarity: 
				smartDeletion.add(ReliableClassificationBySimilarity(stepFile->getGraph())); 
				break;
			case StepClassification::Complexity: 
				smartDeletion.add(ClassificationByComplexity(stepFile->getGraph(), 1000)); 
				break;
			case StepClassification::Volume: 
				smartDeletion.add(ClassificationByVolumeMBB(stepFile->getGraph(), 1000)); 
				break;
        }

        info = smartDeletion.getClasses();
		controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SPLASH_SCREEN_SIMPLIFY_STEP_DATA_PROCESSING, 3));
		controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(info.str().c_str())));
		info.clear();

		double newPercent;
		try {
			smartDeletion.performDeletion(keepPercent, newPercent);
		}
		catch (std::runtime_error e)
		{
			IOLOG << e.what() << LOGENDL;
			return ObjectAllocation::ReturnCode::Load_File_Error;
		}

		stopTimer(info);
		controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SPLASH_SCREEN_SIMPLIFY_STEP_DATA_PROCESSING, 4));
		controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(info.str().c_str())));
		info.clear();

		if (newPercent != keepPercent * 100)
			info << std::endl << "New Keep Percent : " << 100 - newPercent << std::endl << std::endl;

        // Statistics display
		info << std::endl << "Number of deleted pieces : " << stepFile->getGraph()->getNumberOfDeletedPieces() << std::endl;
		info << "Initial number of pieces in the model : " << stepFile->getPieces().size() << std::endl;
		info << "Actual number of pieces in the model : " << stepFile->getGraph()->getNumberOfPiece() << std::endl;

        // Writing result
        startTimer("Writing result...", info);
		controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(info.str().c_str())));
		info.clear();

        stepFile->write(outputFilePath.string());
        stopTimer(info);
		controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SPLASH_SCREEN_SIMPLIFY_STEP_DATA_PROCESSING, 5));
		controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(info.str().c_str())));
		info.clear();

        // Restoring graph to initial state
		/*startTimer("Restore all pieces in the model...", info);
        stepFile->getGraph()->restoreInitialModel();
        stopTimer(info);*/

        /*std::cout << "Do you want to continue? (Y/N) ";
        std::cin >> answer;*/
    /*}
    while((answer =='Y') || (answer =='y'));*/

    //totalRunningTime();
		return ObjectAllocation::ReturnCode::Success;
}

// TO DO LIST:
/*  Améliorer la lecture du fichier :
 *      DONE - l'utilisateur doit pouvoir se tromper dans l'extension du fichier
 *      - l'utilisateur doit pouvoir ne spécifier que le nom du fichier sans avoir à spécifier son chemin (à condition que le fichier se trouve dans le répertoire de maquettes)
 *      DONE - l'utilisateur doit pouvoir se dispenser de spécifier le chemin de sortie : (par défaut : même chemin qu'en entrée avec l'ajout de .out dans le nom du fichier)
 *
 * Tester la suppression et la restoratiocn de toutes les pièces sur l'ensemble des maquettes du jeu de données :
 *      - elle doit établir une liste de maquettes sur lesquelles il y a des problèmes et tenter de les résoudre si il suffit de changer le comportement des entités
 *
 * Terminer l'automatisation des méthodes de supression qui jusqu'alors est partielle (fonctionne seulement pour le critère de similarité)
 *      - étendre pour les critères de volume et de complexité (et vérifier la cohérence) DONE
 *      - généraliser la méthode de suppression par coefficient pour qu'elle soit utilisable sur n'importe quel type de vecteur (ou liste) DONE
 *      - proposer des méthodes de suppressions basés sur des combinaisons de critères DONE
 *
 * Ajout d'un onglet dédié à la simplification dans OpenScanTools avec Qt
 *      - configurer MVSC avec Qt et parvenir à compiler le projet OpenScanTools sur le PC Windows
 *      - Reflexion sur l'intéraction et proposition d'une implémentation facilité par les fonctions de simplification et de restoration du graphe
 *
 * QUESTIONS :
 *      - Pourquoi la construction du graphe est beaucoup plus longue qu'avant (~ + 30%) ?
 *      Hypothèse : A cause de la regew pour détecter les fausses références        DONE
 */


// Displaying of entities (why nulll entities ?)
/* auto numberOfNullEntities = 0;
std::cout << "List of null entities : " << std::endl;
for (auto e : stepFile->getEntities()) {
    if (e.second == NULL) {
        std::cout << e.first << ", ";
        numberOfNullEntities++;
    }
}
std::cout << "\rNumber of null entities : " << numberOfNullEntities << std::endl;*/


// CLASSIFICATION AND DELETION BASED ON SIMILARITY CRITERIA
// startTimer("Classifying and deleting with classification by similarity...");
/* double percentageOfPieceToKeep = 0.2;
unsigned int threshold = 5;
SmartDeletion del = SmartDeletion(ReliableClassificationBySimilarity(stepFile->getGraph()), threshold);
del.performDeletion();*/

// CLASSIFICATION BASED ON SIMILARITY CRITERIA
/*startTimer("Classifying with classification by similarity...");
auto classificationBySimmilarity = ReliableClassificationBySimilarity(stepFile->getGraph());
std::list<std::list<StepPiece*>> classesBySimilarity = classificationBySimmilarity.classify();
for (auto cls : classesBySimilarity) {
    if (cls.size() > 2) {
        DeleteClass del = DeleteClass(cls, stepFile->getGraph());
        del.deleteClass();
        nbPiecesDeleted += cls.size();
    }
}
stopTimer();*/

// CLASSIFICATION BASED ON VOLUME CRITERIA
/*startTimer("Classifying with classification by volume...");
auto classificationByVolume = ClassificationByVolumeMBB(stepFile->getGraph(), 20);
std::list<std::list<StepPiece*>> classesByVolume = classificationByVolume.classify();*/
/*auto numberOfClass = 1;
for (auto cls : classesByVolume) {
    if (numberOfClass < classesByVolume.size() - 1) {
        DeleteClass del = DeleteClass(cls, stepFile->getGraph());
        del.deleteClass();
        nbPiecesDeleted += cls.size();
    }
    numberOfClass++;
}*/

// CLASSIFICATION BASED ON COMPLEXITY CRITERIA
/* startTimer("Classifying with classification by complexity...");
 auto classificationByComplexity = ClassificationByComplexity(stepFile->getGraph(), 10);
 std::list<std::list<StepPiece*>> classesByComplexity = classificationByComplexity.classify();
 auto numberOfClass = 1;
 for (auto cls : classesByVolume) {
     if (numberOfClass < classesByVolume.size() - 1) {
         DeleteClass del = DeleteClass(cls, stepFile->getGraph());
         del.deleteClass();
         nbPiecesDeleted += cls.size();
     }
     numberOfClass++;
 }
 stopTimer();*/

/*
    if (classesByVolume.back().front()->getVolumeMBB() == 0) std::cout << "Number of pieces with no volume : " << classesByVolume.back().size() << std::endl;
*/

/*
    stepFile->write(pathOutputStepFile);
*/

/*******************************************************************************************
 *                                      STATISTICS
 *******************************************************************************************/
/*auto inputFileSize = filesize(pathInputStepFile);
auto outputFileSize = filesize(pathOutputStepFile);*/
/*std::cout << "The product results in a " << (inputFileSize - outputFileSize) * 100 / inputFileSize << "% reduction of the input file" << std::endl;
std::cout << "Size of input file : " << filesize("../../step-models/in/Antoine/901006134_30_TL00650KCKL.STEP") << std::endl;
std::cout << "Size of output file : " << filesize("../../step-models/in/Antoine/901006134_30_TL00650KCKL.out.STEP") << std::endl;*/
/*std::cout << "Initial number of pieces in the model : " << stepFile->getPieces().size() << std::endl;
std::cout << "Actual number of pieces in the model : " << stepFile->getGraph()->getNumberOfPiece() << std::endl;
std::cout << "Number of deleted pieces : " << stepFile->getGraph()->getInitialNumberOfPieces() - stepFile->getGraph()->getNumberOfPiece() << std::endl;*/
