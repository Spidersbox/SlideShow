#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QLabel>
#include <QImageReader>
#include <QPixmap>
#include <QTime>
#include <QTimer>
#include <iostream>
#include <QContextMenuEvent>
#include <QListWidget>
#include <QMovie>
#include <QTextStream>


#include <QIcon>
#include <QStyle>
#include <QFileDialog>
#include <QFile>
#if QT_VERSION < 0x060000
  #include <QDesktopWidget>
#else
  #include <QScreen>
#endif

#include <QMessageBox>
#include <QDebug>

#include "mainwindow.h"
#include "editform.h"


int period;
bool timer_checker;
QString current_fileName;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  ui->imageLabel->setBackgroundRole(QPalette::Base);
  ui->imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  ui->imageLabel->setScaledContents(true);

  setWindowTitle(tr("SlideShow"));

// overall height and width of screen (not desktop)
#if QT_VERSION < 0x060000
  QDesktopWidget *mydesktop = QApplication::desktop();
  desktopWidth = mydesktop->width();
  desktopHeight = mydesktop->height();
#else
// figure it out later
#endif

  // center form on screen
#if QT_VERSION < 0x060000
  QRect desktopRect = QApplication::desktop()->availableGeometry(this);
  QPoint center = desktopRect.center();
  move(center.x() - width() * 0.5, center.y() - height() * 0.5);
#else
  QScreen *screen = QGuiApplication::primaryScreen();
  QRect  screenGeometry = screen->geometry();
  appWidth = screenGeometry.width()/2;
  appHeight = screenGeometry.height()/2;
  move(AppWidth-340,appHeight-285);
#endif


  QIcon icon(":/images/favicon"); 
  setWindowIcon(icon);
#ifndef Q_OS_MAC
    qApp->setWindowIcon(QIcon(":images/favicon"));
    setWindowIcon(QIcon(":images/favicon"));
#else
    setUnifiedTitleAndToolBarOnMac(true);
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

  createActions();
  createMenuBar();
  menuBar()->setNativeMenuBar(false);

  ui->currentList->setVisible(false);
  ui->doneList->setVisible(false);

}

MainWindow::~MainWindow()
{
    delete ui;
}

//-----------------------------------------------------------------------------------------
void MainWindow::createMenuBar()
{
#ifdef Q_OS_MAC
  /** Create a decoupled menu bar on Mac */
  /** which stays even if the window is closed */
  appMenuBar = new QMenuBar();
#else
  /** Get the main window's menu bar on other platforms */
  appMenuBar = menuBar();
#endif

  /** Configure the menus */
  QMenu *file = appMenuBar->addMenu(tr("&File"));
  file->addAction(openFolderAction);
  file->addAction(openPlayListAction);
  file->addAction(quitAction);

  QMenu *settings = appMenuBar->addMenu(tr("&Options"));
  QMenu *submenu=settings->addMenu(QIcon(":/images/bt_playlist"),"&Playlist");
  submenu->addAction(openPlayListSubAction);
  submenu->addAction(editAction);
  submenu->addAction(saveAction);

  settings->addAction(randomAction);
  settings->addAction(randomizerAction);
  settings->addAction(loopAction);
  settings->addAction(fullScreenAction);

//  popup context menu
  mainMenu=new QMenu();
  QMenu *fileMenu=mainMenu->addMenu(tr("&File"));
  fileMenu->addAction(openFolderAction);
  fileMenu->addAction(openPlayListAction);
  fileMenu->addAction(quitAction);

  QMenu *optionsMenu=mainMenu->addMenu(tr("&Options"));
  optionsMenu->addAction(pauseAction);
  optionsMenu->addAction(continueAction);
  optionsMenu->addSeparator();
  optionsMenu->addAction(nextAction);

  QMenu *settingsMenu=mainMenu->addMenu(tr("&Settings"));
  settingsMenu->addAction(randomAction);
  settingsMenu->addAction(randomizerAction);
  settingsMenu->addAction(loopAction);
  settingsMenu->addAction(fullScreenAction);
}

