/*Format Junkie
A tool for converting your media files to
all the popular formats
Copyright © 2012 by Alex Solanos and Leon Vytanos

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.*/

#define QT_NO_KEYWORDS

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "glob.h"

#include <iostream>
#include <unity/unity/unity.h>
#include <libnotify/notify.h>

#include <QIcon>
#include <QUrl>
#include <QFileDialog>
#include <QDirIterator>
#include <QProcess>
#include <QObject>
#include <QRegExp>
#include <QCloseEvent>
#include <QSettings>
#include <QShortcut>
#include <QMessageBox>
#include <QFileInfo>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QDBusInterface>
#include <QDesktopWidget>
#include <QMovie>
#include <QTimer>
#include <QMimeData>

using namespace std;

QProcess *audio_converter=new QProcess(0);
QProcess *video_converter=new QProcess(0);
QProcess *image_converter=new QProcess(0);
QProcess *files_to_iso=new QProcess(0);
QProcess *iso_to_cso=new QProcess(0);
QProcess *cso_to_iso=new QProcess(0);
QProcess *sub_encoding=new QProcess(0);
float duration_secs=0;
QString input_file="";
QString output_file="";
//tmp file to be removed after the wav to m4r conversion
QString output_file_rm="";
QString command="";
QString body="";
QString file_exists="";
QString file_not_exists="";
//glob_pro_label is a string
//which is placed in a completed file
QString globpro_string="100%";
QString all_errors="";
QString current_file_errors="";
QStringList command_arguments;

QString video_resize_value="hd720";
int num_files=0;
int current_file=0;
int quality=0;
int cur_index=0;
int error_count=0;
int image_conversion_result=0;
int show_errors_timer_count=0;
/*AUDIO PREFERENCES START*/
bool audio_bitrate=false;
bool audio_samplerate=false;
bool audio_channels=false;
bool audio_volume=false;
int audio_bitrate_value=198;
int audio_samplerate_value=44100;
int audio_channels_value=2;
int audio_volume_value=256;
int image_width=300;
int image_height=300;
int already_completed=0;
/*AUDIO PREFERENCES END*/
bool m4r_or_a_time=false;
bool m4r_or_a_previous_was_error=false;
bool faac_percent_found=false;
bool full_percent_instant=false;
bool overwrite_existing=false;
bool delete_original=false;
bool open_output=false;
bool audio_stop_pressed=false;
bool video_stop_pressed=false;
bool image_stop_pressed=false;
bool sub_stop_pressed=false;
bool globpro_hasbeendeleted=false;
bool about_shown=false;
bool unity_progressbar=true;
bool unity_count=true;
bool desktop_notifications=true;
bool indicator=false;
bool image_width_height=false;
bool image_respect_ratio=false;
bool image_colors_number=false;
int image_colors_number_value=256;
bool video_crop=false;
int video_width_value=100;
int video_height_value=100;
int video_x_value=0;
int video_y_value=0;
bool video_resize=false;
bool video_bitrate=false;
bool video_framerate=false;
bool image_comment=false;
int video_bitrate_value=3900000;
int video_framerate_value=44100;
QString image_comment_value="";
bool image_crop=false;
bool indicator_has_been_created=false;
bool iso_to_cso_stop_pressed=false;
bool cso_to_iso_stop_pressed=false;

bool conversion_process_on=false;

int image_width_value=100;
int image_height_value=100;
int image_x_value=0;
int image_y_value=0;

int type_timer=0;
int widget_timer=0;

//variable to check if the program should proceed to
//faac conversion after converting to a temp file
//for converting to m4r and m4a files
//The variable becomes false when the return_code of
//ffmpeg is not 0
bool proceed_to_faac_conversion=true;

//notification specific...
bool first_notification=true;
NotifyNotification* notification;
gboolean            success;
GError*             error = NULL;

//unity launcher integration
UnityLauncherEntry *unity_progress;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QRect rect = QApplication::desktop()->availableGeometry();
    this->move(rect.center() - this->rect().center());

    ui->stop->hide();
    ui->stop_2->hide();
    ui->stop_3->hide();
    ui->stop_6->hide();
    ui->progressBar->hide();
    ui->progressBar_2->hide();
    ui->progressBar_3->hide();
    ui->startBtn->setEnabled(false);
    ui->startBtn_2->setEnabled(false);
    ui->startBtn_3->setEnabled(false);
    ui->iso_to_cso_stop->hide();
    ui->cso_to_iso_stop->hide();
    ui->encode_stop->hide();

    on_after_convertion2_currentIndexChanged(ui->after_convertion2->currentIndex());

    //connecting audio...
    QObject::connect(audio_converter, SIGNAL(readyReadStandardOutput()), this, SLOT(audio_read_output()));
    QObject::connect(audio_converter, SIGNAL(readyReadStandardError()), this, SLOT(audio_read_output()));
    QObject::connect(audio_converter, SIGNAL(finished(int)), this, SLOT(audio_conversion(int)));

    //connecting video...
    QObject::connect(video_converter, SIGNAL(readyReadStandardOutput()), this, SLOT(video_read_output()));
    QObject::connect(video_converter, SIGNAL(readyReadStandardError()), this, SLOT(video_read_output()));
    QObject::connect(video_converter, SIGNAL(finished(int)),this, SLOT(video_conversion(int)));

    //connecting image...
    QObject::connect(image_converter, SIGNAL(readyReadStandardOutput()), this, SLOT(image_read_output()));
    QObject::connect(image_converter, SIGNAL(readyReadStandardError()), this, SLOT(image_read_output()));
    QObject::connect(image_converter, SIGNAL(finished(int)),this, SLOT(image_conversion(int)));

    //connection files_to_iso
    QObject::connect(files_to_iso, SIGNAL(finished(int)),this, SLOT(files_to_iso_end(int)));

    //connection iso_to_cso
    QObject::connect(iso_to_cso, SIGNAL(finished(int)),this, SLOT(iso_to_cso_end(int)));

    //connection cso_to_iso
    QObject::connect(cso_to_iso, SIGNAL(finished(int)),this, SLOT(cso_to_iso_end(int)));

    //connecting sub_encoding
    QObject::connect(sub_encoding, SIGNAL(finished(int)),this, SLOT(sub_encoding_end(int)));

    QSettings settings( "FormatJunkie", "MainWindow" );
    ui->outputFormatComboBox->setCurrentIndex(settings.value("output_format_audio").toInt());
    ui->outputFormatComboBox_2->setCurrentIndex(settings.value("output_format_video").toInt());
    ui->outputFormatComboBox_3->setCurrentIndex(settings.value("output_format_image").toInt());
    if(!add_audio && !add_video && !add_image){//<-we will head to the appropriate Index for each of these...
        cur_index=settings.value("current_index",0).toInt();
        ui->stackedWidget->setCurrentIndex(cur_index);
        ui->stackedWidget_3->setCurrentIndex(settings.value("current_index_iso_cso",0).toInt());
        if(ui->stackedWidget_3->currentIndex()==1)
            ui->radioButton_iso_cso->setChecked(true);
        switch(settings.value("current_index",0).toInt()){
        case 0:
            ui->musicButton->setChecked(true);
            break;
        case 1:
            ui->videoButton->setChecked(true);
            break;
        case 2:
            ui->imageButton->setChecked(true);
            break;
        case 3:
            ui->rom_deviceButton->setChecked(true);
            break;
        case 4:
            ui->advancedButton->setChecked(true);
            break;
        default:
            ui->musicButton->setChecked(true);
            break;
        }
    }
    else
    {
        //a command line argument is hereee (checked from main.cpp)!
        if(add_audio){
            add_audio_files();
        }
        else if(add_video){
            add_video_files();
        }
        else if(add_image){
            add_image_files();
        }
    }

    QSettings settings2( "FormatJunkie", "Preferences" );
    //GENERAL
    quality=settings2.value("quality", 100).toInt();
    if (quality<0 || quality>100)
        quality=100;
    overwrite_existing=settings2.value("overwrite", false).toBool();
    delete_original=settings2.value("deleteoriginal", false).toBool();
    unity_progressbar=settings2.value("unity_progressbar", true).toBool();
    unity_count=settings2.value("unity_count",true).toBool();
    desktop_notifications=settings2.value("desktop_notifications", true).toBool();
    indicator=settings2.value("indicator", false).toBool();
    //AUDIO
    audio_bitrate=settings2.value("audio_bitrate", false).toBool();
    audio_samplerate=settings2.value("audio_sample_rate", false).toBool();
    audio_channels=settings2.value("audio_channels", false).toBool();
    audio_volume=settings2.value("audio_volume", false).toBool();
    audio_bitrate_value=settings2.value("audio_bitrate_spinbox", 198).toInt();
    audio_samplerate_value=settings2.value("audio_sample_rate_spinbox", 44100).toInt();
    audio_channels_value=settings2.value("audio_channels_spinbox", 2).toInt();
    audio_volume_value=settings2.value("audio_volume_spinbox", 256).toInt();
    //VIDEO
    video_bitrate=settings2.value("video_bitrate", false).toBool();
    video_bitrate_value=settings2.value("video_bitrate_spinbox", 3900000).toInt();
    video_framerate=settings2.value("video_frame_rate", false).toBool();
    video_crop=settings2.value("video_crop", false).toBool();
    video_width_value=settings2.value("video_width_spinbox", 100).toInt();
    video_height_value=settings2.value("video_height_spinbox", 100).toInt();
    video_x_value=settings2.value("video_x_spinbox", 100).toInt();
    video_y_value=settings2.value("video_y_spinbox", 100).toInt();
    video_resize=settings2.value("video_resize", false).toBool();
    if(video_resize){
        set_video_resize_value(settings.value("video_resize_index",0).toInt());
    }
    //IMAGE
    image_width_height=settings2.value("image_width_height", false).toBool();
    image_width=settings2.value("image_width", 300).toInt();
    image_height=settings2.value("image_height", 300).toInt();
    image_respect_ratio=settings2.value("image_respect_ratio", false).toBool();
    image_colors_number=settings2.value("image_colors_number", false).toBool();
    image_colors_number_value=settings2.value("image_colors_number_spinbox", 256).toBool();
    image_comment=settings2.value("image_comment", false).toBool();
    image_comment_value=settings2.value("image_comment_lineedit", "").toString();
    image_crop=settings2.value("image_crop", false).toBool();
    image_width_value=settings2.value("image_width_spinbox", 100).toInt();
    image_height_value=settings2.value("image_height_spinbox", 100).toInt();
    image_x_value=settings2.value("image_x_spinbox", 100).toInt();
    image_y_value=settings2.value("image_y_spinbox", 100).toInt();

    QString qoutput_folder=settings.value("output",QDir::homePath()+"/Documents/FJOutput").toString();
    //crop the output folder if it doesn't fit...
    crop_output_folder(qoutput_folder);

    ui->tableWidget->setColumnWidth(0, 370);
    ui->tableWidget->verticalHeader()->hide();
    ui->tableWidget_2->setColumnWidth(0,370);
    ui->tableWidget_2->verticalHeader()->hide();
    ui->tableWidget_3->setColumnWidth(0,370);
    ui->tableWidget_3->verticalHeader()->hide();
    ui->tableWidget_6->setColumnWidth(0,380);
    ui->tableWidget_6->verticalHeader()->hide();

    globpro = new QProgressBar(this);
    globpro->hide();

    ui->show_errors->hide();
    setup_shortcuts();
    QDBusConnection::sessionBus().connect(QString(),QString(), "do.action", "formatjunkie_message", this, SLOT(dbus_action(QString)));
    unity_progress = unity_launcher_entry_get_for_desktop_id("formatjunkie.desktop");

    //accept drag en drop
    setAcceptDrops(true);

    //creating the indicator

    show_errors_timer = new QTimer(this);
    connect(show_errors_timer,SIGNAL(timeout()),this,SLOT(show_errors_animation()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent( QCloseEvent * ){
    QApplication::setQuitOnLastWindowClosed(true);
    QSettings settings( "FormatJunkie", "MainWindow" );
    settings.setValue( "output", ui->output_folder->toolTip() );
    settings.setValue( "output_format_audio", ui->outputFormatComboBox->currentIndex() );
    settings.setValue( "output_format_video", ui->outputFormatComboBox_2->currentIndex() );
    settings.setValue( "output_format_image", ui->outputFormatComboBox_3->currentIndex() );
    settings.setValue( "current_index", cur_index );
    settings.setValue( "current_index_iso_cso", ui->stackedWidget_3->currentIndex());
    settings.sync();
}

void MainWindow::setup_shortcuts(){
    (void) new QShortcut(Qt::CTRL + Qt::Key_P, this, SLOT(on_actionPreferences_triggered()));
    (void) new QShortcut(Qt::CTRL + Qt::Key_Q, this, SLOT(close()));
    (void) new QShortcut(Qt::CTRL + Qt::Key_M, this, SLOT(add_audio_files()));
    (void) new QShortcut(Qt::CTRL + Qt::Key_V, this, SLOT(add_video_files()));
    (void) new QShortcut(Qt::CTRL + Qt::Key_I, this, SLOT(add_image_files()));
    (void) new QShortcut(Qt::Key_F1, this, SLOT(on_actionContents_F1_triggered()));
    (void) new QShortcut(Qt::Key_Delete, this, SLOT(on_removeBtn_clicked()));
    (void) new QShortcut(Qt::ALT + Qt::Key_1, this, SLOT(goto_1()));
    (void) new QShortcut(Qt::ALT + Qt::Key_2, this, SLOT(goto_2()));
    (void) new QShortcut(Qt::ALT + Qt::Key_3, this, SLOT(goto_3()));
    (void) new QShortcut(Qt::ALT + Qt::Key_4, this, SLOT(goto_4()));
    (void) new QShortcut(Qt::ALT + Qt::Key_5, this, SLOT(goto_5()));
}

//drag and drop
void MainWindow::dragEnterEvent(QDragEnterEvent *event){
    event->acceptProposedAction();
}


void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urlList = event->mimeData()->urls();
    QStringList audio,video,image;

    for (QList<QUrl>::const_iterator i = urlList.begin(); i != urlList.end();i++)
    {
       QString received_file = (*i).toString();
       received_file=received_file.mid(7,received_file.length()-2);
       if(received_file.endsWith(".mp3") || received_file.endsWith(".MP3") || received_file.endsWith(".wav") || received_file.endsWith(".WAV") || received_file.endsWith(".ogg") || received_file.endsWith(".OGG") || received_file.endsWith(".wma") || received_file.endsWith(".WMA") || received_file.endsWith(".flac") || received_file.endsWith(".FLAC") || received_file.endsWith(".aac") || received_file.endsWith(".AAC") || received_file.endsWith(".mp2") || received_file.endsWith(".MP2") || received_file.endsWith(".wv") || received_file.endsWith(".WV")){
          audio << received_file;

       }
       else if(received_file.endsWith(".avi") || received_file.endsWith(".AVI") || received_file.endsWith(".wmv") || received_file.endsWith(".WMV") || received_file.endsWith(".ogv") || received_file.endsWith(".OGV") || received_file.endsWith(".vob") || received_file.endsWith(".VOB") || received_file.endsWith(".3gp") || received_file.endsWith(".3GP") || received_file.endsWith(".mkv") || received_file.endsWith(".MKV") || received_file.endsWith(".mpg") || received_file.endsWith(".MPG") || received_file.endsWith(".flv") || received_file.endsWith(".FLV") || received_file.endsWith(".mov") || received_file.endsWith(".MOV")){
           video << received_file;
       }
       else if(received_file.endsWith(".png") || received_file.endsWith(".gif") || received_file.endsWith(".bmp") || received_file.endsWith(".jpg") || received_file.endsWith(".jpeg") || received_file.endsWith(".svg") || received_file.endsWith(".PNG") || received_file.endsWith(".GIF") || received_file.endsWith(".BMP") || received_file.endsWith(".JPG") || received_file.endsWith(".JPEG") || received_file.endsWith(".SVG")){
           image << received_file;
       }
       else if(QDir(received_file).exists()){
           //a folder has been dropped, process it!
           process_dir(received_file);
       }
    }
    int audio_count=audio.count();
    int video_count=video.count();
    int image_count=image.count();
    if(audio_count){
        int old_r_count=ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+audio_count);
        int new_r_count=ui->tableWidget->rowCount();
        QFileInfo inf;
        for(int i=old_r_count;i<new_r_count;i++){
            QString file=audio.at(i-old_r_count);
            inf.setFile(file);
            QTableWidgetItem *qtitem = new QTableWidgetItem;
            qtitem->setText(inf.completeBaseName()+"."+inf.suffix());
            qtitem->setToolTip(file);
            ui->tableWidget->setItem(i,0,qtitem);
            QTableWidgetItem *qtsize = new QTableWidgetItem;
            float size=(float)inf.size()/1048576;
            qtsize->setText(QString::number(size, 'f', 2)+" MB");
            ui->tableWidget->setItem(i,1,qtsize);
            QTableWidgetItem *qtwait = new QTableWidgetItem;
            qtwait->setText("Waiting");
            ui->tableWidget->setItem(i,2,qtwait);
        }
        if(ui->outputFormatComboBox->isEnabled())
            ui->startBtn->setEnabled(true);
        else if(unity_progressbar)
            update_unity_count_value(2);
    }
    if(video_count){
        int old_r_count=ui->tableWidget_2->rowCount();
        ui->tableWidget_2->setRowCount(ui->tableWidget_2->rowCount()+video_count);
        int new_r_count=ui->tableWidget_2->rowCount();
        QFileInfo inf;
        for(int i=old_r_count;i<new_r_count;i++){
            QString file=video.at(i-old_r_count);
            inf.setFile(file);
            QTableWidgetItem *qtitem = new QTableWidgetItem;
            qtitem->setText(inf.completeBaseName()+"."+inf.suffix());
            qtitem->setToolTip(file);
            ui->tableWidget_2->setItem(i,0,qtitem);
            QTableWidgetItem *qtsize = new QTableWidgetItem;
            float size=(float)inf.size()/1048576;
            qtsize->setText(QString::number(size, 'f', 2)+" MB");
            ui->tableWidget_2->setItem(i,1,qtsize);
            QTableWidgetItem *qtwait = new QTableWidgetItem;
            qtwait->setText("Waiting");
            ui->tableWidget_2->setItem(i,2,qtwait);
        }
        if(ui->outputFormatComboBox_2->isEnabled())
            ui->startBtn_2->setEnabled(true);
        else if(unity_progressbar)
            update_unity_count_value(2);
    }
    if(image_count){
        int old_r_count=ui->tableWidget_3->rowCount();
        ui->tableWidget_3->setRowCount(ui->tableWidget_3->rowCount()+image_count);
        int new_r_count=ui->tableWidget_3->rowCount();
        QFileInfo inf;
        for(int i=old_r_count;i<new_r_count;i++){
            QString file=image.at(i-old_r_count);
            inf.setFile(file);
            QTableWidgetItem *qtitem = new QTableWidgetItem;
            qtitem->setText(inf.completeBaseName()+"."+inf.suffix());
            qtitem->setToolTip(file);
            ui->tableWidget_3->setItem(i,0,qtitem);
            QTableWidgetItem *qtsize = new QTableWidgetItem;
            float size=(float)inf.size()/1048576;
            qtsize->setText(QString::number(size, 'f', 3)+" MB");
            ui->tableWidget_3->setItem(i,1,qtsize);
            QTableWidgetItem *qtwait = new QTableWidgetItem;
            qtwait->setText("Waiting");
            ui->tableWidget_3->setItem(i,2,qtwait);
        }
        if(ui->outputFormatComboBox_3->isEnabled())
            ui->startBtn_3->setEnabled(true);
        else if(unity_progressbar)
            update_unity_count_value(2);
    }
}

