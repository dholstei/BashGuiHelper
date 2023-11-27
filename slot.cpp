#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMessageBox>

class ComboBoxApp : public QWidget {
public:
    ComboBoxApp(QWidget *parent = nullptr) : QWidget(parent) {
        // Create a combo box
        comboBox = new QComboBox(this);

        // Add items to the combo box
        comboBox->addItem("Option 1");
        comboBox->addItem("Option 2");
        comboBox->addItem("Option 3");

        // Connect the signal for item selection to the custom slot
        connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxSelectionChanged(int)));

        // Create a layout and set it for the main window
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(comboBox);

        setLayout(layout);
    }

public slots:
    // Custom slot to handle combo box selection changes
    void onComboBoxSelectionChanged(int index) {
        QString selectedOption = comboBox->itemText(index);
        QMessageBox::information(this, "Selection Changed", "Selected: " + selectedOption);
    }

private:
    QComboBox *comboBox;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    ComboBoxApp window;
    window.setWindowTitle("ComboBox Application");
    window.resize(300, 150);
    window.show();

    return app.exec();
}
