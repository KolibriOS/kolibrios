#ifndef INCLUDE_LIBXML_H
#define INCLUDE_LIBXML_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_DLL_H
#include "../lib/dll.h"
#endif
dword XML = #aXML_lib;
char aXML_lib[]               = "/sys/lib/xml.obj";

dword XML_lib_init            = #aXMLlib_init;

dword initializeParser        = #aInitializeParser;
dword releaseParser           = #aReleaseParser;
dword parse                   = #aParse;
dword initializeClassParser   = #aInitializeClassParser;
dword releaseCkassParser      = #aReleaseClassParser;
dword classFromElement        = #aClassFromElement;
dword classFromString         = #aClassFromString;

$DD 2 dup 0

char aXMLlib_init []          = "lib_init";
char aInitializeParser[]      = "ax_initializeParser";
char aReleaseParser[]         = "ax_releaseParser";
char aParse[]                 = "ax_parse";
char aInitializeClassParser[] = "ax_initializeClassParser";
char aReleaseClassParser[]    = "ax_releaseClassParser";
char aClassFromElement[]      = "ax_classFromElement";
char aClassFromString[]       = "ax_classFromString";

//-----------------------------------------------------------------------------
// Error Codes
//-----------------------------------------------------------------------------
#define RC_OK                           0 // everything is ok
#define RC_MEMORY                       1 // out of memory

#define RC_EMPTY_NAME                  10 // name empty or not defined
#define RC_ATTR_DEFINED                11 // attribute already defined
#define RC_ELEM_DEFINED                12 // element already defined
#define RC_SCHEMA_EMPTY                13 // schema does not contains a document
#define RC_DOCUMENT_DEFINED            14 // schema contains more than one document
#define RC_UNDEFINED_CLASS             15 // can't find collection in reference
#define RC_UNDEFINED_GROUP             16 // can't find a group in include
#define RC_INVALID_ID                  17 // id is not a valid number
#define RC_INVALID_IGNORE              18 // ignore is not 'yes' or 'no'

#define RC_INVALID_ENTITY_REFERENCE    20 // must be amp, quot, lt, gt, or apos
#define RC_UNEXPECTED_END              21 // found last char too early
#define RC_INVALID_CHAR                22 // wrong char
#define RC_OVERFLOW                    23 // number to big in char reference
#define RC_NO_START_TAG                24 // xml does not start with a tag
#define RC_TAG_MISMATCH                25 // invalid close tag
#define RC_INVALID_TAG                 26 // invalid root element
#define RC_INVALID_ATTRIBUTE           27 // unknown attribute
#define RC_INVALID_PI                  28 // invalid processing instruction (<?xml)
#define RC_INVALID_DOCTYPE             29 // duplicate doctype or after main element
#define RC_VERSION_EXPECTED            30 // version is missing in xml declaration

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------

struct AXElementClass
{
  dword offset;                 // (int)              Offset of the element in attribute list
  dword name;                   // (char*)            Name of the element (not zero terminated)
  dword nameLimit;              // (char*)            End of the name of the element
  dword size;                   // (unsigned int)     size in bytes of an element of this class
  dword id;                     // (unsigned int)     container, text or mixed
  dword type;                   // (unsigned int)     container, text or mixed
  dword propertyCount;          // (unsigned int)     number of attributes and text elements
  dword childCount;             // (unsigned int)     number of child classes
  dword attributes;             // (int*)             (internal) attribute map
  dword elements;               // (int*)             (internal) element map
  dword children;               // (AXElementClass*)  The list of child classes.
                                // The order is the one defined in the class
                                // definition file.
  dword reserved;               // (int)
  dword reserved2;              // (void*)
};

struct AXClassContext
{
  dword base;                   // (void*)
  dword limit;                  // (void*)
  dword chunks;                 // (void*)
  dword chunkSize;              // (int)
  dword errorCode;              // (int)
  dword line;                   // (int)
  dword column;                 // (int)
  dword classes;                // (AXElementClass**) all global classes
  dword rootClass;              // (AXElementClass*)  the root class
  dword rootElement;            // (AXElement*)
};

struct AXAttribute
{
  dword begin;                   // (const char*)      the value (not zero terminated)
                                 // This slot can also contain an element if
                                 // a <element> has been defined in schema;
                                 // use ax_getElement() to retrieve it.
  dword limit;                   // (const char*)      the end of the value
};

struct AXElement
{
  dword id;                       // (int)              the class of the element
  dword nextSibling;              // (AXElement*)       the next sibling element
  dword firstChild;               // (AXElement*)       the first child element
  dword lastChild;                // (AXElement*)       the last child element
  AXAttribute reserved;           // (AXAttribute)      do not use
  AXAttribute attributes;         // (AXAttribute[])    the array of attributes
};

struct AXParseContext
{
  dword base;                   // (void*)
  dword limit;                  // (void*)
  dword chunks;                 // (void*)
  dword chunkSize;              // (int)
  dword errorCode;              // (int)
  dword source;                 // (const char*)
  dword current;                // (const char*)
  dword line;                   // (int)
  dword column;                 // (int)
  dword root;                   // (AXElement*)
  AXAttribute version;          // (AXAttribute)
  AXAttribute encoding;         // (AXAttribute)
  dword strict;                 // (int)
  dword reserved1;              // (int)
  AXElement reserved2;          // (AXElement)
};

#endif