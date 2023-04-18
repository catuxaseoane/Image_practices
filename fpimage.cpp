#include "fpimage.h"
#include "ui_fpimage.h"

#include <QFileDialog>
#include <QPainter>
#include "tgmath.h"

//OCV #include <opencv2/opencv.hpp>
//OCV #include "opencv2/imgproc.hpp"

//OCV using namespace cv;

//--------------------------------------------------
//-- Filtro de eventos para capturar mouse clicks --
//--------------------------------------------------
bool FPImage::eventFilter(QObject *Ob, QEvent *Ev)
{
    // Comprobamos que el evento capturado es un  mouseclick
    if(Ev->type()==QEvent::MouseButtonPress) {
        // Comprobamos que el click ocurrió sobre nuestro QLabel
        if(Ob==ui->Ecran) {
            // Hacemos un cast del evento para poder acceder a sus propiedades
            const QMouseEvent *me=static_cast<const QMouseEvent *>(Ev);
            // Nos interesan las coordenadas del click
            int y=me->y(), x=me->x();
            // Si estamos fuera de la imagen, nos vamos
            if(y>=H||x>=W) return true;
            // Hacemos algo con las coordenadas y el píxel
            statusBar()->showMessage(QString::number(x)+":"+
                                      QString::number(y)+" "+
                                      QString::number(pixR[(y*S+3*x)])+":"+
                                      QString::number(pixG[(y*S+3*x)])+":"+
                                      QString::number(pixB[(y*S+3*x)]));
            // Devolvemos un "true" que significa que hemos gestionado el evento
            return true;
        } else return false;  // No era para nosotros, lo dejamos en paz
    } else return false;
}

//-------------------------------------------------
//-- Constructor: Conexiones e inicializaciones ---
//-------------------------------------------------
FPImage::FPImage(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FPImage)
{
    ui->setupUi(this);

    // CONEXIONES de nuestros objetos (botones, etc...) a nuestros slots
    connect(ui->BLoad,SIGNAL(clicked()),this,SLOT(Load()));
    connect(ui->BDoIt,SIGNAL(clicked()),this,SLOT(DoIt()));


    // "Instalamos" un "filtro de eventos" en nuestro QLabel Ecran
    // para capturar clicks de ratón sobre la imagen
    ui->Ecran->installEventFilter(this);


    // INICIALIZACIONES
    W=H=0;      // Empezamos sin imagen cargada
    Path="..";  // Carpeta inicial
    ImageAux = NULL;

    Xw = 0.4360747*255 + 0.3850649*255 + 0.1430804*255;
    Yw = 0.2225045*255 + 0.7168786*255 + 0.0606169*255;
    Zw = 0.0139322*255 + 0.0971045*255 + 0.7141733*255;
    Xb = Yb = Zb= 0;

    m = 1;
    b = 0;
    nivelDeteccion = 20;
    selectorNorma = 1;

    // Inicializamos a negro los lienzos (QPixmap) y los asociamos a las "pantallas" gráficas (QLabel)
    //   Le damos tamaño
    Dib1=QPixmap(256,100);
    //   Lo pintamos de negro
    Dib1.fill(Qt::black);
    //   Lo asignamos a un QLabel
    ui->EcranHistoR->setPixmap(Dib1);

    //   Idem
    Dib2=QPixmap(256,100);
    Dib2.fill(Qt::black);
    ui->EcranHistoG->setPixmap(Dib2);

    // De ídem
    Dib3=QPixmap(256,100);
    Dib3.fill(Qt::black);

    ui->EcranHistoB->setPixmap(Dib3);
    // Ejemplo de cómo dibujar usando funciones de alto nivel (QPainter)
    //   Declaramos un pintor (QPainter) y lo asociamos a un lienzo (QPixmap)
    /*QPainter p(&Dib3);
    //   Escogemos un lápiz (QPen; también hay pinceles, QBrush, para los rellenos)
    p.setPen(QPen(Qt::yellow));
    //   Trazamos un par de líneas, por ejemplo
    p.drawLine(0,0,255,99);

    */
}

//-------------------------------------------------
//------ Destructor: Limpieza antes de salir ------
//-------------------------------------------------
FPImage::~FPImage()
{
    delete ui;
    if (ImageAux) delete [] ImageAux;
}

