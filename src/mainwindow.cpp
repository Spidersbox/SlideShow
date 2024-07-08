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

#include <QIcon>
#include <QStyle>
#include <QFileDialog>
#include <QFile>
#if QT_VERSION < 0x060000
  #include <QDesktopWidget>
#else
  #include <QScreen>
#endif

#include <QMessageBox> // for debuging
#include <QDebug>

#include "mainwindow.h"

QString imageDir[128], tempDir;
QDir imagePath;
int i, count, period;
float anim_period = 0.5;
bool timer_checker, shuffle_checker, comboBox_checker, image_checker, mouse_click_checker, anim_checker;

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
  desktopWidth = mydesktop->width(); // pixels;
  desktopHeight = mydesktop->height(); // pixels;
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
  int height = screenGeometry.height()/2;
  int width = screenGeometry.width()/2;
  move(width-340,height-285);
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
  file->addAction(openAction);
  file->addAction(quitAction);

  QMenu *settings = appMenuBar->addMenu(tr("&Options"));
  QMenu *submenu=settings->addMenu("&Playlist");
//  submenu->addAction(editAction);
//  submenu->addAction(saveAction);

  settings->addAction(randomAction);
  settings->addAction(randomizerAction);
  settings->addAction(loopAction);
}

//-----------------------------------------------------------------------------------------
void MainWindow::createActions()
{
  /** main menu actions */
  openAction = new QAction(QIcon(":/images/bt_open"), tr("&Open"), this);
  openAction->setToolTip(tr("Open a playlist file"));
  quitAction = new QAction(QIcon(":/images/bt_close"), tr("E&xit"), this);
  quitAction->setToolTip(tr("Quit application"));

  pauseAction = new QAction(QIcon(":/images/bt_pause"), tr("&Pause"), this);
  pauseAction->setToolTip(tr("Pause slideshow"));
  continueAction = new QAction(QIcon(":/images/bt_play"), tr("&Continue"), this);
  continueAction->setToolTip(tr("Continue slideshow"));

//  editAction = new QAction(QIcon(":/images/bt_edit"), tr("&Edit Playlist"), this);
//  editAction->setToolTip(tr("open the playlist editor options"));
//  saveAction = new QAction(QIcon(":/images/bt_save"), tr("&Save Playlist"), this);
//  saveAction->setToolTip(tr("save the playlist"));

  randomAction = new QAction(QIcon(":/images/bt_shuffle"), tr("&Shuffle (do it NOW)"), this);
  randomAction->setToolTip(tr("randomize the playlist now"));

  randomizerAction = new QAction(QIcon(":/images/bt_shuffle"), tr("Shu&ffle (switch)"), this);
  randomizerAction->setToolTip(tr("randomize the playlist each time"));
  randomizerAction->setCheckable(true);

  loopAction = new QAction(QIcon(":/images/bt_repeat"), tr("&Repeat (switch)"), this);
  loopAction->setToolTip(tr("repeat playing in a loop"));
  loopAction->setCheckable(true);



  /** main menu signals */
  connect(openAction, SIGNAL(triggered()), this, SLOT(openClicked()));
  connect(quitAction, SIGNAL(triggered()), this, SLOT(quitClicked()));

  connect(pauseAction, SIGNAL(triggered()), this, SLOT(pauseClicked()));
  connect(continueAction, SIGNAL(triggered()), this, SLOT(continueClicked()));

//  connect(editAction, SIGNAL(triggered()), this, SLOT(editClicked()));
//  connect(saveAction, SIGNAL(triggered()), this, SLOT(saveClicked()));
  connect(randomAction, SIGNAL(triggered()), this, SLOT(randomClicked()));
  connect(randomizerAction, SIGNAL(triggered()), this, SLOT(randomizerClicked()));
  connect(loopAction, SIGNAL(triggered()), this, SLOT(loopClicked()));

  /** popup menu for form and imageLabel */
  connect(ui->imageLabel, SIGNAL(customContextMenuRequested(QPoint))
         ,this, SLOT(contextualMenu(QPoint)));
  connect(this, SIGNAL(customContextMenuRequested(QPoint))
         ,this, SLOT(contextualMenu(QPoint)));

}

//-----------------------------------------------------------------------------------------
/** exit this app */
void MainWindow::quitClicked()
{
  qApp->quit();
}

