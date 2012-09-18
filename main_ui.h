#ifndef MAIN_UI_H
#define MAIN_UI_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "pso_cluster.h"
#include "segmenter_engine.h"

namespace Ui {
    class MainUI;
}

class MainUI : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainUI(QWidget *parent = 0);
    ~MainUI();

private:
    void loadImage();
    void showImage(IplImage *image);
    void segmentImage();
    QImage *IplImage2QImage(IplImage *iplImg);

private slots:
    void on_btnStart_clicked();
    void on_actionOpen_Image_triggered();
    void on_actionAbout_triggered();
    void on_actionExit_triggered();

private:
    Ui::MainUI *ui;
    SegmenterEngine engine_;
    char *arr_image_;
    int *flat_datas_;
    Data *datas_;
    int width_;
    int height_;
    int channel_;
    int depth_;
    int data_size_;
};

#endif // MAIN_UI_H