void MainWindow::set_video_resize_value(int index_value){
    switch(index_value)
    {
    case 0:
        video_resize_value="sqcif";
        break;
    case 1:
        video_resize_value="qqvga";
        break;
    case 2:
        video_resize_value="qcif";
        break;
    case 3:
        video_resize_value="cga";
        break;
    case 4:
        video_resize_value="qvga";
        break;
    case 5:
        video_resize_value="cif";
        break;
    case 6:
        video_resize_value="4cif";
        break;
    case 7:
        video_resize_value="ega";
        break;
    case 8:
        video_resize_value="vga";
        break;
    case 9:
        video_resize_value="svga";
        break;
    case 10:
        video_resize_value="hd480";
        break;
    case 11:
        video_resize_value="wvga";
        break;
    case 12:
        video_resize_value="xga";
        break;
    case 13:
        video_resize_value="hd720";
        break;
    case 14:
        video_resize_value="sxga";
        break;
    case 15:
        video_resize_value="wxga";
        break;
    case 16:
        video_resize_value="16cif";
        break;
    case 17:
        video_resize_value="uxga";
        break;
    case 18:
        video_resize_value="wsxga";
        break;
    case 19:
        video_resize_value="hd1080";
        break;
    case 20:
        video_resize_value="wuxga";
        break;
    case 21:
        video_resize_value="qxga";
        break;
    case 22:
        video_resize_value="woxga";
        break;
    case 23:
        video_resize_value="qsxga";
        break;
    case 24:
        video_resize_value="wqsxga";
        break;
    case 25:
        video_resize_value="wquxga";
        break;
    case 26:
        video_resize_value="hsxga";
        break;
    case 27:
        video_resize_value="whsxga";
        break;
    case 28:
        video_resize_value="whuxga";
        break;
    }
}

void MainWindow::add_audio_files(){
    cur_index=0;
    ui->stackedWidget->setCurrentIndex(0);
    ui->musicButton->setChecked(true);
    on_addBtn_clicked();
}

void MainWindow::add_video_files(){
    cur_index=1;
    ui->stackedWidget->setCurrentIndex(1);
    ui->videoButton->setChecked(true);
    on_addBtn_clicked();
}

void MainWindow::add_image_files(){
    cur_index=2;
    ui->stackedWidget->setCurrentIndex(2);
    ui->imageButton->setChecked(true);
    on_addBtn_clicked();
}

void MainWindow::crop_output_folder(QString output_folder_location){

    if(output_folder_location.count()>30){
        QString print_to_btn=output_folder_location.left(12)+"..."+output_folder_location.right(13);
        ui->output_folder->setText(print_to_btn);
    }
    else
        ui->output_folder->setText(output_folder_location);
    ui->output_folder->setToolTip(output_folder_location);
}

void MainWindow::audio_read_output(){
    //read all possible output so as to calculate the percentages...
    QByteArray newData_stdout=audio_converter->readAllStandardOutput();
    QString stdout=QString::fromLocal8Bit(newData_stdout);
    QByteArray newData_stderr=audio_converter->readAllStandardError();
    QString stderr=QString::fromLocal8Bit(newData_stderr);
    QString out=stdout+stderr;
    current_file_errors+=out;
    int format_index=ui->outputFormatComboBox->currentIndex();
        if((format_index==5 || format_index==7) && m4r_or_a_time==false && out.contains("%)|")){
            //M4R/M4A MUSIC FILES
            faac_percent_found=true;
            QRegExp regex("\\(.*%");//("  (.*%)|");
            regex.setMinimal(true);
            regex.indexIn(out);
            QString unformatted=regex.capturedTexts().at(0);
            unformatted.replace("%","");
            unformatted.replace("( ","");
            int percentage=unformatted.toInt();
            if(percentage!=0){
                int simplified = 20+percentage/1.25;
                globpro->setFormat(QString::number(simplified)+"%");
                globpro->setValue(20+percentage/1.25);
                ui->progressBar->setFormat(QString::number(simplified/(num_files-already_completed)+(current_file-1-already_completed)*(100/(num_files-already_completed)))+"%");
                ui->progressBar->setValue(simplified/(num_files-already_completed)+(current_file-1-already_completed)*(100/(num_files-already_completed)));
            }
        }
        else
        {
            //ALL OTHER MUSIC FILES
            //duration has been checked
            //Now process the output so as to get how much of the duration of
            //the output file has been processed so as to print the percentage
            if(out.contains("kB time=")){
                QRegExp regex("time=.* ");
                regex.setMinimal(true);

                QStringList list;
                int pos = 0;

                while ((pos = regex.indexIn(out, pos)) != -1)
                {
                    list << regex.cap(0).remove(QChar(' '),Qt::CaseInsensitive);
                    pos += regex.matchedLength();
                }
                if(!list.count()){
                    //error
                    return;
                }
                QString current=list.at(0);
                current.replace("time=","");
                float cur_time=current.toFloat();
                int percent=0;
                if(format_index==5 || format_index==7)
                    percent=(cur_time*100/duration_secs)/5;
                else
                    percent=cur_time*100/duration_secs;

                globpro->setFormat(QString::number(percent)+"%");
                globpro->setValue(percent);
                if(format_index==5 || format_index==7){
                    ui->progressBar->setFormat(QString::number(percent/(num_files-already_completed)+(current_file-already_completed)*(100/(num_files-already_completed)))+"%");
                    ui->progressBar->setValue(percent/(num_files-already_completed)+(current_file-already_completed)*(100/(num_files-already_completed)));
                    /*
                    ui->progressBar->setFormat(QString::number(percent/num_files+(current_file)*(100/num_files))+"%");
                    ui->progressBar->setValue(percent/num_files+(current_file)*(100/num_files));*/
                }
                else
                {
                    ui->progressBar->setFormat(QString::number(percent/(num_files-already_completed)+(current_file-1-already_completed)*(100/(num_files-already_completed)))+"%");
                    ui->progressBar->setValue(percent/(num_files-already_completed)+(current_file-1-already_completed)*(100/(num_files-already_completed)));
                    /*
                    ui->progressBar->setFormat(QString::number(percent/num_files+(current_file-1)*(100/num_files))+"%");
                    ui->progressBar->setValue(percent/num_files+(current_file-1)*(100/num_files));
                    */
                }
            }
        }
}

//Change pages when one of the 5 buttons (Audio,video,Image..) are pressed
void MainWindow::on_musicButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_videoButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_imageButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_rom_deviceButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::on_advancedButton_clicked()
{

    //encode subtitles
    //mencoder has to be installed
    int pr_index=cur_index;
    if(QFile("/usr/bin/mencoder").exists()){
        ui->stackedWidget->setCurrentIndex(4);
    }
    else
    {
        QMessageBox msgBox;
        QPushButton *Yes;
        msgBox.setWindowTitle("Format Junkie");
        msgBox.setInformativeText("This function (subtitle encoding) requires 'mencoder', while it isn't installed! Do you want to install mencoder now?");
        msgBox.setText(tr("<b>Mencoder is not installed</b>"));
        msgBox.addButton("No", QMessageBox::ActionRole);
        Yes = msgBox.addButton("Yes", QMessageBox::ActionRole);
        msgBox.setIconPixmap(QIcon::fromTheme("info").pixmap(QSize(60,60)));
        msgBox.exec();
        if (msgBox.clickedButton() == Yes){
            QMessageBox::information(this, "Mencoder Installation", "The software center will now launch. Once mencoder is installed, please try to use this function again!");
            if(system("software-center apt://mencoder&"))
                cerr << "Error while launching software-center! Please install mencoder manually using 'sudo apt-get install mencoder'\n";
            //returning to the previous index, meanwhile...
            switch(pr_index){
            case 0:
                ui->musicButton->setChecked(true);
                break;
            case 1:
                ui->videoButton->setChecked(true);
                break;
            case 2:
                ui->imageButton->setChecked(true);
                break;
            case 3:
                ui->rom_deviceButton->setChecked(true);
                break;
            default:
                ui->musicButton->setChecked(true);
                break;
            }
        }
        else
        {
            //user chose not to install mencoder, return to the previous index...
            switch(pr_index){
            case 0:
                ui->musicButton->setChecked(true);
                break;
            case 1:
                ui->videoButton->setChecked(true);
                break;
            case 2:
                ui->imageButton->setChecked(true);
                break;
            case 3:
                ui->rom_deviceButton->setChecked(true);
                break;
            default:
                ui->musicButton->setChecked(true);
                break;
            }
        }
    }
}

void MainWindow::on_addfolder_clicked()
{
    if (!ui->addfolder->isEnabled())
        return;
    if(cur_index==0){

        /* ---AUDIO--- */

        QString qpath;
        if(QDir(QDir::homePath()+"/Music").exists())
            qpath= QFileDialog::getExistingDirectory(this, "Choose Folder", QDir::homePath()+"/Music");
        else
            qpath= QFileDialog::getExistingDirectory(this, "Choose Folder", QDir::homePath());
        if(!qpath.count())
            return;
        //getting all the subfolders of the folder specified...
        QStringList all_folders = list_folders(qpath);
        int all_folders_count=all_folders.count();
        for(int i=0;i<all_folders_count;i++){
            //adding the files of all subfolders...
            add_files(all_folders.at(i), cur_index);
        }

        if (ui->tableWidget->rowCount())
            ui->startBtn->setEnabled(true);
        num_files=ui->tableWidget->rowCount();
        if(!ui->outputFormatComboBox->isEnabled())
            update_unity_count_value(1);
    }
    else if(cur_index==1){

        /* ---VIDEO--- */


    }
    else if(cur_index==2){

        /* ---IMAGE--- */


    }
    else if(cur_index==3){
        /* ---ISO IMAGES AND STUFF--- */

    }
}

void MainWindow::on_addBtn_clicked()
{
    if(!ui->addBtn->isEnabled())
        return;
    QStringList path;
    if(cur_index==0){

        /* ---AUDIO--- */

        if(QDir(QDir::homePath()+"/Music").exists())
            path = QFileDialog::getOpenFileNames(this, "Choose Audio Files", QDir::homePath()+"/Music", "*.mp3 *.wav *.ogg *.flac *.wma *.aac *.wv *.mp2");
        else
            path = QFileDialog::getOpenFileNames(this, "Choose Audio Files", QDir::homePath(), "*.mp3 *.wav *.ogg *.flac *.wma *.aac *.wv *.mp2");
        if(!path.count()){
            if(add_audio){
                exit(0);
            }
            return;
        }
        if(add_audio)
            add_audio=false;
        //remove_complete();
        int path_count = path.count();
        int old_r_count=ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+path_count);
        QFileInfo inf;
        for(int count=0; count < path_count; count++){
            QString file=path[count];
            inf.setFile(file);
            QTableWidgetItem *qtitem = new QTableWidgetItem;
            qtitem->setText(inf.completeBaseName()+"."+inf.suffix());
            qtitem->setToolTip(file);
            ui->tableWidget->setItem(old_r_count+count,0,qtitem);
            QTableWidgetItem *qtsize = new QTableWidgetItem;
            float size=(float)inf.size()/1048576;
            qtsize->setText(QString::number(size, 'f', 2)+" MB");
            ui->tableWidget->setItem(old_r_count+count,1,qtsize);
            QTableWidgetItem *qtwait = new QTableWidgetItem;
            qtwait->setText("Waiting");
            ui->tableWidget->setItem(old_r_count+count,2,qtwait);
        }
        if (ui->tableWidget->rowCount())
            ui->startBtn->setEnabled(true);
        num_files=ui->tableWidget->rowCount();
        if(!ui->outputFormatComboBox->isEnabled())
            update_unity_count_value(0);
    }
    else if(cur_index==1){

        /* ---VIDEO--- */


    }
    else if(cur_index==2){

        /* ---PICTURE--- */


    }
    else if(cur_index==3){

        /* ---CSO/ISO AND STUFF--- */


    }
}

void MainWindow::remove_complete(){
    int r_count=ui->tableWidget->rowCount();
    for(int i=0;i<r_count;i++){
        if(ui->tableWidget->item(0,2)->toolTip()=="Completed"){
            ui->tableWidget->removeRow(0);
        }
        else{
            /*
              Only files in a row can be 'Completed'. The 'Completed'
              files are not allowed to move in any possible way, thus
              it is logical to break this for loop after an item is to
              be found not labeled as 'Completed'...
            */
            break;
        }
    }
}


void MainWindow::clear_convertion(){
    QMessageBox msgBox;
    QPushButton *Continue;
    msgBox.setWindowTitle("Format Junkie");
    msgBox.setInformativeText("By clearing the list the current process will stop. Are you sure you want to do so?");
    msgBox.setText(tr("<b>Clear while process is running.</b>"));
    msgBox.addButton("Cancel", QMessageBox::ActionRole);
    Continue = msgBox.addButton("Continue", QMessageBox::ActionRole);
    msgBox.setIconPixmap(QIcon::fromTheme("info").pixmap(QSize(60,60)));
    msgBox.exec();
    if (msgBox.clickedButton() == Continue){

        //stop the current conversion progress
        if(cur_index==0){
            //audio
            on_stop_clicked();
        }
        else if(cur_index==1){
            //video
            on_stop_2_clicked();
        }
        else if(cur_index==2){
            //image
            on_stop_3_clicked();
        }
        //clear
        on_clearBtn_clicked();
    }
}

void MainWindow::on_clearBtn_clicked()
{
    if(cur_index==0){
        //audio
        if(ui->outputFormatComboBox->isEnabled()){
            //the conversion process is off!
            ui->tableWidget->setRowCount(0);
            ui->startBtn->setEnabled(false);
        }
        else
        {
            //the conversion is running! Display an informative message!
            clear_convertion();
        }
    }
    else if(cur_index==1){
        //video

    }
    else if(cur_index==2){
        //image

    }
    else if(cur_index==3){
        //burn

    }
}


void MainWindow::on_removeBtn_clicked()
{
    if(cur_index==0){
        /* ---AUDIO---*/
        if(!ui->tableWidget->rowCount())
            return;
        int cur_row=ui->tableWidget->currentRow();
        if(ui->tableWidget->cellWidget(cur_row,2) && ui->tableWidget->item(cur_row,2)->toolTip()!="Completed"){
            //this file is being processed
            return;
        }
        if(!ui->outputFormatComboBox->isEnabled()){
            if(ui->tableWidget->currentRow()<current_file)
                current_file--;
            else
            {
                if(unity_progressbar)
                    update_unity_count_value(2);
            }
            num_files--;
        }
        if (ui->tableWidget->rowCount()==1)
        {
            ui->tableWidget->setRowCount(0);
            ui->startBtn->setEnabled(false);
        }
        else
        {
            if(cur_row!=0){
                if(ui->tableWidget->item(cur_row-1,2)->toolTip()=="Completed"){
                    if((cur_row+1)==ui->tableWidget->rowCount())
                        ui->startBtn->setEnabled(false);
                }
            }
            ui->tableWidget->removeRow(cur_row);
        }
    }
    else if(cur_index==1){

        /* ---VIDEO---*/

    }
    else if(cur_index==2){

        /* ---PICTURE---*/

    }
    else if(cur_index==3){

    }
}

void MainWindow::on_moveupButton_clicked()
{
    if(cur_index==0){

        /* ---AUDIO--- */

        if(ui->tableWidget->rowCount()>1 && ui->tableWidget->selectionModel()->hasSelection() && ui->tableWidget->currentRow()!=0){
            if(ui->tableWidget->item(ui->tableWidget->currentRow(),2)->text()!="Waiting")
                return;
            if(ui->tableWidget->item(ui->tableWidget->currentRow()-1,2)->text()!="Waiting")
                return;
            if(ui->tableWidget->cellWidget(ui->tableWidget->currentRow(),2))
                return;
            if(ui->tableWidget->cellWidget(ui->tableWidget->currentRow()-1,2))
                return;

            QStringList item_up;
            item_up << ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()-1,0)->text();
            item_up << ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()-1,1)->text();
            item_up << ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()-1,2)->text();
            item_up << ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()-1,2)->toolTip();
            item_up << ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()-1,0)->toolTip();
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()-1,0)->setText(ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),0)->text());
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()-1,1)->setText(ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),1)->text());
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()-1,2)->setText(ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),2)->text());
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()-1,2)->setToolTip(ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),2)->toolTip());
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()-1,0)->setToolTip(ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),0)->toolTip());
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),0)->setText(item_up.at(0));
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),1)->setText(item_up.at(1));
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),2)->setText(item_up.at(2));
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),2)->setToolTip(item_up.at(3));
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),0)->setToolTip(item_up.at(4));
            ui->tableWidget->selectRow(ui->tableWidget->selectedItems().at(0)->row()-1);
        }
    }
    else if(cur_index==1){

        /* ---VIDEO--- */


    }
    else if(cur_index==2){

        /* ---PICTURE--- */


    }
    else if(cur_index==3){

    }
}

void MainWindow::on_movedownButton_clicked()
{
    if(cur_index==0){

        /* ---AUDIO--- */

        if(ui->tableWidget->rowCount()>1 && ui->tableWidget->selectionModel()->hasSelection() && ui->tableWidget->currentRow()!=ui->tableWidget->rowCount()-1){
            if(ui->tableWidget->item(ui->tableWidget->currentRow(),2)->text()!="Waiting")
                return;
            if(ui->tableWidget->item(ui->tableWidget->currentRow()+1,2)->text()!="Waiting")
                return;
            if(ui->tableWidget->cellWidget(ui->tableWidget->currentRow(),2))
                return;
            if(ui->tableWidget->cellWidget(ui->tableWidget->currentRow()+1,2))
                return;
            QStringList item_down;
            item_down << ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()+1,0)->text();
            item_down << ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()+1,1)->text();
            item_down << ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()+1,2)->text();
            item_down << ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()+1,2)->toolTip();
            item_down << ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()+1,0)->toolTip();
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()+1,0)->setText(ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),0)->text());
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()+1,1)->setText(ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),1)->text());
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()+1,2)->setText(ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),2)->text());
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()+1,2)->setToolTip(ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),2)->toolTip());
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row()+1,0)->setToolTip(ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),0)->toolTip());
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),0)->setText(item_down.at(0));
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),1)->setText(item_down.at(1));
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),2)->setText(item_down.at(2));
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),2)->setToolTip(item_down.at(3));
            ui->tableWidget->item(ui->tableWidget->selectedItems().at(0)->row(),0)->setToolTip(item_down.at(4));
            ui->tableWidget->selectRow(ui->tableWidget->selectedItems().at(0)->row()+1);
        }
    }
    else if(cur_index==1){

        /* ---VIDEO--- */


    }
    else if(cur_index==2){

        /* ---PICTURE--- */


    }
    else if(cur_index){

    }
}

void MainWindow::update_unity_count_value(int after){
    /*
      'after' handles the remove button.
      If after is 2, then the user has pressed the remove button
      (so, after it has been done current_file++), but this function
      is also called from audio_conversion(int) function, before the current_file++
    */
    if(after==1)
        unity_launcher_entry_set_count (unity_progress, num_files-current_file-1);
    else if(after==0)
        unity_launcher_entry_set_count (unity_progress, num_files-current_file);
    else if(after==2)
        unity_launcher_entry_set_count (unity_progress,unity_launcher_entry_get_count(unity_progress)-1);
}