//-------------------------------------------------
//----------- Carga una imagen de disco -----------
//-------------------------------------------------
void FPImage::Load(void)
{
    // Permite al usuario escoger un fichero de imagen
    QString file=QFileDialog::getOpenFileName(this,tr("Abrir imagen"),Path,tr("Image Files (*.png *.jpg *.bmp)"));
    // Si no escogió nada, nos vamos
    if(file.isEmpty()) return;

    // Creamos un QFileInfo para manipular cómodamente el nombre del fichero a efectos informativos
    // Por ejemplo deshacernos del path para que el nombre no ocupe demasiado
    QFileInfo finfo(file);
    // Memorizamos la carpeta usando la variable global Path, para la próxima vez
    Path=finfo.path();
    // Ponemos el nombre del fichero en el recuadro de texto
    ui->EFile->setText(finfo.fileName());
    // Decoración: Añadimos el nombre del fichero al título de la ventana
    setWindowTitle("FPImage v0.1b - "+finfo.fileName());

    // Cargamos la imagen a nuestra variable "Image" usando la función apropiada de la clase QImage
    Image.load(file);
    // Convertimos a RGB (eliminamos el canal A)
    Image=Image.convertToFormat(QImage::Format_RGB888);

    // Almacenamos las dimensiones de la imagen
    W=Image.width();
    H=Image.height();


    // Ponemos nuestros punteros apuntando a cada canal del primer píxel
    pixB=(pixG=(pixR=Image.bits())+1)+1;


    // Ojo! La imagen puede llevar "relleno" ("zero padding"):
    // Longitud en bytes de cada línea incluyendo el padding
    S=Image.bytesPerLine();
    // Padding
    Padding=S-3*W;

    if (ImageAux)
        delete [] ImageAux;
    ImageAux = new uchar[S*H];
    oriB=(oriG=(oriR=ImageAux)+1)+1;
    memcpy(ImageAux, pixR, S*H);

    // Creamos una Mat de OpenCV (Ima) que "encapsula" los pixels de la QImage Image
//OCV     Ima=Mat(H,W,CV_8UC3,pixR,S);

    // Mostramos algo de texto informativo
    ui->ERes->appendPlainText("Loaded "+finfo.fileName());
    ui->ERes->appendPlainText("Size "+QString::number(W)+"x"+QString::number(H));
    ui->ERes->appendPlainText("Padded length "+QString::number(S));
    ui->ERes->appendPlainText("Pad "+QString::number(Padding));
    ui->ERes->appendPlainText("");

    // Ponemos algo en la barra de estado
    statusBar()->showMessage("Loaded.");

    // Ajustamos el tamaño de la "pantalla" al de la imagen
    ui->Ecran->setFixedWidth(W);
    ui->Ecran->setFixedHeight(H);

    memset(histoR, 0, 256 * sizeof (int));
    memset(histoG, 0, 256 * sizeof (int));
    memset(histoB, 0, 256 * sizeof (int));

    // Volcamos la imagen a pantalla
    ShowIt();
}

//-------------------------------------------------
//------------- Jugamos con la imagen -------------
//-------------------------------------------------
void FPImage::DoIt(void)
{
    // Nos aseguramos de que hay una imagen cargada
    if(!H) return;

    // Ejemplo de procesamiento A BAJO NIVEL
    //   Recorremos toda la imagen manipulando los píxeles uno a uno
    //   Atención a los límites y a los saltos de 3 en 3 (3 canales)
    for (int y=0,i=0 ; y<H ; y++,i+=Padding)
        for (int x=0 ; x<W ; x++,i+=3) {
            pixR [i] = 255 - pixR [i];
            pixG [i] = 255 - pixG [i];
            pixB [i] = 255 - pixB [i];
    }


    // Ejemplo de procesamiento CON OpenCV
//OCV     Mat radio5(11,11,CV_8U,Scalar(0));
//OCV     circle(radio5,Point(5,5),5,Scalar(1),-1);
//OCV     erode(Ima,Ima,radio5);

    // Sacamos algo de texto informativo
    ui->ERes->appendPlainText("Did it");

    // Ponemos algo en la barra de estado
    statusBar()->showMessage("Did it.");

    // Volcamos la imagen a pantalla
    // OJO: Si os olvidáis de esto, la imagen en pantalla no refleja los cambios y
    // pensaréis que no habéis hecho nada, pero Image e Ima (que son la misma) sí
    // que han cambiado aunqu eno lo veáis
    ShowIt();
}