//-----------------------------------------------------------------------------------------
void MainWindow::createActions()
{
  /** main menu actions */
  openFolderAction = new QAction(QIcon(":/images/bt_open"), tr("open &Folder"), this);
  openFolderAction->setToolTip(tr("Open a Folder to display"));

  openPlayListAction = new QAction(QIcon(":/images/bt_open"), tr("open &Playlist"), this);
  openPlayListAction->setToolTip(tr("Open a Playlist to display"));

  openPlayListSubAction = new QAction(QIcon(":/images/bt_open"), tr("open &Playlist"), this);
  openPlayListSubAction->setToolTip(tr("Open a Playlist to display"));

  quitAction = new QAction(QIcon(":/images/bt_close"), tr("E&xit"), this);
  quitAction->setToolTip(tr("Quit application"));

/** setting switches */
  pauseAction = new QAction(QIcon(":/images/bt_pause"), tr("&Pause"), this);
  pauseAction->setToolTip(tr("Pause slideshow"));
  continueAction = new QAction(QIcon(":/images/bt_play"), tr("&Continue"), this);
  continueAction->setToolTip(tr("Continue slideshow"));
  nextAction = new QAction(QIcon(":/images/bt_next"), tr("&Next"), this);
  nextAction->setToolTip(tr("Next image"));
  fullScreenAction = new QAction(QIcon(":/images/bt_arrows-out"), tr("&Full screen"), this);
  fullScreenAction->setToolTip(tr("switch full screen"));
  fullScreenAction->setCheckable(true);

  editAction = new QAction(QIcon(":/images/bt_edit"), tr("&Edit Playlist"), this);
  editAction->setToolTip(tr("open the playlist editor options"));
  saveAction = new QAction(QIcon(":/images/bt_save"), tr("&Save Playlist"), this);
  saveAction->setToolTip(tr("save the playlist"));

  randomAction = new QAction(QIcon(":/images/bt_shuffle"), tr("&Shuffle (do it NOW)"), this);
  randomAction->setToolTip(tr("randomize the playlist now"));

  randomizerAction = new QAction(QIcon(":/images/bt_shuffle"), tr("Shu&ffle (switch)"), this);
  randomizerAction->setToolTip(tr("randomize the playlist each time"));
  randomizerAction->setCheckable(true);

  loopAction = new QAction(QIcon(":/images/bt_repeat"), tr("&Repeat (switch)"), this);
  loopAction->setToolTip(tr("repeat playing in a loop"));
  loopAction->setCheckable(true);



  /** popup menu signals */
  connect(openFolderAction, SIGNAL(triggered()), this, SLOT(openFolderClicked()));
  connect(openPlayListAction, SIGNAL(triggered()), this, SLOT(openPlayListClicked()));
  connect(openPlayListSubAction, SIGNAL(triggered()), this, SLOT(openPlayListClicked()));
  connect(quitAction, SIGNAL(triggered()), this, SLOT(quitClicked()));

  connect(pauseAction, SIGNAL(triggered()), this, SLOT(pauseClicked()));
  connect(continueAction, SIGNAL(triggered()), this, SLOT(continueClicked()));
  connect(nextAction, SIGNAL(triggered()), this, SLOT(nextClicked()));

  connect(editAction, SIGNAL(triggered()), this, SLOT(editClicked()));
  connect(saveAction, SIGNAL(triggered()), this, SLOT(saveClicked()));

  connect(fullScreenAction, SIGNAL(triggered()), this, SLOT(fullScreenClicked()));
  connect(randomAction, SIGNAL(triggered()), this, SLOT(randomClicked()));

  /** to open popup menu for form and imageLabel */
  connect(ui->imageLabel, SIGNAL(customContextMenuRequested(QPoint))
         ,this, SLOT(contextualMenu()));
  connect(this, SIGNAL(customContextMenuRequested(QPoint))
         ,this, SLOT(contextualMenu()));

}

//-----------------------------------------------------------------------------------------
/** exit this app */
void MainWindow::quitClicked()
{
  qApp->quit();
}

//-----------------------------------------------------------------------------------------
/** Show open Playlist dialog */
void MainWindow::openPlayListClicked()
{
std::cout<<" open playlist triggered\n";
  this->setStyleSheet("background:rgb(250,250,250); color:rgb(0,0,0);");
  showNormal();

  QString filename=QFileDialog::getOpenFileName(
              this,"Open Playlist",qApp->applicationDirPath(),
              "playlist files (*.p3u)");

  QFile file(filename);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    stopTimer();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    playlist.clear();
    ui->currentList->clear();
    ui->doneList->clear();

    QTextStream textStream(&file);
    while (!textStream.atEnd())
    {
      QString line=textStream.readLine();
      playlist<<line;
      ui->currentList->addItem(line);
    }
    file.close();
    QApplication::restoreOverrideCursor();

    if(ui->currentList->count() >0)
    {
      QListWidgetItem *item = ui->currentList->takeItem(0);
      ui->doneList->addItem(item->text());
      setImage(item->text());
      delete item;
      menuBar()->setVisible(false);
      period=10;
      this->setStyleSheet("background:rgb(0, 0, 0); color:rgb(255,255,255);");
      setTimer();
    }
  }
  else
  {
    QString mess="Could not open file: "+filename;
    QMessageBox::warning(this,"SlideShow",mess);
  }
}