void MainWindow::on_startBtn_clicked()
{
    if(!ui->tableWidget->rowCount())
        return;
    if(!QDir(ui->output_folder->toolTip()).exists()){
        QDir dir;
        if(!dir.mkpath(ui->output_folder->toolTip())){
            QMessageBox::warning(this, tr("Error"), tr("The output folder doesn't exist! Please specify an existent directory!"));
            return;
        }
    }
    if(conversion_process_on){
        QMessageBox::warning(this, tr("Error"), tr("Please wait the other conversion processes to finish before starting a new one!"));
        return;
    }
    conversion_process_on=true;
    audio_stop_pressed=false;
    num_files=ui->tableWidget->rowCount();
    if(unity_count)
        unity_launcher_entry_set_count (unity_progress, num_files);
    ui->progressBar->setFormat("0%");
    ui->progressBar->setValue(0);
    //                    ^ ^
    progressbar_animation(0,0); //<< doesn't this look like an owl?

    already_completed=0;
    for(int i=0;i<num_files;i++){
        if(ui->tableWidget->item(i,2)->toolTip()!="Completed"){
            current_file=i;
            already_completed=i;
            break;
        }
    }
    if(current_file==0){
        //that's a completely new conversion
        all_errors.clear();
        error_count=0;
        ui->show_errors->hide();
    }
    ui->outputFormatComboBox->setEnabled(false);
    globpro_string="100%";
    if(unity_progressbar)
        unity_launcher_entry_set_progress_visible(unity_progress, true);
    if(unity_count)
        unity_launcher_entry_set_count_visible(unity_progress, true);
    audio_conversion(0);
}

void MainWindow::actions_after_convert(){
    int conv2_index=ui->after_convertion2->currentIndex();
    if(open_output && conv2_index==0)
        on_output_folder_clicked();
    if(conv2_index==5)
        this->close();
    else if(conv2_index!=0){
        /*
          1=shutdown
          2=suspend
          3=restart
          4=lock
        */
        QString after_convert_cmd="";
        switch(conv2_index){
        case 1:
            after_convert_cmd="dbus-send --system --print-reply --dest=org.freedesktop.ConsoleKit /org/freedesktop/ConsoleKit/Manager org.freedesktop.ConsoleKit.Manager.Stop > /dev/null 2> /dev/null&";
            break;
        case 2:
            after_convert_cmd="dbus-send --system --print-reply --dest=org.freedesktop.UPower /org/freedesktop/UPower org.freedesktop.UPower.Suspend > /dev/null 2> /dev/null&";
            break;
        case 3:
            after_convert_cmd="dbus-send --system --print-reply --dest=org.freedesktop.ConsoleKit /org/freedesktop/ConsoleKit/Manager org.freedesktop.ConsoleKit.Manager.Restart > /dev/null 2> /dev/null&";
            break;
        case 4:
            after_convert_cmd="gnome-screensaver-command --lock > /dev/null 2> /dev/null&";
            break;
        default:
            break;
        }
        if(system(after_convert_cmd.toLocal8Bit().data()))
            cerr << QString("Error running command '"+after_convert_cmd+"'\n").toLocal8Bit().data();
    }
}

void MainWindow::audio_conversion(int return_code){
    /*
      This function is called like audio_conversion(0) the 1st time
      so as to begin the conversion. Then, it is called again
      when the 1st conversion has finished and launches the
      2nd conversion and so on, untill no files are left
      for conversion.
    */
    //Values:
    //current_file <-> 0 1 2
    //num_files    <-> 1 2 3
    int format_index=ui->outputFormatComboBox->currentIndex();
    QString format_text=ui->outputFormatComboBox->currentText();
    //checking if the conversion is about m4(a/r) files or other
    if(format_index==5 || format_index==7){
        /*^^^^^^^^^^^^^^^^^^^^^^^^^^^M4A OR M4R FILES^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
        if(audio_stop_pressed){
            if(ui->tableWidget->item(current_file-1,2)->toolTip()!="Completed")
                ui->tableWidget->removeCellWidget(current_file-1,2);
            else
                ui->tableWidget->removeCellWidget(current_file,2);
            globpro_hasbeendeleted=true;
            QFileInfo inf;
            inf.setFile(output_file);
            QString tmp_file_rm="/tmp/"+inf.completeBaseName()+".wav";
            QString output_file_rm=ui->output_folder->toolTip()+"/"+inf.completeBaseName()+"."+format_text;
            QFile::remove(tmp_file_rm);
            QFile::remove(output_file_rm);
            m4r_or_a_time=false;
            return;
        }
        if(return_code){
            /*
              The conversion of the previous file wasn't successful!
              Print error and guide the user to see the error logs...
            */
            if(all_errors.isEmpty())
                all_errors+="'<b>"+input_file+"</b>'<br>";
            else
                all_errors+="\n'<b>"+input_file+"</b>'<br>";
            current_file_errors.replace("\n","<br>");
            if(m4r_or_a_time){
                //error while converting the input file to temp wav
                //current_file_errors=input_file+": File non-existent or bad filetype!<br>";
                proceed_to_faac_conversion=false;
            }
            all_errors+="<font color=red>"+current_file_errors+"</font>";
            QTableWidgetItem *item_error1 = new QTableWidgetItem;
            item_error1->setText(ui->tableWidget->item(current_file,0)->text()+" - ERROR");
            item_error1->setToolTip(ui->tableWidget->item(current_file,0)->toolTip());
            item_error1->setBackgroundColor(Qt::red);
            item_error1->setTextColor(Qt::white);
            ui->tableWidget->setItem(current_file,0, item_error1);
            QTableWidgetItem *item_error2 = new QTableWidgetItem;
            item_error2->setText(ui->tableWidget->item(current_file,1)->text());
            item_error2->setToolTip(ui->tableWidget->item(current_file,1)->toolTip());
            item_error2->setBackgroundColor(Qt::red);
            item_error2->setTextColor(Qt::white);
            ui->tableWidget->setItem(current_file,1, item_error2);
            show_errors_timer_count=5;
            show_errors_animation();
            show_errors_timer->start(1000);
            //take it like the percentage was found, so as to continue normally to the next file...
            m4r_or_a_previous_was_error=true;
            error_count++;
        }
        else
        {
            //IF NOT RETURN CODE
            if(!m4r_or_a_time){
                //if it is to convert to m4r
                current_file_errors.clear();
            }
            /*
              The conversion of the previous file was successful!
              Check if the original (source) file is to be deleted...
            */
            if(delete_original){
                if(QFile(input_file).exists()){
                    if(!QFile::remove(input_file))
                        cerr << QString("Could not delete source file '"+input_file+"'\nDo I have the permissions to do so?\n").toLocal8Bit().data();
                }
            }
        }
        if (current_file>num_files-1){
            /*
              !!!ALL FILES HAVE BEEN CONVERTED!!!
              Update the progressbars and do the actions
              that the user has chosen to do from bottom
              right!!!
            */
            //just update the progressbar
            conversion_process_on=false;
            ui->progressBar->setFormat("100%");
            ui->progressBar->setValue(100);
            globpro->setFormat(globpro_string);
            globpro->setValue(100);
            QTableWidgetItem *item_progress = new QTableWidgetItem;
            item_progress->setToolTip("Completed");
            ui->tableWidget->setItem(current_file-1,2, item_progress);
            ui->startBtn->setEnabled(false);
            ui->startBtn->raise();
            progressbar_animation(1,0);
            //removing the last tmp file...
            QFile::remove(output_file_rm);
            ui->outputFormatComboBox->setEnabled(true);
            if(unity_progressbar)
                unity_launcher_entry_set_progress_visible(unity_progress, false);
            if(unity_count)
                unity_launcher_entry_set_count_visible(unity_progress, false);
            if(desktop_notifications)
                show_desktop_notification(0);
            error_count=0;
            actions_after_convert();
            return;
        }
        if(ui->tableWidget->item(current_file,2)->toolTip()=="Completed"){
            /*
              The file's conversion has been completed, proceed to the next file...
              The program goes through here if the user add more files after the
              previous conversion has finished and choose to start converting...
            */
            current_file++;
            audio_conversion(0);
            return;
        }
        if(!m4r_or_a_time || globpro_string=="Already Exists"){
            /*
              The program always goes through there, unless
              the user has chosen to convert the file to m4r
              and this is the 2nd conversion that has to be
              done (audio files to m4r are being converted
              to wav first and then to m4r with faac)
              So, if(m4r_or_a_time) then the conversion is 20%,
              because a 2nd conversion has to be done...
            */
            if(current_file!=0){
                if(globpro_hasbeendeleted){
                    globpro = new QProgressBar(this);
                    globpro_hasbeendeleted=false;
                }
                if(globpro->format()!="Already Exists" && ui->tableWidget->item(current_file-1,2)->toolTip()!="Completed"){
                    QProgressBar *qtpro=new QProgressBar(this);
                    ui->tableWidget->setCellWidget(current_file-1, 2, qtpro);
                    QTableWidgetItem *item_progress = new QTableWidgetItem;
                    item_progress->setToolTip("Completed");
                    ui->tableWidget->setItem(current_file-1,2, item_progress);
                    qtpro->setFormat(globpro_string);
                    globpro_string="100%";
                    qtpro->setValue(100);
                }
            }
            if(!ui->tableWidget->cellWidget(current_file,2)){
                QProgressBar *qtpro=new QProgressBar(this);
                ui->tableWidget->setCellWidget(current_file, 2, qtpro);
                if(!globpro)
                    globpro = new QProgressBar(this);
                globpro=qtpro;
            }
        }
        //Action starts here...
        input_file=ui->tableWidget->item(current_file,0)->toolTip();
        QFileInfo info;
        info.setFile(input_file);
        if(!m4r_or_a_time){
            //Convert the input file to a temp wav file
            if(current_file!=0){
                //a tmp file has been created from the previous conversion. Delete this file...
                QFile::remove(output_file_rm);
            }
            m4r_or_a_time=true;
            if(m4r_or_a_previous_was_error)
                faac_percent_found=true;
            if(!faac_percent_found && current_file!=0 && globpro_string!="Already Exists"){
                /*
                  We couldn't find the percentage sent by
                  faac, because the file was extremely small
                  so, go instantly to 100%
                */
                full_percent_instant=true;
            }
            output_file=output_file_rm="/tmp/"+info.completeBaseName()+".wav";
            if(QFile(output_file).exists()){
                if(!QFile::remove(output_file))
                    cerr << QString("Error removing temp file '"+output_file+"'\n").toLocal8Bit().data();
            }
            command="avconv";
            command_arguments.clear();
            command_arguments << "-i" << input_file << "-aq" << QString::number(quality);
            if(audio_bitrate)
                command_arguments << "-ab" << QString::number(audio_bitrate_value) + "k";
            if(audio_samplerate)
                command_arguments << "-ar" << QString::number(audio_samplerate_value);
            if(audio_channels)
                command_arguments << "-ac" << QString::number(audio_channels_value);
            if(audio_volume)
                command_arguments << "-vol " << QString::number(audio_volume_value);
            command_arguments << output_file;
        }
        else{
            if(!proceed_to_faac_conversion){
                proceed_to_faac_conversion=true;
                //next file PLEASE :D
                current_file++;
                m4r_or_a_time=false;
                audio_conversion(0);
                return;
            }
            m4r_or_a_time=false;
            faac_percent_found=false;
            output_file=ui->output_folder->toolTip()+"/"+info.completeBaseName()+"."+format_text;
            command="faac";
            command_arguments.clear();
            command_arguments << "-b" << "192" << "-c" << "44100" << "/tmp/"+info.completeBaseName()+".wav" << "--mpeg-vers" << "4" << "-o" << output_file;
        }
        /*
          Checking whether the output_file already exists. If it does, then
          overwrite it, if it checked so, or proceed to the next file, if not...
         */
        //checking if the final file exists, not if the temp wav file exists...
        if(m4r_or_a_time){
            QString file_rm=ui->output_folder->toolTip()+"/"+info.completeBaseName()+"."+format_text;
            if(QFile(file_rm).exists()){
                if(overwrite_existing){
                    if(!QFile::remove(file_rm)){
                        cerr << QString("Couldn't remove the already existing file '"+output_file+"'\n").toLocal8Bit().data();
                        current_file++;
                        globpro_string="Already Exists";
                        faac_percent_found=true;
                        m4r_or_a_time=false;
                        audio_conversion(0);
                        return;
                    }
                }
                else
                {
                    current_file++;
                    globpro_string="Already Exists";
                    faac_percent_found=true;
                    m4r_or_a_time=false;
                    audio_conversion(0);
                    return;
                }
            }
        }
        //getting the duration of the input file...
        duration_secs=Global::duration(input_file);

        audio_converter->start(command, command_arguments);
        if(full_percent_instant){
            full_percent_instant=false;
            if(!m4r_or_a_previous_was_error){
                m4r_or_a_previous_was_error=false;
                globpro->setValue(100);
                globpro->setFormat(globpro_string);
            }
        }
        if(!m4r_or_a_time){
            current_file++;
        }
        else
        {
            update_unity_count_value(0);
        }

    }
    else
    {
        /*^^^^^^^^^^^^^^^^^^^^^^^^^^^ALL OTHER AUDIO FILES^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
        if(audio_stop_pressed){
            if(current_file!=0)
                ui->tableWidget->removeCellWidget(current_file-1,2);
            else
                ui->tableWidget->removeCellWidget(current_file,2);
            globpro_hasbeendeleted=true;
            if(!QFile::remove(output_file))
                cerr << QString("Error removing file '"+output_file+"'\n").toLocal8Bit().data();
            return;
        }
        if(return_code){
            /*
              The conversion of the previous file wasn't successful!
              Print error and guide the user to see the error logs...
            */
            if(all_errors.isEmpty())
                all_errors+="'<b>"+input_file+"</b>'<br>";
            else
                all_errors+="\n'<b>"+input_file+"</b>'<br>";
            current_file_errors.replace("\n","<br>");
            all_errors+="<font color=red>"+current_file_errors+"</font><br>";
            int to_apply=current_file;
            if(to_apply>0)
                to_apply--;
            QTableWidgetItem *item_error1 = new QTableWidgetItem;
            item_error1->setText(ui->tableWidget->item(to_apply,0)->text()+" - ERROR");
            item_error1->setToolTip(ui->tableWidget->item(to_apply,0)->toolTip());
            item_error1->setBackgroundColor(Qt::red);
            item_error1->setTextColor(Qt::white);
            ui->tableWidget->setItem(to_apply,0, item_error1);
            QTableWidgetItem *item_error2 = new QTableWidgetItem;
            item_error2->setText(ui->tableWidget->item(to_apply,1)->text());
            item_error2->setToolTip(ui->tableWidget->item(to_apply,1)->toolTip());
            item_error2->setBackgroundColor(Qt::red);
            item_error2->setTextColor(Qt::white);
            ui->tableWidget->setItem(to_apply,1, item_error2);
            show_errors_timer_count=5;
            show_errors_animation();
            show_errors_timer->start(1000);
            current_file_errors.clear();
            error_count++;
        }
        else
        {
            //IF NOT RETURN CODE
            current_file_errors.clear();
            /*
              The conversion of the previous file was successful!
              Check if the original (source) file is to be deleted...
            */
            if(delete_original){
                if(QFile(input_file).exists()){
                    if(!QFile::remove(input_file))
                        cerr << QString("Could not delete source file '"+input_file+"'\nDo I have the permissions to do so?\n").toLocal8Bit().data();
                }
            }
        }
        if (current_file>num_files-1){
            /*
              !!!ALL FILES HAVE BEEN CONVERTED!!!
              Update the progressbars and do the actions
              that the user has chosen to do from bottom
              right!!!
            */
            //all files have been converted!
            //just update the progressbar
            conversion_process_on=false;
            ui->progressBar->setFormat("100%");
            ui->progressBar->setValue(100);
            globpro->setFormat(globpro_string);
            globpro->setValue(100);
            QTableWidgetItem *item_progress = new QTableWidgetItem;
            item_progress->setToolTip("Completed");
            ui->tableWidget->setItem(current_file-1,2, item_progress);
            ui->startBtn->setEnabled(false);
            ui->startBtn->raise();
            progressbar_animation(1,0);
            ui->outputFormatComboBox->setEnabled(true);
            if(unity_progressbar)
                unity_launcher_entry_set_progress_visible(unity_progress, false);
            if(unity_count)
                unity_launcher_entry_set_count_visible(unity_progress, false);
            if(desktop_notifications)
                show_desktop_notification(0);
            actions_after_convert();
            return;
        }
        if(ui->tableWidget->item(current_file,2)->toolTip()=="Completed"){
            /*
              The file's conversion has been completed, proceed to the next file...
              The program goes through here if the user add more files after the
              previous conversion has finished and choose to start converting...
            */
            current_file++;
            audio_conversion(0);
            return;
        }
        if(globpro_string=="Already Exists" || !m4r_or_a_time){
            /*
              The program always goes through there, unless
              the user has chosen to convert the file to m4r
              and this is the 2nd conversion that has to be
              done (audio files to m4r are being converted
              to wav first and then to m4r with faac)
              So, if(m4r_or_a_time) then the conversion is 50%,
              because a 2nd conversion has to be done...
            */
            if(current_file!=0){
                if(globpro_hasbeendeleted){
                    globpro = new QProgressBar(this);
                    globpro_hasbeendeleted=false;
                }
                if(globpro->format()!="Already Exists" && ui->tableWidget->item(current_file-1,2)->toolTip()!="Completed"){
                    QProgressBar *qtpro=new QProgressBar(this);
                    ui->tableWidget->setCellWidget(current_file-1, 2, qtpro);
                    QTableWidgetItem *item_progress = new QTableWidgetItem;
                    item_progress->setToolTip("Completed");
                    ui->tableWidget->setItem(current_file-1,2, item_progress);
                    qtpro->setFormat(globpro_string);
                    qtpro->setValue(100);
                }
            }
            QProgressBar *qtpro=new QProgressBar(this);
            ui->tableWidget->setCellWidget(current_file, 2, qtpro);
            if(!globpro)
                globpro = new QProgressBar(this);
            globpro=qtpro;
        }
        //Action starts here...
        input_file=ui->tableWidget->item(current_file,0)->toolTip();
        QFileInfo info;
        info.setFile(input_file);
        output_file=ui->output_folder->toolTip()+"/"+info.completeBaseName()+"."+format_text;
        command="avconv";
        command_arguments.clear();
        command_arguments << "-i" << input_file << "-aq" << QString::number(quality);
        if(format_text=="aac")
            command_arguments << "-acodec" << "aac" << "-strict" << "experimental";
        if(audio_bitrate)
           command_arguments << "-ab" << QString::number(audio_bitrate_value)+"k";
        if(audio_samplerate)
           command_arguments << "-ar" << QString::number(audio_samplerate_value);
        if(audio_channels)
           command_arguments << "-ac" << QString::number(audio_channels_value);
        if(audio_volume)
           command_arguments << "-vol" <<QString::number(audio_volume_value);
       command_arguments << output_file;
        /*
          Checking whether the output_file already exists. If it does, then
          overwrite it, if it checked so, or proceed to the next file, if not...
         */
        if(QFile(output_file).exists()){
            if(overwrite_existing){
                if(!QFile::remove(output_file)){
                    cerr << QString("Couldn't remove the already existing file '"+output_file+"'\n").toLocal8Bit().data();
                    current_file++;
                    globpro_string="Already Exists";
                    audio_conversion(0);
                    return;
                }
            }
            else
            {
                current_file++;
                globpro_string="Already Exists";
                audio_conversion(0);
                return;
            }
        }
        //getting the duration of the input file...
        duration_secs=Global::duration(input_file);
        globpro_string="100%";
        audio_converter->start(command,command_arguments);
        if(full_percent_instant){
            full_percent_instant=false;
            globpro->setValue(100);
            globpro->setFormat(globpro_string);
        }
        update_unity_count_value(0);
        current_file++;
    }
}