//-----------------------------------------------------------------------------------------
/** Show open file dialog */
void MainWindow::openClicked()
{
  stopTimer();
  this->setStyleSheet("background:rgb(250,250,250); color:rgb(0,0,0);");
  showNormal();
  count = 0;
  imagePath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  QStringList images = imagePath.entryList(QStringList() << "*.jpg" << "*.jpeg" << "*.bmp" << "*.pbm" << "*.pgm" << "*.ppm" << "*.xbm" << "*.xpm" << "*.png", QDir::Files);

  foreach(QString filename, images)
  {
    imageDir[count] = imagePath.filePath(filename);
    count++;
  }
  
  if (count > 0)
  {
    setImage(imageDir[0]);
  }
  period=10;
  this->setStyleSheet("background:rgb(0, 0, 0); color:rgb(255,255,255);");
  setTimer();
}

//-----------------------------------------------------------------------------------------
void MainWindow::setImage(const QString &image)
{
  showImage(image);
}

//-----------------------------------------------------------------------------------------
void MainWindow::showImage(QString fileName)
{
  QPixmap image = fileName;
  ui->imageLabel->setPixmap(image);
  scaleImage();
}

//-----------------------------------------------------------------------------------------
void MainWindow::scaleImage()
{
  int screenWidth=desktopWidth;
  int screenHeight=desktopHeight;

  int imageWidth=ui->imageLabel->pixmap()->width();
  int imageHeight=ui->imageLabel->pixmap()->height();


//  statusBar()->hide();
  showFullScreen();

float iWidth;
float iHeight;
float z;

  float x=(screenWidth*1.0)/(imageWidth*1.0);
  float y=(screenHeight*1.0)/(imageHeight*1.0);
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
  if (i == count - 1)
  {
    i = 0;
  }
  else
  {
    i++;
  }
  tempDir = imageDir[i];
  setImage(tempDir);

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
void MainWindow::contextualMenu(const QPoint &point)
{
  QMenu mainMenu(this);
  QMenu *fileMenu=mainMenu.addMenu(tr("&File"));
  fileMenu->addAction(openAction);
  fileMenu->addAction(quitAction);

  QMenu *optionsMenu=mainMenu.addMenu(tr("&Options"));
  optionsMenu->addAction(pauseAction);
  optionsMenu->addAction(continueAction);

  QMenu *settingsMenu=mainMenu.addMenu(tr("&Settings"));
  settingsMenu->addAction(randomAction);
  settingsMenu->addAction(randomizerAction);
  settingsMenu->addAction(loopAction);


  mainMenu.exec(QCursor::pos());
}

//-----------------------------------------------------------------------------------------

/** pause the slideshow */
void MainWindow::pauseClicked()
{
  stopTimer();
//  showMaximized();
  this->setStyleSheet("background:rgb(250,250,250); color:rgb(0,0,0);");
  showNormal();
}

//-----------------------------------------------------------------------------------------

/** continue the slideshow */
void MainWindow::continueClicked()
{
  period=10;
  this->setStyleSheet("background:rgb(0, 0, 0); color:rgb(255,255,255);");
  showFullScreen();
  setTimer();

}

//-----------------------------------------------------------------------------------------
/** Open playlist editor dialog */
void MainWindow::editClicked()
{
//  EditForm *dialog = new EditForm();
  /** for retrieving data from editform to mainform */
//  connect(dialog, SIGNAL(sendData(QStringList)), this, SLOT(printData(QStringList)));

//  dialog->receivePlaylist(playlist);
//  dialog->show();

}

//-----------------------------------------------------------------------------------------
/** save playlist to a file */
void MainWindow::saveClicked()
{
/*
  QString filename = QFileDialog::getSaveFileName(this, tr("save as"), "/", tr("playlist files(*.m3u)"));
  if (filename.isEmpty())
    return;

  // check for extension
  // windows will add the extension
  // ubuntu will not
  int x=filename.indexOf("m3u");
  if(x<1)
    filename=filename+".m3u";

  QFile file(filename);
  //Open the file
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;

  QTextStream out(&file);
  for(int i=0;i<playlist.size();++i)
    out<<playlist[i]<<"\n";

  file.close();
*/
}

//-----------------------------------------------------------------------------------------
/** randomize now trigger */
void MainWindow::randomClicked()
{
//  QMessageBox::warning(this,"Random Player","the Random now was triggered");
}

//-----------------------------------------------------------------------------------------
/** toggle randomizer switch */
void MainWindow::randomizerClicked()
{
//  QMessageBox::warning(this,"Random Player","the Randomize switch was triggered");

}

//-----------------------------------------------------------------------------------------
/** toggle play loop switch */
void MainWindow::loopClicked()
{
//  QMessageBox::warning(this,"Random Player","the loop switch was triggered");
}

