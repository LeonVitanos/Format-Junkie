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

#include "error_logs.h"
#include "ui_error_logs.h"

error_logs::error_logs(QString all_errors, QWidget *parent):
    QDialog(parent),
    ui(new Ui::error_logs)
{
    ui->setupUi(this);
    ui->log_viewer->setText(all_errors);
    optionsGroupBox = new QGroupBox();
    optionsGroupBoxLayout = new QGridLayout;

    optionsGroupBoxLayout->addWidget(ui->log_viewer);
    optionsGroupBox->setLayout(optionsGroupBoxLayout);
    buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(ui->close);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(optionsGroupBox);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);
}

error_logs::~error_logs()
{
    delete ui;
}

void error_logs::on_close_clicked()
{
    this->close();
}

