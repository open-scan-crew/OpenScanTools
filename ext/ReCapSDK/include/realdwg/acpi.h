//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////



/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Mon Jan 18 19:14:07 2038
 */
/* Compiler settings for idlsource\acpi.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0622 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __acpi_i_h__
#define __acpi_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IAcPiPropertyIdentifier_FWD_DEFINED__
#define __IAcPiPropertyIdentifier_FWD_DEFINED__
typedef interface IAcPiPropertyIdentifier IAcPiPropertyIdentifier;

#endif 	/* __IAcPiPropertyIdentifier_FWD_DEFINED__ */


#ifndef __IAcPiPropertyInspector_FWD_DEFINED__
#define __IAcPiPropertyInspector_FWD_DEFINED__
typedef interface IAcPiPropertyInspector IAcPiPropertyInspector;

#endif 	/* __IAcPiPropertyInspector_FWD_DEFINED__ */


#ifndef __IAcPiPropertyInspector2_FWD_DEFINED__
#define __IAcPiPropertyInspector2_FWD_DEFINED__
typedef interface IAcPiPropertyInspector2 IAcPiPropertyInspector2;

#endif 	/* __IAcPiPropertyInspector2_FWD_DEFINED__ */


#ifndef __IAcPiPropertyInspectorTheme_FWD_DEFINED__
#define __IAcPiPropertyInspectorTheme_FWD_DEFINED__
typedef interface IAcPiPropertyInspectorTheme IAcPiPropertyInspectorTheme;

#endif 	/* __IAcPiPropertyInspectorTheme_FWD_DEFINED__ */


#ifndef __IAcPiPropertyEditControl_FWD_DEFINED__
#define __IAcPiPropertyEditControl_FWD_DEFINED__
typedef interface IAcPiPropertyEditControl IAcPiPropertyEditControl;

#endif 	/* __IAcPiPropertyEditControl_FWD_DEFINED__ */


#ifndef __IAcPiPropertyEditControl2_FWD_DEFINED__
#define __IAcPiPropertyEditControl2_FWD_DEFINED__
typedef interface IAcPiPropertyEditControl2 IAcPiPropertyEditControl2;

#endif 	/* __IAcPiPropertyEditControl2_FWD_DEFINED__ */


#ifndef __IAcPiPropertyEditControl3_FWD_DEFINED__
#define __IAcPiPropertyEditControl3_FWD_DEFINED__
typedef interface IAcPiPropertyEditControl3 IAcPiPropertyEditControl3;

#endif 	/* __IAcPiPropertyEditControl3_FWD_DEFINED__ */


#ifndef __IAcPiPropertyEditEventsSink_FWD_DEFINED__
#define __IAcPiPropertyEditEventsSink_FWD_DEFINED__
typedef interface IAcPiPropertyEditEventsSink IAcPiPropertyEditEventsSink;

#endif 	/* __IAcPiPropertyEditEventsSink_FWD_DEFINED__ */


#ifndef __IAcPiPropertyEditEventsSink2_FWD_DEFINED__
#define __IAcPiPropertyEditEventsSink2_FWD_DEFINED__
typedef interface IAcPiPropertyEditEventsSink2 IAcPiPropertyEditEventsSink2;

#endif 	/* __IAcPiPropertyEditEventsSink2_FWD_DEFINED__ */


#ifndef __IAcPiPropertyUnspecified_FWD_DEFINED__
#define __IAcPiPropertyUnspecified_FWD_DEFINED__
typedef interface IAcPiPropertyUnspecified IAcPiPropertyUnspecified;

#endif 	/* __IAcPiPropertyUnspecified_FWD_DEFINED__ */


#ifndef __IAcPiPropertyExpression_FWD_DEFINED__
#define __IAcPiPropertyExpression_FWD_DEFINED__
typedef interface IAcPiPropertyExpression IAcPiPropertyExpression;

#endif 	/* __IAcPiPropertyExpression_FWD_DEFINED__ */


#ifndef __IAcPiPropertyDisplay_FWD_DEFINED__
#define __IAcPiPropertyDisplay_FWD_DEFINED__
typedef interface IAcPiPropertyDisplay IAcPiPropertyDisplay;

#endif 	/* __IAcPiPropertyDisplay_FWD_DEFINED__ */


#ifndef __IAcPiPropertyDisplay2_FWD_DEFINED__
#define __IAcPiPropertyDisplay2_FWD_DEFINED__
typedef interface IAcPiPropertyDisplay2 IAcPiPropertyDisplay2;

#endif 	/* __IAcPiPropertyDisplay2_FWD_DEFINED__ */


#ifndef __IAcPiCategorizeProperties_FWD_DEFINED__
#define __IAcPiCategorizeProperties_FWD_DEFINED__
typedef interface IAcPiCategorizeProperties IAcPiCategorizeProperties;

#endif 	/* __IAcPiCategorizeProperties_FWD_DEFINED__ */


#ifndef __IAcPiPropCommandButtons_FWD_DEFINED__
#define __IAcPiPropCommandButtons_FWD_DEFINED__
typedef interface IAcPiPropCommandButtons IAcPiPropCommandButtons;

#endif 	/* __IAcPiPropCommandButtons_FWD_DEFINED__ */


#ifndef __IAcPiCommandButton_FWD_DEFINED__
#define __IAcPiCommandButton_FWD_DEFINED__
typedef interface IAcPiCommandButton IAcPiCommandButton;

#endif 	/* __IAcPiCommandButton_FWD_DEFINED__ */


#ifndef __IAcPiCommandButton2_FWD_DEFINED__
#define __IAcPiCommandButton2_FWD_DEFINED__
typedef interface IAcPiCommandButton2 IAcPiCommandButton2;

#endif 	/* __IAcPiCommandButton2_FWD_DEFINED__ */


#ifndef __IAcPiCommandButton3_FWD_DEFINED__
#define __IAcPiCommandButton3_FWD_DEFINED__
typedef interface IAcPiCommandButton3 IAcPiCommandButton3;

#endif 	/* __IAcPiCommandButton3_FWD_DEFINED__ */


#ifndef __IAcPiPropertyInspectorEventsSink_FWD_DEFINED__
#define __IAcPiPropertyInspectorEventsSink_FWD_DEFINED__
typedef interface IAcPiPropertyInspectorEventsSink IAcPiPropertyInspectorEventsSink;

#endif 	/* __IAcPiPropertyInspectorEventsSink_FWD_DEFINED__ */


#ifndef __IAcPiPropertyInspectorRefreshEventSink_FWD_DEFINED__
#define __IAcPiPropertyInspectorRefreshEventSink_FWD_DEFINED__
typedef interface IAcPiPropertyInspectorRefreshEventSink IAcPiPropertyInspectorRefreshEventSink;

#endif 	/* __IAcPiPropertyInspectorRefreshEventSink_FWD_DEFINED__ */


#ifndef __IAcPiPropertyInspectorInputEventSink_FWD_DEFINED__
#define __IAcPiPropertyInspectorInputEventSink_FWD_DEFINED__
typedef interface IAcPiPropertyInspectorInputEventSink IAcPiPropertyInspectorInputEventSink;

#endif 	/* __IAcPiPropertyInspectorInputEventSink_FWD_DEFINED__ */


#ifndef __IAcPropertyFilter_FWD_DEFINED__
#define __IAcPropertyFilter_FWD_DEFINED__
typedef interface IAcPropertyFilter IAcPropertyFilter;

#endif 	/* __IAcPropertyFilter_FWD_DEFINED__ */


#ifndef __IAcPiFiltering_FWD_DEFINED__
#define __IAcPiFiltering_FWD_DEFINED__
typedef interface IAcPiFiltering IAcPiFiltering;

#endif 	/* __IAcPiFiltering_FWD_DEFINED__ */


#ifndef __AcPiPropertyInspector_FWD_DEFINED__
#define __AcPiPropertyInspector_FWD_DEFINED__

#ifdef __cplusplus
typedef class AcPiPropertyInspector AcPiPropertyInspector;
#else
typedef struct AcPiPropertyInspector AcPiPropertyInspector;
#endif /* __cplusplus */

#endif 	/* __AcPiPropertyInspector_FWD_DEFINED__ */


#ifndef __AcPiPropertyIdentifier_FWD_DEFINED__
#define __AcPiPropertyIdentifier_FWD_DEFINED__

#ifdef __cplusplus
typedef class AcPiPropertyIdentifier AcPiPropertyIdentifier;
#else
typedef struct AcPiPropertyIdentifier AcPiPropertyIdentifier;
#endif /* __cplusplus */

#endif 	/* __AcPiPropertyIdentifier_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_acpi_0000_0000 */
/* [local] */ 

#define ACPI_DEFAULT             0x00000000
#define ACPI_CLEARCACHE_REFRESH  0x00000001
#define ACPI_CLEARCACHE_DYNAMIC  0x00000002
#define ACPI_CLEARCACHE_INCLUDED 0x00000004
#define ACPI_ACAD_DYN            0x00000008
#define ACPI_UNION               0x00000010
#define ACPI_SORTBYNAME          0x00000020
#define ACPI_SORTBYWEIGHT        0x00000040
#define ACPI_REFRESH             0x00000080
#define ACPI_FILTERABLE          0x00000100
#define ACPI_EMPTYDISPLAY        0x00000200
#define ACPI_SETCONTROLFOCUS     0x00000001
#define ACPI_SCROLLTOCONTROL     0x00000002
#define     AC_TVS_TOOLTIPS             0x00000001
#define     AC_TVS_GRIDLINES            0x00000002
#define     AC_TVS_SELHIGHLIGHT         0x00000004
#define     AC_TVS_GRIDMARKER           0x00000008
#define     AC_TVS_GRIDROUNDSTYLE       0x00000010
#define     AC_TVS_GRIDSTATUSBAR        0x00000020
#define     AC_TVS_GRIDSCROLLRIGHT      0x00000040
#define     AC_TVS_CALCULATOR           0x00000080
#define     AC_TVS_NOGRIDINDENTATION    0x00000100
#define     AC_TVS_NOCATEGORIES         0x00000200
#define     AC_TVS_NOCONTEXTMENU        0x00000400
#define     AC_TVS_GRIDGRADIENTSTYLE    0x00000800
// default style is as follows:
#define     AC_TVS_GRID                 ((AC_TVS_SELHIGHLIGHT) | (AC_TVS_GRIDSTATUSBAR) | (AC_TVS_GRIDLINES) | (AC_TVS_TOOLTIPS) | (AC_TVS_GRIDROUNDSTYLE))
#define DISPID_READONLYBACKCOLOR -2000
#define DISPID_READONLYFORECOLOR -2001
#define DISPID_RBUTTONDOWN -2030
#define DISPID_MBUTTONDOWN -2031
#define DISPID_CHECKED -2010
#define DISPID_BEGINPROPERTYUPDATE -2002
#define DISPID_ENDPROPERTYUPDATE -2003
typedef /* [uuid] */  DECLSPEC_UUID("A8562C86-3B34-4539-A880-1AB4089BCF10") 
enum AcPiColorIndex
    {
        ACPI_COLOR_TOP	= 0,
        ACPI_COLOR_CATEGORY	= 0x1,
        ACPI_COLOR_ITEM	= 0x2,
        ACPI_COLOR_BORDER	= 0x3,
        ACPI_COLOR_BORDER_ITEM	= 0x4,
        ACPI_COLOR_TEXT_TOP	= 0x5,
        ACPI_COLOR_TEXT_CATEGORY	= 0x6,
        ACPI_COLOR_TEXT_ITEM	= 0x7,
        ACPI_COLOR_FORE_HIGHLIGHT	= 0x8,
        ACPI_COLOR_BACK_HIGHLIGHT	= 0x9,
        ACPI_COLOR_3DSHADOW	= 0xa,
        ACPI_COLOR_BACK_CONTROL	= 0xb,
        ACPI_COLOR_SCROLLBACKGROUND	= 0xc,
        ACPI_COLOR_SCROLLTHUMB	= 0xd,
        ACPI_COLOR_SCROLLARROW	= 0xe,
        ACPI_COLOR_DOUBLECHEVRON1	= 0xf,
        ACPI_COLOR_DOUBLECHEVRON2	= 0x10,
        ACPI_COLOR_SINGLECHEVRON	= 0x11,
        ACPI_COLOR_TOP_GRADIENT1	= 0x12,
        ACPI_COLOR_TOP_GRADIENT2	= 0x13,
        ACPI_COLOR_TOP_CLOSED_OUTER_BORDER	= 0x14,
        ACPI_COLOR_TOP_OPEN_OUTER_BORDER	= 0x15,
        ACPI_COLOR_TOP_INNER_BORDER	= 0x16,
        ACPI_COLOR_SCROLL_BACKGROUNDBORDER	= 0x17,
        ACPI_COLOR_SCROLL_BACKGROUNDGRADIENT1	= 0x18,
        ACPI_COLOR_SCROLL_BACKGROUNDGRADIENT2	= 0x19,
        ACPI_COLOR_SCROLL_SLIDER_INACTIVE	= 0x1a,
        ACPI_COLOR_SCROLL_SLIDER_ACTIVE_OUTERBORDER	= 0x1b,
        ACPI_COLOR_SCROLL_SLIDER_ACTIVE_OUTERGRADIENT1	= 0x1c,
        ACPI_COLOR_SCROLL_SLIDER_ACTIVE_OUTERGRADIENT2	= 0x1d,
        ACPI_COLOR_SCROLL_SLIDER_ACTIVE_OUTERGRADIENT3	= 0x1e,
        ACPI_COLOR_SCROLL_SLIDER_ACTIVE_OUTERGRADIENT4	= 0x1f,
        ACPI_COLOR_SCROLL_SLIDER_ACTIVE_INNERGRADIENT1	= 0x20,
        ACPI_COLOR_SCROLL_SLIDER_ACTIVE_INNERGRADIENT2	= 0x21,
        ACPI_COLOR_TEXT_LABEL	= 0x22,
        ACPI_COLOR_CELL_HIGHLIGHT	= 0x23,
        ACPI_COLOR_DISABLED_TEXT_ITEM	= 0x24
    } 	AcPiColorIndex;

