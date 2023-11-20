/* 
    LibXML2 Class Library - For XML documents and nodes based on LibXML2
    Programmer: Danny Holstein
 */

#include <libxml/xmlmemory.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/parser.h>
#include <string>
#include <variant>
#include <vector>

struct error
    {int errno; std::string *msg, *data, *src;};   //  API message, data that precipitated error, and API in which error generated

class xNode {
  public:
    xmlNodePtr ptr = NULL; //  XML Node pointer
    error *err = NULL;      //  Error structure when error state exists
    xmlXPathContextPtr xpathCtx;    //  XPath context

// constructors
    xNode(xmlNodePtr n);
    xNode(xmlNodePtr n, xmlXPathContextPtr ctxt);
    xNode(xmlDocPtr doc);
    xNode(xmlDocPtr doc, xmlNsPtr ns, const xmlChar * name, const xmlChar * content);
    xNode(xmlNodePtr parent, xmlNsPtr ns, const xmlChar * name, const xmlChar * content);
//  destructor
    ~xNode();
};

class xDoc {
  public:
    xmlDocPtr ptr = NULL;   //  XML Document tree if the file was wellformed, NULL otherwise.
    error *err = NULL;      //  Error structure when error state exists
    bool PtrOwner = true;   //  Object owns the DOM memory and may free it
    xmlXPathContextPtr xpathCtx;    //  XPath context

// constructors
    xDoc(xmlDocPtr d);
    xDoc(const xmlChar* version);
    xDoc(const char * filename, const char * encoding, int options);
    xDoc(const char * buffer, int size, const char * URL, const char * encoding, int options);
//  destructor
    ~xDoc();

    xNode RootNode();

  private:
    void SetError();
};

class XPathObj
{
  public:
    std::variant<xmlDocPtr, xmlNodePtr> node; //  DOM doc or node pointer from LibXML2
    std::string query = "";                   //  XPath query
    xmlXPathObjectPtr results = NULL;         //  Pointer to XPath object
    xmlXPathContextPtr xpathCtx = NULL;       //  Pointer to XPath context, internal use
    error *err = NULL;                        //  Error structure when error state exists, non-NULL value indicates failure

    XPathObj(std::variant<xmlDocPtr, xmlNodePtr> node, const xmlChar * str);
    XPathObj(std::variant<xmlDocPtr, xmlNodePtr> node, const xmlChar * str, xmlXPathContextPtr ctx);
    ~XPathObj();

    //  RESULTS

      // Retreive XPATH boolean value.  Number values (such as count()) are converted to boolean
    bool Bool();
      // Retreive XPATH double value
    double Float();
      // Retreive XPATH integer value
    int Int();
      // Retreive XPATH string value
    std::string Str();
      // Retreive XPATH node set as c++ vector of xNode objects
    std::vector<xNode> Nodes();

      // This error is set when there is an issue with the Query, such as a syntax error
    void SetError();
      // This error is set when an incompatible data type is retrieved, such as a node set when expecting a floating point
    void SetValError(const char* msg, int type);
};
