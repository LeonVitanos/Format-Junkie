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

#include "about.h"
#include "ui_about.h"
#include "glob.h"
#include "iostream"

about::about(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::about)
{
    ui->setupUi(this);

    ui->title->setText("Format Junkie "+QString::number(version));

    QString start_string="<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans'; font-size:10pt;\">";
    QString middle_string1=" &lt;</span><a href=\"mailto:";
    QString middle_string2="\"><span style=\" font-family:'Sans'; font-size:10pt; text-decoration: underline; color:#ff5500;\">";
    QString end_string="</span></a><span style=\" font-family:'Sans'; font-size:10pt;\">&gt;</span></p>";

    ui->label_5->setText(QString("<a href=\"https://launchpad.net/format-junkie"+middle_string2+"%1</span></a>").arg(tr("Website")));
    ui->label_4->setText(QString::fromUtf8("© 2012 ") + "The Format Junkie authors");

    //CODED BY
    ui->textBrowser->setText(QString("<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; }</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">%1%2").arg("In random order").arg(":</p>"\
    +start_string+"Alex Solanos"+middle_string1+"alexsol.developer@gmail.com"+middle_string2+"alexsol.developer@gmail.com"+end_string+
    start_string+"Leon Vitanos"+middle_string1+"leon.check.me@gmail.com"+middle_string2+"leon.check.me@gmail.com"+end_string+

    "</body></html>"));

    //ARTWORK BY
    ui->textBrowser_2->setText(QString("<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; }</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">%1%2").arg("The following artist").arg(":</p>"\

    +start_string+"Kostis Solanos"+middle_string1+"ymilyta@gmail.com"+middle_string2+"ymilyta@gmail.com"+end_string+"(<a href=http://ymily.deviantart.com>ymily.deviantart.com</a>)</body></html>"));

    ui->about_button->setChecked(true);
}

about::~about()
{
    delete ui;
}

void about::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void about::on_closeButton_clicked()
{
    close();
}

void about::on_about_button_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void about::on_credits_button_clicked()
{
     ui->stackedWidget->setCurrentIndex(1);
}

void about::on_license_button_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void about::on_label_5_linkActivated()
{
    if(system("xdg-open https://launchpad.net/format-junkie"))
        std::cerr << "Error while opening https://launchpad.net/format-junkie\n";
}