//All

void MainWindow::add_files(QString path, int type){
    //this function is for adding files from add-folder function or from drag-en-drop
        /*
          type 0 -> audio
          type 1 -> video
          type 2 -> image
          type 3 -> all of the above
        */
    if(type<3){
        QDir dir_get_all_files;
        dir_get_all_files.setPath(path);
        dir_get_all_files.setFilter(QDir::Files);
        QStringList filters;
        if (type==0)
            filters << "*.mp3" << "*.MP3" << "*.wav" << "*.WAV" << "*.ogg" << "*.OGG" << "*.flac" << "*.FLAC" << "*.m4r" << "*.M4R" << "*.WMA" << "*.wma" << "*.AAC" << "*.aac" << "*.WV" << "*.wv" << "*.MP2" << "*.mp2";
        else if (type==1)
            filters << "*.avi" << "*.AVI" << "*ogv" << "*OGV" << "*vob" << "*VOB" << "*.mp4" << "*.MP4" << "*3gp" << "*3GP" << "*wmv" << "*WMV" << "*.mkv" << "*.MKV" << "*mpg" << "*MPG" << "*mov" << "*MOV" << "*flv" << "*FLV";
        else if (type==2)
            filters << "*.png" << "*.PNG" << "*.jpg" << "*.JPG" << "*.jpeg" << "*.JPEG" << "*.gif" << "*.GIF" << "*.bmp" << "*.BMP" << "*.svg" << "*.SVG";
        QStringList all_files=dir_get_all_files.entryList(filters);
        int all_files_count = all_files.count();
        QString temp;
        int old_r_count=0;
        if(type==0){
            old_r_count=ui->tableWidget->rowCount();
            ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+all_files_count);
        }
        else if(type==1){
            old_r_count=ui->tableWidget_2->rowCount();
            ui->tableWidget_2->setRowCount(ui->tableWidget_2->rowCount()+all_files_count);
        }
        else if(type==2){
            old_r_count=ui->tableWidget_3->rowCount();
            ui->tableWidget_3->setRowCount(ui->tableWidget_3->rowCount()+all_files_count);
        }
        QFileInfo inf;
        if(type==0){

            /* ---AUDIO--- */

            for(int i=0; i<all_files_count; i++){
                temp=path+"/"+all_files.at(i);
                inf.setFile(temp);
                QTableWidgetItem *qtitem = new QTableWidgetItem;
                qtitem->setText(all_files.at(i));
                qtitem->setToolTip(temp);
                ui->tableWidget->setItem(old_r_count+i,0,qtitem);
                QTableWidgetItem *qtsize = new QTableWidgetItem;
                float size=(float)inf.size()/1048576;
                qtsize->setText(QString::number(size, 'f', 2)+" MB");
                ui->tableWidget->setItem(old_r_count+i,1,qtsize);
                QTableWidgetItem *qtwait = new QTableWidgetItem;
                qtwait->setText("Waiting");
                ui->tableWidget->setItem(old_r_count+i,2,qtwait);
            }
        }
        else if(type==1){

            /* ---VIDEO--- */

            for(int i=0; i<all_files_count; i++){
                temp=path+"/"+all_files.at(i);
                inf.setFile(temp);
                QTableWidgetItem *qtitem = new QTableWidgetItem;
                qtitem->setText(all_files.at(i));
                qtitem->setToolTip(temp);
                ui->tableWidget_2->setItem(old_r_count+i,0,qtitem);
                QTableWidgetItem *qtsize = new QTableWidgetItem;
                float size=(float)inf.size()/1048576;
                qtsize->setText(QString::number(size, 'f', 2)+" MB");
                ui->tableWidget_2->setItem(old_r_count+i,1,qtsize);
                QTableWidgetItem *qtwait = new QTableWidgetItem;
                qtwait->setText("Waiting");
                ui->tableWidget_2->setItem(old_r_count+i,2,qtwait);
            }
        }
        else if(type==2){

            /* ---PICTURE--- */

            for(int i=0; i<all_files_count; i++){
                temp=path+"/"+all_files.at(i);
                inf.setFile(temp);
                QTableWidgetItem *qtitem = new QTableWidgetItem;
                qtitem->setText(all_files.at(i));
                qtitem->setToolTip(temp);
                ui->tableWidget_3->setItem(old_r_count+i,0,qtitem);
                QTableWidgetItem *qtsize = new QTableWidgetItem;
                float size=(float)inf.size()/1048576;
                qtsize->setText(QString::number(size, 'f', 3)+" MB");
                ui->tableWidget_3->setItem(old_r_count+i,1,qtsize);
                QTableWidgetItem *qtwait = new QTableWidgetItem;
                qtwait->setText("Waiting");
                ui->tableWidget_3->setItem(old_r_count+i,2,qtwait);
            }
        }
    }
    else
    {
        /* ---ALL--- */
        QDir dir_get_all_files;
        dir_get_all_files.setPath(path);
        dir_get_all_files.setFilter(QDir::Files);
        QStringList audio_filters, video_filters, image_filters;
        audio_filters << "*.mp3" << "*.MP3" << "*.wav" << "*.WAV" << "*.ogg" << "*.OGG" << "*.flac" << "*.FLAC" << "*.m4r" << "*.M4R" << "*.WMA" << "*.wma";
        video_filters << "*.avi" << "*.AVI" << "*ogv" << "*OGV" << "*vob" << "*VOB" << "*.mp4" << "*.MP4" << "*3gp" << "*3GP" << "*wmv" << "*WMV" << "*.mkv" << "*.MKV" << "*mpg" << "*MPG" << "*mov" << "*MOV" << "*flv" << "*FLV";
        image_filters << "*.png" << "*.PNG" << "*.jpg" << "*.JPG" << "*.jpeg" << "*.JPEG" << "*.gif" << "*.GIF" << "*.bmp" << "*.BMP" << "*.svg" << "*.SVG";
        QStringList audio_files=dir_get_all_files.entryList(audio_filters);
        QStringList video_files=dir_get_all_files.entryList(video_filters);
        QStringList image_files=dir_get_all_files.entryList(image_filters);
        int audio_files_count = audio_files.count();
        int video_files_count = video_files.count();
        int image_files_count = image_files.count();
        if(audio_files_count){
            QString temp;
            int old_r_count=ui->tableWidget->rowCount();
            ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+audio_files_count);
            QFileInfo inf;
            for(int i=0; i<audio_files_count; i++){
                temp=path+"/"+audio_files.at(i);
                inf.setFile(temp);
                QTableWidgetItem *qtitem = new QTableWidgetItem;
                qtitem->setText(audio_files.at(i));
                qtitem->setToolTip(temp);
                ui->tableWidget->setItem(old_r_count+i,0,qtitem);
                QTableWidgetItem *qtsize = new QTableWidgetItem;
                float size=(float)inf.size()/1048576;
                qtsize->setText(QString::number(size, 'f', 2)+" MB");
                ui->tableWidget->setItem(old_r_count+i,1,qtsize);
                QTableWidgetItem *qtwait = new QTableWidgetItem;
                qtwait->setText("Waiting");
                ui->tableWidget->setItem(old_r_count+i,2,qtwait);
            }
        }
        if(video_files_count){
            QString temp;
            int old_r_count=ui->tableWidget_2->rowCount();
            ui->tableWidget_2->setRowCount(ui->tableWidget_2->rowCount()+video_files_count);
            QFileInfo inf;
            for(int i=0; i<video_files_count; i++){
                temp=path+"/"+video_files.at(i);
                inf.setFile(temp);
                QTableWidgetItem *qtitem = new QTableWidgetItem;
                qtitem->setText(video_files.at(i));
                qtitem->setToolTip(temp);
                ui->tableWidget_2->setItem(old_r_count+i,0,qtitem);
                QTableWidgetItem *qtsize = new QTableWidgetItem;
                float size=(float)inf.size()/1048576;
                qtsize->setText(QString::number(size, 'f', 2)+" MB");
                ui->tableWidget_2->setItem(old_r_count+i,1,qtsize);
                QTableWidgetItem *qtwait = new QTableWidgetItem;
                qtwait->setText("Waiting");
                ui->tableWidget_2->setItem(old_r_count+i,2,qtwait);
            }
        }
        if(image_files_count){
            QString temp;
            int old_r_count=ui->tableWidget_3->rowCount();
            ui->tableWidget_3->setRowCount(ui->tableWidget_3->rowCount()+image_files_count);
            QFileInfo inf;
            for(int i=0; i<image_files_count; i++){
                temp=path+"/"+image_files.at(i);
                inf.setFile(temp);
                QTableWidgetItem *qtitem = new QTableWidgetItem;
                qtitem->setText(image_files.at(i));
                qtitem->setToolTip(temp);
                ui->tableWidget_3->setItem(old_r_count+i,0,qtitem);
                QTableWidgetItem *qtsize = new QTableWidgetItem;
                float size=(float)inf.size()/1048576;
                qtsize->setText(QString::number(size, 'f', 3)+" MB");
                ui->tableWidget_3->setItem(old_r_count+i,1,qtsize);
                QTableWidgetItem *qtwait = new QTableWidgetItem;
                qtwait->setText("Waiting");
                ui->tableWidget_3->setItem(old_r_count+i,2,qtwait);
            }
        }
    }
}

float MainWindow::get_size(QString folder){
    /*
      This function returns the size of a specified folder in MB (non recursively)
    */
    QDir dir_get_all_files;
    dir_get_all_files.setPath(folder);
    dir_get_all_files.setFilter(QDir::Files);
    QStringList all_files=dir_get_all_files.entryList();
    int all_files_count = all_files.count();
    float complete_size=0;
    if(all_files_count){
        QString temp;
        QFileInfo inf;
        for(int i=0; i<all_files_count; i++){
            temp=folder+"/"+all_files.at(i);
            inf.setFile(temp);
            complete_size+=(float)inf.size()/1048576;
        }
        return complete_size;
    }
    else
        return complete_size;
}

float MainWindow::folder_size(QString folder){
    /*
      this function returns a float number, which
      represents the size of the parent folder 'folder'
      in MB!
    */

    //getting all the subfolders of the folder specified...
    QStringList all_folders = list_folders(folder);
    int all_folders_count=all_folders.count();
    float complete_size=0;
    for(int i=0;i<all_folders_count;i++){
        //getting the size of each of the subfolders
        float folder_size=get_size(all_folders.at(i));
        complete_size+=folder_size;
    }
    return complete_size;
}

