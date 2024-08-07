#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include <QMainWindow>
#include <QListView>
#include "ui_mainwindow.h"

QT_USE_NAMESPACE
//class QLabel;
//class QScrollArea;
//class QScrollBar;
//class QMenu;

class MainWindow : public QMainWindow
{
  Q_OBJECT


public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

  QTimer *timer;
  QStringList playlist;

private:
  Ui::MainWindow *ui;

  double scaleFactor;
  int desktopWidth;
  int desktopHeight;
  int appWidth;
  int appHeight;

  QMenuBar *appMenuBar;
  QImage image;
  QMenu *contextMenu;
  QMenu *mainMenu;
  QAction *openFolderAction;
  QAction *openPlayListAction;
  QAction *openPlayListSubAction;
  QAction *quitAction;

  QAction *pauseAction;
  QAction *continueAction;
  QAction *nextAction;

  /** playlist submenu options */
  QAction *editAction;
  QAction *saveAction;
  QAction *randomAction;
  QAction *randomizerAction;
  QAction *loopAction;
  QAction *fullScreenAction;

  /** Create the main UI actions. */
  void createActions();

  /** Create the menu bar and sub-menus. */
  void createMenuBar();

  void showImage(QString filen);
  void setImage(const QString &image);
  void nextImage();
  void scaleImage();

  void setTimer();
  void stopTimer();


private slots:
  void timerTriggered();

  /** Show open Folder dialog */
  void openFolderClicked();
  /** Show open PlayList dialog */
  void openPlayListClicked();
  /** exit this app */
  void quitClicked();

  /** play options */
  void pauseClicked();
  void continueClicked();
  void nextClicked();

  /** show playlist editor */
  void editClicked();
  /** save playlist */
  void saveClicked();

  /** randomize now trigger */
  void randomClicked();

  /** full screen switch */
  void fullScreenClicked();

  void printData(QStringList);

  /** Spawn contextual menu (right mouse menu) */
  void contextualMenu();

};

#endif