typedef /* [uuid] */  DECLSPEC_UUID("51917C11-4E70-4996-A62D-EF014BCCE14B") 
enum ValueType
    {
        VALUE_REGULAR	= 0,
        VALUE_VARIES	= ( VALUE_REGULAR + 1 ) ,
        VALUE_UNSPECIFIED	= ( VALUE_VARIES + 1 ) ,
        VALUE_EXPRESSION	= ( VALUE_UNSPECIFIED + 1 ) 
    } 	ValueType;

#define IPropertyIdentifier IAcPiPropertyIdentifier
#define IID_IPropertyIdentifier IID_IAcPiPropertyIdentifier
#define CLSID_PropertyIdentifier CLSID_AcPiPropertyIdentifier


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0000_v0_0_s_ifspec;

#ifndef __IAcPiPropertyIdentifier_INTERFACE_DEFINED__
#define __IAcPiPropertyIdentifier_INTERFACE_DEFINED__

/* interface IAcPiPropertyIdentifier */
/* [unique][helpstring][uuid][dual][object] */ 


EXTERN_C const IID IID_IAcPiPropertyIdentifier;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B2BB79F7-06BD-42FB-814F-EFD656C1698C")
    IAcPiPropertyIdentifier : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Type( 
            /* [retval][out] */ VARTYPE *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Type( 
            /* [in] */ VARTYPE newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ControlCLSID( 
            /* [retval][out] */ CLSID *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ControlCLSID( 
            /* [in] */ CLSID newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Value( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Value( 
            /* [in] */ VARIANT newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Categories( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Categories( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ValueAsString( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ValueAsString( 
            /* [in] */ BSTR newVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyIdentifierVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyIdentifier * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyIdentifier * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyIdentifier * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAcPiPropertyIdentifier * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAcPiPropertyIdentifier * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAcPiPropertyIdentifier * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAcPiPropertyIdentifier * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IAcPiPropertyIdentifier * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IAcPiPropertyIdentifier * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IAcPiPropertyIdentifier * This,
            /* [retval][out] */ VARTYPE *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            IAcPiPropertyIdentifier * This,
            /* [in] */ VARTYPE newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ControlCLSID )( 
            IAcPiPropertyIdentifier * This,
            /* [retval][out] */ CLSID *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ControlCLSID )( 
            IAcPiPropertyIdentifier * This,
            /* [in] */ CLSID newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            IAcPiPropertyIdentifier * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            IAcPiPropertyIdentifier * This,
            /* [in] */ VARIANT newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Categories )( 
            IAcPiPropertyIdentifier * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Categories )( 
            IAcPiPropertyIdentifier * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ValueAsString )( 
            IAcPiPropertyIdentifier * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ValueAsString )( 
            IAcPiPropertyIdentifier * This,
            /* [in] */ BSTR newVal);
        
        END_INTERFACE
    } IAcPiPropertyIdentifierVtbl;

    interface IAcPiPropertyIdentifier
    {
        CONST_VTBL struct IAcPiPropertyIdentifierVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyIdentifier_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyIdentifier_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyIdentifier_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyIdentifier_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IAcPiPropertyIdentifier_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IAcPiPropertyIdentifier_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IAcPiPropertyIdentifier_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IAcPiPropertyIdentifier_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IAcPiPropertyIdentifier_put_Name(This,newVal)	\
    ( (This)->lpVtbl -> put_Name(This,newVal) ) 

#define IAcPiPropertyIdentifier_get_Type(This,pVal)	\
    ( (This)->lpVtbl -> get_Type(This,pVal) ) 

#define IAcPiPropertyIdentifier_put_Type(This,newVal)	\
    ( (This)->lpVtbl -> put_Type(This,newVal) ) 

#define IAcPiPropertyIdentifier_get_ControlCLSID(This,pVal)	\
    ( (This)->lpVtbl -> get_ControlCLSID(This,pVal) ) 

#define IAcPiPropertyIdentifier_put_ControlCLSID(This,newVal)	\
    ( (This)->lpVtbl -> put_ControlCLSID(This,newVal) ) 

#define IAcPiPropertyIdentifier_get_Value(This,pVal)	\
    ( (This)->lpVtbl -> get_Value(This,pVal) ) 

#define IAcPiPropertyIdentifier_put_Value(This,newVal)	\
    ( (This)->lpVtbl -> put_Value(This,newVal) ) 

#define IAcPiPropertyIdentifier_get_Categories(This,pVal)	\
    ( (This)->lpVtbl -> get_Categories(This,pVal) ) 

#define IAcPiPropertyIdentifier_put_Categories(This,newVal)	\
    ( (This)->lpVtbl -> put_Categories(This,newVal) ) 

#define IAcPiPropertyIdentifier_get_ValueAsString(This,pVal)	\
    ( (This)->lpVtbl -> get_ValueAsString(This,pVal) ) 

#define IAcPiPropertyIdentifier_put_ValueAsString(This,newVal)	\
    ( (This)->lpVtbl -> put_ValueAsString(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyIdentifier_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0001 */
/* [local] */ 

#define IPropertyInspector IAcPiPropertyInspector
#define IID_IPropertyInspector IID_IAcPiPropertyInspector
#define CLSID_PropertyInspector CLSID_AcPiPropertyInspector


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0001_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0001_v0_0_s_ifspec;

#ifndef __IAcPiPropertyInspector_INTERFACE_DEFINED__
#define __IAcPiPropertyInspector_INTERFACE_DEFINED__

/* interface IAcPiPropertyInspector */
/* [unique][helpstring][uuid][dual][object] */ 


EXTERN_C const IID IID_IAcPiPropertyInspector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("22A2DE3F-1F1E-4F04-BD55-D0F1703281F9")
    IAcPiPropertyInspector : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Reset( 
            /* [in] */ VARIANT *pObjUnkArray,
            /* [in] */ VARIANT *pDynUnkArray,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Style( 
            /* [retval][out] */ DWORD *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Style( 
            /* [in] */ DWORD newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_KeepFocus( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetColor( 
            /* [in] */ AcPiColorIndex index,
            /* [in] */ OLE_COLOR color) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetColor( 
            /* [in] */ AcPiColorIndex __MIDL__IAcPiPropertyInspector0000,
            /* [retval][out] */ OLE_COLOR *pColor) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Font( 
            /* [retval][out] */ IUnknown **pIFont) = 0;
        
        virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_Font( 
            /* [in] */ IUnknown *pIFont) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_BoldFont( 
            /* [retval][out] */ IUnknown **pIFont) = 0;
        
        virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_BoldFont( 
            /* [in] */ IUnknown *pIFont) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetPersistence( 
            /* [in] */ IUnknown *pINode) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetPersistence( 
            /* [in] */ IUnknown *pINode) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_DelayRefresh( 
            /* [retval][out] */ VARIANT_BOOL *pbDelayRefresh) = 0;
        
        virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_DelayRefresh( 
            /* [in] */ VARIANT_BOOL bDelayRefresh) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyInspectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyInspector * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyInspector * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyInspector * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAcPiPropertyInspector * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAcPiPropertyInspector * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAcPiPropertyInspector * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAcPiPropertyInspector * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IAcPiPropertyInspector * This,
            /* [in] */ VARIANT *pObjUnkArray,
            /* [in] */ VARIANT *pDynUnkArray,
            /* [in] */ DWORD dwFlags);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Style )( 
            IAcPiPropertyInspector * This,
            /* [retval][out] */ DWORD *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Style )( 
            IAcPiPropertyInspector * This,
            /* [in] */ DWORD newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_KeepFocus )( 
            IAcPiPropertyInspector * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetColor )( 
            IAcPiPropertyInspector * This,
            /* [in] */ AcPiColorIndex index,
            /* [in] */ OLE_COLOR color);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetColor )( 
            IAcPiPropertyInspector * This,
            /* [in] */ AcPiColorIndex __MIDL__IAcPiPropertyInspector0000,
            /* [retval][out] */ OLE_COLOR *pColor);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Font )( 
            IAcPiPropertyInspector * This,
            /* [retval][out] */ IUnknown **pIFont);
        
        /* [helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Font )( 
            IAcPiPropertyInspector * This,
            /* [in] */ IUnknown *pIFont);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_BoldFont )( 
            IAcPiPropertyInspector * This,
            /* [retval][out] */ IUnknown **pIFont);
        
        /* [helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_BoldFont )( 
            IAcPiPropertyInspector * This,
            /* [in] */ IUnknown *pIFont);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetPersistence )( 
            IAcPiPropertyInspector * This,
            /* [in] */ IUnknown *pINode);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetPersistence )( 
            IAcPiPropertyInspector * This,
            /* [in] */ IUnknown *pINode);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_DelayRefresh )( 
            IAcPiPropertyInspector * This,
            /* [retval][out] */ VARIANT_BOOL *pbDelayRefresh);
        
        /* [helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_DelayRefresh )( 
            IAcPiPropertyInspector * This,
            /* [in] */ VARIANT_BOOL bDelayRefresh);
        
        END_INTERFACE
    } IAcPiPropertyInspectorVtbl;

    interface IAcPiPropertyInspector
    {
        CONST_VTBL struct IAcPiPropertyInspectorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyInspector_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyInspector_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyInspector_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyInspector_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IAcPiPropertyInspector_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IAcPiPropertyInspector_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IAcPiPropertyInspector_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IAcPiPropertyInspector_Reset(This,pObjUnkArray,pDynUnkArray,dwFlags)	\
    ( (This)->lpVtbl -> Reset(This,pObjUnkArray,pDynUnkArray,dwFlags) ) 

#define IAcPiPropertyInspector_get_Style(This,pVal)	\
    ( (This)->lpVtbl -> get_Style(This,pVal) ) 

#define IAcPiPropertyInspector_put_Style(This,newVal)	\
    ( (This)->lpVtbl -> put_Style(This,newVal) ) 

#define IAcPiPropertyInspector_get_KeepFocus(This,pVal)	\
    ( (This)->lpVtbl -> get_KeepFocus(This,pVal) ) 

#define IAcPiPropertyInspector_SetColor(This,index,color)	\
    ( (This)->lpVtbl -> SetColor(This,index,color) ) 

#define IAcPiPropertyInspector_GetColor(This,__MIDL__IAcPiPropertyInspector0000,pColor)	\
    ( (This)->lpVtbl -> GetColor(This,__MIDL__IAcPiPropertyInspector0000,pColor) ) 

#define IAcPiPropertyInspector_get_Font(This,pIFont)	\
    ( (This)->lpVtbl -> get_Font(This,pIFont) ) 

#define IAcPiPropertyInspector_put_Font(This,pIFont)	\
    ( (This)->lpVtbl -> put_Font(This,pIFont) ) 

#define IAcPiPropertyInspector_get_BoldFont(This,pIFont)	\
    ( (This)->lpVtbl -> get_BoldFont(This,pIFont) ) 

#define IAcPiPropertyInspector_put_BoldFont(This,pIFont)	\
    ( (This)->lpVtbl -> put_BoldFont(This,pIFont) ) 

#define IAcPiPropertyInspector_SetPersistence(This,pINode)	\
    ( (This)->lpVtbl -> SetPersistence(This,pINode) ) 

#define IAcPiPropertyInspector_GetPersistence(This,pINode)	\
    ( (This)->lpVtbl -> GetPersistence(This,pINode) ) 

#define IAcPiPropertyInspector_get_DelayRefresh(This,pbDelayRefresh)	\
    ( (This)->lpVtbl -> get_DelayRefresh(This,pbDelayRefresh) ) 

#define IAcPiPropertyInspector_put_DelayRefresh(This,bDelayRefresh)	\
    ( (This)->lpVtbl -> put_DelayRefresh(This,bDelayRefresh) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyInspector_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0002 */
/* [local] */ 

#define IPropertyInspector2 IAcPiPropertyInspector2
#define IID_IPropertyInspector2 IID_IAcPiPropertyInspector2


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0002_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0002_v0_0_s_ifspec;

#ifndef __IAcPiPropertyInspector2_INTERFACE_DEFINED__
#define __IAcPiPropertyInspector2_INTERFACE_DEFINED__

/* interface IAcPiPropertyInspector2 */
/* [unique][helpstring][uuid][dual][object] */ 


EXTERN_C const IID IID_IAcPiPropertyInspector2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("75C47098-AEEA-4355-A3B2-5E08789614EA")
    IAcPiPropertyInspector2 : public IAcPiPropertyInspector
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetHeight( 
            /* [in] */ VARIANT_BOOL bFull,
            /* [retval][out] */ int *height) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_EnableScrollbar( 
            /* [retval][out] */ VARIANT_BOOL *pbEnableScrollbar) = 0;
        
        virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_EnableScrollbar( 
            /* [in] */ VARIANT_BOOL bEnableScrollbar) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyInspector2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyInspector2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyInspector2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyInspector2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAcPiPropertyInspector2 * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAcPiPropertyInspector2 * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAcPiPropertyInspector2 * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAcPiPropertyInspector2 * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IAcPiPropertyInspector2 * This,
            /* [in] */ VARIANT *pObjUnkArray,
            /* [in] */ VARIANT *pDynUnkArray,
            /* [in] */ DWORD dwFlags);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Style )( 
            IAcPiPropertyInspector2 * This,
            /* [retval][out] */ DWORD *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Style )( 
            IAcPiPropertyInspector2 * This,
            /* [in] */ DWORD newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_KeepFocus )( 
            IAcPiPropertyInspector2 * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetColor )( 
            IAcPiPropertyInspector2 * This,
            /* [in] */ AcPiColorIndex index,
            /* [in] */ OLE_COLOR color);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetColor )( 
            IAcPiPropertyInspector2 * This,
            /* [in] */ AcPiColorIndex __MIDL__IAcPiPropertyInspector0000,
            /* [retval][out] */ OLE_COLOR *pColor);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Font )( 
            IAcPiPropertyInspector2 * This,
            /* [retval][out] */ IUnknown **pIFont);
        
        /* [helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Font )( 
            IAcPiPropertyInspector2 * This,
            /* [in] */ IUnknown *pIFont);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_BoldFont )( 
            IAcPiPropertyInspector2 * This,
            /* [retval][out] */ IUnknown **pIFont);
        
        /* [helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_BoldFont )( 
            IAcPiPropertyInspector2 * This,
            /* [in] */ IUnknown *pIFont);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetPersistence )( 
            IAcPiPropertyInspector2 * This,
            /* [in] */ IUnknown *pINode);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetPersistence )( 
            IAcPiPropertyInspector2 * This,
            /* [in] */ IUnknown *pINode);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_DelayRefresh )( 
            IAcPiPropertyInspector2 * This,
            /* [retval][out] */ VARIANT_BOOL *pbDelayRefresh);
        
        /* [helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_DelayRefresh )( 
            IAcPiPropertyInspector2 * This,
            /* [in] */ VARIANT_BOOL bDelayRefresh);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetHeight )( 
            IAcPiPropertyInspector2 * This,
            /* [in] */ VARIANT_BOOL bFull,
            /* [retval][out] */ int *height);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_EnableScrollbar )( 
            IAcPiPropertyInspector2 * This,
            /* [retval][out] */ VARIANT_BOOL *pbEnableScrollbar);
        
        /* [helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_EnableScrollbar )( 
            IAcPiPropertyInspector2 * This,
            /* [in] */ VARIANT_BOOL bEnableScrollbar);
        
        END_INTERFACE
    } IAcPiPropertyInspector2Vtbl;

    interface IAcPiPropertyInspector2
    {
        CONST_VTBL struct IAcPiPropertyInspector2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyInspector2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyInspector2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyInspector2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyInspector2_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IAcPiPropertyInspector2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IAcPiPropertyInspector2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IAcPiPropertyInspector2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IAcPiPropertyInspector2_Reset(This,pObjUnkArray,pDynUnkArray,dwFlags)	\
    ( (This)->lpVtbl -> Reset(This,pObjUnkArray,pDynUnkArray,dwFlags) ) 

#define IAcPiPropertyInspector2_get_Style(This,pVal)	\
    ( (This)->lpVtbl -> get_Style(This,pVal) ) 

#define IAcPiPropertyInspector2_put_Style(This,newVal)	\
    ( (This)->lpVtbl -> put_Style(This,newVal) ) 

#define IAcPiPropertyInspector2_get_KeepFocus(This,pVal)	\
    ( (This)->lpVtbl -> get_KeepFocus(This,pVal) ) 

#define IAcPiPropertyInspector2_SetColor(This,index,color)	\
    ( (This)->lpVtbl -> SetColor(This,index,color) ) 

#define IAcPiPropertyInspector2_GetColor(This,__MIDL__IAcPiPropertyInspector0000,pColor)	\
    ( (This)->lpVtbl -> GetColor(This,__MIDL__IAcPiPropertyInspector0000,pColor) ) 

#define IAcPiPropertyInspector2_get_Font(This,pIFont)	\
    ( (This)->lpVtbl -> get_Font(This,pIFont) ) 

#define IAcPiPropertyInspector2_put_Font(This,pIFont)	\
    ( (This)->lpVtbl -> put_Font(This,pIFont) ) 

#define IAcPiPropertyInspector2_get_BoldFont(This,pIFont)	\
    ( (This)->lpVtbl -> get_BoldFont(This,pIFont) ) 

#define IAcPiPropertyInspector2_put_BoldFont(This,pIFont)	\
    ( (This)->lpVtbl -> put_BoldFont(This,pIFont) ) 

#define IAcPiPropertyInspector2_SetPersistence(This,pINode)	\
    ( (This)->lpVtbl -> SetPersistence(This,pINode) ) 

#define IAcPiPropertyInspector2_GetPersistence(This,pINode)	\
    ( (This)->lpVtbl -> GetPersistence(This,pINode) ) 

#define IAcPiPropertyInspector2_get_DelayRefresh(This,pbDelayRefresh)	\
    ( (This)->lpVtbl -> get_DelayRefresh(This,pbDelayRefresh) ) 

#define IAcPiPropertyInspector2_put_DelayRefresh(This,bDelayRefresh)	\
    ( (This)->lpVtbl -> put_DelayRefresh(This,bDelayRefresh) ) 


#define IAcPiPropertyInspector2_GetHeight(This,bFull,height)	\
    ( (This)->lpVtbl -> GetHeight(This,bFull,height) ) 

#define IAcPiPropertyInspector2_get_EnableScrollbar(This,pbEnableScrollbar)	\
    ( (This)->lpVtbl -> get_EnableScrollbar(This,pbEnableScrollbar) ) 

#define IAcPiPropertyInspector2_put_EnableScrollbar(This,bEnableScrollbar)	\
    ( (This)->lpVtbl -> put_EnableScrollbar(This,bEnableScrollbar) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyInspector2_INTERFACE_DEFINED__ */


#ifndef __IAcPiPropertyInspectorTheme_INTERFACE_DEFINED__
#define __IAcPiPropertyInspectorTheme_INTERFACE_DEFINED__

/* interface IAcPiPropertyInspectorTheme */
/* [helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPiPropertyInspectorTheme;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0DB7BC6A-444E-4477-A8E3-CC1FF5053ACD")
    IAcPiPropertyInspectorTheme : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetTheme( 
            /* [in] */ BSTR bstrThemeName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetThemePtr( 
            /* [retval][out] */ LONG_PTR *pTheme) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyInspectorThemeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyInspectorTheme * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyInspectorTheme * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyInspectorTheme * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetTheme )( 
            IAcPiPropertyInspectorTheme * This,
            /* [in] */ BSTR bstrThemeName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetThemePtr )( 
            IAcPiPropertyInspectorTheme * This,
            /* [retval][out] */ LONG_PTR *pTheme);
        
        END_INTERFACE
    } IAcPiPropertyInspectorThemeVtbl;

    interface IAcPiPropertyInspectorTheme
    {
        CONST_VTBL struct IAcPiPropertyInspectorThemeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyInspectorTheme_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyInspectorTheme_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyInspectorTheme_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyInspectorTheme_SetTheme(This,bstrThemeName)	\
    ( (This)->lpVtbl -> SetTheme(This,bstrThemeName) ) 

#define IAcPiPropertyInspectorTheme_GetThemePtr(This,pTheme)	\
    ( (This)->lpVtbl -> GetThemePtr(This,pTheme) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyInspectorTheme_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0004 */
/* [local] */ 

#define IPropertyEditControl IAcPiPropertyEditControl
#define IID_IPropertyEditControl IID_IAcPiPropertyEditControl


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0004_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0004_v0_0_s_ifspec;

#ifndef __IAcPiPropertyEditControl_INTERFACE_DEFINED__
#define __IAcPiPropertyEditControl_INTERFACE_DEFINED__

/* interface IAcPiPropertyEditControl */
/* [unique][helpstring][uuid][dual][object] */ 


EXTERN_C const IID IID_IAcPiPropertyEditControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6371C569-72C0-43a0-A457-1E9D2EB9201E")
    IAcPiPropertyEditControl : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE InitEditControl( 
            /* [in] */ LPVARIANT pVarObjDispArray,
            /* [in] */ LPVARIANT pVarDynPropArray,
            /* [in] */ LPVARIANT pDispidArray,
            /* [in] */ VARIANT_BOOL bReadOnly) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Refresh( 
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_KeepFocus( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [id][helpstring][requestedit][bindable][propput] */ HRESULT STDMETHODCALLTYPE put_BackColor( 
            /* [in] */ OLE_COLOR clr) = 0;
        
        virtual /* [id][helpstring][requestedit][bindable][propget] */ HRESULT STDMETHODCALLTYPE get_BackColor( 
            /* [retval][out] */ OLE_COLOR *pclr) = 0;
        
        virtual /* [id][helpstring][requestedit][bindable][propput] */ HRESULT STDMETHODCALLTYPE put_ForeColor( 
            /* [in] */ OLE_COLOR clr) = 0;
        
        virtual /* [id][helpstring][requestedit][bindable][propget] */ HRESULT STDMETHODCALLTYPE get_ForeColor( 
            /* [retval][out] */ OLE_COLOR *pclr) = 0;
        
        virtual /* [id][helpstring][requestedit][bindable][propput] */ HRESULT STDMETHODCALLTYPE put_ReadOnlyBackColor( 
            /* [in] */ OLE_COLOR clr) = 0;
        
        virtual /* [id][helpstring][requestedit][bindable][propget] */ HRESULT STDMETHODCALLTYPE get_ReadOnlyBackColor( 
            /* [retval][out] */ OLE_COLOR *pclr) = 0;
        
        virtual /* [id][helpstring][requestedit][bindable][propput] */ HRESULT STDMETHODCALLTYPE put_ReadOnlyForeColor( 
            /* [in] */ OLE_COLOR clr) = 0;
        
        virtual /* [id][helpstring][requestedit][bindable][propget] */ HRESULT STDMETHODCALLTYPE get_ReadOnlyForeColor( 
            /* [retval][out] */ OLE_COLOR *pclr) = 0;
        
        virtual /* [id][helpstring][requestedit][bindable][propput] */ HRESULT STDMETHODCALLTYPE put_Font( 
            /* [in] */ IUnknown *pIFont) = 0;
        
        virtual /* [id][helpstring][requestedit][bindable][propget] */ HRESULT STDMETHODCALLTYPE get_Font( 
            /* [retval][out] */ IUnknown **pIFont) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyEditControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyEditControl * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyEditControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyEditControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAcPiPropertyEditControl * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAcPiPropertyEditControl * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAcPiPropertyEditControl * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAcPiPropertyEditControl * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *InitEditControl )( 
            IAcPiPropertyEditControl * This,
            /* [in] */ LPVARIANT pVarObjDispArray,
            /* [in] */ LPVARIANT pVarDynPropArray,
            /* [in] */ LPVARIANT pDispidArray,
            /* [in] */ VARIANT_BOOL bReadOnly);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IAcPiPropertyEditControl * This,
            /* [in] */ DWORD dwFlags);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_KeepFocus )( 
            IAcPiPropertyEditControl * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BackColor )( 
            IAcPiPropertyEditControl * This,
            /* [in] */ OLE_COLOR clr);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BackColor )( 
            IAcPiPropertyEditControl * This,
            /* [retval][out] */ OLE_COLOR *pclr);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ForeColor )( 
            IAcPiPropertyEditControl * This,
            /* [in] */ OLE_COLOR clr);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ForeColor )( 
            IAcPiPropertyEditControl * This,
            /* [retval][out] */ OLE_COLOR *pclr);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReadOnlyBackColor )( 
            IAcPiPropertyEditControl * This,
            /* [in] */ OLE_COLOR clr);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReadOnlyBackColor )( 
            IAcPiPropertyEditControl * This,
            /* [retval][out] */ OLE_COLOR *pclr);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReadOnlyForeColor )( 
            IAcPiPropertyEditControl * This,
            /* [in] */ OLE_COLOR clr);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReadOnlyForeColor )( 
            IAcPiPropertyEditControl * This,
            /* [retval][out] */ OLE_COLOR *pclr);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Font )( 
            IAcPiPropertyEditControl * This,
            /* [in] */ IUnknown *pIFont);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Font )( 
            IAcPiPropertyEditControl * This,
            /* [retval][out] */ IUnknown **pIFont);
        
        END_INTERFACE
    } IAcPiPropertyEditControlVtbl;

    interface IAcPiPropertyEditControl
    {
        CONST_VTBL struct IAcPiPropertyEditControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyEditControl_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyEditControl_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyEditControl_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyEditControl_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IAcPiPropertyEditControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IAcPiPropertyEditControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IAcPiPropertyEditControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IAcPiPropertyEditControl_InitEditControl(This,pVarObjDispArray,pVarDynPropArray,pDispidArray,bReadOnly)	\
    ( (This)->lpVtbl -> InitEditControl(This,pVarObjDispArray,pVarDynPropArray,pDispidArray,bReadOnly) ) 

#define IAcPiPropertyEditControl_Refresh(This,dwFlags)	\
    ( (This)->lpVtbl -> Refresh(This,dwFlags) ) 

#define IAcPiPropertyEditControl_get_KeepFocus(This,pVal)	\
    ( (This)->lpVtbl -> get_KeepFocus(This,pVal) ) 

#define IAcPiPropertyEditControl_put_BackColor(This,clr)	\
    ( (This)->lpVtbl -> put_BackColor(This,clr) ) 

#define IAcPiPropertyEditControl_get_BackColor(This,pclr)	\
    ( (This)->lpVtbl -> get_BackColor(This,pclr) ) 

#define IAcPiPropertyEditControl_put_ForeColor(This,clr)	\
    ( (This)->lpVtbl -> put_ForeColor(This,clr) ) 

#define IAcPiPropertyEditControl_get_ForeColor(This,pclr)	\
    ( (This)->lpVtbl -> get_ForeColor(This,pclr) ) 

#define IAcPiPropertyEditControl_put_ReadOnlyBackColor(This,clr)	\
    ( (This)->lpVtbl -> put_ReadOnlyBackColor(This,clr) ) 

#define IAcPiPropertyEditControl_get_ReadOnlyBackColor(This,pclr)	\
    ( (This)->lpVtbl -> get_ReadOnlyBackColor(This,pclr) ) 

#define IAcPiPropertyEditControl_put_ReadOnlyForeColor(This,clr)	\
    ( (This)->lpVtbl -> put_ReadOnlyForeColor(This,clr) ) 

#define IAcPiPropertyEditControl_get_ReadOnlyForeColor(This,pclr)	\
    ( (This)->lpVtbl -> get_ReadOnlyForeColor(This,pclr) ) 

#define IAcPiPropertyEditControl_put_Font(This,pIFont)	\
    ( (This)->lpVtbl -> put_Font(This,pIFont) ) 

#define IAcPiPropertyEditControl_get_Font(This,pIFont)	\
    ( (This)->lpVtbl -> get_Font(This,pIFont) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyEditControl_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0005 */
/* [local] */ 

#define IPropertyEditControl2 IAcPiPropertyEditControl2
#define IID_IPropertyEditControl2 IID_IAcPiPropertyEditControl2


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0005_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0005_v0_0_s_ifspec;

#ifndef __IAcPiPropertyEditControl2_INTERFACE_DEFINED__
#define __IAcPiPropertyEditControl2_INTERFACE_DEFINED__

/* interface IAcPiPropertyEditControl2 */
/* [unique][helpstring][uuid][dual][object] */ 


EXTERN_C const IID IID_IAcPiPropertyEditControl2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FB6BE6DF-75C6-4F7C-9434-9B49AF5D9868")
    IAcPiPropertyEditControl2 : public IAcPiPropertyEditControl
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PropertyIdentifier( 
            /* [retval][out] */ IAcPiPropertyIdentifier **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PropertyIdentifier( 
            /* [in] */ IAcPiPropertyIdentifier *newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetPropertyValueEx( 
            /* [in] */ VARIANT *var,
            /* [in] */ ValueType valType) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE InvokePropertyDialog( 
            /* [in] */ BSTR bstrText,
            /* [in] */ int mode) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyEditControl2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyEditControl2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyEditControl2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyEditControl2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAcPiPropertyEditControl2 * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAcPiPropertyEditControl2 * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAcPiPropertyEditControl2 * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAcPiPropertyEditControl2 * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *InitEditControl )( 
            IAcPiPropertyEditControl2 * This,
            /* [in] */ LPVARIANT pVarObjDispArray,
            /* [in] */ LPVARIANT pVarDynPropArray,
            /* [in] */ LPVARIANT pDispidArray,
            /* [in] */ VARIANT_BOOL bReadOnly);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IAcPiPropertyEditControl2 * This,
            /* [in] */ DWORD dwFlags);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_KeepFocus )( 
            IAcPiPropertyEditControl2 * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BackColor )( 
            IAcPiPropertyEditControl2 * This,
            /* [in] */ OLE_COLOR clr);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BackColor )( 
            IAcPiPropertyEditControl2 * This,
            /* [retval][out] */ OLE_COLOR *pclr);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ForeColor )( 
            IAcPiPropertyEditControl2 * This,
            /* [in] */ OLE_COLOR clr);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ForeColor )( 
            IAcPiPropertyEditControl2 * This,
            /* [retval][out] */ OLE_COLOR *pclr);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReadOnlyBackColor )( 
            IAcPiPropertyEditControl2 * This,
            /* [in] */ OLE_COLOR clr);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReadOnlyBackColor )( 
            IAcPiPropertyEditControl2 * This,
            /* [retval][out] */ OLE_COLOR *pclr);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReadOnlyForeColor )( 
            IAcPiPropertyEditControl2 * This,
            /* [in] */ OLE_COLOR clr);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReadOnlyForeColor )( 
            IAcPiPropertyEditControl2 * This,
            /* [retval][out] */ OLE_COLOR *pclr);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Font )( 
            IAcPiPropertyEditControl2 * This,
            /* [in] */ IUnknown *pIFont);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Font )( 
            IAcPiPropertyEditControl2 * This,
            /* [retval][out] */ IUnknown **pIFont);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PropertyIdentifier )( 
            IAcPiPropertyEditControl2 * This,
            /* [retval][out] */ IAcPiPropertyIdentifier **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PropertyIdentifier )( 
            IAcPiPropertyEditControl2 * This,
            /* [in] */ IAcPiPropertyIdentifier *newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetPropertyValueEx )( 
            IAcPiPropertyEditControl2 * This,
            /* [in] */ VARIANT *var,
            /* [in] */ ValueType valType);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *InvokePropertyDialog )( 
            IAcPiPropertyEditControl2 * This,
            /* [in] */ BSTR bstrText,
            /* [in] */ int mode);
        
        END_INTERFACE
    } IAcPiPropertyEditControl2Vtbl;

    interface IAcPiPropertyEditControl2
    {
        CONST_VTBL struct IAcPiPropertyEditControl2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyEditControl2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyEditControl2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyEditControl2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyEditControl2_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IAcPiPropertyEditControl2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IAcPiPropertyEditControl2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IAcPiPropertyEditControl2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IAcPiPropertyEditControl2_InitEditControl(This,pVarObjDispArray,pVarDynPropArray,pDispidArray,bReadOnly)	\
    ( (This)->lpVtbl -> InitEditControl(This,pVarObjDispArray,pVarDynPropArray,pDispidArray,bReadOnly) ) 

#define IAcPiPropertyEditControl2_Refresh(This,dwFlags)	\
    ( (This)->lpVtbl -> Refresh(This,dwFlags) ) 

#define IAcPiPropertyEditControl2_get_KeepFocus(This,pVal)	\
    ( (This)->lpVtbl -> get_KeepFocus(This,pVal) ) 

#define IAcPiPropertyEditControl2_put_BackColor(This,clr)	\
    ( (This)->lpVtbl -> put_BackColor(This,clr) ) 

#define IAcPiPropertyEditControl2_get_BackColor(This,pclr)	\
    ( (This)->lpVtbl -> get_BackColor(This,pclr) ) 

#define IAcPiPropertyEditControl2_put_ForeColor(This,clr)	\
    ( (This)->lpVtbl -> put_ForeColor(This,clr) ) 

#define IAcPiPropertyEditControl2_get_ForeColor(This,pclr)	\
    ( (This)->lpVtbl -> get_ForeColor(This,pclr) ) 

#define IAcPiPropertyEditControl2_put_ReadOnlyBackColor(This,clr)	\
    ( (This)->lpVtbl -> put_ReadOnlyBackColor(This,clr) ) 

#define IAcPiPropertyEditControl2_get_ReadOnlyBackColor(This,pclr)	\
    ( (This)->lpVtbl -> get_ReadOnlyBackColor(This,pclr) ) 

#define IAcPiPropertyEditControl2_put_ReadOnlyForeColor(This,clr)	\
    ( (This)->lpVtbl -> put_ReadOnlyForeColor(This,clr) ) 

#define IAcPiPropertyEditControl2_get_ReadOnlyForeColor(This,pclr)	\
    ( (This)->lpVtbl -> get_ReadOnlyForeColor(This,pclr) ) 

#define IAcPiPropertyEditControl2_put_Font(This,pIFont)	\
    ( (This)->lpVtbl -> put_Font(This,pIFont) ) 

#define IAcPiPropertyEditControl2_get_Font(This,pIFont)	\
    ( (This)->lpVtbl -> get_Font(This,pIFont) ) 


#define IAcPiPropertyEditControl2_get_PropertyIdentifier(This,pVal)	\
    ( (This)->lpVtbl -> get_PropertyIdentifier(This,pVal) ) 

#define IAcPiPropertyEditControl2_put_PropertyIdentifier(This,newVal)	\
    ( (This)->lpVtbl -> put_PropertyIdentifier(This,newVal) ) 

#define IAcPiPropertyEditControl2_SetPropertyValueEx(This,var,valType)	\
    ( (This)->lpVtbl -> SetPropertyValueEx(This,var,valType) ) 

#define IAcPiPropertyEditControl2_InvokePropertyDialog(This,bstrText,mode)	\
    ( (This)->lpVtbl -> InvokePropertyDialog(This,bstrText,mode) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyEditControl2_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0006 */
/* [local] */ 

#define IPropertyEditControl3 IAcPiPropertyEditControl3
#define IID_IPropertyEditControl3 IID_IAcPiPropertyEditControl3


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0006_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0006_v0_0_s_ifspec;

#ifndef __IAcPiPropertyEditControl3_INTERFACE_DEFINED__
#define __IAcPiPropertyEditControl3_INTERFACE_DEFINED__

/* interface IAcPiPropertyEditControl3 */
/* [unique][helpstring][uuid][dual][object] */ 


EXTERN_C const IID IID_IAcPiPropertyEditControl3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7728A03B-B356-4658-ABF5-D165D527CB9D")
    IAcPiPropertyEditControl3 : public IAcPiPropertyEditControl2
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetTheme( 
            /* [in] */ IAcPiPropertyInspectorTheme *pVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyEditControl3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyEditControl3 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyEditControl3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyEditControl3 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAcPiPropertyEditControl3 * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAcPiPropertyEditControl3 * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAcPiPropertyEditControl3 * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAcPiPropertyEditControl3 * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *InitEditControl )( 
            IAcPiPropertyEditControl3 * This,
            /* [in] */ LPVARIANT pVarObjDispArray,
            /* [in] */ LPVARIANT pVarDynPropArray,
            /* [in] */ LPVARIANT pDispidArray,
            /* [in] */ VARIANT_BOOL bReadOnly);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IAcPiPropertyEditControl3 * This,
            /* [in] */ DWORD dwFlags);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_KeepFocus )( 
            IAcPiPropertyEditControl3 * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BackColor )( 
            IAcPiPropertyEditControl3 * This,
            /* [in] */ OLE_COLOR clr);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BackColor )( 
            IAcPiPropertyEditControl3 * This,
            /* [retval][out] */ OLE_COLOR *pclr);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ForeColor )( 
            IAcPiPropertyEditControl3 * This,
            /* [in] */ OLE_COLOR clr);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ForeColor )( 
            IAcPiPropertyEditControl3 * This,
            /* [retval][out] */ OLE_COLOR *pclr);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReadOnlyBackColor )( 
            IAcPiPropertyEditControl3 * This,
            /* [in] */ OLE_COLOR clr);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReadOnlyBackColor )( 
            IAcPiPropertyEditControl3 * This,
            /* [retval][out] */ OLE_COLOR *pclr);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReadOnlyForeColor )( 
            IAcPiPropertyEditControl3 * This,
            /* [in] */ OLE_COLOR clr);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReadOnlyForeColor )( 
            IAcPiPropertyEditControl3 * This,
            /* [retval][out] */ OLE_COLOR *pclr);
        
        /* [id][helpstring][requestedit][bindable][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Font )( 
            IAcPiPropertyEditControl3 * This,
            /* [in] */ IUnknown *pIFont);
        
        /* [id][helpstring][requestedit][bindable][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Font )( 
            IAcPiPropertyEditControl3 * This,
            /* [retval][out] */ IUnknown **pIFont);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PropertyIdentifier )( 
            IAcPiPropertyEditControl3 * This,
            /* [retval][out] */ IAcPiPropertyIdentifier **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PropertyIdentifier )( 
            IAcPiPropertyEditControl3 * This,
            /* [in] */ IAcPiPropertyIdentifier *newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetPropertyValueEx )( 
            IAcPiPropertyEditControl3 * This,
            /* [in] */ VARIANT *var,
            /* [in] */ ValueType valType);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *InvokePropertyDialog )( 
            IAcPiPropertyEditControl3 * This,
            /* [in] */ BSTR bstrText,
            /* [in] */ int mode);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetTheme )( 
            IAcPiPropertyEditControl3 * This,
            /* [in] */ IAcPiPropertyInspectorTheme *pVal);
        
        END_INTERFACE
    } IAcPiPropertyEditControl3Vtbl;

    interface IAcPiPropertyEditControl3
    {
        CONST_VTBL struct IAcPiPropertyEditControl3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyEditControl3_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyEditControl3_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyEditControl3_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyEditControl3_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IAcPiPropertyEditControl3_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IAcPiPropertyEditControl3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IAcPiPropertyEditControl3_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IAcPiPropertyEditControl3_InitEditControl(This,pVarObjDispArray,pVarDynPropArray,pDispidArray,bReadOnly)	\
    ( (This)->lpVtbl -> InitEditControl(This,pVarObjDispArray,pVarDynPropArray,pDispidArray,bReadOnly) ) 

#define IAcPiPropertyEditControl3_Refresh(This,dwFlags)	\
    ( (This)->lpVtbl -> Refresh(This,dwFlags) ) 

#define IAcPiPropertyEditControl3_get_KeepFocus(This,pVal)	\
    ( (This)->lpVtbl -> get_KeepFocus(This,pVal) ) 

#define IAcPiPropertyEditControl3_put_BackColor(This,clr)	\
    ( (This)->lpVtbl -> put_BackColor(This,clr) ) 

#define IAcPiPropertyEditControl3_get_BackColor(This,pclr)	\
    ( (This)->lpVtbl -> get_BackColor(This,pclr) ) 

#define IAcPiPropertyEditControl3_put_ForeColor(This,clr)	\
    ( (This)->lpVtbl -> put_ForeColor(This,clr) ) 

#define IAcPiPropertyEditControl3_get_ForeColor(This,pclr)	\
    ( (This)->lpVtbl -> get_ForeColor(This,pclr) ) 

#define IAcPiPropertyEditControl3_put_ReadOnlyBackColor(This,clr)	\
    ( (This)->lpVtbl -> put_ReadOnlyBackColor(This,clr) ) 

#define IAcPiPropertyEditControl3_get_ReadOnlyBackColor(This,pclr)	\
    ( (This)->lpVtbl -> get_ReadOnlyBackColor(This,pclr) ) 

#define IAcPiPropertyEditControl3_put_ReadOnlyForeColor(This,clr)	\
    ( (This)->lpVtbl -> put_ReadOnlyForeColor(This,clr) ) 

#define IAcPiPropertyEditControl3_get_ReadOnlyForeColor(This,pclr)	\
    ( (This)->lpVtbl -> get_ReadOnlyForeColor(This,pclr) ) 

#define IAcPiPropertyEditControl3_put_Font(This,pIFont)	\
    ( (This)->lpVtbl -> put_Font(This,pIFont) ) 

#define IAcPiPropertyEditControl3_get_Font(This,pIFont)	\
    ( (This)->lpVtbl -> get_Font(This,pIFont) ) 


#define IAcPiPropertyEditControl3_get_PropertyIdentifier(This,pVal)	\
    ( (This)->lpVtbl -> get_PropertyIdentifier(This,pVal) ) 

#define IAcPiPropertyEditControl3_put_PropertyIdentifier(This,newVal)	\
    ( (This)->lpVtbl -> put_PropertyIdentifier(This,newVal) ) 

#define IAcPiPropertyEditControl3_SetPropertyValueEx(This,var,valType)	\
    ( (This)->lpVtbl -> SetPropertyValueEx(This,var,valType) ) 

#define IAcPiPropertyEditControl3_InvokePropertyDialog(This,bstrText,mode)	\
    ( (This)->lpVtbl -> InvokePropertyDialog(This,bstrText,mode) ) 


#define IAcPiPropertyEditControl3_SetTheme(This,pVal)	\
    ( (This)->lpVtbl -> SetTheme(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyEditControl3_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0007 */
/* [local] */ 

#define IPropEditUIEventsSink IAcPiPropertyEditEventsSink
#define IID_IPropEditUIEventsSink IID_IAcPiPropertyEditEventsSink


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0007_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0007_v0_0_s_ifspec;

#ifndef __IAcPiPropertyEditEventsSink_INTERFACE_DEFINED__
#define __IAcPiPropertyEditEventsSink_INTERFACE_DEFINED__

/* interface IAcPiPropertyEditEventsSink */
/* [object][helpstring][uuid] */ 


EXTERN_C const IID IID_IAcPiPropertyEditEventsSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5D6FA1C4-61B7-43C6-9050-DC24B92F4745")
    IAcPiPropertyEditEventsSink : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnClick( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnDblClick( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnKeyDown( 
            /* [in] */ LONG_PTR pnChar,
            /* [in] */ LONG_PTR keyData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnKeyUp( 
            /* [in] */ LONG_PTR pnChar,
            /* [in] */ LONG_PTR keyData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnRButtonDown( 
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnMButtonDown( 
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnBeginPropertyUpdate( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnEndPropertyUpdate( void) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyEditEventsSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyEditEventsSink * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyEditEventsSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyEditEventsSink * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnClick )( 
            IAcPiPropertyEditEventsSink * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnDblClick )( 
            IAcPiPropertyEditEventsSink * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnKeyDown )( 
            IAcPiPropertyEditEventsSink * This,
            /* [in] */ LONG_PTR pnChar,
            /* [in] */ LONG_PTR keyData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnKeyUp )( 
            IAcPiPropertyEditEventsSink * This,
            /* [in] */ LONG_PTR pnChar,
            /* [in] */ LONG_PTR keyData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnRButtonDown )( 
            IAcPiPropertyEditEventsSink * This,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnMButtonDown )( 
            IAcPiPropertyEditEventsSink * This,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnBeginPropertyUpdate )( 
            IAcPiPropertyEditEventsSink * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnEndPropertyUpdate )( 
            IAcPiPropertyEditEventsSink * This);
        
        END_INTERFACE
    } IAcPiPropertyEditEventsSinkVtbl;

    interface IAcPiPropertyEditEventsSink
    {
        CONST_VTBL struct IAcPiPropertyEditEventsSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyEditEventsSink_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyEditEventsSink_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyEditEventsSink_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyEditEventsSink_OnClick(This)	\
    ( (This)->lpVtbl -> OnClick(This) ) 

#define IAcPiPropertyEditEventsSink_OnDblClick(This)	\
    ( (This)->lpVtbl -> OnDblClick(This) ) 

#define IAcPiPropertyEditEventsSink_OnKeyDown(This,pnChar,keyData)	\
    ( (This)->lpVtbl -> OnKeyDown(This,pnChar,keyData) ) 

#define IAcPiPropertyEditEventsSink_OnKeyUp(This,pnChar,keyData)	\
    ( (This)->lpVtbl -> OnKeyUp(This,pnChar,keyData) ) 

#define IAcPiPropertyEditEventsSink_OnRButtonDown(This,wParam,lParam)	\
    ( (This)->lpVtbl -> OnRButtonDown(This,wParam,lParam) ) 

#define IAcPiPropertyEditEventsSink_OnMButtonDown(This,wParam,lParam)	\
    ( (This)->lpVtbl -> OnMButtonDown(This,wParam,lParam) ) 

#define IAcPiPropertyEditEventsSink_OnBeginPropertyUpdate(This)	\
    ( (This)->lpVtbl -> OnBeginPropertyUpdate(This) ) 

#define IAcPiPropertyEditEventsSink_OnEndPropertyUpdate(This)	\
    ( (This)->lpVtbl -> OnEndPropertyUpdate(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyEditEventsSink_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0008 */
/* [local] */ 

#define IPropEditUIEventsSink2 IAcPiPropertyEditEventsSink2
#define IID_IPropEditUIEventsSink2 IID_IAcPiPropertyEditEventsSink2


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0008_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0008_v0_0_s_ifspec;

#ifndef __IAcPiPropertyEditEventsSink2_INTERFACE_DEFINED__
#define __IAcPiPropertyEditEventsSink2_INTERFACE_DEFINED__

/* interface IAcPiPropertyEditEventsSink2 */
/* [object][helpstring][uuid] */ 


EXTERN_C const IID IID_IAcPiPropertyEditEventsSink2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("355AD1D1-59EC-49AF-BB77-064C54F0AB00")
    IAcPiPropertyEditEventsSink2 : public IAcPiPropertyEditEventsSink
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnBeginPropertyDialog( 
            int mode) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnEndPropertyDialog( 
            int mode) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyEditEventsSink2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyEditEventsSink2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyEditEventsSink2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyEditEventsSink2 * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnClick )( 
            IAcPiPropertyEditEventsSink2 * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnDblClick )( 
            IAcPiPropertyEditEventsSink2 * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnKeyDown )( 
            IAcPiPropertyEditEventsSink2 * This,
            /* [in] */ LONG_PTR pnChar,
            /* [in] */ LONG_PTR keyData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnKeyUp )( 
            IAcPiPropertyEditEventsSink2 * This,
            /* [in] */ LONG_PTR pnChar,
            /* [in] */ LONG_PTR keyData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnRButtonDown )( 
            IAcPiPropertyEditEventsSink2 * This,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnMButtonDown )( 
            IAcPiPropertyEditEventsSink2 * This,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnBeginPropertyUpdate )( 
            IAcPiPropertyEditEventsSink2 * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnEndPropertyUpdate )( 
            IAcPiPropertyEditEventsSink2 * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnBeginPropertyDialog )( 
            IAcPiPropertyEditEventsSink2 * This,
            int mode);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnEndPropertyDialog )( 
            IAcPiPropertyEditEventsSink2 * This,
            int mode);
        
        END_INTERFACE
    } IAcPiPropertyEditEventsSink2Vtbl;

    interface IAcPiPropertyEditEventsSink2
    {
        CONST_VTBL struct IAcPiPropertyEditEventsSink2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyEditEventsSink2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyEditEventsSink2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyEditEventsSink2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyEditEventsSink2_OnClick(This)	\
    ( (This)->lpVtbl -> OnClick(This) ) 

#define IAcPiPropertyEditEventsSink2_OnDblClick(This)	\
    ( (This)->lpVtbl -> OnDblClick(This) ) 

#define IAcPiPropertyEditEventsSink2_OnKeyDown(This,pnChar,keyData)	\
    ( (This)->lpVtbl -> OnKeyDown(This,pnChar,keyData) ) 

#define IAcPiPropertyEditEventsSink2_OnKeyUp(This,pnChar,keyData)	\
    ( (This)->lpVtbl -> OnKeyUp(This,pnChar,keyData) ) 

#define IAcPiPropertyEditEventsSink2_OnRButtonDown(This,wParam,lParam)	\
    ( (This)->lpVtbl -> OnRButtonDown(This,wParam,lParam) ) 

#define IAcPiPropertyEditEventsSink2_OnMButtonDown(This,wParam,lParam)	\
    ( (This)->lpVtbl -> OnMButtonDown(This,wParam,lParam) ) 

#define IAcPiPropertyEditEventsSink2_OnBeginPropertyUpdate(This)	\
    ( (This)->lpVtbl -> OnBeginPropertyUpdate(This) ) 

#define IAcPiPropertyEditEventsSink2_OnEndPropertyUpdate(This)	\
    ( (This)->lpVtbl -> OnEndPropertyUpdate(This) ) 


#define IAcPiPropertyEditEventsSink2_OnBeginPropertyDialog(This,mode)	\
    ( (This)->lpVtbl -> OnBeginPropertyDialog(This,mode) ) 

#define IAcPiPropertyEditEventsSink2_OnEndPropertyDialog(This,mode)	\
    ( (This)->lpVtbl -> OnEndPropertyDialog(This,mode) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyEditEventsSink2_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0009 */
/* [local] */ 

#define IPropertyUnspecified  IAcPiPropertyUnspecified
#define IID_IPropertyUnspecified IID_IAcPiPropertyUnspecified


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0009_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0009_v0_0_s_ifspec;

#ifndef __IAcPiPropertyUnspecified_INTERFACE_DEFINED__
#define __IAcPiPropertyUnspecified_INTERFACE_DEFINED__

/* interface IAcPiPropertyUnspecified */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPiPropertyUnspecified;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("63ECCF04-51B1-4fe1-972C-9EFD420ADEF4")
    IAcPiPropertyUnspecified : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsValueUnspecified( 
            /* [in] */ VARIANT varId,
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetUnspecified( 
            /* [in] */ VARIANT varId,
            /* [in] */ VARIANT_BOOL bIsUnspecified) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsUnspecifiedAllowed( 
            /* [in] */ VARIANT varId,
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetUnspecifiedString( 
            /* [in] */ VARIANT varId,
            /* [retval][out] */ BSTR *pVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyUnspecifiedVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyUnspecified * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyUnspecified * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyUnspecified * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsValueUnspecified )( 
            IAcPiPropertyUnspecified * This,
            /* [in] */ VARIANT varId,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetUnspecified )( 
            IAcPiPropertyUnspecified * This,
            /* [in] */ VARIANT varId,
            /* [in] */ VARIANT_BOOL bIsUnspecified);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsUnspecifiedAllowed )( 
            IAcPiPropertyUnspecified * This,
            /* [in] */ VARIANT varId,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetUnspecifiedString )( 
            IAcPiPropertyUnspecified * This,
            /* [in] */ VARIANT varId,
            /* [retval][out] */ BSTR *pVal);
        
        END_INTERFACE
    } IAcPiPropertyUnspecifiedVtbl;

    interface IAcPiPropertyUnspecified
    {
        CONST_VTBL struct IAcPiPropertyUnspecifiedVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyUnspecified_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyUnspecified_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyUnspecified_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyUnspecified_IsValueUnspecified(This,varId,pVal)	\
    ( (This)->lpVtbl -> IsValueUnspecified(This,varId,pVal) ) 

#define IAcPiPropertyUnspecified_SetUnspecified(This,varId,bIsUnspecified)	\
    ( (This)->lpVtbl -> SetUnspecified(This,varId,bIsUnspecified) ) 

#define IAcPiPropertyUnspecified_IsUnspecifiedAllowed(This,varId,pVal)	\
    ( (This)->lpVtbl -> IsUnspecifiedAllowed(This,varId,pVal) ) 

#define IAcPiPropertyUnspecified_GetUnspecifiedString(This,varId,pVal)	\
    ( (This)->lpVtbl -> GetUnspecifiedString(This,varId,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyUnspecified_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0010 */
/* [local] */ 

#define IPropertyExpression  IAcPiPropertyExpression
#define IID_IPropertyExpression IID_IAcPiPropertyExpression


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0010_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0010_v0_0_s_ifspec;

#ifndef __IAcPiPropertyExpression_INTERFACE_DEFINED__
#define __IAcPiPropertyExpression_INTERFACE_DEFINED__

/* interface IAcPiPropertyExpression */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPiPropertyExpression;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("214D0B04-B4BF-4D7C-A2B8-364E945EF311")
    IAcPiPropertyExpression : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ExpressionAllowed( 
            /* [in] */ VARIANT varId,
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Expression( 
            /* [in] */ VARIANT varId,
            /* [retval][out] */ BSTR *pExpression) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Expression( 
            /* [in] */ VARIANT varId,
            /* [in] */ BSTR strExpression) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyExpressionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyExpression * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyExpression * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyExpression * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ExpressionAllowed )( 
            IAcPiPropertyExpression * This,
            /* [in] */ VARIANT varId,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Expression )( 
            IAcPiPropertyExpression * This,
            /* [in] */ VARIANT varId,
            /* [retval][out] */ BSTR *pExpression);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Expression )( 
            IAcPiPropertyExpression * This,
            /* [in] */ VARIANT varId,
            /* [in] */ BSTR strExpression);
        
        END_INTERFACE
    } IAcPiPropertyExpressionVtbl;

    interface IAcPiPropertyExpression
    {
        CONST_VTBL struct IAcPiPropertyExpressionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyExpression_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyExpression_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyExpression_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyExpression_ExpressionAllowed(This,varId,pVal)	\
    ( (This)->lpVtbl -> ExpressionAllowed(This,varId,pVal) ) 

#define IAcPiPropertyExpression_get_Expression(This,varId,pExpression)	\
    ( (This)->lpVtbl -> get_Expression(This,varId,pExpression) ) 

#define IAcPiPropertyExpression_put_Expression(This,varId,strExpression)	\
    ( (This)->lpVtbl -> put_Expression(This,varId,strExpression) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyExpression_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0011 */
/* [local] */ 

#define IPerPropertyDisplay IAcPiPropertyDisplay
#define IID_IPerPropertyDisplay IID_IAcPiPropertyDisplay


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0011_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0011_v0_0_s_ifspec;

#ifndef __IAcPiPropertyDisplay_INTERFACE_DEFINED__
#define __IAcPiPropertyDisplay_INTERFACE_DEFINED__

/* interface IAcPiPropertyDisplay */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPiPropertyDisplay;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6a0d7861-8773-4d56-ac16-95aff3051ebd")
    IAcPiPropertyDisplay : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCustomPropertyCtrl( 
            /* [in] */ VARIANT Id,
            /* [in] */ LCID lcid,
            /* [out] */ BSTR *pProgId) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetPropertyIcon( 
            /* [in] */ VARIANT Id,
            /* [out] */ IUnknown **pIcon) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetPropTextColor( 
            /* [in] */ VARIANT Id,
            /* [out] */ OLE_COLOR *pTextColor) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsFullView( 
            /* [in] */ VARIANT Id,
            /* [out] */ VARIANT_BOOL *pbVisible,
            /* [out] */ DWORD *pIntegralHeight) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetPropertyWeight( 
            /* [in] */ VARIANT Id,
            /* [out] */ long *pPropertyWeight) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyDisplayVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyDisplay * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyDisplay * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyDisplay * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetCustomPropertyCtrl )( 
            IAcPiPropertyDisplay * This,
            /* [in] */ VARIANT Id,
            /* [in] */ LCID lcid,
            /* [out] */ BSTR *pProgId);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetPropertyIcon )( 
            IAcPiPropertyDisplay * This,
            /* [in] */ VARIANT Id,
            /* [out] */ IUnknown **pIcon);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetPropTextColor )( 
            IAcPiPropertyDisplay * This,
            /* [in] */ VARIANT Id,
            /* [out] */ OLE_COLOR *pTextColor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsFullView )( 
            IAcPiPropertyDisplay * This,
            /* [in] */ VARIANT Id,
            /* [out] */ VARIANT_BOOL *pbVisible,
            /* [out] */ DWORD *pIntegralHeight);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetPropertyWeight )( 
            IAcPiPropertyDisplay * This,
            /* [in] */ VARIANT Id,
            /* [out] */ long *pPropertyWeight);
        
        END_INTERFACE
    } IAcPiPropertyDisplayVtbl;

    interface IAcPiPropertyDisplay
    {
        CONST_VTBL struct IAcPiPropertyDisplayVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyDisplay_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyDisplay_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyDisplay_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyDisplay_GetCustomPropertyCtrl(This,Id,lcid,pProgId)	\
    ( (This)->lpVtbl -> GetCustomPropertyCtrl(This,Id,lcid,pProgId) ) 

#define IAcPiPropertyDisplay_GetPropertyIcon(This,Id,pIcon)	\
    ( (This)->lpVtbl -> GetPropertyIcon(This,Id,pIcon) ) 

#define IAcPiPropertyDisplay_GetPropTextColor(This,Id,pTextColor)	\
    ( (This)->lpVtbl -> GetPropTextColor(This,Id,pTextColor) ) 

#define IAcPiPropertyDisplay_IsFullView(This,Id,pbVisible,pIntegralHeight)	\
    ( (This)->lpVtbl -> IsFullView(This,Id,pbVisible,pIntegralHeight) ) 

#define IAcPiPropertyDisplay_GetPropertyWeight(This,Id,pPropertyWeight)	\
    ( (This)->lpVtbl -> GetPropertyWeight(This,Id,pPropertyWeight) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyDisplay_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0012 */
/* [local] */ 

#define IPerPropertyDisplay2 IAcPiPropertyDisplay2
#define IID_IPerPropertyDisplay2 IID_IAcPiPropertyDisplay2


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0012_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0012_v0_0_s_ifspec;

#ifndef __IAcPiPropertyDisplay2_INTERFACE_DEFINED__
#define __IAcPiPropertyDisplay2_INTERFACE_DEFINED__

/* interface IAcPiPropertyDisplay2 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPiPropertyDisplay2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8C64DA20-9571-489A-987B-390EDB20820D")
    IAcPiPropertyDisplay2 : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDarkPropertyIcon( 
            /* [in] */ VARIANT Id,
            /* [out] */ IUnknown **pIcon) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDarkPropTextColor( 
            /* [in] */ VARIANT Id,
            /* [out] */ OLE_COLOR *pTextColor) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyDisplay2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyDisplay2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyDisplay2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyDisplay2 * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDarkPropertyIcon )( 
            IAcPiPropertyDisplay2 * This,
            /* [in] */ VARIANT Id,
            /* [out] */ IUnknown **pIcon);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDarkPropTextColor )( 
            IAcPiPropertyDisplay2 * This,
            /* [in] */ VARIANT Id,
            /* [out] */ OLE_COLOR *pTextColor);
        
        END_INTERFACE
    } IAcPiPropertyDisplay2Vtbl;

    interface IAcPiPropertyDisplay2
    {
        CONST_VTBL struct IAcPiPropertyDisplay2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyDisplay2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyDisplay2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyDisplay2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyDisplay2_GetDarkPropertyIcon(This,Id,pIcon)	\
    ( (This)->lpVtbl -> GetDarkPropertyIcon(This,Id,pIcon) ) 

#define IAcPiPropertyDisplay2_GetDarkPropTextColor(This,Id,pTextColor)	\
    ( (This)->lpVtbl -> GetDarkPropTextColor(This,Id,pTextColor) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyDisplay2_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0013 */
/* [local] */ 

#define ICategorizeProperties2 IAcPiCategorizeProperties
#define IID_ICategorizeProperties2 IID_IAcPiCategorizeProperties


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0013_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0013_v0_0_s_ifspec;

#ifndef __IAcPiCategorizeProperties_INTERFACE_DEFINED__
#define __IAcPiCategorizeProperties_INTERFACE_DEFINED__

/* interface IAcPiCategorizeProperties */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPiCategorizeProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8590b992-d9a1-48d1-899a-358d77e1512a")
    IAcPiCategorizeProperties : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MapPropertyToCategory( 
            /* [in] */ DISPID dispID,
            /* [out] */ int *pCatID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCategoryName( 
            /* [in] */ int CatID,
            /* [in] */ LCID lCid,
            /* [out] */ BSTR *pCategoryName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCategoryDescription( 
            /* [in] */ int CatID,
            /* [in] */ LCID lCid,
            /* [out] */ BSTR *pCategoryDescription) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCategoryWeight( 
            /* [in] */ int CatID,
            /* [out] */ long *pCategoryWeight) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetParentCategory( 
            /* [in] */ int CatID,
            /* [out] */ int *pParentCatID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetUniqueID( 
            /* [out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCommandButtons( 
            /* [in] */ int CatID,
            /* [out] */ VARIANT *pCatCmdBtns) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiCategorizePropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiCategorizeProperties * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiCategorizeProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiCategorizeProperties * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *MapPropertyToCategory )( 
            IAcPiCategorizeProperties * This,
            /* [in] */ DISPID dispID,
            /* [out] */ int *pCatID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetCategoryName )( 
            IAcPiCategorizeProperties * This,
            /* [in] */ int CatID,
            /* [in] */ LCID lCid,
            /* [out] */ BSTR *pCategoryName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetCategoryDescription )( 
            IAcPiCategorizeProperties * This,
            /* [in] */ int CatID,
            /* [in] */ LCID lCid,
            /* [out] */ BSTR *pCategoryDescription);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetCategoryWeight )( 
            IAcPiCategorizeProperties * This,
            /* [in] */ int CatID,
            /* [out] */ long *pCategoryWeight);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetParentCategory )( 
            IAcPiCategorizeProperties * This,
            /* [in] */ int CatID,
            /* [out] */ int *pParentCatID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetUniqueID )( 
            IAcPiCategorizeProperties * This,
            /* [out] */ BSTR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetCommandButtons )( 
            IAcPiCategorizeProperties * This,
            /* [in] */ int CatID,
            /* [out] */ VARIANT *pCatCmdBtns);
        
        END_INTERFACE
    } IAcPiCategorizePropertiesVtbl;

    interface IAcPiCategorizeProperties
    {
        CONST_VTBL struct IAcPiCategorizePropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiCategorizeProperties_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiCategorizeProperties_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiCategorizeProperties_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiCategorizeProperties_MapPropertyToCategory(This,dispID,pCatID)	\
    ( (This)->lpVtbl -> MapPropertyToCategory(This,dispID,pCatID) ) 

#define IAcPiCategorizeProperties_GetCategoryName(This,CatID,lCid,pCategoryName)	\
    ( (This)->lpVtbl -> GetCategoryName(This,CatID,lCid,pCategoryName) ) 

#define IAcPiCategorizeProperties_GetCategoryDescription(This,CatID,lCid,pCategoryDescription)	\
    ( (This)->lpVtbl -> GetCategoryDescription(This,CatID,lCid,pCategoryDescription) ) 

#define IAcPiCategorizeProperties_GetCategoryWeight(This,CatID,pCategoryWeight)	\
    ( (This)->lpVtbl -> GetCategoryWeight(This,CatID,pCategoryWeight) ) 

#define IAcPiCategorizeProperties_GetParentCategory(This,CatID,pParentCatID)	\
    ( (This)->lpVtbl -> GetParentCategory(This,CatID,pParentCatID) ) 

#define IAcPiCategorizeProperties_GetUniqueID(This,pVal)	\
    ( (This)->lpVtbl -> GetUniqueID(This,pVal) ) 

#define IAcPiCategorizeProperties_GetCommandButtons(This,CatID,pCatCmdBtns)	\
    ( (This)->lpVtbl -> GetCommandButtons(This,CatID,pCatCmdBtns) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiCategorizeProperties_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0014 */
/* [local] */ 

#define IPropCommandButtons IAcPiPropCommandButtons
#define IID_IPropCommandButtons IID_IAcPiPropCommandButtons


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0014_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0014_v0_0_s_ifspec;

#ifndef __IAcPiPropCommandButtons_INTERFACE_DEFINED__
#define __IAcPiPropCommandButtons_INTERFACE_DEFINED__

/* interface IAcPiPropCommandButtons */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPiPropCommandButtons;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A6CD83FA-32A5-4c1e-9F2E-487A612F4A77")
    IAcPiPropCommandButtons : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetButtons( 
            /* [retval][out] */ VARIANT *pButtons) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropCommandButtonsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropCommandButtons * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropCommandButtons * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropCommandButtons * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetButtons )( 
            IAcPiPropCommandButtons * This,
            /* [retval][out] */ VARIANT *pButtons);
        
        END_INTERFACE
    } IAcPiPropCommandButtonsVtbl;

    interface IAcPiPropCommandButtons
    {
        CONST_VTBL struct IAcPiPropCommandButtonsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropCommandButtons_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropCommandButtons_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropCommandButtons_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropCommandButtons_GetButtons(This,pButtons)	\
    ( (This)->lpVtbl -> GetButtons(This,pButtons) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropCommandButtons_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0015 */
/* [local] */ 

#define ICommandButton IAcPiCommandButton
#define IID_ICommandButton IID_IAcPiCommandButton


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0015_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0015_v0_0_s_ifspec;

#ifndef __IAcPiCommandButton_INTERFACE_DEFINED__
#define __IAcPiCommandButton_INTERFACE_DEFINED__

/* interface IAcPiCommandButton */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPiCommandButton;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A5EAB6FD-EB1A-43d5-BCE0-2DA8C40F21BA")
    IAcPiCommandButton : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetName( 
            /* [in] */ LCID lcid,
            /* [retval][out] */ BSTR *name) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_EnabledPicture( 
            /* [retval][out] */ IUnknown **pPicture) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_DisabledPicture( 
            /* [retval][out] */ IUnknown **pPicture) = 0;
        
        virtual /* [helpstring][bindable][propget][id] */ HRESULT STDMETHODCALLTYPE get_Enabled( 
            /* [retval][out] */ VARIANT_BOOL *bEnabled) = 0;
        
        virtual /* [helpstring][bindable][propput][id] */ HRESULT STDMETHODCALLTYPE put_Enabled( 
            /* [in] */ VARIANT_BOOL bEnabled) = 0;
        
        virtual /* [helpstring][bindable][propget][id] */ HRESULT STDMETHODCALLTYPE get_Checked( 
            /* [retval][out] */ VARIANT_BOOL *bChecked) = 0;
        
        virtual /* [helpstring][bindable][propput][id] */ HRESULT STDMETHODCALLTYPE put_Checked( 
            /* [in] */ VARIANT_BOOL bChecked) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_ButtonStyle( 
            /* [retval][out] */ long *style) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Execute( 
            /* [in] */ VARIANT Objects) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiCommandButtonVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiCommandButton * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiCommandButton * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiCommandButton * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IAcPiCommandButton * This,
            /* [in] */ LCID lcid,
            /* [retval][out] */ BSTR *name);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_EnabledPicture )( 
            IAcPiCommandButton * This,
            /* [retval][out] */ IUnknown **pPicture);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_DisabledPicture )( 
            IAcPiCommandButton * This,
            /* [retval][out] */ IUnknown **pPicture);
        
        /* [helpstring][bindable][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Enabled )( 
            IAcPiCommandButton * This,
            /* [retval][out] */ VARIANT_BOOL *bEnabled);
        
        /* [helpstring][bindable][propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Enabled )( 
            IAcPiCommandButton * This,
            /* [in] */ VARIANT_BOOL bEnabled);
        
        /* [helpstring][bindable][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Checked )( 
            IAcPiCommandButton * This,
            /* [retval][out] */ VARIANT_BOOL *bChecked);
        
        /* [helpstring][bindable][propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Checked )( 
            IAcPiCommandButton * This,
            /* [in] */ VARIANT_BOOL bChecked);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_ButtonStyle )( 
            IAcPiCommandButton * This,
            /* [retval][out] */ long *style);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Execute )( 
            IAcPiCommandButton * This,
            /* [in] */ VARIANT Objects);
        
        END_INTERFACE
    } IAcPiCommandButtonVtbl;

    interface IAcPiCommandButton
    {
        CONST_VTBL struct IAcPiCommandButtonVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiCommandButton_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiCommandButton_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiCommandButton_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiCommandButton_GetName(This,lcid,name)	\
    ( (This)->lpVtbl -> GetName(This,lcid,name) ) 

#define IAcPiCommandButton_get_EnabledPicture(This,pPicture)	\
    ( (This)->lpVtbl -> get_EnabledPicture(This,pPicture) ) 

#define IAcPiCommandButton_get_DisabledPicture(This,pPicture)	\
    ( (This)->lpVtbl -> get_DisabledPicture(This,pPicture) ) 

#define IAcPiCommandButton_get_Enabled(This,bEnabled)	\
    ( (This)->lpVtbl -> get_Enabled(This,bEnabled) ) 

#define IAcPiCommandButton_put_Enabled(This,bEnabled)	\
    ( (This)->lpVtbl -> put_Enabled(This,bEnabled) ) 

#define IAcPiCommandButton_get_Checked(This,bChecked)	\
    ( (This)->lpVtbl -> get_Checked(This,bChecked) ) 

#define IAcPiCommandButton_put_Checked(This,bChecked)	\
    ( (This)->lpVtbl -> put_Checked(This,bChecked) ) 

#define IAcPiCommandButton_get_ButtonStyle(This,style)	\
    ( (This)->lpVtbl -> get_ButtonStyle(This,style) ) 

#define IAcPiCommandButton_Execute(This,Objects)	\
    ( (This)->lpVtbl -> Execute(This,Objects) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiCommandButton_INTERFACE_DEFINED__ */


#ifndef __IAcPiCommandButton2_INTERFACE_DEFINED__
#define __IAcPiCommandButton2_INTERFACE_DEFINED__

/* interface IAcPiCommandButton2 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPiCommandButton2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AF4178BF-3E04-477e-80B3-33FC2DAE494E")
    IAcPiCommandButton2 : public IUnknown
    {
    public:
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_CheckedPicture( 
            /* [retval][out] */ IUnknown **pPicture) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_UncheckedPicture( 
            /* [retval][out] */ IUnknown **pPicture) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiCommandButton2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiCommandButton2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiCommandButton2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiCommandButton2 * This);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_CheckedPicture )( 
            IAcPiCommandButton2 * This,
            /* [retval][out] */ IUnknown **pPicture);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_UncheckedPicture )( 
            IAcPiCommandButton2 * This,
            /* [retval][out] */ IUnknown **pPicture);
        
        END_INTERFACE
    } IAcPiCommandButton2Vtbl;

    interface IAcPiCommandButton2
    {
        CONST_VTBL struct IAcPiCommandButton2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiCommandButton2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiCommandButton2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiCommandButton2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiCommandButton2_get_CheckedPicture(This,pPicture)	\
    ( (This)->lpVtbl -> get_CheckedPicture(This,pPicture) ) 

#define IAcPiCommandButton2_get_UncheckedPicture(This,pPicture)	\
    ( (This)->lpVtbl -> get_UncheckedPicture(This,pPicture) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiCommandButton2_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0017 */
/* [local] */ 

#define ICommandButton3 IAcPiCommandButton3
#define IID_ICommandButton3 IID_IAcPiCommandButton3


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0017_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0017_v0_0_s_ifspec;

#ifndef __IAcPiCommandButton3_INTERFACE_DEFINED__
#define __IAcPiCommandButton3_INTERFACE_DEFINED__

/* interface IAcPiCommandButton3 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPiCommandButton3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C969FAA8-0072-4CB6-9768-F9467F633060")
    IAcPiCommandButton3 : public IUnknown
    {
    public:
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_DarkEnabledPicture( 
            /* [retval][out] */ IUnknown **pPicture) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_DarkDisabledPicture( 
            /* [retval][out] */ IUnknown **pPicture) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_DarkCheckedPicture( 
            /* [retval][out] */ IUnknown **pPicture) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_DarkUncheckedPicture( 
            /* [retval][out] */ IUnknown **pPicture) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiCommandButton3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiCommandButton3 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiCommandButton3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiCommandButton3 * This);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_DarkEnabledPicture )( 
            IAcPiCommandButton3 * This,
            /* [retval][out] */ IUnknown **pPicture);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_DarkDisabledPicture )( 
            IAcPiCommandButton3 * This,
            /* [retval][out] */ IUnknown **pPicture);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_DarkCheckedPicture )( 
            IAcPiCommandButton3 * This,
            /* [retval][out] */ IUnknown **pPicture);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_DarkUncheckedPicture )( 
            IAcPiCommandButton3 * This,
            /* [retval][out] */ IUnknown **pPicture);
        
        END_INTERFACE
    } IAcPiCommandButton3Vtbl;

    interface IAcPiCommandButton3
    {
        CONST_VTBL struct IAcPiCommandButton3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiCommandButton3_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiCommandButton3_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiCommandButton3_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiCommandButton3_get_DarkEnabledPicture(This,pPicture)	\
    ( (This)->lpVtbl -> get_DarkEnabledPicture(This,pPicture) ) 

#define IAcPiCommandButton3_get_DarkDisabledPicture(This,pPicture)	\
    ( (This)->lpVtbl -> get_DarkDisabledPicture(This,pPicture) ) 

#define IAcPiCommandButton3_get_DarkCheckedPicture(This,pPicture)	\
    ( (This)->lpVtbl -> get_DarkCheckedPicture(This,pPicture) ) 

#define IAcPiCommandButton3_get_DarkUncheckedPicture(This,pPicture)	\
    ( (This)->lpVtbl -> get_DarkUncheckedPicture(This,pPicture) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiCommandButton3_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0018 */
/* [local] */ 

#define IPropertyInspectorSink IAcPiPropertyInspectorEventsSink
#define IID_IPropertyInspectorSink IID_IAcPiPropertyInspectorEventsSink


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0018_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0018_v0_0_s_ifspec;

#ifndef __IAcPiPropertyInspectorEventsSink_INTERFACE_DEFINED__
#define __IAcPiPropertyInspectorEventsSink_INTERFACE_DEFINED__

/* interface IAcPiPropertyInspectorEventsSink */
/* [helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPiPropertyInspectorEventsSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3EAF5E3F-5D7B-4873-B8F4-A6D9B2D1EB33")
    IAcPiPropertyInspectorEventsSink : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Rebuild( 
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RefreshControl( 
            /* [in] */ VARIANT var,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetPropertyControl( 
            /* [in] */ VARIANT var,
            /* [out] */ IUnknown **ppUnk) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyInspectorEventsSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyInspectorEventsSink * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyInspectorEventsSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyInspectorEventsSink * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Rebuild )( 
            IAcPiPropertyInspectorEventsSink * This,
            /* [in] */ DWORD dwFlags);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RefreshControl )( 
            IAcPiPropertyInspectorEventsSink * This,
            /* [in] */ VARIANT var,
            /* [in] */ DWORD dwFlags);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetPropertyControl )( 
            IAcPiPropertyInspectorEventsSink * This,
            /* [in] */ VARIANT var,
            /* [out] */ IUnknown **ppUnk);
        
        END_INTERFACE
    } IAcPiPropertyInspectorEventsSinkVtbl;

    interface IAcPiPropertyInspectorEventsSink
    {
        CONST_VTBL struct IAcPiPropertyInspectorEventsSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyInspectorEventsSink_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyInspectorEventsSink_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyInspectorEventsSink_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyInspectorEventsSink_Rebuild(This,dwFlags)	\
    ( (This)->lpVtbl -> Rebuild(This,dwFlags) ) 

#define IAcPiPropertyInspectorEventsSink_RefreshControl(This,var,dwFlags)	\
    ( (This)->lpVtbl -> RefreshControl(This,var,dwFlags) ) 

#define IAcPiPropertyInspectorEventsSink_GetPropertyControl(This,var,ppUnk)	\
    ( (This)->lpVtbl -> GetPropertyControl(This,var,ppUnk) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyInspectorEventsSink_INTERFACE_DEFINED__ */


#ifndef __IAcPiPropertyInspectorRefreshEventSink_INTERFACE_DEFINED__
#define __IAcPiPropertyInspectorRefreshEventSink_INTERFACE_DEFINED__

/* interface IAcPiPropertyInspectorRefreshEventSink */
/* [helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPiPropertyInspectorRefreshEventSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("61D3D5C0-29D0-47a1-8389-D804F095224A")
    IAcPiPropertyInspectorRefreshEventSink : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnRefresh( 
            /* [in] */ WPARAM code,
            /* [in] */ LPARAM dispId) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyInspectorRefreshEventSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyInspectorRefreshEventSink * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyInspectorRefreshEventSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyInspectorRefreshEventSink * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnRefresh )( 
            IAcPiPropertyInspectorRefreshEventSink * This,
            /* [in] */ WPARAM code,
            /* [in] */ LPARAM dispId);
        
        END_INTERFACE
    } IAcPiPropertyInspectorRefreshEventSinkVtbl;

    interface IAcPiPropertyInspectorRefreshEventSink
    {
        CONST_VTBL struct IAcPiPropertyInspectorRefreshEventSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyInspectorRefreshEventSink_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyInspectorRefreshEventSink_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyInspectorRefreshEventSink_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyInspectorRefreshEventSink_OnRefresh(This,code,dispId)	\
    ( (This)->lpVtbl -> OnRefresh(This,code,dispId) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyInspectorRefreshEventSink_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_acpi_0000_0020 */
/* [local] */ 

#define IPropertyInspectorInputEventSink IAcPiPropertyInspectorInputEventSink
#define IID_IPropertyInspectorInputEventSink IID_IAcPiPropertyInspectorInputEventSink


extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0020_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_acpi_0000_0020_v0_0_s_ifspec;

#ifndef __IAcPiPropertyInspectorInputEventSink_INTERFACE_DEFINED__
#define __IAcPiPropertyInspectorInputEventSink_INTERFACE_DEFINED__

/* interface IAcPiPropertyInspectorInputEventSink */
/* [helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPiPropertyInspectorInputEventSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B6E9DB08-8B45-4ad2-B33C-985BBE4F3756")
    IAcPiPropertyInspectorInputEventSink : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnKeyDown( 
            /* [in] */ long pnChar,
            /* [in] */ long keyData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnPopulateContextMenu( 
            /* [in] */ VARIANT *flagArray,
            /* [in] */ VARIANT *idArray,
            /* [in] */ VARIANT *szItemArray) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnContextMenuCommand( 
            /* [in] */ int cmdId) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiPropertyInspectorInputEventSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiPropertyInspectorInputEventSink * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiPropertyInspectorInputEventSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiPropertyInspectorInputEventSink * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnKeyDown )( 
            IAcPiPropertyInspectorInputEventSink * This,
            /* [in] */ long pnChar,
            /* [in] */ long keyData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnPopulateContextMenu )( 
            IAcPiPropertyInspectorInputEventSink * This,
            /* [in] */ VARIANT *flagArray,
            /* [in] */ VARIANT *idArray,
            /* [in] */ VARIANT *szItemArray);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OnContextMenuCommand )( 
            IAcPiPropertyInspectorInputEventSink * This,
            /* [in] */ int cmdId);
        
        END_INTERFACE
    } IAcPiPropertyInspectorInputEventSinkVtbl;

    interface IAcPiPropertyInspectorInputEventSink
    {
        CONST_VTBL struct IAcPiPropertyInspectorInputEventSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiPropertyInspectorInputEventSink_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiPropertyInspectorInputEventSink_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiPropertyInspectorInputEventSink_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiPropertyInspectorInputEventSink_OnKeyDown(This,pnChar,keyData)	\
    ( (This)->lpVtbl -> OnKeyDown(This,pnChar,keyData) ) 

#define IAcPiPropertyInspectorInputEventSink_OnPopulateContextMenu(This,flagArray,idArray,szItemArray)	\
    ( (This)->lpVtbl -> OnPopulateContextMenu(This,flagArray,idArray,szItemArray) ) 

#define IAcPiPropertyInspectorInputEventSink_OnContextMenuCommand(This,cmdId)	\
    ( (This)->lpVtbl -> OnContextMenuCommand(This,cmdId) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiPropertyInspectorInputEventSink_INTERFACE_DEFINED__ */


#ifndef __IAcPropertyFilter_INTERFACE_DEFINED__
#define __IAcPropertyFilter_INTERFACE_DEFINED__

/* interface IAcPropertyFilter */
/* [helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPropertyFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("96D606CB-1705-476d-83B4-56961F951A26")
    IAcPropertyFilter : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ApplyPropertyFilter( 
            /* [in] */ IUnknown *pEntUnk,
            /* [in] */ VARIANT catIDArray,
            /* [in] */ BSTR propName,
            /* [in] */ GUID *pGuid,
            /* [out] */ VARIANT_BOOL *bFilter) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPropertyFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPropertyFilter * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPropertyFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPropertyFilter * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ApplyPropertyFilter )( 
            IAcPropertyFilter * This,
            /* [in] */ IUnknown *pEntUnk,
            /* [in] */ VARIANT catIDArray,
            /* [in] */ BSTR propName,
            /* [in] */ GUID *pGuid,
            /* [out] */ VARIANT_BOOL *bFilter);
        
        END_INTERFACE
    } IAcPropertyFilterVtbl;

    interface IAcPropertyFilter
    {
        CONST_VTBL struct IAcPropertyFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPropertyFilter_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPropertyFilter_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPropertyFilter_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPropertyFilter_ApplyPropertyFilter(This,pEntUnk,catIDArray,propName,pGuid,bFilter)	\
    ( (This)->lpVtbl -> ApplyPropertyFilter(This,pEntUnk,catIDArray,propName,pGuid,bFilter) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPropertyFilter_INTERFACE_DEFINED__ */


#ifndef __IAcPiFiltering_INTERFACE_DEFINED__
#define __IAcPiFiltering_INTERFACE_DEFINED__

/* interface IAcPiFiltering */
/* [helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAcPiFiltering;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A3421DB2-9202-4325-9996-308B2CA32E07")
    IAcPiFiltering : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RegisterPropertyFilter( 
            /* [in] */ IAcPropertyFilter *pPropFilter) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE UnRegisterPropertyFilter( 
            /* [in] */ IAcPropertyFilter *pPropFilter) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IAcPiFilteringVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAcPiFiltering * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAcPiFiltering * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAcPiFiltering * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RegisterPropertyFilter )( 
            IAcPiFiltering * This,
            /* [in] */ IAcPropertyFilter *pPropFilter);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *UnRegisterPropertyFilter )( 
            IAcPiFiltering * This,
            /* [in] */ IAcPropertyFilter *pPropFilter);
        
        END_INTERFACE
    } IAcPiFilteringVtbl;

    interface IAcPiFiltering
    {
        CONST_VTBL struct IAcPiFilteringVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAcPiFiltering_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAcPiFiltering_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAcPiFiltering_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAcPiFiltering_RegisterPropertyFilter(This,pPropFilter)	\
    ( (This)->lpVtbl -> RegisterPropertyFilter(This,pPropFilter) ) 

#define IAcPiFiltering_UnRegisterPropertyFilter(This,pPropFilter)	\
    ( (This)->lpVtbl -> UnRegisterPropertyFilter(This,pPropFilter) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAcPiFiltering_INTERFACE_DEFINED__ */



#ifndef __ACPILib_LIBRARY_DEFINED__
#define __ACPILib_LIBRARY_DEFINED__

/* library ACPILib */
/* [custom][helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_ACPILib;

EXTERN_C const CLSID CLSID_AcPiPropertyInspector;

#ifdef __cplusplus

class DECLSPEC_UUID("31DBB451-9109-41EA-854C-B25905225AEA")
AcPiPropertyInspector;
#endif

EXTERN_C const CLSID CLSID_AcPiPropertyIdentifier;

#ifdef __cplusplus

class DECLSPEC_UUID("8B049801-6BC7-46E5-AA22-95AEA239BE54")
AcPiPropertyIdentifier;
#endif
#endif /* __ACPILib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

unsigned long             __RPC_USER  BSTR_UserSize64(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal64(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal64(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree64(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize64(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal64(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal64(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree64(     unsigned long *, VARIANT * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


