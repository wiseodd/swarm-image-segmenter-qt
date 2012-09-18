#include "main_ui.h"
#include "ui_main_ui.h"
#include <QTime>
#include <QDebug>

MainUI::MainUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainUI)
{
    ui->setupUi(this);

    arr_image_ = NULL;
    flat_datas_ = NULL;
    datas_ = NULL;
}

MainUI::~MainUI()
{
    delete ui;
}

void MainUI::loadImage()
{
    // Open file dialog, only some image files can be selected
    QString file_name;
    file_name = QFileDialog::getOpenFileName(this, "Select image",
                                            "/home/adi/Pictures",
                                            "Image Files (*.png *.jpg *.bmp)");

    // Load image
    IplImage* input = NULL;
    input = cvLoadImage(file_name.toStdString().c_str(), -1);

    int w = width_ = input->width;
    int h = height_ = input->height;
    int c = channel_ = input->nChannels;
    depth_ = input->depth;
    data_size_ = w * h;

    if(w > 640 || h > 480)
    {
        QMessageBox::information(this, "Error",
                                 "Maximum image size is 640x480!",
                                 QMessageBox::Ok);

        return;
    }

    arr_image_ = new char[w * h * c];
    flat_datas_ = new int[w * h * c];
    datas_ = new Data[w * h];

    // Load image to array
    for (int i = 0; i < w * h; i++)
    {
        arr_image_[i * c + 0] = (unsigned char)input->imageData[i * c + 0];
        arr_image_[i * c + 1] = (unsigned char)input->imageData[i * c + 1];
        arr_image_[i * c + 2] = (unsigned char)input->imageData[i * c + 2];

        flat_datas_[i * c + 0] = (unsigned char)input->imageData[i * c + 0];
        flat_datas_[i * c + 1] = (unsigned char)input->imageData[i * c + 1];
        flat_datas_[i * c + 2] = (unsigned char)input->imageData[i * c + 2];

        Data d;

        d.info[0] = (unsigned char)input->imageData[i * c + 0];
        d.info[1] = (unsigned char)input->imageData[i * c + 1];
        d.info[2] = (unsigned char)input->imageData[i * c + 2];

        datas_[i] = d;
    }

    // Show original image
    showImage(input);

    // Cleanup
    cvReleaseImage(&input);
}

QImage* MainUI::IplImage2QImage(IplImage *iplImg)
{
    int h = iplImg->height;
    int w = iplImg->width;
    int channels = iplImg->nChannels;
    QImage *qimg = new QImage(w, h, QImage::Format_ARGB32);
    char *data = iplImg->imageData;

    for(int y = 0; y < h; y++, data += iplImg->widthStep)
    {
        for(int x = 0; x < w; x++)
        {
            char r, g, b, a = 0;

            if(channels == 1)
            {
                r = data[x * channels];
                g = data[x * channels];
                b = data[x * channels];
            }
            else if(channels == 3 || channels == 4)
            {
                r = data[x * channels + 2];
                g = data[x * channels + 1];
                b = data[x * channels];
            }

            if(channels == 4)
            {
                a = data[x * channels + 3];
                qimg->setPixel(x, y, qRgba(r, g, b, a));
            }
            else
                qimg->setPixel(x, y, qRgb(r, g, b));
        }
    }

    return qimg;
}

void MainUI::showImage(IplImage *image)
{
    // Convert IplImage to QImage and show it in QLabel
    QImage *img = IplImage2QImage(image);
    ui->label->setPixmap(QPixmap::fromImage(*img));
}

void MainUI::segmentImage()
{
    // Get parameters
    int particle_size = ui->txtParticle->text().toInt();
    int cluster_size = ui->txtCluster->text().toInt();
    int max_iter = ui->txtIter->text().toInt();
    int cuda_enabled = ui->chkCuda->isChecked();

    GBest gBest;
    QTime timer;
    timer.start();

    // Start PSO
    if(cuda_enabled)
        gBest = engine_.segmentImageDevice(datas_, flat_datas_, data_size_,
                                           particle_size, cluster_size,
                                           max_iter);
    else
        gBest = engine_.segmentImageHost(datas_, data_size_, particle_size,
                                         cluster_size, max_iter);

    float time_elapsed = timer.elapsed();
    float quant_error;

    // Calculate final error
    if(cuda_enabled)
        quant_error = devFitness(gBest.gBestAssign, flat_datas_,
                                 gBest.arrCentroids, data_size_, cluster_size);
    else
        quant_error = fitness(gBest.gBestAssign, datas_, gBest.centroids,
                              data_size_, cluster_size);


    // List for cluster color
    unsigned char colorList[9][3] = { { 0, 0, 255 }, { 255, 0, 0 },
                                     { 0, 255, 0 }, { 255, 255, 0 },
                                     { 255, 0, 255 }, { 255, 128, 128 },
                                     { 128, 128, 128 }, { 128, 0, 0 },
                                     { 255, 128, 0 } };

    // Coloring clusters
    for (int i = 0; i < data_size_; i++)
    {
       for (int j = 0; j < cluster_size; j++)
       {
           if (gBest.gBestAssign[i] == j)
           {
               arr_image_[i * channel_ + 0] = colorList[j][0];
               arr_image_[i * channel_ + 1] = colorList[j][1];
               arr_image_[i * channel_ + 2] = colorList[j][2];
           }
       }
    }

    // Save resulted image
    IplImage* outImage = cvCreateImage(cvSize(width_, height_), depth_,
                                       channel_);
    outImage->imageData = arr_image_;
    cvSaveImage("Result.jpg", outImage);
    // Show image
    showImage(outImage);
    // Print time elapsed and error
    QString message = QString("Quantization error : ")
            + QString::number(quant_error)
            + QString(", Elapsed time : ")
            + QString::number((float)time_elapsed / 1000)
            + QString("s");
    ui->statusBar->showMessage(message);

    cvReleaseImage(&outImage);
}

void MainUI::on_btnStart_clicked()
{
    // Validate input
    if(datas_ == NULL)
    {
        ui->statusBar->showMessage("Load the image first!");
        return;
    }

    if(ui->txtParticle->text().isEmpty())
    {
        ui->statusBar->showMessage("Insert number of particle!");
        return;
    }

    if(ui->txtCluster->text().isEmpty())
    {
        ui->statusBar->showMessage("Insert number of cluster!");
        return;
    }

    if(ui->txtIter->text().isEmpty())
    {
        ui->statusBar->showMessage("Insert max iteration!");
        return;
    }

    segmentImage();
}

void MainUI::on_actionOpen_Image_triggered()
{
    loadImage();
}

void MainUI::on_actionExit_triggered()
{
    // Quit application
    QApplication::quit();
}

void MainUI::on_actionAbout_triggered()
{

}
