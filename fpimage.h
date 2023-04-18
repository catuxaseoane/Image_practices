#ifndef FPIMAGE_H
#define FPIMAGE_H

//OCV #include <cv.h>
//OCV #include <cxcore.h>
//OCV #include <highgui.h>
//OCV #include <ml.h>

#include <QMainWindow>

namespace Ui {
class FPImage;
}

class FPImage : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit FPImage(QWidget *parent = 0);
    ~FPImage();
    
private:
    Ui::FPImage *ui;

    QString Path;               // Para recordar la carpeta al cargar imágenes
//OCV     cv::Mat Ima;                // La imagen OpenCV (encapsula la imagen Qt)
    QImage Image;               // Imagen Qt
    uchar *pixR, *pixG, *pixB;  // Punteros a los tres canales R, G y B
    uchar *oriR, *oriG, *oriB;
    uchar *ImageAux;
    int W, H;                   // Tamaño de la imagen actual
    int S, Padding;                // Step en imágenes no continuas

    QPixmap Dib1, Dib2, Dib3;   // Tres lienzos en los que dibujar

    void ShowIt(void);          // Muestra la imagen actual

    bool eventFilter(QObject *Ob, QEvent *Ev);  // Un "filtro de eventos"
    double Xw, Yw, Zw, Xb, Yb, Zb;

    double m, b;

    int nivelDeteccion;
    int selectorNorma;

    int histoR[256], histoB[256], histoG[256];

    uchar LUT[256];

private slots:
    void Load (void);    // Slot para el botón de carga de imagen
    void DoIt (void);    // Slot para el botón de hacer algo con la imagen


    void on_BBW_clicked();
    void on_Luz_valueChanged (int);
    void on_Contraste_valueChanged (int);
    void ByC (void);
    void on_Edges_pressed();
    void on_Edges_released();
    void on_deteccion_valueChanged (int);
    void on_deteccion_sliderReleased();
    void on_deteccionSAM_valueChanged (int);
    void on_deteccionSAM_sliderReleased();
    void on_norma1_clicked();
    void on_deteccionSAM_2_sliderReleased();
    void on_deteccionSAM_2_valueChanged (int);
    void on_norma2_clicked (void);
    void on_normaoo_clicked (void);
    void calcularHistograma (void);
    void on_estiraLineal_clicked();
};

#endif // FPIMAGE_H
