#include "dirchoosedialog.h"
#include "ui_dirchoosedialog.h"

#include <QDebug>

dirChooseDialog::dirChooseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dirChooseDialog)
{
    ui->setupUi(this);

    connect(ui->acceptButton,SIGNAL(clicked()),
            this, SLOT(acceptClicked()));
    connect(ui->cancelButton,SIGNAL(clicked()),
            this, SLOT(cancelClicked()));

    dir = new QFileSystemModel(this);
    dir->setRootPath("C:/");
    ui->treeView->setModel(dir);
    ui->treeView->setColumnWidth(0,175);
    ui->treeView->setColumnWidth(1,60);
    ui->treeView->setColumnWidth(2,60);
}

dirChooseDialog::~dirChooseDialog()
{
    delete ui;
}

void dirChooseDialog::acceptClicked()
{
    emit sendDir(dir->filePath(ui->treeView->currentIndex()));
    this->close();
}

void dirChooseDialog::cancelClicked()
{
    this->close();
}