//-----------------------------------------------------------------------------------------
/** Show open file dialog */
void MainWindow::openFolderClicked()
{
  stopTimer();
  this->setStyleSheet("background:rgb(250,250,250); color:rgb(0,0,0);");
  showNormal();
  QDir imagePath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  QStringList images = imagePath.entryList(QStringList() << "*.jpg" << "*.jpeg" << "*.bmp" << "*.pbm" << "*.pgm" << "*.ppm" << "*.xbm" << "*.xpm" << "*.png" << "*.gif", QDir::Files);

  playlist.clear();
  ui->currentList->clear();
  ui->doneList->clear();

  foreach(QString filename, images)
  {
    QString line=imagePath.filePath(filename);
    playlist<<line;
    ui->currentList->addItem(line);
  }

  if(ui->currentList->count() >0)
  {
    QListWidgetItem *item = ui->currentList->takeItem(0);
    ui->doneList->addItem(item->text());
    setImage(item->text());
    delete item;
    menuBar()->setVisible(false);
  }

  period=10;
  this->setStyleSheet("background:rgb(0, 0, 0); color:rgb(255,255,255);");
  setTimer();
}

//-----------------------------------------------------------------------------------------
void MainWindow::setImage(const QString &image)
{
  current_fileName=image;
  showImage(image);
}

//-----------------------------------------------------------------------------------------
void MainWindow::showImage(QString fileName)
{
//std::cout<<"fileName = "<<fileName.toStdString()<<"\n";
  QFileInfo fn(fileName);
  QString fname = fn.completeSuffix().toLower();
  if(fname=="gif")
  {
    QMovie* movie = new QMovie(fileName, QByteArray(), this );
    if (!movie->isValid())
    {
      qDebug() << "Movie is not valid";
      return;
    }

// load gif as normal image to get screen size
    QPixmap image = fileName;
    ui->imageLabel->setPixmap(image);
    scaleImage();

    ui->imageLabel->setMovie(movie);
    movie->start();
  }
  else
  {
    QPixmap image = fileName;
    ui->imageLabel->setPixmap(image);
    scaleImage();
  }
}

//-----------------------------------------------------------------------------------------
void MainWindow::scaleImage()
{
  int screenWidth=desktopWidth;
  int screenHeight=desktopHeight;


//  menuBar()->setVisible(false);
  if(fullScreenAction->isChecked())
  {
    showFullScreen();
  }
  else
  {
    showNormal();
    screenWidth=width();
    screenHeight=height();
  }

  int imageWidth=ui->imageLabel->pixmap()->width();
  int imageHeight=ui->imageLabel->pixmap()->height();

  float iWidth;
  float iHeight;
  float x,y,z;

  x=(screenWidth*1.0)/(imageWidth*1.0);
  y=(screenHeight*1.0)/(imageHeight*1.0);
  x<=y ? z=x:z=y;
  iWidth=imageWidth*z;
  iHeight=imageHeight*z;
  ui->imageLabel->resize(iWidth,iHeight);

// center imageLabel
  int itop=(screenHeight-iHeight)/2;
  int ileft=(screenWidth-iWidth)/2;

  ui->imageLabel->move(ileft,itop);
}

//-----------------------------------------------------------------------------------------
void MainWindow::setTimer()
{
  if(period >= 5)
  {
    timer = new QTimer(this);
    timer_checker = true;
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTriggered()));
    timer->start(period*1000);
  }
  else if(period != 0)
  {
    QMessageBox::warning(this, "Warning", "The period is below 5 seconds. Set a higher value.");
  }
}

//-----------------------------------------------------------------------------------------
void MainWindow::timerTriggered()
{
  nextImage();
}

