/* 
    LibXML2 Class Library - For XML documents and nodes based on LibXML2
    Programmer: Danny Holstein
 */

#include "LibXML2.h"
#include <string.h>
const char* types[] = {"XPATH_UNDEFINED", "XPATH_NODESET", "XPATH_BOOLEAN", "XPATH_NUMBER", "XPATH_STRING", "XPATH_POINT", "XPATH_RANGE", "XPATH_LOCATIONSET", "XPATH_USERS", "XPATH_XSLT_TREE"};

// xNode constructors
XPathObj::XPathObj(std::variant<xmlDocPtr, xmlNodePtr> n, const xmlChar * str) {
    query = std::string((char*) str); node = n;
    switch (node.index())
        {
        case 0:
            if (! xpathCtx)
               {xpathCtx = xmlXPathNewContext(std::get<xmlDocPtr>(node));
                if(xpathCtx == NULL) {SetError(); return;}}
            results = xmlXPathEval(str, xpathCtx);
            break;        
        case 1:
            if (! xpathCtx) 
               {xpathCtx = xmlXPathNewContext(std::get<xmlNodePtr>(node)->doc);
                if(xpathCtx == NULL) {SetError(); return;}}
            results = xmlXPathNodeEval(std::get<xmlNodePtr>(node), str, xpathCtx);
            break;        
        default:
            break;
        }
        if(results == NULL) SetError();
    }
XPathObj::XPathObj(std::variant<xmlDocPtr, xmlNodePtr> n, const xmlChar * str, xmlXPathContextPtr ctx) {
    xpathCtx = ctx;
    query = std::string((char*) str); node = n;
    switch (node.index())
        {
        case 0:
            results = xmlXPathEval(str, xpathCtx);
            break;        
        case 1:
            results = xmlXPathNodeEval(std::get<xmlNodePtr>(node), str, xpathCtx);
            break;        
        default:
            break;
        }
    }
XPathObj::~XPathObj() {}

// Retreive XPATH double value, convert from valid string if available
double XPathObj::Float() {
    double ans=0.0; const char *dest, *src;
    switch (results->type)
        {
        case XPATH_NUMBER:
            return (double) results->floatval;
        case XPATH_STRING:
            src = (std::string((char*) results->stringval)).c_str();
            dest = (std::string("ERR")).c_str();
            ans = strtod(src, (char**) &dest);
            if (dest != src) return ans;
            SetValError("FLoating conversion failed", XPATH_NUMBER); err->src = new std::string("Text: "); err->src->append((char*) results->stringval);
            return (double) 0.0;
        case XPATH_UNDEFINED:
            SetValError("Query yielded no results", XPATH_NUMBER);
            return (double) 0.0;
        default:
            SetValError("Wrong data type", XPATH_NUMBER);
            return (double) 0.0;
        }
    }

// Retreive XPATH integer value, cast of Float()
int XPathObj::Int() {return (int) Float();}

// Retreive XPATH string value
std::string XPathObj::Str() {
    switch (results->type)
        {
        case XPATH_STRING:
            return std::string((char*) results->stringval);
        case XPATH_UNDEFINED:
            SetValError("Query yielded no results", XPATH_STRING);
            return std::string();
        default:
            SetValError("Wrong data type", XPATH_STRING);
            return std::string();
        }
    }

// Retreive XPATH boolean value.  Number values (such as count()) are converted to boolean
bool XPathObj::Bool() {
    switch (results->type)
        {
        case XPATH_BOOLEAN:
            return (bool) results->boolval;
        case XPATH_NUMBER:
            return (bool) results->floatval;
        case XPATH_UNDEFINED:
            SetValError("Query yielded no results", XPATH_BOOLEAN);
            return false;
        default:
            SetValError("Wrong data type", XPATH_BOOLEAN);
            return false;
        }
    }

// Retreive XPATH node set as std::vector<xNode>
std::vector<xNode> XPathObj::Nodes() {
    int cnt = 0;
    if (results->nodesetval) cnt = results->nodesetval->nodeNr;
    std::vector<xNode> NS;
    if (!cnt) return NS;
    switch (results->type)
        {
        case XPATH_NODESET:
            for (size_t i = 0; i < cnt; i++) NS.push_back(xNode(results->nodesetval->nodeTab[i], xpathCtx));
            return NS;
        case XPATH_UNDEFINED:
            SetValError("Query yielded no results", XPATH_NODESET);
            return NS;
        default:
            SetValError("Wrong data type", XPATH_NODESET);
            return NS;
        }
    }
