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

#ifndef IMAGE_PROPERTIES_H
#define IMAGE_PROPERTIES_H

#include <QDialog>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace Ui {
class image_properties;
}

class image_properties : public QDialog
{
    Q_OBJECT
    
public:
    explicit image_properties(QString img, QWidget *parent = 0);
    ~image_properties();
    
private Q_SLOTS:
    void on_close_clicked();

private:
    Ui::image_properties *ui;
    QGroupBox *optionsGroupBox;
    QPixmap originalPixmap;
    QGridLayout *optionsGroupBoxLayout;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonsLayout;
    void createOptionsGroupBox();
    void createButtonsLayout();
    void updateScreenshotLabel();

protected:
    void resizeEvent(QResizeEvent *event);
};

#endif // IMAGE_PROPERTIES_H
