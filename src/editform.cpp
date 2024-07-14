#include <QDir>
#include <QTableView>
#include <QTreeView>
#include <QComboBox>
#include <QStringList>
#include <QFileDialog>
#include <QListView>
#if QT_VERSION < 0x060000
  #include <QDesktopWidget>
#else
  #include <QScreen>
#endif

#include <QMessageBox> // for debuging
#include <QDebug>

#include "editform.h"
#include "ui_editform.h"
#include "mainwindow.h"

EditForm::EditForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditForm)
{
  ui->setupUi(this);

// center form on screen
#if QT_VERSION < 0x060000
  QRect desktopRect = QApplication::desktop()->availableGeometry(this);
  QPoint center = desktopRect.center();
  move(center.x() - width() * 0.5, center.y() - height() * 0.5);
#else
  QScreen *screen = QGuiApplication::primaryScreen();
  QRect  screenGeometry = screen->geometry();
  int height = screenGeometry.height()/2;
  int width = screenGeometry.width()/2;
  move(width-324,height-250);
#endif


  // Create actions for the toolbar, menu bar and tray/dock icon
  SetSignals();

  /** populate combobox with drives */
  QDir drives;
  foreach(QFileInfo item, drives.drives() )
  {
    ui->comboBox->addItem(item.absoluteFilePath());
  }
  connect(ui->comboBox,&QComboBox::currentTextChanged, this,&EditForm::on_driveChanged);

  /** setup dir tree */
  QString mPath="/home";
  /** Dirs */
  dirModel = new QFileSystemModel(this);
  // Set filter
  dirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
  // QFileSystemModel requires root path
  dirModel->setRootPath(mPath);

  // Attach the dir model to the view
  ui->treeView->setModel(dirModel);


  /** files */
  fileModel = new QFileSystemModel(this);
  // Set filter
  fileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
  fileModel->setRootPath(mPath);

  // Attach the file model to the view
  ui->listView->setModel(fileModel);



  /** hide some of the columns */
  /** (have to do it after setmodel) */
  ui->treeView->setColumnHidden(1,1);
  ui->treeView->setColumnHidden(2,1);
  ui->treeView->setColumnHidden(3,1);

  updateFilters();
}

EditForm::~EditForm()
{
    delete ui;
}

//-----------------------------------------------------------------------------------------------------------------
void EditForm::receivePlaylist(QStringList list)
{
  playlist=list;
  ui->listWidget->clear();
  ui->listWidget->addItems(list);
}
//-----------------------------------------------------------------------------------------------------------------
void EditForm::SetSignals()
{
  /** button signals */
  connect(ui->bt_right, &QPushButton::pressed,this, &EditForm::bt_right_clicked);
  connect(ui->bt_rightAll,&QPushButton::released,this, &EditForm::bt_rightAll_clicked);
  connect(ui->bt_left, &QPushButton::released, this, &EditForm::bt_left_clicked);

  connect(ui->bt_ok, &QPushButton::released, this, &EditForm::bt_ok_clicked);
  connect(ui->bt_cancel, &QPushButton::released, this, &EditForm::bt_cancel_clicked);
  connect(ui->bt_save, &QPushButton::released, this, &EditForm::bt_save_clicked);

  connect(ui->cb_jpg, &QPushButton::released, this, &EditForm::cb_jpg_clicked);
  connect(ui->cb_png, &QPushButton::released, this, &EditForm::cb_png_clicked);
  connect(ui->cb_all, &QPushButton::released, this, &EditForm::cb_all_clicked);
}

//-----------------------------------------------------------------------------------------------------------------
void EditForm::on_driveChanged(QString choice)
{
  ui->treeView->setRootIndex(dirModel->setRootPath(choice));
}

//-----------------------------------------------------------------------------------------------------------------
void EditForm::bt_right_clicked()
{
  QString path=ui->listView->toolTip()+"/";
  foreach(const QModelIndex &index,ui->listView->selectionModel()->selectedIndexes())
    ui->listWidget->addItem(path+index.data(Qt::DisplayRole).toString());
}

//-----------------------------------------------------------------------------------------------------------------
void EditForm::bt_rightAll_clicked()
{
  QString path=ui->listView->toolTip()+"/";
  ui->listView->selectAll();

  int mColumn=0;
  foreach(const QModelIndex &index,ui->listView->selectionModel()->selectedIndexes())
  {// need to skip other columns
    if(mColumn == 0)
      ui->listWidget->addItem(path+index.data(Qt::DisplayRole).toString());

    mColumn++;
    if(mColumn > 3)
      mColumn=0;
  }
}

//-----------------------------------------------------------------------------------------------------------------
void EditForm::bt_left_clicked()
{
  QList<QListWidgetItem*> items = ui->listWidget->selectedItems();
  foreach(QListWidgetItem * item, items)
    delete ui->listWidget->takeItem(ui->listWidget->row(item));
}

//-----------------------------------------------------------------------------------------------------------------
void EditForm::bt_ok_clicked()
{
  QStringList lines;

  for(int i=0;i<ui->listWidget->count(); i++)
    lines << ui->listWidget->item(i)->text();

  emit sendData(lines);
  close();
}

//-----------------------------------------------------------------------------------------------------------------
void EditForm::on_treeView_clicked(const QModelIndex &index)
{
  QString mPath = dirModel->fileInfo(index).absoluteFilePath();
  ui->listView->setRootIndex(fileModel->setRootPath(mPath));
  ui->listView->setToolTip(mPath);
}

//-----------------------------------------------------------------------------------------------------------------
void EditForm::on_listView_doubleClicked(const QModelIndex &index)
{
  QString path=ui->listView->toolTip()+"/";
  ui->listWidget->addItem(path+index.data(Qt::DisplayRole).toString());
}

//-----------------------------------------------------------------------------------------------------------------
void EditForm::cb_jpg_clicked()
{
  updateFilters();
}

//-----------------------------------------------------------------------------------------------------------------
void EditForm::cb_png_clicked()
{
  updateFilters();
}

//-----------------------------------------------------------------------------------------------------------------
void EditForm::cb_all_clicked()
{
  updateFilters();
}

//-----------------------------------------------------------------------------------------------------------------
void EditForm::bt_cancel_clicked()
{
  close();
}

//-----------------------------------------------------------------------------------------------------------------
void EditForm::updateFilters()
{
  QStringList filters;
  if(ui->cb_jpg->isChecked())
    filters << "*.jpg";

  if(ui->cb_png->isChecked())
  {
    filters << "*.png";
  }
  if(ui->cb_all->isChecked())
  {
    filters << "*.*";
//    filters << "*.wave";
  }

  fileModel->setNameFilters(filters);
  fileModel->setNameFilterDisables(false);
}

//-----------------------------------------------------------------------------------------------------------------
void EditForm::bt_save_clicked()
{
  /** update master playlist */
  playlist.clear();
  for(int i=0;i<ui->listWidget->count(); i++)
    playlist << ui->listWidget->item(i)->text();

  QString filename = QFileDialog::getSaveFileName(this, tr("save as"), "/", tr("playlist files(*.p3u)"));
  if (filename.isEmpty())
    return;

  // check for extension
  // windows will add the extension
  // ubuntu will not
  int x=filename.indexOf("p3u");
  if(x<1)
    filename=filename+".p3u";

  QFile file(filename);
  //Open the file
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;

  QTextStream out(&file);
  for(int i=0;i<playlist.size();++i)
    out<<playlist[i]<<"\n";

  file.close();
}
