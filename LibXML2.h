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
    xmlNodePtr ptr = NULL;  //  XML Node pointer
    error *err = NULL;      //  Error structure when error state exists
    xmlXPathContextPtr xpathCtx = NULL; //  XPath context

    xNode(xmlNodePtr n);    //  Object from library node pointer
    xNode(xmlNodePtr n, xmlXPathContextPtr ctxt); //  Object from library node pointe, add known context
    xNode(xmlDocPtr doc);   //  Node from XML
    xNode(xmlDocPtr doc, xmlNsPtr ns, const xmlChar * name, const xmlChar * content);
    xNode(xmlNodePtr parent, xmlNsPtr ns, const xmlChar * name, const xmlChar * content);
    xNode(const char* XML, const char *encoding = "UTF-8");
    ~xNode();

    std::string XML();                //  Rendered XML for this node only
    void AddChild(xNode child);       //  Add child node as last child of this node
    void AddPrevSibling(xNode node);  //  Add node ahead of this to parent
    void AddSibling(xNode node);      //  Add node after this node
  private:
    void SetError();
};

class xDoc {
  public:
    xmlDocPtr ptr = NULL;   //  XML Document tree if the file was wellformed, NULL otherwise.
    error *err = NULL;      //  Error structure when error state exists
    bool PtrOwner = true;   //  Object owns the DOM memory and may free it
    xmlXPathContextPtr xpathCtx;    //  XPath context

    xDoc(xmlDocPtr d);
    xDoc(const xmlChar* version = (const xmlChar*) "1.0");
    xDoc(const char * filename, const char * encoding, int options);
    xDoc(const char * buffer, int size, const char * URL, const char * encoding, int options);
    ~xDoc();

    xNode RootNode();
    std::string XML(const char * txt_encoding = "UTF-8");  //  Rendered XML document
    int XML(std::string filename, const char * txt_encoding = "UTF-8");  //  Rendered XML document to file

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
      // Retreive XPATH double value, convert from valid string if available
    double Float();
      // Retreive XPATH integer value, cast of Float()
    int Int();
      // Retreive XPATH string value
    std::string Str();
      // Retreive XPATH node set as std::vector<xNode>
    std::vector<xNode> Nodes();

      // This error is set when there is an issue with the Query, such as a syntax error
    void SetError();
      // This error is set when an incompatible data type is retrieved, such as a node set when expecting a floating point
    void SetValError(const char* msg, int type);
};
