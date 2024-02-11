/* 
    BashGuiHelper - Use FLTK to prompt user for files, list selection, numeric input, date, etcetera
    Programmer: Danny Holstein
 */

#include <string>
#include <iostream>
#include <map>
#include "LibXML2.h"
#include <Fl.H>
#include <Fl_Native_File_Chooser.H>
#include <Fl_Window.H>
#include <Fl_Choice.H>
#include <Fl_Tree.H>
#include <Fl_Pixmap.H>
#include <Fl_Widget.H>
#include <Fl_Tooltip.H>
#include <fl_ask.H>

#define ERROR() return 1
#define CANCELLED() return 2

//  HCHoice class
//| Extended from Fl_Choice to take tab-separated list from command line, print and exit(0) when item is selected
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

Fl_Tree_Prefs p;

#include "res/not-selected.xpm"
#include "res/selected.xpm"
#include "res/tri-state.xpm"
Fl_Pixmap *SelectedIcon[3];

//  HTreeItem class
//| Extended from Fl_Tree_Item to include XML tree node for XML operations
class HTreeItem:    public Fl_Tree_Item
{
public:
    xmlNodePtr  node;
    std::string  tooltip;
    Fl_Tree_Prefs prefs = p;

    HTreeItem(xmlNodePtr n):  Fl_Tree_Item(p) {
        node = n;
        usericon(SelectedIcon[0]);  //  indices 0, 1, and 2; correspond to not-selected, selected, and tri-state; respectively
        XPathObj obj = XPathObj(node, (xmlChar*) "string(text())");
        if (!obj.err) this->label(obj.Str().c_str());
        obj = XPathObj(node, (xmlChar*) "string(@tip)");
        if (!obj.err) tooltip = obj.Str();
    }
    ~HTreeItem(){;}

//  SelectBranch
//| Item/Branch has been selected, toggle selection icon 
//| If branch selected, set children selection to parent value
    void SelectBranch(bool sel) {
        usericon(SelectedIcon[(int) sel]);
        for (size_t i = 0; i < children(); i++) ((HTreeItem*) child(i))->SelectBranch(sel);
    }

//  FixParent
//| Set parent selection to that of it's children, set to tri-state if mixed
    void FixParent() {
        HTreeItem* p = (HTreeItem*) parent();
        if (!p) return;
        auto icon = ((HTreeItem*) p->child(0))->usericon();
        for (size_t i = 1; i < p->children(); i++) {
            if (icon == SelectedIcon[2]) break;
            if (icon != ((HTreeItem*) p->child(i))->usericon()) {
                icon = SelectedIcon[2]; break;}
        };
        p->usericon(icon);
        p->FixParent();
    }
};

//  MyTree class
//| Extended from Fl_Tree to take XML tree from command line and give user ability select multiple branches or individual nodes
//| Methods will include saving out modified XML with "selected" attributes, or edited tree nodes
class MyTree:   public Fl_Tree
{
public:
    xDoc doc;
    int size = 0;   //  number of tree items
    std::vector<HTreeItem *> items = {};
    HTreeItem* ToolTipItem = nullptr;

