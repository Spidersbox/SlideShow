#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"

QT_USE_NAMESPACE
//class QLabel;
//class QScrollArea;
//class QScrollBar;

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

  QTimer *timer;

private:
  Ui::MainWindow *ui;

  double scaleFactor;

  QMenuBar *appMenuBar;
  QImage image;
//  QLabel *imageLabel;
//  QScrollArea *scrollArea;
  QAction *openAction;
  QAction *quitAction;

  /** playlist submenu options */
  QAction *editAction;
  QAction *saveAction;
  QAction *randomAction;
  QAction *randomizerAction;
  QAction *loopAction;

  /** Create the main UI actions. */
  void createActions();

  /** Create the menu bar and sub-menus. */
  void createMenuBar();

  void showImage(QString filen);
  void setImage(const QString &image);
  void nextImage();
//  void scaleImage(double factor);
  void scaleImage();

  void setTimer();
  void stopTimer();


private slots:
  void timerTriggered();

  /** Show open file dialog */
  void openClicked();
  /** exit this app */
  void quitClicked();

  /** show playlist editor */
  void editClicked();
  /** save playlist */
  void saveClicked();
  /** randomize now trigger */
  void randomClicked();
  /** toggle randomizer switch */
  void randomizerClicked();
  /** toggle play loop switch */
  void loopClicked();

};

#endif