QStringList MainWindow::list_folders(QString parent_folder){
    //this function return a QStringList with all the subdirectories
    //of a parent_folder, and the parent_folder itself.
    QStringList all_dirs;
    all_dirs << parent_folder;
    QDirIterator directories(parent_folder, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    while(directories.hasNext()){
        directories.next();
        all_dirs << directories.filePath();
    }
    return all_dirs;
}

void MainWindow::on_stop_clicked()
{
    audio_stop_pressed=true;
    conversion_process_on=false;
    progressbar_animation(1,0);
    //ui->startBtn->raise();
    if(audio_converter->isOpen())
        audio_converter->close();
    ui->outputFormatComboBox->setEnabled(true);
    if(unity_progressbar)
        unity_launcher_entry_set_progress_visible(unity_progress, false);
    if(unity_count)
        unity_launcher_entry_set_count_visible(unity_progress, false);
}

void MainWindow::on_output_folder_clicked()
{
    QString output_dir=ui->output_folder->toolTip();
    if(!QDir(output_dir).exists()){
        QDir dir;
        if(!dir.mkpath(output_dir)){
            QMessageBox::information(this, "Error", "Folder '"+output_dir+"' doesn't exist and I failed to create it, please choose another output_folder");
            return;
        }
    }
    if(system(QString("xdg-open \""+output_dir+"\"&").toLocal8Bit().data()))
        cerr << QString("Error opening folder '"+output_dir+"'\n").toLocal8Bit().data();
}

void MainWindow::on_actionPreferences_triggered()
{
    pref =new Preferences;
    connect(this,SIGNAL(send_output_folder(QString)),pref,SLOT(put_output_folder_at_line(QString)));
    connect(pref,SIGNAL(send_new_output(QString)),this,SLOT(crop_output_folder(QString)));
    Q_EMIT send_output_folder(ui->output_folder->toolTip());
    pref->exec();
}

void MainWindow::delete_indicator(){
    if(indicator_has_been_created)
        trayIcon->hide();
}

void MainWindow::progressbar_animation(int end_value, int type){
    /*
      If end_value is 1, then show the stop button
      0, show the start buttonS
      type==0->audio
      type==1->video
      type==2->picture
    */

    //Setting up opacity effects
    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(this);
    if(end_value)
        opacityEffect->setOpacity(0.0);
    else
        opacityEffect->setOpacity(1.0);

    QGraphicsOpacityEffect* opacityEffect2 = new QGraphicsOpacityEffect(this);
    if(end_value)
        opacityEffect2->setOpacity(1.0);
    else
        opacityEffect2->setOpacity(0.0);

    QGraphicsOpacityEffect* opacityEffect3 = new QGraphicsOpacityEffect(this);
    if(end_value)
        opacityEffect3->setOpacity(1.0);
    else
        opacityEffect3->setOpacity(0.0);

    if(type==0)
    {
        ui->startBtn->setGraphicsEffect(opacityEffect);
        ui->stop->setGraphicsEffect(opacityEffect2);
        ui->progressBar->setGraphicsEffect(opacityEffect3);
    }
    else if(type==1)
    {
        ui->startBtn_2->setGraphicsEffect(opacityEffect);
        ui->stop_2->setGraphicsEffect(opacityEffect2);
        ui->progressBar_2->setGraphicsEffect(opacityEffect3);
    }
    else
    {
        ui->startBtn_3->setGraphicsEffect(opacityEffect);
        ui->stop_3->setGraphicsEffect(opacityEffect2);
        ui->progressBar_3->setGraphicsEffect(opacityEffect3);
    }

    //Animation for Start Buttons

    QPropertyAnimation* anim = new QPropertyAnimation(this);
    anim->setTargetObject(opacityEffect);
    anim->setPropertyName("opacity");
    anim->setDuration(500);
    anim->setStartValue(opacityEffect->opacity());
    anim->setEndValue(end_value);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    if(end_value)
    {
        if(type==0)
            ui->startBtn->show();
        else if(type==1)
            ui->startBtn_2->show();
        else if(type==2)
            ui->startBtn_3->show();
    }
    else
    {
        if(type==0)
            ui->startBtn->hide();
        else if(type==1)
            ui->startBtn_2->hide();
        else if(type==2)
            ui->startBtn_3->hide();
    }

    //Animation for Stop Buttons

    QPropertyAnimation* anim2 = new QPropertyAnimation(this);
    anim2->setTargetObject(opacityEffect2);
    anim2->setPropertyName("opacity");
    anim2->setDuration(500);
    anim2->setStartValue(opacityEffect2->opacity());
    anim2->setEndValue(!end_value);
    anim2->setEasingCurve(QEasingCurve::OutQuad);
    anim2->start(QAbstractAnimation::DeleteWhenStopped);
    if(!end_value)
    {
        if(type==0)
            ui->stop->show();
        else if(type==1)
            ui->stop_2->show();
        else if(type==2)
            ui->stop_3->show();
    }
    else
    {
        if(type==0)
            ui->stop->hide();
        else if(type==1)
            ui->stop_2->hide();
        else if(type==2)
            ui->stop_3->hide();
    }

    //Animation for ProgressBars

    QPropertyAnimation* anim3 = new QPropertyAnimation(this);
    anim3->setTargetObject(opacityEffect3);
    anim3->setPropertyName("opacity");
    anim3->setDuration(500);
    anim3->setStartValue(opacityEffect3->opacity());
    anim3->setEndValue(!end_value);
    anim3->setEasingCurve(QEasingCurve::OutQuad);
    anim3->start(QAbstractAnimation::DeleteWhenStopped);
    if(!end_value)
    {
        if(type==0)
            ui->progressBar->show();
        else if(type==1)
            ui->progressBar_2->show();
        else if(type==2)
            ui->progressBar_3->show();
    }
    else
    {
        type_timer=type;
        QTimer::singleShot(500, this, SLOT(timer_to_hide()));
    }
}

void MainWindow::timer_to_hide()
{
    if(type_timer==0)
        ui->progressBar->hide();
    else if(type_timer==1)
        ui->progressBar_2->hide();
    else
        ui->progressBar_3->hide();
}

void MainWindow::on_stackedWidget_currentChanged(int arg1)
{
    cur_index=arg1;
    if((cur_index==3 && ui->stackedWidget_3->currentIndex()==1) || cur_index==4){
        ui->addBtn->hide();
        ui->addfolder->hide();
        ui->clearBtn->hide();
        ui->removeBtn->hide();
        ui->movedownButton->hide();
        ui->moveupButton->hide();
    }
    else if(ui->addBtn->isHidden()){
        ui->addBtn->show();
        ui->addfolder->show();
        ui->clearBtn->show();
        ui->removeBtn->show();
        ui->movedownButton->show();
        ui->moveupButton->show();
    }
}

void MainWindow::on_show_errors_clicked()
{
    showlogs =new error_logs(all_errors, this);
    showlogs->exec();
}

void MainWindow::on_actionAbout_triggered()
{
    About = new about(this);
    if(about_shown)
    {
        About->raise();
        About->activateWindow();
    }
    else
    {
        about_shown=true;
        About->exec();
        about_shown=false;
    }
}

void MainWindow::on_actionQuit_triggered()
{
    this->close();
}

void MainWindow::on_actionGet_Help_Online_triggered()
{
    if(system("xdg-open https://answers.launchpad.net/format-junkie/+addquestion"))
        cerr << "Error opening https://answers.launchpad.net/format-junkie/+addquestion\n";
}
void MainWindow::on_actionReport_A_Bug_triggered()
{
    if(system("xdg-open https://bugs.launchpad.net/format-junkie/+filebug"))
        cerr << "Error opening https://bugs.launchpad.net/format-junkie/+filebug\n";
}

void MainWindow::dbus_action(QString msg){
    //this function checks for dbus messages!
    if(msg=="focus"){
        show_me();
    }
    else if(msg=="--add-audio"){
        cur_index=0;
        ui->stackedWidget->setCurrentIndex(0);
        ui->musicButton->setChecked(true);
        on_addBtn_clicked();
    }
    else if(msg=="--add-video"){
        cur_index=1;
        ui->stackedWidget->setCurrentIndex(1);
        ui->videoButton->setChecked(true);
        on_addBtn_clicked();
    }
    else if(msg=="--add-image"){
        cur_index=2;
        ui->stackedWidget->setCurrentIndex(2);
        ui->imageButton->setChecked(true);
        on_addBtn_clicked();
    }
    else if(msg=="--open-output"){
        on_output_folder_clicked();
    }
    else{
        cerr << QString("Received message "+msg+" but it's not recognized as an argument!\nPlease give formatjunkie --help/-h for all available options!\n").toLocal8Bit().data();
    }
}

void MainWindow::update_unity_value(int value){
    unity_launcher_entry_set_progress(unity_progress, (float)value/100);
}

void MainWindow::on_progressBar_valueChanged(int value)
{
    if(unity_progressbar)
        update_unity_value(value);
}

void MainWindow::on_progressBar_2_valueChanged(int value)
{
    if(unity_progressbar)
        update_unity_value(value);
}

void MainWindow::on_progressBar_3_valueChanged(int value)
{
    if(unity_progressbar)
        update_unity_value(value);
}

void MainWindow::handle_it(){

}

void MainWindow::show_desktop_notification(int noti_type){
    /*
      noti_type:
      0->Notification about convertion complete
      1->Notification about a file that already exists
      2->Notification about successful ISO creation
      3->Notification about a file that doesn't exist while it should
      4->Notification about successful CSO creation
      5->Notification about unsuccessful ISO creation
      6->Notification about unsuccessful CSO creation
      7->Notification about already existent avi file (on subtitles encoding)
      8->Notification about encoding successful
    */
    if (noti_type==0){
    QString body1,body2,body3;
    body1="The conversion of "+QString::number(num_files-already_completed);
    if(num_files-already_completed>1)
        body2=" files has finished";
    else
        body2=" file has finished";
    if(error_count){
        if(error_count>1)
            body3=" with "+QString::number(error_count)+" errors!";
        else
            body3=" with 1 error!";
    }
    else
        body3="!";

    body=body1+body2+body3;
    }
    else if(noti_type==1)
        body="The file "+file_exists+" already exists.";
    else if(noti_type==2)
        body="ISO file has been created successfully";
    else if(noti_type==3)
        body="File "+file_not_exists+" doesn't exist.";
    else if(noti_type==4)
        body="CSO file has been created successfully";
    else if(noti_type==5)
        body="Error creating your ISO file!";
    else if(noti_type==6)
        body="Error creating your CSO file!";
    else if(noti_type==7)
        body="The file"+file_exists+" already exists!";
    else if(noti_type==8)
        body="Encoding has been completed successfully ";
    else if(noti_type==9)
        body="Error while trying to encode!";

    if (!notify_init ("update-notifications")){
        cerr << "Couldn't display notification, unknown error!\n";
        return;
    }
    if(first_notification){
        first_notification=false;
        notification = notify_notification_new ( "Format Junkie", body.toLocal8Bit().data(), "/opt/extras.ubuntu.com/formatjunkie/pixmap/fjt.png");
        error = NULL;
        success = notify_notification_show (notification, &error);
        if (!success)
        {
                g_print ("The notification did not work ... \"%s\".\n", error->message);
                g_error_free (error);
        }

        g_signal_connect (G_OBJECT (notification), "closed", G_CALLBACK (&handle_it), NULL);

        return;
    }
    success = notify_notification_update (notification, "Format Junkie",body.toLocal8Bit().data(), "/opt/extras.ubuntu.com/formatjunkie/pixmap/fjt.png");
    error = NULL;
    success = notify_notification_show (notification, &error);
    if (!success)
    {
            g_print ("The notification did not work ... \"%s\".\n", error->message);
            g_error_free (error);
            return;
    }
    g_signal_connect (G_OBJECT (notification), "closed", G_CALLBACK (&handle_it), NULL);
}

void MainWindow::on_actionContents_F1_triggered()
{
    if(system("yelp file:///opt/extras.ubuntu.com/formatjunkie/help/C/formatjunkie.xml 2> /dev/null&"))
        cerr << "Error opening /opt/extras.ubuntu.com/formatjunkie/help/C/formatjunkie.xml with yelp! Check for file existence and/or for /usr/bin/yelp\n";
}

void MainWindow::process_dir(QString dir){
    /*
     This function that takes a QString parent dir as an argument and process it.
     Specifically, it takes  all the files from the subfolders of the 'dir' and
     adds them to the corresponding tableWidget of mainwindow.ui
     It is used from the drag and drop event (if a folder has been dropped)!
    */
    QStringList all_folders=list_folders(dir);
    int all_folders_count=all_folders.count();
    for(int i=0;i<all_folders_count;i++){
        add_files(all_folders.at(i), 3);
    }
    if(ui->tableWidget->rowCount() && ui->outputFormatComboBox->isEnabled())
        ui->startBtn->setEnabled(true);
    if(ui->tableWidget_2->rowCount() && ui->outputFormatComboBox_2->isEnabled())
        ui->startBtn_2->setEnabled(true);
    if(ui->tableWidget_3->rowCount() && ui->outputFormatComboBox_3->isEnabled())
        ui->startBtn_3->setEnabled(true);
}

void MainWindow::on_actionAdd_Audio_Files_triggered()
{
    add_audio_files();
}

void MainWindow::on_actionAdd_Video_Files_triggered()
{
    add_video_files();
}

void MainWindow::on_actionAdd_Image_Files_triggered()
{
    add_image_files();
}

void MainWindow::show_me(){
    //this function gives strong focus to the application. It's called from the trayIcon
    this->show();
    Qt::WindowFlags flags=windowFlags();
    setWindowFlags(flags|Qt::WindowStaysOnTopHint);
    setWindowFlags(flags);
    this->show();
    this->setWindowState(this->windowState() & (~Qt::WindowMinimized | Qt::WindowActive));
    delete_indicator();
}

void MainWindow::changeEvent(QEvent *e)
{
    if (indicator && e->type()==QEvent::WindowStateChange && isMinimized()){
        make_indicator();
        this->hide();
    }

    QMainWindow::changeEvent(e);
}

void MainWindow::make_indicator()
{
    QApplication::setQuitOnLastWindowClosed(false);
    if(indicator_has_been_created){
        trayIcon->show();
        return;
    }
    indicator_has_been_created=true;
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/pictures/Pictures/tray.png"));
    trayIcon->setToolTip("Format Junkie Tray Icon");

    QMenu *changer_menu = new QMenu;

    Show_action = new QAction("S&how",this);
    Show_action->setIconVisibleInMenu(true);
    connect(Show_action, SIGNAL(triggered()), this, SLOT(show_me()));
    changer_menu->addAction(Show_action);

    changer_menu->addSeparator();

    Show_outputfolder = new QAction(QIcon::fromTheme("folder"), "Open Output Folder", this);
    Show_outputfolder->setIconVisibleInMenu(true);
    connect(Show_outputfolder, SIGNAL(triggered()), this, SLOT(on_output_folder_clicked()));
    changer_menu->addAction(Show_outputfolder);

    changer_menu->addSeparator();

    Show_audio = new QAction(QIcon::fromTheme("audio-x-generic"), "Add Audio files", this);
    Show_audio->setIconVisibleInMenu(true);
    connect(Show_audio, SIGNAL(triggered()), this, SLOT(add_audio_files()));
    changer_menu->addAction(Show_audio);

    Show_video = new QAction(QIcon::fromTheme("video-x-generic"), "Add Video files", this);
    Show_video->setIconVisibleInMenu(true);
    connect(Show_video, SIGNAL(triggered()), this, SLOT(add_video_files()));
    changer_menu->addAction(Show_video);

    Show_image = new QAction(QIcon::fromTheme("image-x-generic"), "Add Image files", this);
    Show_image->setIconVisibleInMenu(true);
    connect(Show_image, SIGNAL(triggered()), this, SLOT(add_image_files()));
    changer_menu->addAction(Show_image);

    changer_menu->addSeparator();

    Show_preferences = new QAction("Preferences", this);
    Show_preferences->setIconVisibleInMenu(true);
    connect(Show_preferences, SIGNAL(triggered()), this, SLOT(on_actionPreferences_triggered()));
    changer_menu->addAction(Show_preferences);

    Show_about = new QAction("About", this);
    Show_about->setIconVisibleInMenu(true);
    connect(Show_about, SIGNAL(triggered()), this, SLOT(on_actionAbout_triggered()));
    changer_menu->addAction(Show_about);

    changer_menu->addSeparator();

    Quit_action = new QAction("&Quit", this);
    Quit_action->setIconVisibleInMenu(true);
    connect(Quit_action, SIGNAL(triggered()), this, SLOT(close()));
    changer_menu->addAction(Quit_action);


    trayIcon->setContextMenu(changer_menu);
    trayIcon->show();
}

void MainWindow::image_read_output(){
    //read all possible output so as to calculate the percentages...
    QByteArray newData_stdout=image_converter->readAllStandardOutput();
    QString stdout=QString::fromLocal8Bit(newData_stdout);
    QByteArray newData_stderr=image_converter->readAllStandardError();
    QString stderr=QString::fromLocal8Bit(newData_stderr);
    QString out=stdout+stderr;
    current_file_errors+=out;
}

void MainWindow::image_conversion(int return_code){
    QString format_text=ui->outputFormatComboBox_3->currentText();

    if(image_stop_pressed){
        if(ui->tableWidget_3->item(current_file-1,2)->toolTip()!="Completed")
            ui->tableWidget_3->removeCellWidget(current_file-1,2);
        else
            ui->tableWidget_3->removeCellWidget(current_file,2);
        ui->tableWidget_3->item(current_file-1,2)->setText("Waiting");
        QFileInfo inf;
        inf.setFile(output_file);
        QString output_file_rm=ui->output_folder->toolTip()+"/"+inf.completeBaseName()+"."+format_text;
        QFile::remove(output_file_rm);
        return;
    }
    if(return_code){
        //AN ERROR HAS OCCURED!
        image_conversion_result=2;
        image_conversion(0);
        return;
    }
    else
    {
        //IF NOT RETURN CODE
        /*
          The conversion of the previous file was successful!
          Check if the original (source) file is to be deleted...
        */
        if(delete_original){
            if(QFile(input_file).exists()){
                if(!QFile::remove(input_file))
                    cerr << QString("Could not delete source file '"+input_file+"'\nDo I have the permissions to do so?\n").toLocal8Bit().data();
            }
        }
    }

    if (current_file>num_files-1){
        /*
          !!!ALL IMAGE FILES HAVE BEEN CONVERTED!!!
          Update the progressbars and do the actions
          that the user has chosen to do from bottom
          right!!!
        */
        //just set to the last file the appropriate icon!
        conversion_process_on=false;
        ui->startBtn_3->setEnabled(false);
        ui->startBtn_3->raise();
        ui->progressBar_3->setFormat("100%");
        ui->progressBar_3->setValue(100);
        progressbar_animation(1,2);
        if(ui->tableWidget_3->cellWidget(current_file-1, 2))
            ui->tableWidget_3->removeCellWidget(current_file-1, 2);
        if(image_conversion_result==0){
            if(ui->tableWidget_3->item(current_file-1,2)->toolTip()!="Completed"){
                current_file_errors.clear();
                QLabel *lab = new QLabel(this);
                lab->hide();
                ui->tableWidget_3->item(current_file-1, 2)->setText("");
                lab->setPixmap(QPixmap(":/pictures/Pictures/tick.png"));
                ui->tableWidget_3->setCellWidget(current_file-1, 2, lab);
                ui->tableWidget_3->item(current_file-1,2)->setToolTip("Completed");
            }
        }
        else if(image_conversion_result==1){
            image_conversion_result=0;
            if(ui->tableWidget_3->item(current_file-1,2)->toolTip()!="Completed"){
                current_file_errors.clear();
                if(ui->tableWidget_3->cellWidget(current_file-1, 2))
                    ui->tableWidget_3->removeCellWidget(current_file-1, 2);
                ui->tableWidget_3->item(current_file-1, 2)->setText("Already Exists!");
                ui->tableWidget_3->item(current_file-1,2)->setToolTip("Completed");
            }
        }
        else if(image_conversion_result==2){
            image_conversion_result=0;
            if(ui->tableWidget_3->item(current_file-1,2)->toolTip()!="Completed"){
                error_count++;
                QLabel *lab = new QLabel(this);
                lab->hide();
                ui->tableWidget_3->item(current_file-1, 2)->setText("");
                lab->setPixmap(QPixmap(":/pictures/Pictures/error.png"));
                ui->tableWidget_3->setCellWidget(current_file-1, 2, lab);
                if(all_errors.isEmpty())
                    all_errors+="'<b>"+input_file+"</b>'<br>";
                else
                    all_errors+="\n'<b>"+input_file+"</b>'<br>";
                current_file_errors.replace("\n","<br>");
                all_errors+="<font color=red>"+current_file_errors+"</font>";
                QTableWidgetItem *item_error1 = new QTableWidgetItem;
                item_error1->setText(ui->tableWidget_3->item(current_file-1,0)->text()+" - ERROR");
                item_error1->setToolTip(ui->tableWidget_3->item(current_file-1,0)->toolTip());
                item_error1->setBackgroundColor(Qt::red);
                item_error1->setTextColor(Qt::white);
                ui->tableWidget_3->setItem(current_file-1,0, item_error1);
                QTableWidgetItem *item_error2 = new QTableWidgetItem;
                item_error2->setText(ui->tableWidget_3->item(current_file-1,1)->text());
                item_error2->setToolTip(ui->tableWidget_3->item(current_file-1,1)->toolTip());
                item_error2->setBackgroundColor(Qt::red);
                item_error2->setTextColor(Qt::white);
                ui->tableWidget_3->setItem(current_file-1,1, item_error2);
                QTableWidgetItem *item_error3 = new QTableWidgetItem;
                item_error3->setBackgroundColor(Qt::red);
                item_error3->setTextColor(Qt::white);
                ui->tableWidget_3->setItem(current_file-1,2, item_error3);
                show_errors_timer_count=5;
                show_errors_animation();
                show_errors_timer->start(1000);
                ui->tableWidget_3->item(current_file-1,2)->setToolTip("Completed");
            }
        }
        ui->outputFormatComboBox_3->setEnabled(true);
        if(unity_progressbar)
            unity_launcher_entry_set_progress_visible(unity_progress, false);
        if(unity_count)
            unity_launcher_entry_set_count_visible(unity_progress, false);
        if(desktop_notifications)
            show_desktop_notification(0);
        error_count=0;
        actions_after_convert();
        return;
    }
    if(current_file!=0){
        /*
          Setting to the previous file the image_conversion_result.
          It will be OK (tick) if the previous conversion was OK
          ,FAIL (x) if the previous conversion has failed
          and it will be 'Already Exists' if
          the previous file already existed. This has no sense
          if current_file==0, because there isn't any previous
          file!
          So:
          image_conversion_result==0 -> OK!
          image_conversion_result==1 -> Already Exists!
          image_conversion_result==2 -> ERROR!
        */
        //removing the previous file's gif, unless it is already completed(the user has added more files after the conversion has finished)...
        if(ui->tableWidget_3->cellWidget(current_file-1, 2) && ui->tableWidget_3->item(current_file-1,2)->toolTip()!="Completed")
            ui->tableWidget_3->removeCellWidget(current_file-1, 2);
        //and setting the new...
        if(image_conversion_result==0){
            if(ui->tableWidget_3->item(current_file-1,2)->toolTip()!="Completed"){
                current_file_errors.clear();
                QLabel *lab = new QLabel(this);
                lab->hide();
                ui->tableWidget_3->item(current_file-1, 2)->setText("");
                lab->setPixmap(QPixmap(":/pictures/Pictures/tick.png"));
                ui->tableWidget_3->setCellWidget(current_file-1, 2, lab);
                ui->tableWidget_3->item(current_file-1,2)->setToolTip("Completed");
            }
        }
        else if(image_conversion_result==1){
            image_conversion_result=0;
            if(ui->tableWidget_3->item(current_file-1,2)->toolTip()!="Completed"){
                current_file_errors.clear();
                ui->tableWidget_3->item(current_file-1, 2)->setText("Already Exists!");
                ui->tableWidget_3->item(current_file-1,2)->setToolTip("Completed");
            }
        }
        else if(image_conversion_result==2){
            image_conversion_result=0;
            if(ui->tableWidget_3->item(current_file-1,2)->toolTip()!="Completed"){
                error_count++;
                QLabel *lab = new QLabel(this);
                lab->hide();
                ui->tableWidget_3->item(current_file-1, 2)->setText("");
                lab->setPixmap(QPixmap(":/pictures/Pictures/error.png"));
                ui->tableWidget_3->setCellWidget(current_file-1, 2, lab);
                if(all_errors.isEmpty())
                    all_errors+="'<b>"+input_file+"</b>'<br>";
                else
                    all_errors+="\n'<b>"+input_file+"</b>'<br>";
                current_file_errors.replace("\n","<br>");
                all_errors+="<font color=red>"+current_file_errors+"</font>";
                QTableWidgetItem *item_error1 = new QTableWidgetItem;
                item_error1->setText(ui->tableWidget_3->item(current_file-1,0)->text()+" - ERROR");
                item_error1->setToolTip(ui->tableWidget_3->item(current_file-1,0)->toolTip());
                item_error1->setBackgroundColor(Qt::red);
                item_error1->setTextColor(Qt::white);
                ui->tableWidget_3->setItem(current_file-1,0, item_error1);
                QTableWidgetItem *item_error2 = new QTableWidgetItem;
                item_error2->setText(ui->tableWidget_3->item(current_file-1,1)->text());
                item_error2->setToolTip(ui->tableWidget_3->item(current_file-1,1)->toolTip());
                item_error2->setBackgroundColor(Qt::red);
                item_error2->setTextColor(Qt::white);
                ui->tableWidget_3->setItem(current_file-1,1, item_error2);
                QTableWidgetItem *item_error3 = new QTableWidgetItem;
                item_error3->setBackgroundColor(Qt::red);
                item_error3->setTextColor(Qt::white);
                ui->tableWidget_3->setItem(current_file-1,2, item_error3);
                show_errors_timer_count=5;
                show_errors_animation();
                show_errors_timer->start(1000);
                ui->tableWidget_3->item(current_file-1,2)->setToolTip("Completed");
            }
        }
    }

    input_file=ui->tableWidget_3->item(current_file,0)->toolTip();

    QFileInfo inf;
    inf.setFile(input_file);
    output_file=ui->output_folder->toolTip()+"/"+inf.completeBaseName()+"."+format_text;

    if(QFile(output_file).exists()){
        if(overwrite_existing){
            if(!QFile(output_file).remove())
                cerr << QString("Error while removing the already existing file '"+output_file+"'\n").toLocal8Bit().data();
        }
        else
        {//overwrite_existing is false, but the output file already exists!
            //continue to the next file...
            image_conversion_result=1;
            current_file++;
            image_conversion(0);
            return;
        }
    }
    command="convert";
    command_arguments.clear();
    command_arguments << input_file;
    if(image_crop){
        command_arguments << "-crop" << QString::number(image_width_value)+"x"+QString::number(image_height_value)+"+"+QString::number(image_x_value)+"+"+QString::number(image_y_value) << "+repage";
    }
    if(image_width_height){
        command_arguments << "-resize" << QString::number(image_width)+"x"+QString::number(image_height);
        if(!image_respect_ratio)
            command_arguments << "!";
    }
    if(image_colors_number){
        command_arguments << "-colors" << QString::number(image_colors_number_value);
    }
    if(image_comment){
        command_arguments << "-comment" << image_comment_value;
    }
    command_arguments << "-quality" << QString::number(quality) << output_file;
    //setting the looping gif to the current file...
    QMovie *mov= new QMovie(this);
    mov->setFileName(":/pictures/Pictures/c.gif");
    QLabel *lab = new QLabel(this);
    lab->hide();
    ui->tableWidget_3->item(current_file, 2)->setText("");
    lab->setMovie(mov);
    ui->tableWidget_3->setCellWidget(current_file, 2, lab);
    mov->start();
    image_converter->start(command,command_arguments);
    update_unity_count_value(0);
    ui->progressBar_3->setValue((current_file-already_completed)*100/(num_files-already_completed));
    ui->progressBar_3->setFormat(QString::number((current_file-already_completed)*100/(num_files-already_completed))+"%");
    current_file++;
}

void MainWindow::on_startBtn_3_clicked()
{
    if(!ui->tableWidget_3->rowCount())
        return;
    if(!QDir(ui->output_folder->toolTip()).exists()){
        QDir dir;
        if(!dir.mkpath(ui->output_folder->toolTip())){
            QMessageBox::warning(this, tr("Error"), tr("The output folder doesn't exist! Please specify an existent directory!"));
            return;
        }
    }
    if(conversion_process_on){
        QMessageBox::warning(this, tr("Error"), tr("Please wait the other conversion processes to finish before starting a new one!"));
        return;
    }
    conversion_process_on=true;
    image_stop_pressed=false;
    num_files=ui->tableWidget_3->rowCount();
    if(unity_count)
        unity_launcher_entry_set_count (unity_progress, num_files);
    ui->progressBar_3->setFormat("0%");
    ui->progressBar_3->setValue(0);
    ui->stop_3->raise();
    progressbar_animation(0,2);
    already_completed=0;
    for(int i=0;i<num_files;i++){
        if(ui->tableWidget_3->item(i,2)->toolTip()!="Completed"){
            current_file=i;
            already_completed=i;
            break;
        }
    }
    if(current_file==0){
        //that's a completely new conversion
        current_file_errors.clear();
        all_errors.clear();
        error_count=0;
        ui->show_errors->hide();
    }
    ui->outputFormatComboBox_3->setEnabled(false);
    if(unity_progressbar)
        unity_launcher_entry_set_progress_visible(unity_progress, true);
    if(unity_count)
        unity_launcher_entry_set_count_visible(unity_progress, true);
    image_conversion(0);
}

void MainWindow::on_stop_3_clicked()
{
    conversion_process_on=false;
    image_stop_pressed=true;
    progressbar_animation(1,2);
    ui->startBtn_3->raise();
    if(image_converter->isOpen())
        image_converter->close();
    ui->outputFormatComboBox_3->setEnabled(true);
    if(unity_progressbar)
        unity_launcher_entry_set_progress_visible(unity_progress, false);
    if(unity_count)
        unity_launcher_entry_set_count_visible(unity_progress, false);
}

void MainWindow::on_startBtn_2_clicked()
{
    if(!ui->tableWidget_2->rowCount())
        return;
    if(!QDir(ui->output_folder->toolTip()).exists()){
        QDir dir;
        if(!dir.mkpath(ui->output_folder->toolTip())){
            QMessageBox::warning(this, tr("Error"), tr("The output folder doesn't exist! Please specify an existent directory!"));
            return;
        }
    }
    if(conversion_process_on){
        QMessageBox::warning(this, tr("Error"), tr("Please wait the other conversion processes to finish before starting a new one!"));
        return;
    }
    conversion_process_on=true;
    video_stop_pressed=false;
    num_files=ui->tableWidget_2->rowCount();
    if(unity_count)
        unity_launcher_entry_set_count (unity_progress, num_files);
    ui->progressBar_2->setFormat("0%");
    ui->progressBar_2->setValue(0);
    ui->stop_2->raise();
    progressbar_animation(0,1);
    already_completed=0;
    for(int i=0;i<num_files;i++){
        if(ui->tableWidget_2->item(i,2)->toolTip()!="Completed"){
            current_file=i;
            already_completed=i;
            break;
        }
    }
    if(current_file==0){
        //that's a completely new conversion
        all_errors.clear();
        error_count=0;
        ui->show_errors->hide();
    }
    ui->outputFormatComboBox_2->setEnabled(false);
    globpro_string="100%";
    if(unity_progressbar)
        unity_launcher_entry_set_progress_visible(unity_progress, true);
    if(unity_count)
        unity_launcher_entry_set_count_visible(unity_progress, true);
    video_conversion(0);
}

void MainWindow::video_conversion(int return_code){
    /*
      This function is called like video_conversion(0) the 1st time
      so as to begin the conversion. Then, it is called again
      when the 1st conversion has finished and launches the
      2nd conversion and so on, untill no files are left
      for conversion.
    */
    //Values:
    //current_file <-> 0 1 2
    //num_files    <-> 1 2 3
    QString format_text=ui->outputFormatComboBox_2->currentText();

        if(video_stop_pressed){
            if(current_file!=0)
                ui->tableWidget_2->removeCellWidget(current_file-1,2);
            else
                ui->tableWidget_2->removeCellWidget(current_file,2);
            globpro_hasbeendeleted=true;
            if(!QFile::remove(output_file))
                cerr << QString("Error removing file '"+output_file+"'\n").toLocal8Bit().data();
            return;
        }
        if(return_code){
            /*
              The conversion of the previous file wasn't successful!
              Print error and guide the user to see the error logs...
            */
            if(all_errors.isEmpty())
                all_errors+="'<b>"+input_file+"</b>'<br>";
            else
                all_errors+="\n'<b>"+input_file+"</b>'<br>";
            current_file_errors.replace("\n","<br>");
            all_errors+="<font color=red>"+current_file_errors+"</font><br>";
            int to_apply=current_file;
            if(to_apply>0)
                to_apply--;
            QTableWidgetItem *item_error1 = new QTableWidgetItem;
            item_error1->setText(ui->tableWidget_2->item(to_apply,0)->text()+" - ERROR");
            item_error1->setToolTip(ui->tableWidget_2->item(to_apply,0)->toolTip());
            item_error1->setBackgroundColor(Qt::red);
            item_error1->setTextColor(Qt::white);
            ui->tableWidget_2->setItem(to_apply,0, item_error1);
            QTableWidgetItem *item_error2 = new QTableWidgetItem;
            item_error2->setText(ui->tableWidget_2->item(to_apply,1)->text());
            item_error2->setToolTip(ui->tableWidget_2->item(to_apply,1)->toolTip());
            item_error2->setBackgroundColor(Qt::red);
            item_error2->setTextColor(Qt::white);
            ui->tableWidget_2->setItem(to_apply,1, item_error2);
            show_errors_timer_count=5;
            show_errors_animation();
            show_errors_timer->start(1000);
            current_file_errors.clear();
            error_count++;
        }
        else
        {
            //IF NOT RETURN CODE
            current_file_errors.clear();
            /*
              The conversion of the previous file was successful!
              Check if the original (source) file is to be deleted...
            */
            if(delete_original){
                if(QFile(input_file).exists()){
                    if(!QFile::remove(input_file))
                        cerr << QString("Could not delete source file '"+input_file+"'\nDo I have the permissions to do so?\n").toLocal8Bit().data();
                }
            }
        }
        if (current_file>num_files-1){
            /*
              !!!ALL FILES HAVE BEEN CONVERTED!!!
              Update the progressbars and do the actions
              that the user has chosen to do from bottom
              right!!!
            */
            //all files have been converted!
            //just update the progressbar
            conversion_process_on=false;
            ui->progressBar_2->setFormat("100%");
            ui->progressBar_2->setValue(100);
            globpro->setFormat(globpro_string);
            globpro->setValue(100);
            QTableWidgetItem *item_progress = new QTableWidgetItem;
            item_progress->setToolTip("Completed");
            ui->tableWidget_2->setItem(current_file-1,2, item_progress);
            ui->startBtn_2->setEnabled(false);
            ui->startBtn_2->raise();
            progressbar_animation(1,1);
            ui->outputFormatComboBox_2->setEnabled(true);
            if(unity_progressbar)
                unity_launcher_entry_set_progress_visible(unity_progress, false);
            if(unity_count)
                unity_launcher_entry_set_count_visible(unity_progress, false);
            if(desktop_notifications)
                show_desktop_notification(0);
            error_count=0;
            actions_after_convert();
            return;
        }
        if(ui->tableWidget_2->item(current_file,2)->toolTip()=="Completed"){
            /*
              The file's conversion has been completed, proceed to the next file...
              The program goes through here if the user add more files after the
              previous conversion has finished and choose to start converting...
            */
            current_file++;
            video_conversion(0);
            return;
        }
        if(globpro_string=="Already Exists" || !m4r_or_a_time){
            if(current_file!=0){
                if(globpro_hasbeendeleted){
                    globpro = new QProgressBar(this);
                    globpro_hasbeendeleted=false;
                }
                if(globpro->format()!="Already Exists" && ui->tableWidget_2->item(current_file-1,2)->toolTip()!="Completed"){
                    QProgressBar *qtpro=new QProgressBar(this);
                    ui->tableWidget_2->setCellWidget(current_file-1, 2, qtpro);
                    QTableWidgetItem *item_progress = new QTableWidgetItem;
                    item_progress->setToolTip("Completed");
                    ui->tableWidget_2->setItem(current_file-1,2, item_progress);
                    qtpro->setFormat(globpro_string);
                    qtpro->setValue(100);
                }
            }
            QProgressBar *qtpro=new QProgressBar(this);
            ui->tableWidget_2->setCellWidget(current_file, 2, qtpro);
            if(!globpro)
                globpro = new QProgressBar(this);
            globpro=qtpro;
        }
        //Action starts here...
        input_file=ui->tableWidget_2->item(current_file,0)->toolTip();
        QFileInfo info;
        info.setFile(input_file);
        output_file=ui->output_folder->toolTip()+"/"+info.completeBaseName()+"."+format_text;
        command="avconv";
        command_arguments.clear();
        command_arguments << "-i" << input_file << "-qscale" << "1";
        //Custom commands for the right extensions
        if(format_text=="mov" ||format_text=="vob" ||format_text=="mp4" ||format_text=="mpg" ||format_text=="wmv" ||format_text=="mkv"){
            if(info.completeSuffix()=="flv")
                command_arguments << "-ar" << "44100" << "-f" << "flv";
            else if(info.completeSuffix()=="3gp"||info.completeSuffix()=="ogv")
                command_arguments << "-ar" << "44100" << "-strict" << "experimental";
            else
                command_arguments << "-strict" << "experimental";
        }
        else if(format_text=="3gp"){
            command_arguments << "-acodec" << "aac" << "-s" << "qcif" << "-ar" << "8000" << "-b" << "120000" << "-vcodec" << "h263" << "-ab" << "10.2k" << "-ac" << "1" << "-strict" << "experimental";
        }
        else if(format_text=="avi"){
            if(info.completeSuffix()=="flv")
                command_arguments << "-ar" << "44100" << "-f" << "flv";
            else if(info.completeSuffix()=="ogv")
                command_arguments << "-vcodec" << "mpeg4" << "-acodec" << "copy";
            else
                command_arguments << "-vcodec" << "copy" << "-acodec" << "copy";
        }
        else if(format_text=="flv" || format_text=="ogv")
            command_arguments << "-ar" << "44100" << "-f" << "flv";
        else if(format_text=="webm")
        {
            if(!video_bitrate)
                command_arguments << "-b" << "3900k" << "-f" << "webm";
            else
                command_arguments << "-f" << "webm";
        }

        if(video_bitrate)
            command_arguments << "-b:v" << QString::number(video_bitrate_value);
        if(video_framerate)
            command_arguments << "-r" << QString::number(video_framerate_value);
        if(video_crop)
            command_arguments << "-vf" << "crop="+QString::number(video_width_value)+":"+QString::number(video_height_value)+":"+QString::number(video_x_value)+":"+QString::number(video_y_value);
        if(video_resize)
            command_arguments << "-s" << video_resize_value;

        command_arguments << output_file;

        /*
          Checking whether the output_file already exists. If it does, then
          overwrite it, if it checked so, or proceed to the next file, if not...
         */
        if(QFile(output_file).exists()){
            if(overwrite_existing){
                if(!QFile::remove(output_file)){
                    cerr << QString("Couldn't remove the already existing file '"+output_file+"'\n").toLocal8Bit().data();
                    current_file++;
                    globpro_string="Already Exists";
                    video_conversion(0);
                    return;
                }
            }
            else
            {
                current_file++;
                globpro_string="Already Exists";
                video_conversion(0);
                return;
            }
        }
        //getting the duration of the input file...
        duration_secs=Global::duration(input_file);
        globpro_string="100%";
        video_converter->start(command, command_arguments);
        if(full_percent_instant){
            full_percent_instant=false;
            globpro->setValue(100);
            globpro->setFormat(globpro_string);
        }
        update_unity_count_value(0);
        current_file++;
}

void MainWindow::video_read_output(){
    //read all possible output so as to calculate the percentages...
    QByteArray newData_stdout=video_converter->readAllStandardOutput();
    QString stdout=QString::fromLocal8Bit(newData_stdout);
    QByteArray newData_stderr=video_converter->readAllStandardError();
    QString stderr=QString::fromLocal8Bit(newData_stderr);
    QString out=stdout+stderr;
    current_file_errors+=out;
    if(out.contains("kB time=")){

        QRegExp regex("time=.* ");
        regex.setMinimal(true);

        QStringList list;
        int pos = 0;

        while ((pos = regex.indexIn(out, pos)) != -1)
        {
            list << regex.cap(0).remove(QChar(' '),Qt::CaseInsensitive);
            pos += regex.matchedLength();
        }
        if(!list.count()){
            //error
            return;
        }
        QString current=list.at(0);
        current.replace("time=","");
        float cur_time=current.toFloat();
        int percent=0;
            percent=cur_time*100/duration_secs;

        globpro->setFormat(QString::number(percent)+"%");
        globpro->setValue(percent);
        ui->progressBar_2->setFormat(QString::number(percent/(num_files-already_completed)+(current_file-1-already_completed)*(100/(num_files-already_completed)))+"%");
        ui->progressBar_2->setValue(percent/(num_files-already_completed)+(current_file-1-already_completed)*(100/(num_files-already_completed)));
    }
}

void MainWindow::on_stop_2_clicked()
{
    conversion_process_on=false;
    video_stop_pressed=true;
    progressbar_animation(1,1);
    ui->startBtn_2->raise();
    if(video_converter->isOpen())
        video_converter->close();
    ui->outputFormatComboBox_2->setEnabled(true);
    if(unity_progressbar)
        unity_launcher_entry_set_progress_visible(unity_progress, false);
    if(unity_count)
        unity_launcher_entry_set_count_visible(unity_progress, false);
}

void MainWindow::on_radioButton_files_to_iso_clicked()
{
    ui->stackedWidget_3->setCurrentIndex(0);
    ui->addBtn->show();
    ui->addfolder->show();
    ui->clearBtn->show();
    ui->removeBtn->show();
    ui->movedownButton->show();
    ui->moveupButton->show();
}

void MainWindow::on_radioButton_iso_cso_clicked()
{
    ui->stackedWidget_3->setCurrentIndex(1);
    ui->addBtn->hide();
    ui->addfolder->hide();
    ui->clearBtn->hide();
    ui->removeBtn->hide();
    ui->movedownButton->hide();
    ui->moveupButton->hide();
}

void MainWindow::on_startBtn_8_clicked()
{
    if(ui->lineEdit_6->text().contains("/")){
        QMessageBox::warning(this, "Error", "The output filename should not contain the '/' character!");
        return;
    }
    QString output_iso=ui->output_folder->toolTip()+"/"+ui->lineEdit_6->text()+".iso";
    if(QFile(output_iso).exists()){
        if(overwrite_existing){
            if(!QFile::remove(output_iso)){
                QMessageBox::warning(this, "Error", "The file '"+output_iso+"' already exists, but I cannot overwrite it!");
                return;
            }
        }
        else{
            if(desktop_notifications){
                file_exists=output_iso;
                show_desktop_notification(1);
                return;
            }
            else
            {
                QMessageBox::warning(this, "Error", "The file '"+output_iso+"' already exists!");
                return;
            }
        }
    }

    command="genisoimage"; command_arguments.clear();
    command_arguments << "-o" << output_iso;
    int table_count=ui->tableWidget_6->rowCount();
    for(int i=0;i<table_count;i++){
        QString file=ui->tableWidget_6->item(i,0)->toolTip();
        if(!QFile(file).exists()){
            if(desktop_notifications){
                file_not_exists=file;
                show_desktop_notification(3);
                return;
            }
            else
            {
                QMessageBox::warning(this, "Error", "The file '"+file+"' doesn't exist!");
                return;
            }
        }
        command_arguments << file;
    }
    ui->loading_label->show();
    if(!ui->loading_label->movie()){
        QMovie *mov= new QMovie(this);
        mov->setFileName(":/pictures/Pictures/c.gif");
        ui->loading_label->setMovie(mov);
        mov->start();
    }
    else
        ui->loading_label->movie()->start();

    files_to_iso->start(command, command_arguments);
    ui->startBtn_8->hide();
    ui->stop_6->show();
    ui->lineEdit_6->setReadOnly(true);
    ui->tableWidget_6->setFocus();
}

void MainWindow::files_to_iso_end(int return_code)
{
    ui->stop_6->hide();
    ui->startBtn_8->show();
    ui->lineEdit_6->setReadOnly(false);
    ui->loading_label->movie()->stop();
    ui->loading_label->hide();
    if(return_code){
        //something went wrong
        if(desktop_notifications)
            show_desktop_notification(5);
        else
            QMessageBox::warning(this, "Error", "The creation of the iso file has failed!");
    }
    else
    {
    if(desktop_notifications)
        show_desktop_notification(2);
    else
        QMessageBox::information(this, "Success!", "Your iso file has been created successfully!");
    }
}

void MainWindow::on_stop_6_clicked()
{
    files_to_iso->close();
}

void MainWindow::on_select_iso_clicked()
{
    QString path;
    path = QFileDialog::getOpenFileName(this, "Choose Iso File", QDir::homePath(), "*.iso");

    if(!path.isEmpty()){
        ui->iso_to_cso_file->setText(path);
        on_iso_to_cso_output_textEdited();
    }

}

void MainWindow::on_select_cso_clicked()
{
    QString path;
    path = QFileDialog::getOpenFileName(this, "Choose Iso File", QDir::homePath(), "*.cso");

    if(!path.isEmpty()){
        ui->cso_to_iso_file->setText(path);
        on_cso_to_iso_output_textEdited();
    }
}

void MainWindow::on_iso_to_cso_start_clicked()
{
    if(overwrite_existing && QFile(ui->output_folder->toolTip()+"/"+ui->iso_to_cso_output->text()+".cso").exists()){
        if(!QFile::remove(ui->output_folder->toolTip()+"/"+ui->cso_to_iso_output->text()+".cso"))
            cerr << QString("Couldn't delete '"+ui->output_folder->toolTip()+"/"+ui->cso_to_iso_output->text()+".cso"+"'\nPlease, unless the file is problematic, report this bug and attach the file that it caused it!\n").toLocal8Bit().data();
    }
    else if(QFile(ui->output_folder->toolTip()+"/"+ui->iso_to_cso_output->text()+".cso").exists()){
        file_exists=ui->output_folder->toolTip()+"/"+ui->iso_to_cso_output->text()+".cso";
        if(desktop_notifications)
            show_desktop_notification(1);
        else{
            QMessageBox::warning(this, "Error", "The file '"+file_exists+"' already exists!");
            return;
        }

        return;
    }
    if(!QFile(ui->iso_to_cso_file->text()).exists()){
        file_not_exists=ui->iso_to_cso_file->text();
        if(desktop_notifications)
            show_desktop_notification(3);
        else{
            QMessageBox::warning(this, "Error", "The file '"+file_not_exists+"' doesn't exist!");
            return;
        }
        return;
    }
    command="ciso"; command_arguments.clear();
    command_arguments << QString::number(ui->iso_to_cso_spinBox->value()) << ui->iso_to_cso_file->text() << ui->output_folder->toolTip()+"/"+ui->iso_to_cso_output->text()+".cso";
    ui->loading_label_2->show();
    if(!ui->loading_label_2->movie()){
        QMovie *mov= new QMovie(this);
        mov->setFileName(":/pictures/Pictures/c.gif");
        ui->loading_label_2->setMovie(mov);
        mov->start();
    }
    else
        ui->loading_label_2->movie()->start();

    iso_to_cso->start(command, command_arguments);
    ui->select_iso->setEnabled(false);
    ui->iso_to_cso_output->setReadOnly(true);
    ui->iso_to_cso_spinBox->setEnabled(false);
    ui->iso_to_cso_start->hide();
    ui->iso_to_cso_stop->show();
}

void MainWindow::on_iso_to_cso_output_textEdited()
{
    if(!ui->iso_to_cso_output->text().isEmpty()&&!ui->iso_to_cso_file->text().isEmpty()&&!ui->iso_to_cso_start->isEnabled())
      ui->iso_to_cso_start->setEnabled(true);
    else if(ui->iso_to_cso_output->text().isEmpty()||ui->iso_to_cso_file->text().isEmpty())
      ui->iso_to_cso_start->setEnabled(false);
}

void MainWindow::on_cso_to_iso_output_textEdited()
{
    if(!ui->cso_to_iso_output->text().isEmpty()&&!ui->cso_to_iso_file->text().isEmpty()&&!ui->cso_to_iso_start->isEnabled())
      ui->cso_to_iso_start->setEnabled(true);
    else if(ui->cso_to_iso_output->text().isEmpty()||ui->cso_to_iso_file->text().isEmpty())
      ui->cso_to_iso_start->setEnabled(false);
}

void MainWindow::on_cso_to_iso_start_clicked()
{
    if(overwrite_existing && QFile(ui->output_folder->toolTip()+"/"+ui->cso_to_iso_output->text()+".iso").exists()){
        if(!QFile::remove(ui->output_folder->toolTip()+"/"+ui->cso_to_iso_output->text()+".iso"))
            cerr << QString("Couldn't delete '"+ui->output_folder->toolTip()+"/"+ui->cso_to_iso_output->text()+".iso"+"'\nPlease, unless the file is problematic, report this bug and attach the file that it caused it!\n").toLocal8Bit().data();
    }
    else if(QFile(ui->output_folder->toolTip()+"/"+ui->cso_to_iso_output->text()+".iso").exists()){
        file_exists=ui->output_folder->toolTip()+"/"+ui->cso_to_iso_output->text()+".iso";
        if(desktop_notifications)
            show_desktop_notification(1);
        else{
            QMessageBox::warning(this, "Error", "The file '"+file_exists+"' already exists!");
            return;
        }
        return;
    }
    if(!QFile(ui->cso_to_iso_file->text()).exists()){
        file_not_exists=ui->cso_to_iso_file->text();
        if(desktop_notifications)
            show_desktop_notification(3);
        else{
            QMessageBox::warning(this, "Error", "The file '"+file_not_exists+"' doesn't exist!");
            return;
        }
        return;
    }
    command="ciso"; command_arguments.clear();
    command_arguments << "0" << ui->cso_to_iso_file->text() << ui->output_folder->toolTip()+"/"+ui->cso_to_iso_output->text()+".iso";
    ui->loading_label_3->show();
    if(!ui->loading_label_3->movie()){
        QMovie *mov= new QMovie(this);
        mov->setFileName(":/pictures/Pictures/c.gif");
        ui->loading_label_3->setMovie(mov);
        mov->start();
    }
    else
        ui->loading_label_3->movie()->start();
    cso_to_iso->start(command,command_arguments);
    ui->select_cso->setEnabled(false);
    ui->cso_to_iso_output->setReadOnly(true);
    ui->cso_to_iso_start->hide();
    ui->cso_to_iso_stop->show();
}

void MainWindow::iso_to_cso_end(int return_code)
{
    if(return_code){
        if(desktop_notifications)
            show_desktop_notification(6);
        else
            QMessageBox::warning(this, "Error", "Your CSO file wasn't created successfully!");
    }
    else
    {
        if(!iso_to_cso_stop_pressed){
            if(desktop_notifications)
                show_desktop_notification(4);
            else
                QMessageBox::information(this, "Success!","Your CSO file has been successfully created!");
        }
     }
    ui->loading_label_2->movie()->stop();
    ui->loading_label_2->hide();
    ui->iso_to_cso_stop->hide();
    ui->iso_to_cso_start->show();
    ui->iso_to_cso_output->setReadOnly(false);
    ui->select_iso->setEnabled(true);
    ui->iso_to_cso_spinBox->setEnabled(true);
}

void MainWindow::cso_to_iso_end(int return_code)
{
    //5
    if(return_code){
        if(desktop_notifications)
            show_desktop_notification(5);
        else
            QMessageBox::warning(this, "Error", "Error creating your ISO file!");
    }
    else
    {
        if(!cso_to_iso_stop_pressed){
            if(desktop_notifications)
                show_desktop_notification(2);
            else
                QMessageBox::information(this, "Success!","Your ISO file has been successfully created!");
        }
    }
    ui->loading_label_3->movie()->stop();
    ui->loading_label_3->hide();
    ui->cso_to_iso_stop->hide();
    ui->cso_to_iso_start->show();
    ui->cso_to_iso_output->setReadOnly(false);
    ui->select_cso->setEnabled(true);
}

void MainWindow::on_iso_to_cso_stop_clicked()
{
    iso_to_cso_stop_pressed=true;
    iso_to_cso->close();
    ui->iso_to_cso_stop->hide();
    ui->iso_to_cso_start->show();
    if(!QFile::remove(ui->output_folder->toolTip()+"/"+ui->iso_to_cso_output->text()+".cso"))
        cerr << QString("Couldn't delete '"+ui->output_folder->toolTip()+"/"+ui->iso_to_cso_output->text()+".cso"+"'\nPlease, unless the file is problematic, report this bug and attach the file that it caused it!\n").toLocal8Bit().data();
}

void MainWindow::on_cso_to_iso_stop_clicked()
{
    cso_to_iso_stop_pressed=true;
    cso_to_iso->close();
    ui->cso_to_iso_stop->hide();
    ui->cso_to_iso_start->show();
    if(!QFile::remove(ui->output_folder->toolTip()+"/"+ui->iso_to_cso_output->text()+".cso"))
        cerr << QString("Couldn't delete '"+ui->output_folder->toolTip()+"/"+ui->iso_to_cso_output->text()+".cso"+"'\nPlease, unless the file is problematic, report this bug and attach the file that it caused it!\n").toLocal8Bit().data();
}

void MainWindow::open_containing_folder(){
    if(QFile("/usr/bin/nautilus").exists() && !system("pidof nautilus > /dev/null")){
        //nautilus is installed and it runs! Better open the file with it, so as to be selected!
        if(cur_index==0){
            if(system(QString("nautilus \""+ui->tableWidget->item(ui->tableWidget->currentRow(), 0)->toolTip()+"\"").toLocal8Bit().data()))
                cerr << "Could not open containing folder with nautilus.\n";
        }
        else if(cur_index==1){
            if(system(QString("nautilus \""+ui->tableWidget_2->item(ui->tableWidget_2->currentRow(), 0)->toolTip()+"\"").toLocal8Bit().data()))
                cerr << "Could not open containing folder with nautilus.\n";
        }
        else if(cur_index==2){
            if(system(QString("nautilus \""+ui->tableWidget_3->item(ui->tableWidget_3->currentRow(), 0)->toolTip()+"\"").toLocal8Bit().data()))
                cerr << "Could not open containing folder with nautilus.\n";
        }
    }
    else
    {
        QFileInfo inf;
        if(cur_index==0)
            inf.setFile(ui->tableWidget->item(ui->tableWidget->currentRow(), 0)->toolTip());
        else if(cur_index==1)
                inf.setFile(ui->tableWidget_2->item(ui->tableWidget_2->currentRow(), 0)->toolTip());
        else if(cur_index==1)
                inf.setFile(ui->tableWidget_3->item(ui->tableWidget_3->currentRow(), 0)->toolTip());
        if(system(QString("xdg-open \""+inf.path()+"\"").toLocal8Bit().data()))
            cerr << "Could not open containing folder.\n";
    }
}

void MainWindow::show_audio_video_item_properties(){
    if(cur_index==0){
        if(!QFile(ui->tableWidget->item(ui->tableWidget->currentRow(), 0)->toolTip()).exists()){
            QMessageBox::information(this, "Error", "The file you specified does not exist!");
            return;
        }
        prop=new audio_video_properties(ui->tableWidget->item(ui->tableWidget->currentRow(), 0)->toolTip(), this);
    }
    else if(cur_index==1){
        if(!QFile(ui->tableWidget_2->item(ui->tableWidget_2->currentRow(), 0)->toolTip()).exists()){
            QMessageBox::information(this, "Error", "The file you specified does not exist!");
            return;
        }
        prop=new audio_video_properties(ui->tableWidget_2->item(ui->tableWidget_2->currentRow(), 0)->toolTip(), this);
    }
    prop->exec();
}

void MainWindow::show_image_properties(){
    if(!QFile(ui->tableWidget_3->item(ui->tableWidget_3->currentRow(), 0)->toolTip()).exists()){
        QMessageBox::information(this, "Error", "The image you specified does not exist!");
        return;
    }
    if(QImage(ui->tableWidget_3->item(ui->tableWidget_3->currentRow(), 0)->toolTip()).isNull()){
        QMessageBox::information(this, "Error", "The image you specified is invalid!");
        return;
    }
    improp=new image_properties(ui->tableWidget_3->item(ui->tableWidget_3->currentRow(), 0)->toolTip(), this);
    improp->exec();
}

void MainWindow::on_tableWidget_customContextMenuRequested()
{
    QMenu menu;
    menu.addAction("Open Folder",this,SLOT(open_containing_folder()));
    menu.addAction("Remove",this,SLOT(on_removeBtn_clicked()));
    menu.addAction("Properties",this,SLOT(show_audio_video_item_properties()));
    if (ui->tableWidget->rowCount() > 0){
        if(! ui->tableWidget->currentIndex().isValid() || !ui->tableWidget->currentItem()->isSelected())
        {
            menu.actions().at(0)->setEnabled(false);
            menu.actions().at(1)->setEnabled(false);
            menu.actions().at(2)->setEnabled(false);
        }
    menu.exec(QCursor::pos());
    }
}

void MainWindow::on_tableWidget_2_customContextMenuRequested()
{
    QMenu menu;
    menu.addAction("Open Folder",this,SLOT(open_containing_folder()));
    menu.addAction("Remove",this,SLOT(on_removeBtn_clicked()));
    menu.addAction("Properties",this,SLOT(show_audio_video_item_properties()));
    if (ui->tableWidget_2->rowCount() > 0){
        if(! ui->tableWidget_2->currentIndex().isValid() || !ui->tableWidget_2->currentItem()->isSelected())
        {
            menu.actions().at(0)->setEnabled(false);
            menu.actions().at(1)->setEnabled(false);
            menu.actions().at(2)->setEnabled(false);
        }
    menu.exec(QCursor::pos());
    }
}

void MainWindow::on_tableWidget_3_customContextMenuRequested()
{
    QMenu menu;
    menu.addAction("Open Folder",this,SLOT(open_containing_folder()));
    menu.addAction("Remove",this,SLOT(on_removeBtn_clicked()));
    menu.addAction("Properties",this,SLOT(show_image_properties()));
    if (ui->tableWidget_3->rowCount() > 0){
        if(! ui->tableWidget_3->currentIndex().isValid() || !ui->tableWidget_3->currentItem()->isSelected())
        {
            menu.actions().at(0)->setEnabled(false);
            menu.actions().at(1)->setEnabled(false);
            menu.actions().at(2)->setEnabled(false);
        }
    menu.exec(QCursor::pos());
    }
}
void MainWindow::on_selectavi_clicked()
{
    QString avi_file;
    if(QFile(QDir::homePath()+"/Videos").exists())
        avi_file=QFileDialog::getOpenFileName(this, "Select avi file", QDir::homePath()+"/Videos", "*.avi");
    else
        avi_file=QFileDialog::getOpenFileName(this, "Select avi file", QDir::homePath(), "*.avi");
    if(avi_file.isEmpty())
        return;
    ui->avifile_path->setText(avi_file);
}

void MainWindow::on_selectsubtitle_clicked()
{
    QString sub_file;
    if(QFile(QDir::homePath()+"/Videos").exists())
        sub_file=QFileDialog::getOpenFileName(this, "Select avi file", QDir::homePath()+"/Videos", "*.sub *.srt");
    else
        sub_file=QFileDialog::getOpenFileName(this, "Select avi file", QDir::homePath(), "*.sub *.srt");
    if(sub_file.isEmpty())
        return;
    ui->subtitlefile_path->setText(sub_file);
}

void MainWindow::on_after_convertion2_currentIndexChanged(int index)
{
    switch(index){
    case 0:
        ui->after_convertion_icon->setPixmap(QIcon::fromTheme("dialog-error").pixmap(22));
        break;
    case 1:
        ui->after_convertion_icon->setPixmap(QIcon::fromTheme("system-shutdown").pixmap(22));
        break;
    case 2:
        ui->after_convertion_icon->setPixmap(QIcon::fromTheme("weather-few-clouds-night").pixmap(22));
        break;
    case 3:
        ui->after_convertion_icon->setPixmap(QIcon::fromTheme("system-restart").pixmap(22));
        break;
    case 4:
        ui->after_convertion_icon->setPixmap(QIcon::fromTheme("system-lock-screen").pixmap(22));
        break;
    case 5:
        ui->after_convertion_icon->setPixmap(QIcon::fromTheme("window-close").pixmap(22));
        break;
    }
}

void MainWindow::on_encode_start_clicked()
{
    QString input_avi=ui->avifile_path->text();
    QString input_sub=ui->subtitlefile_path->text();
    if(!QFile(input_avi).exists()){
        QMessageBox::warning(this, "Error", "The avi file specified does not exist!");
        return;
    }
    if(!QFile(input_sub).exists()){
        QMessageBox::warning(this, "Error", "The subtitle file specified does not exist!");
        return;
    }
    QFileInfo inf;
    inf.setFile(input_avi);

    QString output_avi=ui->output_folder->toolTip()+"/"+inf.completeBaseName()+".avi";
    if(QFile(output_avi).exists()){
        if(overwrite_existing){
            if(!QFile::remove(output_avi)){
                QMessageBox::warning(this, "Error", "The file '"+output_avi+"' already exists, but I cannot overwrite it!");
                return;
            }
        }
        else{
            if(desktop_notifications){
                file_exists=output_avi;
                show_desktop_notification(7);
                return;
            }
            else
            {
                QMessageBox::warning(this, "Error", "The file '"+output_avi+"' already exists!");
                return;
            }
        }
    }

    command="mencoder";command_arguments.clear();
    command_arguments << input_avi << "-oac" << "copy" << "-ovc" << "lavc" << "-sub" << input_sub << "-utf8" << "-o" << output_avi;
    ui->loading_label_sub_encoding->show();
    if(!ui->loading_label->movie()){
        QMovie *mov= new QMovie(this);
        mov->setFileName(":/pictures/Pictures/c.gif");
        ui->loading_label_sub_encoding->setMovie(mov);
        mov->start();
    }
    else
        ui->loading_label->movie()->start();

    sub_encoding->start(command, command_arguments);
    ui->encode_start->hide();
    ui->encode_stop->show();
    ui->selectavi->setEnabled(false);
    ui->selectsubtitle->setEnabled(false);
}

void MainWindow::on_encode_stop_clicked()
{
    sub_stop_pressed=true;
    sub_encoding->close();
    ui->loading_label_sub_encoding->movie()->stop();
    ui->loading_label_sub_encoding->hide();
    ui->encode_stop->hide();
    ui->encode_start->show();
    ui->selectavi->setEnabled(true);
    ui->selectsubtitle->setEnabled(true);
}

void MainWindow::sub_encoding_end(int return_code){
    if(return_code){
        if(!sub_stop_pressed){
            if(desktop_notifications)
                show_desktop_notification(9);
            else
                QMessageBox::warning(this, "Error", "Your subtitles' encoding hasn't finished successfully!");
        }
    }
    else
    {
        if(!iso_to_cso_stop_pressed){
            if(desktop_notifications)
                show_desktop_notification(8);
            else
                QMessageBox::information(this, "Success!","Your subtitles' encoding was successful!");
        }
     }
    ui->loading_label_sub_encoding->movie()->stop();
    ui->loading_label_sub_encoding->hide();
    ui->encode_stop->hide();
    ui->encode_start->show();
    ui->selectavi->setEnabled(true);
    ui->selectsubtitle->setEnabled(true);
}

void MainWindow::createOptionsGroupBox()
{
    optionsGroupBox = new QGroupBox();
    optionsGroupBoxLayout = new QGridLayout;
    optionsGroupBoxLayout->addWidget(ui->widget, 0, 0, 1, 9);
    optionsGroupBoxLayout->addWidget(ui->musicButton, 0,0);
    optionsGroupBoxLayout->addWidget(ui->sep1, 0, 1);
    optionsGroupBoxLayout->addWidget(ui->videoButton,0,2);
    optionsGroupBoxLayout->addWidget(ui->sep2,0,3);
    optionsGroupBoxLayout->addWidget(ui->imageButton, 0,4);
    optionsGroupBoxLayout->addWidget(ui->sep3,0,5);
    optionsGroupBoxLayout->addWidget(ui->rom_deviceButton, 0,6);
    optionsGroupBoxLayout->addWidget(ui->sep4,0,7);
    optionsGroupBoxLayout->addWidget(ui->advancedButton, 0,8);
    optionsGroupBoxLayout->addWidget(ui->stackedWidget, 1, 0, 1, 9);
    optionsGroupBox->setLayout(optionsGroupBoxLayout);
}

void MainWindow::createButtonsLayout()
{
    buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(ui->output_folder);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(ui->label_2);
    buttonsLayout->addWidget(ui->after_convertion2);
    buttonsLayout->addWidget(ui->after_convertion_icon);
}


//Setting actions on add/add folder/remove/clear/move up/move down in every page

//Page 2 -> Video

void MainWindow::on_addBtn_2_clicked()
{
    QStringList path;
    if(QDir(QDir::homePath()+"/Videos").exists())
        path = QFileDialog::getOpenFileNames(this, "Choose Video Files", QDir::homePath()+"/Videos", "*.avi *.ogv *.vob *.mp4 *.3gp *.wmv *.mkv *.mpg *.mov *.flv");
    else
        path = QFileDialog::getOpenFileNames(this, "Choose Video Files", QDir::homePath(), "*.avi *.ogv *.vob *.mp4 *.3gp *.wmv *.mkv *.mpg *.mov *.flv");
    if(!path.count()){
        if(add_video){
            exit(0);
        }
        return;
    }
    if(add_video)
        add_video=false;
    //remove_complete();
    int path_count = path.count();
    int old_r_count=ui->tableWidget_2->rowCount();
    ui->tableWidget_2->setRowCount(ui->tableWidget_2->rowCount()+path_count);
    QFileInfo inf;
    for(int count=0; count < path_count; count++){
        QString file=path[count];
        inf.setFile(file);
        QTableWidgetItem *qtitem = new QTableWidgetItem;
        qtitem->setText(inf.completeBaseName()+"."+inf.suffix());
        qtitem->setToolTip(file);
        ui->tableWidget_2->setItem(old_r_count+count,0,qtitem);
        QTableWidgetItem *qtsize = new QTableWidgetItem;
        float size=(float)inf.size()/1048576;
        qtsize->setText(QString::number(size, 'f', 2)+" MB");
        ui->tableWidget_2->setItem(old_r_count+count,1,qtsize);
        QTableWidgetItem *qtwait = new QTableWidgetItem;
        qtwait->setText("Waiting");
        ui->tableWidget_2->setItem(old_r_count+count,2,qtwait);
    }
    if (ui->tableWidget_2->rowCount())
        ui->startBtn_2->setEnabled(true);
    num_files=ui->tableWidget_2->rowCount();
    if(!ui->outputFormatComboBox_2->isEnabled())
        update_unity_count_value(0);
}

void MainWindow::on_addfolder_2_clicked()
{
    QString qpath;
    if(QDir(QDir::homePath()+"/Videos").exists())
        qpath= QFileDialog::getExistingDirectory(this, "Choose Folder", QDir::homePath()+"/Videos");
    else
        qpath= QFileDialog::getExistingDirectory(this, "Choose Folder", QDir::homePath());
    if(!qpath.count())
        return;
    //getting all the subfolders of the folder specified...
    QStringList all_folders = list_folders(qpath);
    int all_folders_count=all_folders.count();
    for(int i=0;i<all_folders_count;i++){
        //adding the files of all subfolders...
        add_files(all_folders.at(i), cur_index);
    }

    if (ui->tableWidget_2->rowCount())
        ui->startBtn_2->setEnabled(true);
    num_files=ui->tableWidget_2->rowCount();
    if(!ui->outputFormatComboBox_2->isEnabled())
        update_unity_count_value(1);
}

void MainWindow::on_clearBtn_2_clicked()
{
    if(ui->outputFormatComboBox_2->isEnabled()){
        //the conversion process is off!
        ui->tableWidget_2->setRowCount(0);
        ui->startBtn_2->setEnabled(false);
    }
    else
    {
        //the conversion is running! Display an informative message!
        clear_convertion();
    }
}

void MainWindow::on_moveupButton_2_clicked()
{
    if(ui->tableWidget_2->rowCount()>1 && ui->tableWidget_2->selectionModel()->hasSelection() && ui->tableWidget_2->currentRow()!=0){
        if(ui->tableWidget_2->item(ui->tableWidget_2->currentRow(),2)->text()!="Waiting")
            return;
        if(ui->tableWidget_2->item(ui->tableWidget_2->currentRow()-1,2)->text()!="Waiting")
            return;
        if(ui->tableWidget_2->cellWidget(ui->tableWidget_2->currentRow(),2))
            return;
        if(ui->tableWidget_2->cellWidget(ui->tableWidget_2->currentRow()-1,2))
            return;

        QStringList item_up;
        item_up << ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()-1,0)->text();
        item_up << ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()-1,1)->text();
        item_up << ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()-1,2)->text();
        item_up << ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()-1,2)->toolTip();
        item_up << ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()-1,0)->toolTip();
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()-1,0)->setText(ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),0)->text());
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()-1,1)->setText(ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),1)->text());
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()-1,2)->setText(ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),2)->text());
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()-1,2)->setToolTip(ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),2)->toolTip());
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()-1,0)->setToolTip(ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),0)->toolTip());
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),0)->setText(item_up.at(0));
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),1)->setText(item_up.at(1));
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),2)->setText(item_up.at(2));
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),2)->setToolTip(item_up.at(3));
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),0)->setToolTip(item_up.at(4));
        ui->tableWidget_2->selectRow(ui->tableWidget_2->selectedItems().at(0)->row()-1);
    }
}

