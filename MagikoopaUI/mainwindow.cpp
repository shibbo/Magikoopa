#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QPushButton>
#include <QListWidgetItem>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_issueCount(0)
{
    ui->setupUi(this);
    patchMaker = new PatchMaker(this);
    connect(patchMaker, SIGNAL(setBusy(bool)), SLOT(setActionsDisabled(bool)));
    connect(patchMaker, SIGNAL(addOutput(QString,QString,bool)), this, SLOT(appendOutput(QString,QString,bool)));
    connect(patchMaker, SIGNAL(updateStatus(QString)), ui->statusLabel, SLOT(setText(QString)));

    setActionsDisabled(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setActionsDisabled(bool disabled)
{
    ui->makeInsertButton->setEnabled(!disabled);
    ui->makeCleanButton->setEnabled(!disabled);
}

void MainWindow::appendOutput(const QString& catergory, const QString& text, bool showCategory)
{
    QString logText = text;

    if (logText.endsWith('\n'))
        logText.chop(1);

    if (showCategory)
        logText.prepend("[" + catergory + "] ");
    ui->output->appendPlainText(logText);

    QTextCursor c = ui->output->textCursor();
    c.movePosition(QTextCursor::End);
    ui->output->setTextCursor(c);


    QStringList lines = text.split('\n');

    foreach (const QString& line, lines)
    {
        QString lowerLine = line.toLower();
        if (lowerLine.contains("warning"))
        {
            addIssue(logText, Warning);
            break;
        }
        if (lowerLine.contains("error"))
        {
            addIssue(logText, Error);
            break;
        }
    }
}

void MainWindow::addIssue(QString text, IssueType type)
{
    QString iconPath = QCoreApplication::applicationDirPath() + "/3dsPatcher_data/icons/";
    if (type == Warning) iconPath += "warning.png";
    else if (type == Error) iconPath += "error.png";

    int index = text.indexOf(' ');

    QString location = text.left(index-1);
    QStringList locSegs = location.split(':');
    if (locSegs.count() < 2) return;
    QString path = locSegs.at(0);
    int line = locSegs.at(1).toInt();

    QString message = text.mid(text.indexOf(' ', index+1));

    QListWidgetItem* item = new QListWidgetItem(QIcon(iconPath), QString("%1\n%2, Line %3").arg(message).arg(path).arg(line));

    ui->issues->addItem(item);

    m_issueCount++;
    ui->tabWidget->setTabText(1, QString("Issues (%1)").arg(m_issueCount));
}

void MainWindow::clearIssues()
{
    ui->issues->clear();
    m_issueCount = 0;
    ui->tabWidget->setTabText(1, "Issues");
}

void MainWindow::on_actionSet_Working_Directory_triggered()
{
    QString newPath = QFileDialog::getExistingDirectory(this, "Select Working Directory...", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (newPath == "")
        return;

    if (patchMaker->setPath(newPath))
    {
        this->setWindowTitle("Magikoopa - " + patchMaker->path());
        setActionsDisabled(false);
    }
}

void MainWindow::on_makeInsertButton_clicked()
{
    clearIssues();
    ui->output->clear();
    patchMaker->makeInsert();
}

void MainWindow::on_makeCleanButton_clicked()
{
    clearIssues();
    ui->output->clear();
    patchMaker->makeClean();
}

void MainWindow::on_actionQuit_triggered()
{
    this->close();
}