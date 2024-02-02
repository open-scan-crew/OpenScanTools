#ifndef CONTEXT_CONVERT_SCAN_H_
#define CONTEXT_CONVERT_SCAN_H_

#include "controller/functionSystem/AContext.h"
#include "io/ConvertProperties.h"
#include "io/imports/ImportTypes.h"

#include "models/pointCloud/TLS.h"

#include <qstring.h>
#include <sstream>
#include <map>

enum FileType;
namespace tls {
    enum PrecisionType;
};

class IScanFileReader;
class IScanFileWriter;

class ContextConvertionScan: public AContext
{
public:
	enum CheckState { OK, WARNING, FORCE, RENAME, FAIL };
public:
	ContextConvertionScan(const ContextId& id);
	~ContextConvertionScan();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	ContextState validate(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

private:
	void registerConvertedScan(Controller& controller, const std::filesystem::path& filename, const bool& overwritedFile, const float& time);

    void convertFile(Controller& controller, const std::filesystem::path& inputFile, const tls::PrecisionType& uprecision);

	bool convertOne(IScanFileReader* reader, uint32_t readerOffset, IScanFileWriter* writer, tls::PrecisionType outPrec);

	void updateStep(Controller& controller, const QString& state, const uint64_t& step);

	int checkScansExist(const std::filesystem::path& inputFile, std::filesystem::path destDir, std::vector<std::filesystem::path>& outputFiles, std::vector<tls::ScanHeader>& headers, bool& renaming, std::wstring& log);


private:
	std::vector<std::filesystem::path>										m_inputFiles;
	std::vector<glm::dvec3>													m_importScanPosition;
    ConvertProperties														m_properties;
	QString																	m_log;
	uint64_t																m_currentStep;
    bool																	m_userEdits;
    bool																	m_keepOnlyVisiblePoints;
	std::map<std::filesystem::path, AsciiImport::Info>						m_mapAsciiInfo;
};

#endif // !CONTEXT_CONVERT_SCAN_H_