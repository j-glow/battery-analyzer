#ifndef DIRCHOOSEDIALOG_H
#define DIRCHOOSEDIALOG_H

#include <QDialog>
#include <QFileSystemModel>

namespace Ui {
class dirChooseDialog;
}

class dirChooseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit dirChooseDialog(QWidget *parent = nullptr);
    ~dirChooseDialog();

private slots:
    void acceptClicked();
    void cancelClicked();

signals:
    void sendDir(QString path);

private:
    Ui::dirChooseDialog *ui;
    QFileSystemModel* dir;
};

#endif // DIRCHOOSEDIALOG_H