void MainWindow::on_movedownButton_2_clicked()
{
    if(ui->tableWidget_2->rowCount()>1 && ui->tableWidget_2->selectionModel()->hasSelection() && ui->tableWidget_2->currentRow()!=ui->tableWidget_2->rowCount()-1){
        if(ui->tableWidget_2->item(ui->tableWidget_2->currentRow(),2)->text()!="Waiting")
            return;
        if(ui->tableWidget_2->item(ui->tableWidget_2->currentRow()+1,2)->text()!="Waiting")
            return;
        if(ui->tableWidget_2->cellWidget(ui->tableWidget_2->currentRow(),2))
            return;
        if(ui->tableWidget_2->cellWidget(ui->tableWidget_2->currentRow()+1,2))
            return;
        QStringList item_down;
        item_down << ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()+1,0)->text();
        item_down << ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()+1,1)->text();
        item_down << ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()+1,2)->text();
        item_down << ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()+1,2)->toolTip();
        item_down << ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()+1,0)->toolTip();
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()+1,0)->setText(ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),0)->text());
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()+1,1)->setText(ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),1)->text());
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()+1,2)->setText(ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),2)->text());
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()+1,2)->setToolTip(ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),2)->toolTip());
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row()+1,0)->setToolTip(ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),0)->toolTip());
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),0)->setText(item_down.at(0));
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),1)->setText(item_down.at(1));
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),2)->setText(item_down.at(2));
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),2)->setToolTip(item_down.at(3));
        ui->tableWidget_2->item(ui->tableWidget_2->selectedItems().at(0)->row(),0)->setToolTip(item_down.at(4));
        ui->tableWidget_2->selectRow(ui->tableWidget_2->selectedItems().at(0)->row()+1);
    }
}

