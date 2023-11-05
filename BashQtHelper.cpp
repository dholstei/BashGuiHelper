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
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <string>
#include <iostream>
#include <map>
#include <stdio.h>

#define ERROR() return 1

class SelectionListDialog : public QDialog
{
public:
    SelectionListDialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setupUi();
    }

private:
    void setupUi()
    {
        // Create the main layout.
        QVBoxLayout *mainLayout = new QVBoxLayout();

        // Create the tree view.
        QTreeView *treeView = new QTreeView();
        QStandardItemModel *model = new QStandardItemModel();
        QStandardItem *rootItem = model->invisibleRootItem();
        rootItem->appendRow(new QStandardItem("Item 1"));
        rootItem->appendRow(new QStandardItem("Item 2"));
        rootItem->appendRow(new QStandardItem("Item 3"));
        treeView->setModel(model);

        // Create the push button.
        QPushButton *pushButton = new QPushButton("OK");

        // Add the widgets to the layout.
        mainLayout->addWidget(treeView);
        mainLayout->addWidget(pushButton);

        // Set the layout for the dialog.
        setLayout(mainLayout);

        // Connect the signals and slots.
        connect(pushButton, &QPushButton::clicked, this, &SelectionListDialog::accept);
    }

    // The slot that will be called when the dialog is accepted.
    void onAccepted()
    {
        // Get the selected items from the tree view.
        QItemSelectionModel *selectionModel = treeView->selectionModel();
        QList<QModelIndex> selectedIndexes = selectionModel->selectedIndexes();

        // Display the selected items in a label.
        // label->setText(qml::join(", ", selectedIndexes));
    }

private:
    // The tree view.
    QTreeView *treeView;

    // The label that will display the selected items.
    QLabel *label;
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
        if (!  FileName.toStdString().length()) ERROR();
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
        else ERROR();}

    if (ArgList["type"].compare("dirbrowser") == 0) {
        QFileDialog dialog(mainWindow, ArgList["title"].c_str());
        dialog.setDirectory(ArgList["directory"].c_str());
        dialog.setFileMode(QFileDialog::Directory);
        if (dialog.exec()) {
            QStringList DirNames = dialog.selectedFiles();
            for ( const auto& i :  DirNames ) std::cout << i.toStdString() << " ";
            std::cout<< "\n"; return 0;}
        else ERROR();}

    if (ArgList["type"].compare("selection") == 0) {
        QDialog dialog(mainWindow); dialog.setWindowTitle(ArgList["title"].c_str());
        QVBoxLayout layout = QVBoxLayout();
        if (dialog.exec()) {
            std::cout<< "\n"; return 0;}
        // else ERROR();

        // Create the dialog.
        SelectionListDialog ndialog;

        // Show the dialog.
        ndialog.exec();
        }

    else {std::cerr << "No GUI type chosen\n"; ERROR();}
}
