/* 
    BashQtHelp - Use Qt to prompt user for files, list selection, numeric input, date, etcetera
    Programmer: Danny Holstein
 */
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QListView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QComboBox>
#include <string>
#include <iostream>
#include <map>
#include <stdio.h>
#include "LibXML2.h"

#define ERROR() return 1
#define CANCELLED() return 2

#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>

class TreeSelect : public QDialog
{
xDoc doc;
int row = 0;

public:
    TreeSelect(xDoc doc);
    ~TreeSelect();

    void AddItem(std::vector<xNode> nodes, std::variant<QStandardItemModel*, QStandardItem*> rootItem, int level);
};

TreeSelect::TreeSelect(xDoc d) {
    doc = d;
    QVBoxLayout *mainLayout = new QVBoxLayout();

    // Create the tree view.
    QTreeView *treeView = new QTreeView();
    std::variant<QStandardItemModel*, QStandardItem*> model;
    model = new QStandardItemModel();
    // QStandardItem *rootItem = model->invisibleRootItem();

    XPathObj n = XPathObj(doc.ptr, (xmlChar*) "/*/*");
    if (n.err)
    {   // turn into macro
        std::cerr << "ERROR: " << n.err->msg->c_str();
        if (n.err->src) std::cerr << "SRC: " << n.err->src->c_str() << "\n";
        if (n.err->data) std::cerr << "QUERY: " << n.err->data->c_str() << "\n";
        return;
    }
    auto NodeList = n.Nodes();
    AddItem(NodeList, model, 0);

    treeView->setModel(std::get<QStandardItemModel*>(model));

    // Create the push button.
    QPushButton *pushButton = new QPushButton("OK");

    // Add the widgets to the layout.
    mainLayout->addWidget(treeView);
    mainLayout->addWidget(pushButton);

    // Set the layout for the dialog.
    setLayout(mainLayout);

    // Connect the signals and slots.
    // connect(pushButton, &QPushButton::clicked, this, &SelectionListDialog::accept);
    }
TreeSelect::~TreeSelect() {}

// function to recursively add xml node texts to tree browser
void TreeSelect::AddItem(std::vector<xNode> nodes, std::variant<QStandardItemModel*, QStandardItem*> rootItem, int level) {
    for (xNode node : nodes)  {
        XPathObj obj = XPathObj(node.ptr, (xmlChar*) "string(text())");
        if (!obj.err) {
            auto item = new QStandardItem((obj.Str()).c_str());
            item->setCheckable(true);
            if (rootItem.index() == 0)  std::get<QStandardItemModel*>(rootItem)->appendRow(item);
            else                        std::get<QStandardItem*>(rootItem)->appendRow(item);
            
            auto branch = XPathObj(node.ptr, (xmlChar*) "./*");
            if (branch.results->type == XPATH_NODESET)
                {auto NS = branch.Nodes();
                if (NS.size())
                    AddItem(NS, item, level + 1);}
            }
        }}

class MyComboBox: public QComboBox
{
public:
    MyComboBox(QWidget *parent = nullptr): QComboBox(parent) {
        connect(this, SIGNAL(activated(int)), this, SLOT(MyIndexChanged()));
        // connect(this, &activated, this, &MyComboBox::MyIndexChanged);
        }
    ~MyComboBox(){}

public slots:
    void MyIndexChanged() {
        if (currentIndex()) {std::cout << currentText().toStdString() << "\n"; exit(0);}}
};

int main(int argc, char *argv[])
{
    int i=0; QApplication a(argc, argv, i);

    std::map<std::string, std::string> ArgList{};   //  cheap version of argparse
    for (size_t i = 1; i < argc; i++) {
        char *param = strtok(argv[i], "=")+2; char *value = strtok(NULL, "="); ArgList.insert({param, value});}

    QWidget *mainWindow = new QWidget();
    
    if (ArgList["type"].compare("filebrowser") == 0) {
        auto  FileName = QFileDialog::getOpenFileName(mainWindow, ArgList["title"].c_str(), ArgList["directory"].c_str(), ArgList["file_type"].c_str());
        if (! FileName.toStdString().length()) CANCELLED();
        std::cout <<  FileName.toStdString() << "\n"; return 0;}

    if (ArgList["type"].compare("multifiles") == 0) {
        QFileDialog dialog(mainWindow, ArgList["title"].c_str());
        dialog.setDirectory(ArgList["directory"].c_str());
        dialog.setFileMode(QFileDialog::ExistingFiles);
        dialog.setNameFilter(ArgList["file_type"].c_str());
        if (dialog.exec()) {
            QStringList  FileNames = dialog.selectedFiles();
            for ( const auto& i :  FileNames ) std::cout << i.toStdString() << " ";
            std::cout<< "\n"; return 0;}
        else CANCELLED();}

    if (ArgList["type"].compare("dirbrowser") == 0) {
        QFileDialog dialog(mainWindow, ArgList["title"].c_str());
        dialog.setDirectory(ArgList["directory"].c_str());
        dialog.setFileMode(QFileDialog::Directory);
        if (dialog.exec()) {
            QStringList DirNames = dialog.selectedFiles();
            for ( const auto& i :  DirNames ) std::cout << i.toStdString() << " ";
            std::cout<< "\n"; return 0;}
        else CANCELLED();}

    if (ArgList["type"].compare("selection") == 0) {
        // target *t = new target();
        // QComboBox *comboBox = t->obj;
        MyComboBox *comboBox = new MyComboBox();
        // QComboBox::connect(comboBox, SIGNAL(currentIndexChanged(int)), comboBox, &ComboIndexChanged);

        char* token; token = strtok((char*) ArgList["items"].c_str(), "\t");
        comboBox->addItem("---Make Selection---");
        while( token != NULL ) {comboBox->addItem(token); token = strtok(NULL, "\t");}

        comboBox->setWindowTitle(ArgList["title"].c_str()); comboBox->show();
        if (a.exec()) {
            std::cout<< "\n"; return 0;}
        else CANCELLED();
        }

    if (ArgList["type"].compare("tree") == 0) {
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
        
        auto dialog = TreeSelect(*doc);
        dialog.setWindowTitle(ArgList["title"].c_str());
        if (dialog.exec()) {
            std::cout<< "\n"; return 0;}
        else CANCELLED();
        }

    else {std::cerr << "No GUI type chosen\n"; ERROR();}
}