//-------------------------------------------------
//-------------- Mostramos la imagen --------------
//-------------------------------------------------
inline void FPImage::ShowIt (void)
{
    // Creamos un lienzo (QPixmap) a partir de la QImage
    // y lo asignamos a la QLabel central
    ui->Ecran->setPixmap(QPixmap::fromImage(Image));

    FPImage::calcularHistograma();


}

void FPImage::on_BBW_clicked()
{
    if (!H) return;

    for (int y=0,i=0 ; y<H ; y++,i+=Padding)
        for (int x=0 ; x<W ; x++,i+=3) {
            uchar inputRed = pixR[i], inputGreen = pixG[i], inputBlue = pixB[i];

            if (inputGreen + 11 > inputRed)
                pixR[i] = pixG[i] = pixB[i] = inputRed*0.299+inputGreen*0.587+inputBlue*0.114;

    }

    // Sacamos algo de texto informativo
    ui->ERes->appendPlainText("Did it");

    // Ponemos algo en la barra de estado
    statusBar()->showMessage("Did it.");

    ShowIt();

    // recuperas imagen original
}

void FPImage::on_Luz_valueChanged (int value)
{
    if (!H) return;
    b = value;
    ByC();
}

void FPImage::on_Contraste_valueChanged(int value)
{
    if (!H) return;
    m = (259 * (double(value) + 255)) / (255 * (259 - double(value)));
    ByC();
}

void FPImage::ByC(void)
{
    if (!H) return;
    double newLUT;
    for (int n = 0; n < 255; n++)
    {
        newLUT = (m * (double(n) - 128) + 128) + b;
        LUT[n] = newLUT < 0 ? 0 : newLUT>255 ? 255 : newLUT;
    }

    for (int y=0,i=0 ; y<H ; y++,i+=Padding)
        for (int x=0 ; x<W ; x++,i+=3){
            pixR[i] = LUT[oriR[i]];
            pixG[i] = LUT[oriG[i]];
            pixB[i] = LUT[oriB[i]];
    }

    ShowIt();
}

void FPImage::on_Edges_pressed()
{
    if (!H) return;
    double distIzq, distAb;
    for (int y=0,i=0 ; y<H ; y++,i+=Padding)
        for (int x=0 ; x<W ; x++,i+=3){
            pixR[i] = pixG[i] = pixB[i] = 255;

            distIzq = sqrt((oriR[i]-oriR[i-3])*(oriR[i]-oriR[i-3]) +
                           (oriG[i]-oriG[i-3])*(oriG[i]-oriG[i-3]) +
                           (oriB[i]-oriB[i-3])*(oriB[i]-oriB[i-3]));

            if (x != 0)
            {
                if (y != H-1)
                {
                    distAb = sqrt((oriR[i]-oriR[i+S])*(oriR[i]-oriR[i+S]) +
                            (oriG[i]-oriG[i+S])*(oriG[i]-oriG[i+S]) +
                            (oriB[i]-oriB[i+S])*(oriB[i]-oriB[i+S]));
                    if (distIzq > nivelDeteccion ||
                        distAb > nivelDeteccion )
                       pixR[i] = pixG[i] = pixB[i] = 0;
                }
                else
                {
                    if (distIzq > nivelDeteccion)
                       pixR[i] = pixG[i] = pixB[i] = 0;
                }
            }
    }
    ShowIt();
}

void FPImage::on_Edges_released()
{
    if (!H) return;
    memcpy(pixR, ImageAux, S*H);
    ShowIt();
}

void FPImage::on_deteccion_valueChanged(int value)
{
    if (!H) return;
    nivelDeteccion = value;
    on_Edges_pressed();
}

void FPImage::on_deteccion_sliderReleased()
{
    if (!H) return;
    on_Edges_released();
}

