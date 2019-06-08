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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>

namespace Ui {
class Preferences;
}

class Preferences : public QDialog
{
    Q_OBJECT
    
public:
    explicit Preferences(QWidget *parent = 0);
    ~Preferences();
    
private Q_SLOTS:
    void on_audio_bitrate_checkbox_clicked();
    void on_audio_sample_rate_checkbox_clicked();
    void on_audio_channels_checkbox_clicked();
    void on_audio_volume_checkbox_clicked();
    void on_general_btn_clicked();
    void on_audio_btn_clicked();
    void on_video_btn_clicked();
    void on_image_btn_clicked();
    void on_horizontalSlider_valueChanged(int value);
    void on_closeButton_clicked();
    void on_reset_clicked();
    void put_output_folder_at_line(QString output_folder);
    void on_change_output_clicked();
    void on_ubuntu_clicked();
    void on_image_width_height_checkbox_clicked();
    void on_image_colors_number_checkbox_clicked();
    void on_video_crop_checkbox_clicked();
    void set_video_resize_value(int index_value);
    void on_video_resize_checkbox_clicked();
    void on_image_comment_checkbox_clicked();
    void on_image_crop_checkbox_clicked();
    void on_video_bitrate_checkbox_clicked();
    void on_video_frame_rate_checkbox_clicked();

private:
    Ui::Preferences *ui;
    void closeEvent( QCloseEvent * );

Q_SIGNALS:
    void send_new_output(QString);
};

#endif // PREFERENCES_H
