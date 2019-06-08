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

#include "image_properties.h"
#include "ui_image_properties.h"

#include <QFile>
#include <QFileInfo>

#include <stdio.h>

using namespace std;

QString file;

image_properties::image_properties(QString img, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::image_properties)
{
    ui->setupUi(this);
    createOptionsGroupBox();
    createButtonsLayout();
    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(ui->label);
    mainLayout->addWidget(optionsGroupBox);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);
    file=img;
    QImage qimg(file);

    ui->file_height->setText(QString::number(qimg.height()) + " px");
    ui->file_width->setText(QString::number(qimg.width()) + " px");
    QFileInfo qfile(file);
    float size=float(qfile.size())/1048576;
    float rounded_size=((int)(size*100+0.5)/100.0);
    ui->file_size->setText(QString::number(rounded_size) + " MB");
    ui->file_name->setText(qfile.baseName());
    ui->file_location->setText(qfile.path());

    string file_type = "";
    QString qcommand1="file -b \""+file+"\"";
    char buffer[128];
    FILE *pipe = popen(qcommand1.toLocal8Bit().data(), "r");
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
            file_type += buffer;
    }
    pclose(pipe);
    QString qfile_type = QString::fromStdString(file_type).replace(QString("\n"),QString(""));
    ui->file_type->setText(qfile_type);
    ui->file_type->setCursorPosition(0);
    ui->file_location->setCursorPosition(0);
    originalPixmap = QPixmap();
    originalPixmap = QPixmap().fromImage(qimg);
    updateScreenshotLabel();
}

image_properties::~image_properties()
{
    delete ui;
}

void image_properties::on_close_clicked()
{
    this->close();
}

void image_properties::resizeEvent(QResizeEvent * /* event */)
{
    QSize scaledSize = originalPixmap.size();
    scaledSize.scale(ui->label->size(), Qt::KeepAspectRatio);
    if (!ui->label->pixmap() || scaledSize != ui->label->pixmap()->size())
        updateScreenshotLabel();
}

void image_properties::createOptionsGroupBox()
{
    optionsGroupBox = new QGroupBox();
    optionsGroupBoxLayout = new QGridLayout;
    optionsGroupBoxLayout->addWidget(ui->label_2, 1, 0);
    optionsGroupBoxLayout->addWidget(ui->file_name, 1, 1);
    optionsGroupBoxLayout->addWidget(ui->label_3, 2, 0);
    optionsGroupBoxLayout->addWidget(ui->file_size, 2, 1);
    optionsGroupBoxLayout->addWidget(ui->label_4, 3, 0);
    optionsGroupBoxLayout->addWidget(ui->file_width, 3, 1);
    optionsGroupBoxLayout->addWidget(ui->label_5, 5, 0);
    optionsGroupBoxLayout->addWidget(ui->file_height, 5, 1);
    optionsGroupBoxLayout->addWidget(ui->label_6, 6, 0);
    optionsGroupBoxLayout->addWidget(ui->file_type, 6, 1);
    optionsGroupBoxLayout->addWidget(ui->label_7, 7, 0);
    optionsGroupBoxLayout->addWidget(ui->file_location, 7, 1);
    optionsGroupBox->setLayout(optionsGroupBoxLayout);
}

void image_properties::createButtonsLayout()
{
    buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(ui->close);
}

void image_properties::updateScreenshotLabel()
{
    ui->label->setPixmap(originalPixmap.scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