void FPImage::on_deteccionSAM_valueChanged(int value)
{
    if (!H) return;
    nivelDeteccion = value;
    double cosinIzq, cosinAb;
    double modPix, modPixIz, modPixAb;
    for (int y=0,i=0 ; y<H ; y++,i+=Padding)
        for (int x=0 ; x<W ; x++,i+=3){
            pixR[i] = pixG[i] = pixB[i] = 255;

            modPix   = sqrt (double(oriR[i])*oriR[i] + oriB[i]*oriB[i] + oriG[i]*oriG[i]);
            modPixIz = sqrt (double(oriR[i-3])*oriR[i-3] + oriB[i-3]*oriB[i-3] + oriG[i-3]*oriG[i-3]);

            cosinIzq = (double(oriR[i])*oriR[i-3] + oriB[i]*oriB[i-3] + oriG[i]*oriG[i-3])/modPix/modPixIz;

            if (x != 0) {
                if (y != H-1) {
                    if (cosinIzq*1000000000 < nivelDeteccion)
                        pixR[i] = pixG[i] = pixB[i] = 0;
                    else {
                        modPixAb = sqrt (double(oriR[i+S])*oriR[i+S] + oriB[i+S]*oriB[i+S] + oriG[i+S]*oriG[i+S]);
                        cosinAb = (double(oriR[i])*oriR[i+S] + oriB[i]*oriB[i+S] + oriG[i]*oriG[i+S])/modPix/modPixAb;
                        if (cosinAb*1000000000  < nivelDeteccion )
                            pixR[i] = pixG[i] = pixB[i] = 0;
                    }
                } else {
                    if (cosinIzq*1000000000< nivelDeteccion)
                       pixR[i] = pixG[i] = pixB[i] = 0;
                }
            }
    }
    ShowIt();

}

void FPImage::on_deteccionSAM_sliderReleased()
{
    if (!H) return;
    on_Edges_released();
}

void FPImage::on_deteccionSAM_2_sliderReleased()
{
    on_Edges_released();
}

void FPImage::on_deteccionSAM_2_valueChanged(int value)
{
    if (!H) return;
    nivelDeteccion = value;
    double cosinIzq, cosinAb;
    double modPix, modPixIz, modPixAb;
    double xw, yw;
    double X, Y, Z, Xiz, Yiz, Ziz, Xab, Yab, Zab;
    double x, y, xiz, yiz, xab, yab;

    for (int columnas=0,i=0 ; columnas<H ; columnas++,i+=Padding)
        for (int filas=0 ; filas<W ; filas++,i+=3){

            X = 0.4360747*oriR[i] + 0.3850649*oriG[i] + 0.1430804*oriB[i];
            Y = 0.2225045*oriR[i] + 0.7168786*oriG[i] + 0.0606169*oriB[i];
            Z = 0.0139322*oriR[i] + 0.0971045*oriG[i] + 0.7141733*oriB[i];
            x = X/(X+Y+Z);
            y = Y/(X+Y+Z);
            Xiz = 0.4360747*oriR[i-3] + 0.3850649*oriG[i-3] + 0.1430804*oriB[i-3];
            Yiz = 0.2225045*oriR[i-3] + 0.7168786*oriG[i-3] + 0.0606169*oriB[i-3];
            Ziz = 0.0139322*oriR[i-3] + 0.0971045*oriG[i-3] + 0.7141733*oriB[i-3];
            xiz = Xiz/(Xiz+Yiz+Ziz);
            yiz = Yiz/(Xiz+Yiz+Ziz);

            xw = Xw/(Xw+Yw+Zw);
            yw = Yw/(Xw+Yw+Zw);

            modPix   = sqrt ((x-xw)*(x-xw) + (y-yw)*(y-yw));
            modPixIz = sqrt ((xiz-xw)*(xiz-xw) + (yiz-yw)*(yiz-yw));
            cosinIzq = ((x-xw))*(xiz-xw) + ((y-yw)*(yiz-yw)) / modPix / modPixIz;

            pixR[i] = pixG[i] = pixB[i] = 255;

            if (filas != 0) {
                if (columnas != H-1) {
                    if (cosinIzq * 1000000000 < nivelDeteccion)
                        pixR[i] = pixG[i] = pixB[i] = 0;
                    else {
                        Xab = 0.4360747*oriR[i+S] + 0.3850649*oriG[i+S] + 0.1430804*oriB[i+S];
                        Yab = 0.2225045*oriR[i+S] + 0.7168786*oriG[i+S] + 0.0606169*oriB[i+S];
                        Zab = 0.0139322*oriR[i+S] + 0.0971045*oriG[i+S] + 0.7141733*oriB[i+S];
                        xab = Xiz/(Xab+Yab+Zab);
                        yab = Yiz/(Xab+Yab+Zab);

                        modPixAb = sqrt ((xab-xw)*(xab-xw) + (yab-yw)*(yab-yw));
                        cosinAb = ((x-xw)*(xab-xw) + (y-yw)*(yab-yw)) / modPix / modPixAb;

                        if (cosinAb*1000000000  < nivelDeteccion )
                            pixR[i] = pixG[i] = pixB[i] = 0;
                    }
                } else {
                    if (cosinIzq*1000000000< nivelDeteccion)
                       pixR[i] = pixG[i] = pixB[i] = 0;
                }
            }
    }
    ShowIt();


}