//Page 3 -> Image

void MainWindow::on_addBtn_3_clicked()
{
    QStringList path;
    if(QDir(QDir::homePath()+"/Pictures").exists())
        path = QFileDialog::getOpenFileNames(this, "Choose Image Files", QDir::homePath()+"/Pictures", "*.png *.jpg *.jpeg *.gif *.bmp *.svg");
    else
        path = QFileDialog::getOpenFileNames(this, "Choose Image Files", QDir::homePath(), "*.png *.jpg *.jpeg *.gif *.bmp *.svg");
    if(!path.count()){
        if(add_image){
            exit(0);
        }
        return;
    }
    if(add_image)
        add_image=false;
    //remove_complete();
    int path_count = path.count();
    int old_r_count=ui->tableWidget_3->rowCount();
    ui->tableWidget_3->setRowCount(ui->tableWidget_3->rowCount()+path_count);
    QFileInfo inf;
    for(int count=0; count < path_count; count++){
        QString file=path[count];
        inf.setFile(file);
        QTableWidgetItem *qtitem = new QTableWidgetItem;
        qtitem->setText(inf.completeBaseName()+"."+inf.suffix());
        qtitem->setToolTip(file);
        ui->tableWidget_3->setItem(old_r_count+count,0,qtitem);
        QTableWidgetItem *qtsize = new QTableWidgetItem;
        float size=(float)inf.size()/1048576;
        qtsize->setText(QString::number(size, 'f', 3)+" MB");
        ui->tableWidget_3->setItem(old_r_count+count,1,qtsize);
        QTableWidgetItem *qtwait = new QTableWidgetItem;
        qtwait->setText("Waiting");
        ui->tableWidget_3->setItem(old_r_count+count,2,qtwait);
    }
    if (ui->tableWidget_3->rowCount())
        ui->startBtn_3->setEnabled(true);
    num_files=ui->tableWidget_3->rowCount();
    if(unity_count && !ui->outputFormatComboBox_3->isEnabled())
        update_unity_count_value(0);
}

