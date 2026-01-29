#ifndef IMESSAGE_H_
#define IMESSAGE_H_

class IMessage
{
public:
    enum class MessageType {
        UNDO,
        REDO,
        CLEAR,
        MODAL,
        CLICK,
        FILES,
        NEW_PROJECT,
        SUB_PROJECT,
        CONVERTION_OPTION,
        ASCII_INFO,
        OBJECT_EXPORT,
        GRID_EXPORT,
        EXPORT_INIT,
        DUPLICATION_INIT,
        PRIMITIVES_EXPORT_PARAMETERS,
        CLIPPING_EXPORT_PARAMETERS,
        DELETE_POINTS_PARAMETERS,
        STAT_OUTLIER_FILTER_PARAMETERS,
        COLOR_BALANCE_PARAMETERS,
        PCO_CREATION_PARAMETERS,
        VIDEO_EXPORT_PARAMETERS,
        RENDER_CONTEXT,
        COLOR_AVAILABLE_LIST,
        SCANS_RANDOM_COLOR_INFO,
        FULL_CLICK,
        SIMPLE_NUMBER,
        DOUBLE_VECTOR,
        GUID,
        COLOR,
        VECTOR_3,
        DATAID_LIST,
        TREEID,
        CAMERA,
        TEMPLATE,
        TEMPLATE_LIST,
        PIPEMESSAGE,
		PIPECONNECTIONMESSAGE,
        //VIEWPORT_ID,
		SELECTED_DATA,
        IMPORT_SCAN,
        IMPORT_MESHOBJECT,
		STEP_SIMPLIFICATION,
        MESH_BUFFER,
        PLANEMESSAGE,
        BEAMBENDINGMESSAGE,
        SETOFPOINTSMESSAGE,
        GENERALMESSAGE,
        MANIPULATE
    };

	IMessage() {};
    virtual ~IMessage() {};
	virtual MessageType getType() const = 0;
    virtual IMessage* copy() const = 0;

};

#endif //! IMESSAGE_H_