    MyTree(xDoc d):   Fl_Tree(10, 10, 380, 380) {
        doc = d;
        SelectedIcon[0] = new Fl_Pixmap(not_selected_xpm);
        SelectedIcon[1] = new Fl_Pixmap(selected_xpm);
        SelectedIcon[2] = new Fl_Pixmap(tri_state_xpm);
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
    
    // AddItem: Add Fl_Tree_Item.
    //|     std::vector<xNode> nodes:   XML node list with items for this specific level
    //|     std::string path:           FL_Tree item path (e.g. "Flintstones/Fred")
    //|     Fl_Tree_Item* p:            Item to add
    void AddItem(std::vector<xNode> nodes,  std::string path, Fl_Tree_Item* p = NULL) {
        Fl_Tree_Item* item;
        for (xNode node : nodes)  {
            XPathObj obj = XPathObj(node.ptr, (xmlChar*) "string(text())");
            if (!obj.err) {
                HTreeItem *i = new HTreeItem(node.ptr); size++;
                item = this->add(path.c_str(), i);
                items.insert(items.end(), (HTreeItem *) i);
                
                auto branch = XPathObj(node.ptr, (xmlChar*) "./*");
                if (branch.results->type == XPATH_NODESET)
                    {auto NS = branch.Nodes();
                    std::string newPath = (p ? std::string(path.c_str()) + "/" : "") + escape(obj.Str());
                    
                    if (NS.size()) AddItem(NS, newPath, i);}
                }
            }
    }
    
    HTreeItem* at(int index) const {return items[index];}

//  UserAccept
//| User has indicated with FL_End they accept their choices and exit the program
//| Print to stdout XML-formatted selections
    void UserAccept(HTreeItem* item) {
        while (item)
        {
            if (item->usericon() == SelectedIcon[0])    //  non-obvious check for "not-selected"
               {xmlUnlinkNode(item->node);}
            else {if (item->children()) UserAccept((HTreeItem*) item->child(0));}
            
            item = (HTreeItem*) next(item);
        }
        std::cout <<  doc.XML() << "\n"; exit(0);
    }

    int handle(int event) {
        HTreeItem *it = NULL;
        int rc;
        auto reason = callback_reason();
        if (reason == FL_TREE_REASON_OPENED || reason == FL_TREE_REASON_CLOSED) return Fl_Tree::handle(event);
        switch(event) {
            case FL_NO_EVENT:
                break;
            case FL_PUSH:
                rc = Fl_Tree::handle(event);
                it = (HTreeItem*) item_clicked();
                if (it) {
                    if (it->event_on_collapse_icon(it->prefs)) return rc;
                    if (it->usericon() == SelectedIcon[0]) {it->SelectBranch(true);}
                    else it->SelectBranch(false);
                    it->FixParent(); it->select(0);}
                return rc;
            case FL_KEYBOARD:
                if (FL_End == Fl::event_key()) UserAccept((HTreeItem*) first());
                break;
            case FL_FOCUS:
            case FL_DRAG:
            case FL_RELEASE:
                break;
            case FL_MOVE:
                if (true) break;
                if (Fl::event_x() < 0 || Fl::event_y() < 0) break;
                for (int i = 0; i < size; ++i) {
                    HTreeItem* item = (HTreeItem*) this->at(i);
                    if (item && Fl::event_x() >= item->x() && Fl::event_x() < item->x() + item->w() &&
                        Fl::event_y() >= item->y() && Fl::event_y() < item->y() + item->h())
                            {it = item; break;}
                }
                if (!it && !ToolTipItem) break;
                else if (!it->tooltip.length() && !ToolTipItem) break;
                else if ( it && !ToolTipItem)   ToolTipItem = it;
                else if (!it && ToolTipItem)    {Fl_Tooltip::exit((Fl_Widget*) ToolTipItem); ToolTipItem= nullptr; break;}

                Fl_Tooltip::enter_area((Fl_Widget*) ToolTipItem, Fl::event_x(), Fl::event_y(), 200, 20, ToolTipItem->tooltip.c_str());
                break;
            case FL_ENTER:
            case FL_LEAVE:
            case FL_SHORTCUT:
            case FL_SHOW:
            default:
                break;
        }
        return Fl_Tree::handle(event);
    }

    private:std::string escape(const std::string& input) {
        std::string result;
        for (char c : input) {
            if (c == '/' || c == '\\') result += '\\';
            result += c;
        }
        return result;
    }
};

//  MyWindow class
//| Extended from Fl_Window  to include the FL_end interupt
class MyWindow: public Fl_Window
{
public:
    MyTree* tree = 0;

    MyWindow(int w, int h, const char* title= 0):   Fl_Window(w, h, title) {}

    int handle(int event) {
        switch(event) {
            case FL_KEYBOARD:
                if (FL_End == Fl::event_key()) return tree->handle(event);
                break;
            default:
                break;
        }
        return Fl_Window::handle(event);
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

//  Input, text input
    if (ArgList["type"].compare("input") == 0) {
        fl_message_title(ArgList["title"].c_str());
        auto ans  = fl_input(ArgList["prompt"].c_str(), ArgList["default"].c_str()); 
        if (ans == NULL) CANCELLED();
        std::cout <<  ans << "\n"; return 0;
    }

//  password input
    if (ArgList["type"].compare("password") == 0) {
        fl_message_title(ArgList["title"].c_str());
        auto ans  = fl_password(ArgList["prompt"].c_str()); 
        if (ans == NULL) CANCELLED();
        std::cout <<  ans << "\n"; return 0;
    }

//  Choice, max 3 items and return value is index of selection
    if (ArgList["type"].compare("choice") == 0) {
        fl_message_title(ArgList["title"].c_str());
        char *choice[3] = {NULL, NULL, NULL};
        char* token; token = strtok((char*) ArgList["items"].c_str(), "\t"); i=0;
        while( token != NULL ) {choice[i++] = token; token = strtok(NULL, "\t");}
        return fl_choice(ArgList["prompt"].c_str(), choice[0], choice[1], choice[2], i);
        }

//  message output
    if (ArgList["type"].compare("message") == 0) {
        fl_message_title(ArgList["title"].c_str());
        fl_message(ArgList["message"].c_str()); 
        return 0;
    }

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
        
        MyWindow window(400, 400, ArgList["title"].c_str());
        MyTree* tree = new MyTree(*doc); tree->selectmode(FL_TREE_SELECT_MULTI);
        window.add(tree); window.tree = tree;
        window.as_group()->resizable((Fl_Choice*) tree);
        window.show();
        Fl::run();
        CANCELLED();
        }

    else {std::cerr << "No GUI type chosen\n"; ERROR();}
}