void MainWindow::on_addfolder_3_clicked()
{
    QString qpath;
    if(QDir(QDir::homePath()+"/Pictures").exists())
        qpath= QFileDialog::getExistingDirectory(this, "Choose Folder", QDir::homePath()+"/Pictures");
    else
        qpath= QFileDialog::getExistingDirectory(this, "Choose Folder", QDir::homePath());
    if(!qpath.count())
        return;
    //getting all the subfolders of the folder specified...
    QStringList all_folders = list_folders(qpath);
    int all_folders_count=all_folders.count();
    for(int i=0;i<all_folders_count;i++){
        //adding the files of all subfolders...
        add_files(all_folders.at(i), cur_index);
    }

    if (ui->tableWidget_3->rowCount())
        ui->startBtn_3->setEnabled(true);
    num_files=ui->tableWidget_3->rowCount();
    if(!ui->outputFormatComboBox_3->isEnabled())
        update_unity_count_value(1);
}

void MainWindow::on_removeBtn_3_clicked()
{
    if(!ui->tableWidget_3->rowCount())
        return;
    int cur_row=ui->tableWidget_3->currentRow();
    if(ui->tableWidget_3->cellWidget(cur_row,2) && ui->tableWidget_3->item(cur_row,2)->toolTip()!="Completed"){
        //this file is being processed
        return;
    }
    if(!ui->outputFormatComboBox_3->isEnabled()){
        if(ui->tableWidget_3->currentRow()<current_file)
            current_file--;
        else
        {
            if(unity_count)
                update_unity_count_value(2);
        }
        num_files--;
        ui->progressBar_3->setValue((current_file-1)*100/num_files);
        ui->progressBar_3->setFormat(QString::number(ui->progressBar_3->value())+"%");
    }
    if (ui->tableWidget_3->rowCount()==1)
    {
        ui->tableWidget_3->setRowCount(0);
        ui->startBtn_3->setEnabled(false);
    }
    else
    {
        if(cur_row!=0){
            if(ui->tableWidget_3->item(cur_row-1,2)->toolTip()=="Completed"){
                if((cur_row+1)==ui->tableWidget_3->rowCount())
                    ui->startBtn_3->setEnabled(false);
            }
        }
        ui->tableWidget_3->removeRow(cur_row);
    }
}

void MainWindow::on_clearBtn_3_clicked()
{
    if(ui->outputFormatComboBox_3->isEnabled()){
        //the conversion process is off!
        ui->tableWidget_3->setRowCount(0);
        ui->startBtn_3->setEnabled(false);
    }
    else
    {
        //the conversion is running! Display an informative message!
        clear_convertion();
    }
}

void MainWindow::on_moveupButton_3_clicked()
{
    if(ui->tableWidget_3->rowCount()>1 && ui->tableWidget_3->selectionModel()->hasSelection() && ui->tableWidget_3->currentRow()!=0){
        if(ui->tableWidget_3->item(ui->tableWidget_3->currentRow(),2)->text()!="Waiting")
            return;
        if(ui->tableWidget_3->item(ui->tableWidget_3->currentRow()-1,2)->text()!="Waiting")
            return;
        if(ui->tableWidget_3->cellWidget(ui->tableWidget_3->currentRow(),2))
            return;
        if(ui->tableWidget_3->cellWidget(ui->tableWidget_3->currentRow()-1,2))
            return;

        QStringList item_up;
        item_up << ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()-1,0)->text();
        item_up << ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()-1,1)->text();
        item_up << ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()-1,2)->text();
        item_up << ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()-1,2)->toolTip();
        item_up << ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()-1,0)->toolTip();
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()-1,0)->setText(ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),0)->text());
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()-1,1)->setText(ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),1)->text());
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()-1,2)->setText(ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),2)->text());
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()-1,2)->setToolTip(ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),2)->toolTip());
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()-1,0)->setToolTip(ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),0)->toolTip());
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),0)->setText(item_up.at(0));
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),1)->setText(item_up.at(1));
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),2)->setText(item_up.at(2));
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),2)->setToolTip(item_up.at(3));
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),0)->setToolTip(item_up.at(4));
        ui->tableWidget_3->selectRow(ui->tableWidget_3->selectedItems().at(0)->row()-1);
    }
}

void MainWindow::on_movedownButton_3_clicked()
{
    if(ui->tableWidget_3->rowCount()>1 && ui->tableWidget_3->selectionModel()->hasSelection() && ui->tableWidget_3->currentRow()!=ui->tableWidget_3->rowCount()-1){
        if(ui->tableWidget_3->item(ui->tableWidget_3->currentRow(),2)->text()!="Waiting")
            return;
        if(ui->tableWidget_3->item(ui->tableWidget_3->currentRow()+1,2)->text()!="Waiting")
            return;
        if(ui->tableWidget_3->cellWidget(ui->tableWidget_3->currentRow(),2))
            return;
        if(ui->tableWidget_3->cellWidget(ui->tableWidget_3->currentRow()+1,2))
            return;
        QStringList item_down;
        item_down << ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()+1,0)->text();
        item_down << ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()+1,1)->text();
        item_down << ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()+1,2)->text();
        item_down << ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()+1,2)->toolTip();
        item_down << ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()+1,0)->toolTip();
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()+1,0)->setText(ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),0)->text());
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()+1,1)->setText(ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),1)->text());
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()+1,2)->setText(ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),2)->text());
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()+1,2)->setToolTip(ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),2)->toolTip());
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row()+1,0)->setToolTip(ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),0)->toolTip());
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),0)->setText(item_down.at(0));
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),1)->setText(item_down.at(1));
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),2)->setText(item_down.at(2));
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),2)->setToolTip(item_down.at(3));
        ui->tableWidget_3->item(ui->tableWidget_3->selectedItems().at(0)->row(),0)->setToolTip(item_down.at(4));
        ui->tableWidget_3->selectRow(ui->tableWidget_3->selectedItems().at(0)->row()+1);
    }
}

//Page 4 -> Files to Iso
void MainWindow::on_addBtn_4_clicked()
{
    QStringList path;
    path = QFileDialog::getOpenFileNames(this, "Choose Files", QDir::homePath());
    if(!path.count())
        return;
    int path_count = path.count();
    int old_r_count=ui->tableWidget_6->rowCount();
    ui->tableWidget_6->setRowCount(ui->tableWidget_6->rowCount()+path_count);
    QFileInfo inf;
    for(int count=0; count < path_count; count++){
        QString file=path[count];
        inf.setFile(file);
        QTableWidgetItem *qtitem = new QTableWidgetItem;
        qtitem->setText(inf.completeBaseName()+"."+inf.suffix());
        qtitem->setToolTip(file);
        ui->tableWidget_6->setItem(old_r_count+count,0,qtitem);
        QTableWidgetItem *qtsize = new QTableWidgetItem;
        float size=(float)inf.size()/1048576;
        qtsize->setText(QString::number(size, 'f', 2)+" MB");
        ui->tableWidget_6->setItem(old_r_count+count,1,qtsize);
        QTableWidgetItem *qtwait = new QTableWidgetItem;
        qtwait->setText("Waiting");
        ui->tableWidget_6->setItem(old_r_count+count,2,qtwait);
    }
    if (ui->tableWidget_6->rowCount())
        ui->startBtn_8->setEnabled(true);
}

void MainWindow::on_addfolder_4_clicked()
{
    QString qpath;
    if(QDir(QDir::homePath()+"/Documents").exists())
        qpath= QFileDialog::getExistingDirectory(this, "Choose Folder", QDir::homePath()+"/Documents");
    else
        qpath= QFileDialog::getExistingDirectory(this, "Choose Folder", QDir::homePath());
    if(!qpath.count())
        return;
    //getting all the subfolders of the folder specified...
    int old_r_count=ui->tableWidget_6->rowCount();
    ui->tableWidget_6->setRowCount(old_r_count+1);
    QDir dir;
    dir.setPath(qpath);
    QTableWidgetItem *qtitem = new QTableWidgetItem;
    qtitem->setText(dir.dirName());
    qtitem->setToolTip(qpath);
    ui->tableWidget_6->setItem(old_r_count,0,qtitem);
    QTableWidgetItem *qtsize = new QTableWidgetItem;
    qtsize->setText(QString::number(folder_size(qpath), 'f', 1)+" MB");
    ui->tableWidget_6->setItem(old_r_count,1,qtsize);
    ui->startBtn_8->setEnabled(true);
}

void MainWindow::on_removeBtn_4_clicked()
{
    if(!ui->tableWidget_6->rowCount())
        return;
    int cur_row=ui->tableWidget_6->currentRow();
    if (ui->tableWidget_6->rowCount()==1)
    {
        ui->tableWidget_6->setRowCount(0);
        ui->startBtn_8->setEnabled(false);
    }
    else
    {
        ui->tableWidget_6->removeRow(cur_row);
    }
}

void MainWindow::on_clearBtn_4_clicked()
{
    ui->tableWidget_6->setRowCount(0);
    ui->startBtn_8->setEnabled(false);
}

void MainWindow::on_moveupButton_4_clicked()
{
    if(ui->tableWidget_6->rowCount()>1 && ui->tableWidget_6->selectionModel()->hasSelection() && ui->tableWidget_6->currentRow()!=0){
        QStringList item_up;
        item_up << ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row()-1,0)->text();
        item_up << ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row()-1,1)->text();
        item_up << ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row()-1,0)->toolTip();
        ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row()-1,0)->setText(ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row(),0)->text());
        ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row()-1,1)->setText(ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row(),1)->text());
        ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row()-1,0)->setToolTip(ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row(),0)->toolTip());
        ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row(),0)->setText(item_up.at(0));
        ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row(),1)->setText(item_up.at(1));
        ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row(),0)->setToolTip(item_up.at(2));
        ui->tableWidget_6->selectRow(ui->tableWidget_6->selectedItems().at(0)->row()-1);
    }
}

void MainWindow::on_movedownButton_4_clicked()
{
    if(ui->tableWidget_6->rowCount()>1 && ui->tableWidget_6->selectionModel()->hasSelection() && ui->tableWidget_6->currentRow()!=ui->tableWidget_6->rowCount()-1){
        QStringList item_down;
        item_down << ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row()+1,0)->text();
        item_down << ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row()+1,1)->text();
        item_down << ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row()+1,0)->toolTip();
        ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row()+1,0)->setText(ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row(),0)->text());
        ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row()+1,1)->setText(ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row(),1)->text());
        ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row()+1,0)->setToolTip(ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row(),0)->toolTip());
        ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row(),0)->setText(item_down.at(0));
        ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row(),1)->setText(item_down.at(1));
        ui->tableWidget_6->item(ui->tableWidget_6->selectedItems().at(0)->row(),0)->setToolTip(item_down.at(2));
        ui->tableWidget_6->selectRow(ui->tableWidget_6->selectedItems().at(0)->row()+1);
    }
}

void MainWindow::goto_1(){
    ui->musicButton->setChecked(true);
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::goto_2(){
    ui->videoButton->setChecked(true);
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::goto_3(){
    ui->imageButton->setChecked(true);
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::goto_4(){
    ui->rom_deviceButton->setChecked(true);
    ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::goto_5(){
    ui->advancedButton->setChecked(true);
    ui->stackedWidget->setCurrentIndex(4);
}

void MainWindow::show_errors_animation()
{
    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(this);

    if(show_errors_timer_count==1){
        opacityEffect->setOpacity(0.0);
        show_errors_timer->stop();
    }
    else if(show_errors_timer_count%2)
        opacityEffect->setOpacity(0.0);
    else
        opacityEffect->setOpacity(1.0);
    /*
    if (show_errors_timer_count==3 || show_errors_timer_count==5)
        opacityEffect->setOpacity(0.0);
    else if(show_errors_timer_count==2 || show_errors_timer_count==4)
        opacityEffect->setOpacity(1.0);
    else if (show_errors_timer_count==1)
    {
        opacityEffect->setOpacity(0.0);
        show_erros_timer->stop();
    }*/

    ui->show_errors->setGraphicsEffect(opacityEffect);
    if(ui->show_errors->isHidden())
        ui->show_errors->show();

    QPropertyAnimation* anim = new QPropertyAnimation(this);
    anim->setTargetObject(opacityEffect);
    anim->setPropertyName("opacity");
    anim->setDuration(1000);
    anim->setStartValue(opacityEffect->opacity());
    if(show_errors_timer_count==2 || show_errors_timer_count==4)
        anim->setEndValue(0);
    else
        anim->setEndValue(1);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    show_errors_timer_count--;
}