void FPImage::on_norma1_clicked()
{
    selectorNorma = 1;
}

void FPImage::on_norma2_clicked()
{
    selectorNorma = 2;
}

void FPImage::on_normaoo_clicked()
{
    selectorNorma = 0;
}

void FPImage::calcularHistograma()
{
    // Calcula el histograma
    int maxR, maxG, maxB;
    maxR = maxG = maxB = 0;

    memset (histoR, 0, 256 * sizeof (int));
    memset (histoG, 0, 256 * sizeof (int));
    memset (histoB, 0, 256 * sizeof (int));

    for (int y=0,i=0 ; y<H ; y++,i+=Padding)
        for (int x=0 ; x<W ; x++,i+=3){
            histoR [pixR[i]] ++;
            histoG [pixG[i]] ++;
            histoB [pixB[i]] ++;
    }

    for(int i=0; i<256; i++){
        maxR = histoR[i] > maxR ? histoR[i] : maxR;
        maxG = histoG[i] > maxG ? histoG[i] : maxG;
        maxB = histoB[i] > maxB ? histoB[i] : maxB;
    }

    for (int i = 0; i<256; i++)
    {

    }


    Dib1=QPixmap(256,100);
    Dib1.fill(Qt::black);
    Dib2=QPixmap(256,100);
    Dib2.fill(Qt::black);
    Dib3=QPixmap(256,100);
    Dib3.fill(Qt::black);

    QPainter p1(&Dib1);
    QPainter p2(&Dib2);
    QPainter p3(&Dib3);
    p1.setPen(QPen(Qt::red));
    p2.setPen(QPen(Qt::green));
    p3.setPen(QPen(Qt::blue));

    for (int i = 0; i < 256; i++){
        histoR[i] *= 1/double(maxR)*99;
        histoG[i] *= 1/double(maxG)*99;
        histoB[i] *= 1/double(maxB)*99;
        p1.drawLine(i,99,i,99-histoR[i]);
        p2.drawLine(i,99,i,99-histoG[i]);
        p3.drawLine(i,99,i,99-histoB[i]);
    }
    ui->EcranHistoR->setPixmap(Dib1);
    ui->EcranHistoG->setPixmap(Dib2);
    ui->EcranHistoB->setPixmap(Dib3);

}

void FPImage::on_estiraLineal_clicked()
{
    int limit = 3;
    int minR = 0; while(histoR[minR]<limit) minR++;
    int maxR = 255; while(histoR[maxR]<limit) maxR--;
    int minG = 0; while(histoR[minG]<limit) minG++;
    int maxG = 255; while(histoR[maxG]<limit) maxG--;
    int minB = 0; while(histoR[minB]<limit) minB++;
    int maxB = 255; while(histoR[maxB]<limit) maxB--;
    int centroR, centroG, centroB;


    for (int y=0,i=0 ; y<H ; y++,i+=Padding)
        for (int x=0 ; x<W ; x++,i+=3){
            pixR[i] = pixR[i]-minR < 0 ? 0 : pixR[i]-minR > 255 ? 255 : pixR[i]-minR;
            pixR[i] = pixR[i]*255.0f/maxR < 0 ? 0 : pixR[i]*255.0f/maxR > 255 ? 255 : pixR[i]*255.0f/maxR;
            pixG[i] = pixG[i]-minG < 0 ? 0 : pixG[i]-minG > 255 ? 255 : pixG[i]-minG;
            pixG[i] = pixG[i]*255.0f/maxG < 0 ? 0 : pixG[i]*255.0f/maxG > 255 ? 255 : pixG[i]*255.0f/maxG;
            pixB[i] = pixB[i]-minB < 0 ? 0 : pixB[i]-minB > 255 ? 255 : pixB[i]-minB;
            pixB[i] = pixB[i]*255.0f/maxB < 0 ? 0 : pixB[i]*255.0f/maxB > 255 ? 255 : pixB[i]*255.0f/maxB;

    }

    ShowIt();

}
