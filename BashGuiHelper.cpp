/* 
    BashGuiHelper - Use FLTK to prompt user for files, list selection, numeric input, date, etcetera
    Programmer: Danny Holstein
 */

#include <string>
#include <iostream>
#include <map>
#include <stdio.h>
#include "LibXML2.h"
#include <Fl_Native_File_Chooser.H>
#include <Fl_Window.H>
#include <Fl_Choice.H>
#include <Fl_Tree.H>

#define ERROR() return 1
#define CANCELLED() return 2

//  HCHoice class
//  Extended from Fl_Choice to take tab-separated list from command line, print and exit(0) when item is selected
class HChoice:      Fl_Choice
{
public:
    HChoice(const char* list):   Fl_Choice(10, 10, 200, 25) {
        add("---Make Selection---"); value(0);
        char* token; token = strtok((char*) list, "\t");
        while( token != NULL ) {add(token); token = strtok(NULL, "\t");}
        callback(MySelect, this);
        }
    ~HChoice(){;}

static void MySelect(Fl_Widget*, void* a) {std::cout <<  ((Fl_Choice*) a)->text() << "\n"; exit(0);}
};

Fl_Tree_Prefs prefs;
//  HTreeItem class
//  Extended from Fl_Tree_Item to include XML tree node for XML operations
class HTreeItem:    public Fl_Tree_Item
{
public:
    xmlNodePtr      node;

    HTreeItem(xmlNodePtr n):  Fl_Tree_Item(prefs) {
        node = n;
        XPathObj obj = XPathObj(node, (xmlChar*) "string(text())");
        if (!obj.err) this->label(obj.Str().c_str());
    }
    ~HTreeItem(){;}
};

//  MyTree class
//  Extended from Fl_Tree to take XML tree from command line and give user ability select multiple branches or individual nodes
//  Methods will include saving out modified XML with "selected" attributes, or edited tree nodes
class MyTree:       public Fl_Tree
{
public:
    xDoc doc;
    
    MyTree(xDoc d):   Fl_Tree(10, 10, 380, 380) {
        doc = d;

    XPathObj n = XPathObj(doc.ptr, (xmlChar*) "/*/*");
    if (n.err)
    {   // turn into macro
        std::cerr << "ERROR: " << n.err->msg->c_str();
        if (n.err->src) std::cerr << "SRC: " << n.err->src->c_str() << "\n";
        if (n.err->data) std::cerr << "QUERY: " << n.err->data->c_str() << "\n";
        return;
    }
    showroot(1);
    auto NodeList = n.Nodes();
    AddItem(NodeList, std::string("head/"));

    }
    ~MyTree(){;}
    
    void AddItem(std::vector<xNode> nodes,  std::string path, Fl_Tree_Item* p = NULL) {
        Fl_Tree_Item* item;
        for (xNode node : nodes)  {
            XPathObj obj = XPathObj(node.ptr, (xmlChar*) "string(text())");
            if (!obj.err) {
                HTreeItem *i = new HTreeItem(node.ptr);
                item = this->add(path.c_str(), i);
                
                auto branch = XPathObj(node.ptr, (xmlChar*) "./*");
                if (branch.results->type == XPATH_NODESET)
                    {auto NS = branch.Nodes();
                    std::string newPath = (p ? std::string(path.c_str()) + "/" : "") + escape(obj.Str());
                    
                    if (NS.size())
                        AddItem(NS, newPath, i);}
                }
            }
    }

    private:std::string escape(const std::string& input) {
        std::string result;
        for (char c : input) {
            if (c == '/' || c == '\\') {
                result += '\\';
            }
            result += c;
        }
        return result;
    }
};

int main(int argc, char *argv[])
{
    int i=0;

    std::map<std::string, std::string> ArgList{};   //  cheap version of argparse
    for (size_t i = 1; i < argc; i++) {
        char *param = strtok(argv[i], "=")+2; char *value = strtok(NULL, "="); ArgList.insert({param, value});}

//  File Browser, select one file
    if (ArgList["type"].compare("filebrowser") == 0) {
        auto FileName = Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
        FileName.title(ArgList["title"].c_str());
        FileName.filter(ArgList["file_type"].c_str());
        FileName.directory(ArgList["directory"].c_str());
        switch ( FileName.show() ) {
            case -1: 
            case  1: CANCELLED();
            default: std::cout <<  FileName.filename() << "\n"; return 0;
        }}

//  Multi-File Browser, select one or more files
    if (ArgList["type"].compare("multifiles") == 0) {
        auto FileName = Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_MULTI_FILE);
        FileName.title(ArgList["title"].c_str());
        FileName.filter(ArgList["file_type"].c_str());
        FileName.directory(ArgList["directory"].c_str());
        switch ( FileName.show() ) {
            case -1: 
            case  1: CANCELLED();
            default: 
                for (size_t i = 0; i < FileName.count(); i++)
                    std::cout <<  FileName.filename(i) << "\n";
                return 0;
        }}

//  Directory Browser, select a directory
    if (ArgList["type"].compare("dirbrowser") == 0) {
        auto FileName = Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
        FileName.title(ArgList["title"].c_str());
        FileName.directory(ArgList["directory"].c_str());
        switch ( FileName.show() ) {
            case -1: 
            case  1: CANCELLED();
            default: std::cout <<  FileName.filename() << "\n"; return 0;
        }}

//  List Selector, one item from tab-separated list
    if (ArgList["type"].compare("selection") == 0) {
        Fl_Window window(250, 50, ArgList["title"].c_str());
        HChoice* choice = new HChoice(ArgList["items"].c_str());
        window.as_group()->resizable((Fl_Choice*) choice);
        window.show();
        Fl::run();
        CANCELLED();
    }

//  Tree Selector, select items individually or tree branches/nodes
    if (ArgList["type"].compare("tree") == 0) {
#undef Bool
        xDoc *doc;
        if (ArgList["xml"].length())
        {
            doc = new xDoc(ArgList["xml"].c_str(), "1.0", 0);
            if (doc->err)
            {   // turn into macro
                std::cerr << "ERROR: " << doc->err->msg->c_str();
                if (doc->err->src) std::cerr << "SRC: " << doc->err->src->c_str() << "\n";
                ERROR();
            }
            XPathObj cnt = XPathObj(doc->ptr, (xmlChar*) "count(/selection/*)");
            if (!cnt.Bool()) {std::cerr << "No tree objects\n"; ERROR();}
        }
            
        XPathObj n = XPathObj(doc->ptr, (xmlChar*) "/*/*");
        if (n.err)
        {   // turn into macro
            std::cerr << "ERROR: " << n.err->msg->c_str();
            if (n.err->src) std::cerr << "SRC: " << n.err->src->c_str() << "\n";
            if (n.err->data) std::cerr << "QUERY: " << n.err->data->c_str() << "\n";
            ERROR();
        }
#if 0
        xNode r = doc->RootNode();
        auto nn = xNode("<item>sub Item 1</item>");
        auto i = n.Nodes();
        i[2].AddChild(nn);
        auto omg = i[2].XML();
        omg = r.XML();
        int aw = doc->XML(std::string("new.xml"));
#endif
        
        Fl_Window window(400, 400, ArgList["title"].c_str());
        MyTree* tree = new MyTree(*doc); tree->selectmode(FL_TREE_SELECT_MULTI);
        window.add(tree);
        window.as_group()->resizable((Fl_Choice*) tree);
        window.show();
        Fl::run();
        CANCELLED();
        }

    else {std::cerr << "No GUI type chosen\n"; ERROR();}
}
