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

    int dev_count;
    cudaGetDeviceCount(&dev_count);

    if(dev_count <= 0)
    {
        ui->chkCuda->setEnabled(false);
        ui->lblGraphicCard->setText(ui->lblGraphicCard->text() + "None");
        ui->lblMemory->setText(ui->lblMemory->text() + "None");
    }
    else
    {
        cudaDeviceProp dev_prop;
        cudaGetDeviceProperties(&dev_prop, 0);

        ui->lblGraphicCard->setText(ui->lblGraphicCard->text() + dev_prop.name);
        ui->lblMemory->setText(ui->lblMemory->text()
                               + QString::number(dev_prop.totalGlobalMem / 1000000)
                               + " MB");
    }

    ui->labelProgress->setText("Please load an image!");
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
                                            "Image Files (*.jpg *.bmp)");

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
        Data d;

        for(int j = 0; j < c; j++)
        {
            arr_image_[i * c + j] = (unsigned char)input->imageData[i * c + j];
            flat_datas_[i * c + j] = (unsigned char)input->imageData[i * c + j];
            d.info[j] = (unsigned char)input->imageData[i * c + j];
        }

        datas_[i] = d;
    }

    // Show image informations
    ui->lblImageLoc->setText(QString("Image Location : ") + file_name);
    ui->lblImageSize->setText(QString("Size : ") + QString::number(w)
                              + QString(" x ") + QString::number(h));

    // Show original image
    showImage(input);
    ui->labelProgress->setText("Image loaded");

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
                                           channel_, particle_size, cluster_size,
                                           max_iter);
    else
        gBest = engine_.segmentImageHost(datas_, data_size_, channel_,
                                         particle_size, cluster_size, max_iter);

    float time_elapsed = timer.elapsed();
    float quant_error = gBest.quantError;

    ui->labelProgress->setText("Finished!");

    // List for cluster color
    unsigned char colorList[9][3] = { { 0, 0, 255 }, { 255, 0, 0 },
                                     { 0, 255, 0 }, { 255, 255, 0 },
                                     { 255, 0, 255 }, { 255, 128, 128 },
                                     { 128, 128, 128 }, { 128, 0, 0 },
                                     { 255, 128, 0 } };

    // Resulted image always have 3 channel, RGB, to distinguish between cluster
    int channel = 3;
    char *res_image = new char[width_ * height_ * channel];

    // Coloring clusters
    for (int i = 0; i < data_size_; i++)
    {
       for (int j = 0; j < cluster_size; j++)
       {
           if (gBest.gBestAssign[i] == j)
           {
               res_image[i * channel + 0] = colorList[j][0];
               res_image[i * channel + 1] = colorList[j][1];
               res_image[i * channel + 2] = colorList[j][2];
           }
       }
    }

    // Save resulted image
    IplImage* outImage = cvCreateImage(cvSize(width_, height_), depth_, channel);
    outImage->imageData = res_image;
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

    if(cuda_enabled)
        cudaFreeHost(gBest.arrCentroids);
    else
        delete[] gBest.centroids;

    delete[] gBest.gBestAssign;
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
