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
    std::variant<xmlDocPtr, xmlNodePtr> node;
    std::string query = "";
    xmlXPathObjectPtr results = NULL;
    xmlXPathContextPtr xpathCtx = NULL;
    error *err = NULL;      //  Error structure when error state exists

    XPathObj(std::variant<xmlDocPtr, xmlNodePtr> node, const xmlChar * str);
    XPathObj(std::variant<xmlDocPtr, xmlNodePtr> node, const xmlChar * str, xmlXPathContextPtr ctx);
    ~XPathObj();

    //  RESULTS
    bool Bool();
    double Float();
    int Int();
    std::string Str();
    std::vector<xNode> Nodes();

    void SetError();
    void SetValError(const char* msg, int type);
};
