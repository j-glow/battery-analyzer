#include "dirchoosedialog.h"
#include "ui_dirchoosedialog.h"

dirChooseDialog::dirChooseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dirChooseDialog)
{
    ui->setupUi(this);

    connect(ui->acceptButton,SIGNAL(clicked()),
            this, SLOT(acceptClicked()));
    connect(ui->cancelButton,SIGNAL(clicked()),
            this, SLOT(cancelClicked()));

    QFileSystemModel* dir = new QFileSystemModel(this);

    ui->treeView->setModel(dir);
}

dirChooseDialog::~dirChooseDialog()
{
    delete ui;
}

void dirChooseDialog::acceptClicked()
{

    this->close();
}


void dirChooseDialog::cancelClicked()
{
    this->close();
}