void XPathObj::SetError() {
    err = new error();
    xmlErrorPtr m = xmlGetLastError();
    err->msg = new std::string(m->message);
    if (m->file)
        {err->src = new std::string(m->file); err->src->append(": line "); err->src->append(std::to_string(m->line));}
    if (m->str1) err->data = new std::string(m->str1);
    xmlResetError(m);
}
void XPathObj::SetValError(const char* msg, int type) {
    err = new error();
    err->msg = new std::string(msg);
    if (type)
    {
        err->src = new std::string("Result type is: "); err->src->append(types[results->type]);
        err->src->append(", Expected:"); err->src->append(types[type]);
    }
    err->data = new std::string(query);
}

xNode::xNode(xmlNodePtr n) {ptr = n;}
xNode::xNode(xmlNodePtr n, xmlXPathContextPtr ctxt) {ptr = n; xpathCtx = ctxt;}
xNode::xNode(xmlDocPtr doc) {ptr = xmlNewDocFragment(doc);}
xNode::xNode(xmlDocPtr doc, xmlNsPtr ns, const xmlChar * name, const xmlChar * content)
    {ptr = xmlNewDocNode(doc, ns, name, content);}
xNode::xNode(xmlNodePtr parent, xmlNsPtr ns, const xmlChar * name, const xmlChar * content)
    {ptr = xmlNewChild(parent, ns, name, content);}
xNode::~xNode() {if (ptr && false) xmlFreeNode(ptr);}
xNode::xNode(const char* XML, const char *encoding)
{    xmlDocPtr d = xmlReadMemory(XML, strlen(XML), NULL, encoding, 0);
     xmlNodePtr n = xmlDocGetRootElement(d);
     ptr = xmlCopyNode(n, 1);}

//  Rendered XML for this node only
std::string xNode::XML()
{    xmlBufferPtr buf = xmlBufferCreate();
    int sz = xmlNodeDump(buf, ptr->doc, ptr, 0, 0);
    if (sz) return std::string((char*) buf->content);
    SetError();
    return std::string((char*) "");}
//  Add child node as last child of this node
void xNode::AddChild(xNode child)
{   xmlNodePtr n = xmlAddChild(ptr, child.ptr);
    if (n == child.ptr) return;
    else SetError();
}
//  Add node ahead of this to parent
void xNode::AddPrevSibling(xNode node)
{   xmlNodePtr n = xmlAddPrevSibling(ptr, node.ptr);
    if (n == node.ptr) return;
    else SetError();
}
//  Add node after this node
void xNode::AddSibling(xNode node)
{   xmlNodePtr n = xmlAddSibling(ptr, node.ptr);
    if (n == node.ptr) return;
    else SetError();
}
//  XPath expression for this node
std::string xNode::GetNodePath() {return std::string{(const char*) xmlGetNodePath(ptr)};}
void xNode::SetError() {
    xmlErrorPtr m = xmlGetLastError(); if (!m) return;
    err = new error;
    err->msg = new std::string(m->message);
    if (m->file)
        {err->src = new std::string(m->file); err->src->append(": line "); err->src->append(std::to_string(m->line));}
    if (m->str1) err->data = new std::string(m->str1);
    xmlResetError(m);
}

xDoc::xDoc(xmlDocPtr d) {ptr = d; PtrOwner = false;}
xDoc::xDoc(const xmlChar* version) {ptr = xmlNewDoc(version);}
xDoc::xDoc(const char * filename, const char * encoding, int options)
    {ptr = xmlReadFile(filename, encoding, options);
    if (ptr == NULL) {SetError();}}
xDoc::xDoc(const char * buffer, int size, const char * URL, const char *encoding = "UTF-8", int options = 0)
    {ptr = xmlReadMemory(buffer, size, URL, encoding, options);
    if (ptr == NULL) {SetError();}}
xDoc::~xDoc() {if (PtrOwner && ptr && false) {
    try {xmlFreeDoc(ptr);}
    catch(const std::exception& e) {;}
    ptr = NULL;}}
//  Rendered XML document, specify encoding
std::string xDoc::XML(const char * txt_encoding) {
    xmlChar *res; int sz;
    xmlDocDumpMemoryEnc(ptr, &res, &sz, txt_encoding);
    if (res) return std::string((char*) res);
    SetError(); return std::string((char*) "");}
//  Rendered XML document to file
int xDoc::XML(std::string filename, const char * txt_encoding) {
    int i = xmlSaveFileEnc(filename.c_str(), ptr, txt_encoding);
    if (i > 0) return i;
    SetError(); return i;}

xNode xDoc::RootNode() {
    return xNode((xmlNodePtr) xmlDocGetRootElement(ptr));}

void xDoc::SetError() {
    xmlErrorPtr m = xmlGetLastError(); if (!m) return;
    err = new error;
    err->msg = new std::string(m->message);
    if (m->file)
        {err->src = new std::string(m->file); err->src->append(": line "); err->src->append(std::to_string(m->line));}
    if (m->str1) err->data = new std::string(m->str1);
    xmlResetError(m);
}