//-----------------------------------------------------------------------------------------
void MainWindow::nextImage()
{
  if(ui->currentList->count() >0)
  {
    QListWidgetItem *item = ui->currentList->takeItem(0);
    ui->doneList->addItem(item->text());
    setImage(item->text());
    delete item;
  }
  else // end of slideshow,
  {
    // randomize playlist and start again  
    if(loopAction->isChecked())
    {
      if(randomizerAction->isChecked())
      {
        randomClicked();
      }
      else
      {
        ui->doneList->clear();
        ui->currentList->clear();
        ui->currentList->addItems(playlist);
      }

      // just checking if there are images in the currentList
      if(ui->currentList->count() >0)
      {
        QListWidgetItem *item = ui->currentList->takeItem(0);
        ui->doneList->addItem(item->text());
        setImage(item->text());
        delete item;
      }
    }// endif loopaction
else
{
std::cout<<"loop action IS NOT checked\n";
}
  }// end else
}

//-----------------------------------------------------------------------------------------
void MainWindow::stopTimer()
{
  if(timer_checker)
  {
    timer_checker = false;
    timer->stop();
  }
}

//-----------------------------------------------------------------------------------------
void MainWindow::contextualMenu()
{
  mainMenu->exec(QCursor::pos());
}

//-----------------------------------------------------------------------------------------
/** pause the slideshow */
void MainWindow::pauseClicked()
{
  stopTimer();
  if(!fullScreenAction->isChecked())
  {
  this->setStyleSheet("background:rgb(250,250,250); color:rgb(0,0,0);");
  }
  menuBar()->setVisible(true);
  showNormal();
}

//-----------------------------------------------------------------------------------------
/** continue the slideshow */
void MainWindow::continueClicked()
{
  period=10;
  menuBar()->setVisible(false);
  if(fullScreenAction->isChecked())
  {
    this->setStyleSheet("background:rgb(0, 0, 0); color:rgb(255,255,255);");
    showFullScreen();
  }
  setTimer();
}

//-----------------------------------------------------------------------------------------
/** next image */
void MainWindow::nextClicked()
{
  stopTimer();
  nextImage();
}

//-----------------------------------------------------------------------------------------
/** full screen switch */
void MainWindow::fullScreenClicked()
{
  if(fullScreenAction->isChecked())
  {
    fullScreenAction->setIcon(QIcon(":/images/bt_arrows-out"));
    this->setStyleSheet("background:rgb(0, 0, 0); color:rgb(255,255,255);");
    menuBar()->setVisible(false);
    showImage(current_fileName);
  }
  else
  {
    fullScreenAction->setIcon(QIcon(":/images/bt_arrows-in"));
    this->setStyleSheet("background:rgb(250,250,250); color:rgb(0,0,0);");
    menuBar()->setVisible(true);
    showImage(current_fileName);
  }
}

//-----------------------------------------------------------------------------------------
/** Open playlist editor dialog */
void MainWindow::editClicked()
{
  EditForm *dialog = new EditForm();
  /** for retrieving data from editform to mainform */
  connect(dialog, SIGNAL(sendData(QStringList)), this, SLOT(printData(QStringList)));

  dialog->receivePlaylist(playlist);
  dialog->show();

}

//-----------------------------------------------------------------------------------------
/** save playlist to a file */
void MainWindow::saveClicked()
{
  QString filename = QFileDialog::getSaveFileName(this, tr("save as"), "/", tr("playlist files(*.p3u)"));
  if (filename.isEmpty())
  {
std::cout<<"error opening save file\n";
    return;
  }

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

//-----------------------------------------------------------------------------------------
/** recieves playlist from editor */
void MainWindow::printData(QStringList dat)
{
//  stopClicked();
  playlist=dat; // keep a copy of the playlist
  ui->doneList->clear();
  ui->currentList->clear();
  ui->currentList->addItems(dat);
}

//-----------------------------------------------------------------------------------------
/** randomize now trigger */
void MainWindow::randomClicked()
{
std::cout<<"randomClicked triggered\n";
//  stopTimer();

  // let's not randomize the master playlist
  ui->doneList->clear();
  ui->currentList->clear();
  ui->currentList->addItems(playlist);

  // remove image from doneList randomly and add it to currentList
  // note: you could fill the doneList and rnd move to currentList -
  // but doing so would cause the last image to become the first
  // image for the next go around - don't know why ...
  while(ui->currentList->count())
  {
    srand(time(0));
    int random=rand()%(ui->currentList->count());
    QListWidgetItem *item = ui->currentList->takeItem(random);
    ui->doneList->addItem(item->text());
    delete item;
  }

  // move list from doneList to currentList
  while(ui->doneList->count())
  {
    QListWidgetItem *item = ui->doneList->takeItem(0);
    ui->currentList->addItem(item->text());
    delete item;
  }
}


