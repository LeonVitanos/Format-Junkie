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

#include "audio_video_properties.h"
#include "ui_audio_video_properties.h"
#include "glob.h"

#include <QFileInfo>
#include <QDir>

QString complete_path;

audio_video_properties::audio_video_properties(QString filename, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::audio_video_properties)
{
    ui->setupUi(this);

    ui->close->setFocus();
    createOptionsGroupBox();
    createButtonsLayout();
    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(optionsGroupBox);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    complete_path=filename;
    QFileInfo inf;
    inf.setFile(complete_path);
    //BASENAME
    ui->filename->setText(inf.completeBaseName());
    //FILEPATH
    ui->path->setText(inf.path());
    //DURATION
    float duration_secs=Global::duration(complete_path);
    if(duration_secs<60)
    {
        //no more than 1 minute
        ui->duration->setText(QString::number(duration_secs)+" secs");
    }
    else
    {
        //more than 1 minute
        int minutes=duration_secs/60;
        duration_secs-=minutes*60;
        if(minutes<60){
            //no more than an hour
            ui->duration->setText(QString::number(minutes) +" mins, "+QString::number(duration_secs)+" secs");
        }
        else
        {
           //more than an hour
            int hours=minutes/60;
            minutes-=hours*60;
            ui->duration->setText(QString::number(hours)+" hrs, "+QString::number(minutes) +" mins, "+QString::number(duration_secs)+" secs");
        }
    }

    //SIZE
    float size=(float)inf.size()/1048576;
    if(size<1)
        ui->size->setText(QString::number(size, 'f', 3)+" MB");
    else
        ui->size->setText(QString::number(size, 'f', 2)+" MB");

    //FILETYPE

    string image_type = "";
    QString qcommand="file -b \""+complete_path+"\"";
    char buffer[128];
    FILE *pipe = popen(qcommand.toLocal8Bit().data(), "r");
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
            image_type += buffer;
    }
    pclose(pipe);
    QString qfile_type = QString::fromStdString(image_type).replace(QString("\n"),QString(""));
    ui->filetype->setText(qfile_type);
    ui->filetype->setCursorPosition(0);
}

audio_video_properties::~audio_video_properties()
{
    delete ui;
}

void audio_video_properties::on_close_clicked()
{
    this->close();
}

void audio_video_properties::createOptionsGroupBox()
{
    optionsGroupBox = new QGroupBox();
    optionsGroupBoxLayout = new QGridLayout;
    optionsGroupBoxLayout->addWidget(ui->label, 0, 0);
    optionsGroupBoxLayout->addWidget(ui->filename, 0, 1);
    optionsGroupBoxLayout->addWidget(ui->label_2, 1, 0);
    optionsGroupBoxLayout->addWidget(ui->path, 1, 1);
    optionsGroupBoxLayout->addWidget(ui->label_3, 2, 0);
    optionsGroupBoxLayout->addWidget(ui->duration, 2, 1);
    optionsGroupBoxLayout->addWidget(ui->label_4, 3, 0);
    optionsGroupBoxLayout->addWidget(ui->size, 3, 1);
    optionsGroupBoxLayout->addWidget(ui->label_5, 4, 0);
    optionsGroupBoxLayout->addWidget(ui->filetype, 4, 1);
    optionsGroupBox->setLayout(optionsGroupBoxLayout);
}

void audio_video_properties::createButtonsLayout()
{
    buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(ui->close);
}
