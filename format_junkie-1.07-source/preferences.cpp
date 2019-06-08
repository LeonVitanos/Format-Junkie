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

#include "preferences.h"
#include "ui_preferences.h"
#include "glob.h"


#include <QShortcut>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QFileDialog>

int video_resize_index=0;

Preferences::Preferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);
    QSettings settings( "FormatJunkie", "Preferences" );
    //GENERAL
    ui->deleteOriginalCheckBox->setChecked(settings.value("deleteoriginal", false).toBool());
    ui->open_output->setChecked(settings.value("open_output", false).toBool());
    ui->UnityProgressBarCheckbox->setChecked(settings.value("unity_progressbar", true).toBool());
    ui->UnityCountCheckbox->setChecked(settings.value("unity_count", true).toBool());
    ui->DesktopNotificationsCheckbox->setChecked(settings.value("desktop_notifications", true).toBool());
    ui->IndicatorCheckbox->setChecked(settings.value("indicator", false).toBool());
    ui->overwriteCheckBox->setChecked(settings.value("overwrite", false).toBool());
    ui->horizontalSlider->setValue(settings.value("quality", 100).toInt());
    ui->label_5->setText(QString::number(ui->horizontalSlider->value()));
    //AUDIO
    ui->audio_bitrate_checkbox->setChecked(settings.value("audio_bitrate", false).toBool());
    ui->audio_sample_rate_checkbox->setChecked(settings.value("audio_sample_rate", false).toBool());
    ui->audio_channels_checkbox->setChecked(settings.value("audio_channels", false).toBool());
    ui->audio_volume_checkbox->setChecked(settings.value("audio_volume", false).toBool());
    ui->audio_bitrate_spinBox->setValue(settings.value("audio_bitrate_spinbox", 198).toInt());
    ui->audio_sample_rate_spinBox->setValue(settings.value("audio_sample_rate_spinbox", 44100).toInt());
    ui->audio_channels_spinBox->setValue(settings.value("audio_channels_spinbox", 2).toInt());
    ui->audio_volume_spinBox->setValue(settings.value("audio_volume_spinbox", 256).toInt());
    //VIDEO
    ui->video_bitrate_checkbox->setChecked(settings.value("video_bitrate", false).toBool());
    ui->video_bitrate_spinBox->setValue(settings.value("video_bitrate_spinbox", 3900000).toInt());
    ui->video_frame_rate_checkbox->setChecked(settings.value("video_frame_rate", false).toBool());
    ui->video_crop_checkbox->setChecked(settings.value("video_crop", false).toBool());
    ui->video_width_spinbox->setValue(settings.value("video_width_spinbox", 100).toInt());
    ui->video_height_spinbox->setValue(settings.value("video_height_spinbox", 100).toInt());
    ui->video_x_spinbox->setValue(settings.value("video_x_spinbox", 0).toInt());
    ui->video_y_spinbox->setValue(settings.value("video_y_spinbox", 0).toInt());
    ui->video_resize_checkbox->setChecked(settings.value("video_resize", false).toBool());
    video_resize_index=settings.value("video_resize_index",0).toInt();
    if(video_resize_index<0 || video_resize_index>28)
        video_resize_index=0;
    ui->video_resize_combobox->setCurrentIndex(video_resize_index);
    //IMAGE
    ui->image_width_height_checkbox->setChecked(settings.value("image_width_height", false).toBool());
    ui->image_width_spinBox->setValue(settings.value("image_width", 300).toInt());
    ui->image_height_spinBox->setValue(settings.value("image_height", 300).toInt());
    ui->image_respect_ratio_checkbox->setChecked(settings.value("image_respect_ratio", false).toBool());
    ui->image_colors_number_checkbox->setChecked(settings.value("image_colors_number", false).toBool());
    ui->image_colors_number_spinBox->setValue(settings.value("image_colors_number_spinbox", 256).toInt());
    ui->image_comment_checkbox->setChecked(settings.value("image_comment", false).toBool());
    ui->image_comment_lineedit->setText(settings.value("image_comment_lineedit", "").toString());
    ui->image_crop_checkbox->setChecked(settings.value("image_crop", false).toBool());
    ui->image_width_spinbox->setValue(settings.value("image_width_spinbox", 100).toInt());
    ui->image_height_spinbox->setValue(settings.value("image_height_spinbox", 100).toInt());
    ui->image_x_spinbox->setValue(settings.value("image_x_spinbox", 0).toInt());
    ui->image_y_spinbox->setValue(settings.value("image_y_spinbox", 0).toInt());

    //AUDIO
    on_audio_bitrate_checkbox_clicked();
    on_audio_sample_rate_checkbox_clicked();
    on_audio_channels_checkbox_clicked();
    on_audio_volume_checkbox_clicked();
    //VIDEO
    on_video_bitrate_checkbox_clicked();
    on_video_frame_rate_checkbox_clicked();
    on_video_crop_checkbox_clicked();
    on_video_resize_checkbox_clicked();
    //IMAGE
    on_image_width_height_checkbox_clicked();
    on_image_colors_number_checkbox_clicked();
    on_image_comment_checkbox_clicked();
    on_image_crop_checkbox_clicked();

    if(!ui->image_width_height_checkbox->isChecked())
        ui->image_respect_ratio_checkbox->setEnabled(false);
    //without this shortcut, the dialog closes without passing from the closeEvent
    (void) new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

Preferences::~Preferences()
{
    delete ui;
}

void Preferences::closeEvent( QCloseEvent * ){
    //settings the glob values
    //GENERAL
    delete_original=ui->deleteOriginalCheckBox->isChecked();
    open_output=ui->open_output->isChecked();
    overwrite_existing=ui->overwriteCheckBox->isChecked();
    quality=ui->horizontalSlider->value();
    unity_progressbar=ui->UnityProgressBarCheckbox->isChecked();
    unity_count=ui->UnityCountCheckbox->isChecked();
    desktop_notifications=ui->DesktopNotificationsCheckbox->isChecked();
    indicator=ui->IndicatorCheckbox->isChecked();
    //AUDIO
    audio_bitrate=ui->audio_bitrate_checkbox->isChecked();
    audio_samplerate=ui->audio_sample_rate_checkbox->isChecked();
    audio_channels=ui->audio_channels_checkbox->isChecked();
    audio_volume=ui->audio_volume_checkbox->isChecked();
    audio_bitrate_value=ui->audio_bitrate_spinBox->value();
    audio_samplerate_value=ui->audio_sample_rate_spinBox->value();
    audio_channels_value=ui->audio_channels_spinBox->value();
    audio_volume_value=ui->audio_volume_spinBox->value();
    //VIDEO
    video_bitrate=ui->video_bitrate_checkbox->isChecked();
    video_framerate=ui->video_frame_rate_checkbox->isChecked();
    video_bitrate_value=ui->video_bitrate_spinBox->value();
    video_framerate_value=ui->video_frame_rate_spinBox->value();
    video_crop=ui->video_crop_checkbox->isChecked();
    video_width_value=ui->video_width_spinbox->value();
    video_height_value=ui->video_height_spinbox->value();
    video_x_value=ui->video_x_spinbox->value();
    video_y_value=ui->video_y_spinbox->value();
    video_resize=ui->video_resize_checkbox->isChecked();
    video_resize_index=ui->video_resize_combobox->currentIndex();
    set_video_resize_value(video_resize_index);
    //IMAGE
    image_width_height=ui->image_width_height_checkbox->isChecked();
    image_width=ui->image_width_spinBox->value();
    image_height=ui->image_height_spinBox->value();
    image_respect_ratio=ui->image_respect_ratio_checkbox->isChecked();
    image_colors_number=ui->image_colors_number_checkbox->isChecked();
    image_colors_number_value=ui->image_colors_number_spinBox->value();
    image_comment=ui->image_comment_checkbox->isChecked();
    image_comment_value=ui->image_comment_lineedit->text();
    image_crop=ui->image_crop_checkbox->isChecked();
    image_width_value=ui->image_width_spinbox->value();
    image_height_value=ui->image_height_spinbox->value();
    image_x_value=ui->image_x_spinbox->value();
    image_y_value=ui->image_y_spinbox->value();



    //updating the configuration files...
    QSettings settings( "FormatJunkie", "Preferences" );
    //GENERAL
    settings.setValue("deleteoriginal", delete_original);
    settings.setValue("open_output", open_output);
    settings.setValue("overwrite", overwrite_existing);
    settings.setValue("unity_progressbar", unity_progressbar);
    settings.setValue("unity_count", unity_count);
    settings.setValue("desktop_notifications", desktop_notifications);
    settings.setValue("indicator", indicator);
    settings.setValue( "quality", quality );
    //AUDIO
    settings.setValue("audio_bitrate", audio_bitrate);
    settings.setValue("audio_sample_rate", audio_samplerate);
    settings.setValue("audio_channels", audio_channels);
    settings.setValue("audio_volume", audio_volume);
    settings.setValue("audio_bitrate_spinbox", audio_bitrate_value);
    settings.setValue("audio_sample_rate_spinbox", audio_samplerate_value);
    settings.setValue("audio_channels_spinbox", audio_channels_value);
    settings.setValue("audio_volume_spinbox", audio_volume_value);
    //VIDEO
    settings.setValue("video_bitrate", video_bitrate);
    settings.setValue("video_frame_rate", video_framerate);
    settings.setValue("video_bitrate_spinbox", video_bitrate_value);
    settings.setValue("video_sample_rate_spinbox", video_framerate_value);
    settings.setValue("video_crop", video_crop);
    settings.setValue("video_width_spinbox", video_width_value);
    settings.setValue("video_height_spinbox", video_height_value);
    settings.setValue("video_x_spinbox", video_x_value);
    settings.setValue("video_y_spinbox", video_y_value);
    settings.setValue("video_resize", video_resize);
    settings.setValue("video_resize_index", video_resize_index);
    //IMAGE
    settings.setValue("image_width_height", image_width_height);
    settings.setValue("image_width", image_width);
    settings.setValue("image_height", image_height);
    settings.setValue("image_respect_ratio", image_respect_ratio);
    settings.setValue("image_colors_number", image_colors_number);
    settings.setValue("image_colors_number_spinbox", image_colors_number_value);
    settings.setValue("image_comment",image_comment);
    settings.setValue("image_comment_lineedit", image_comment_value);
    settings.setValue("image_crop", image_crop);
    settings.setValue("image_width_spinbox", image_width_value);
    settings.setValue("image_height_spinbox", image_height_value);
    settings.setValue("image_x_spinbox", image_x_value);
    settings.setValue("image_y_spinbox", image_y_value);


    settings.sync();

    //sending the output folder...
    send_new_output(ui->lineEdit->text());

}

void Preferences::set_video_resize_value(int index_value){
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

void Preferences::on_audio_bitrate_checkbox_clicked()
{
    bool state=ui->audio_bitrate_checkbox->isChecked();
    ui->audio_bitrate_label->setEnabled(state);
    ui->audio_bitrate_spinBox->setEnabled(state);
}

void Preferences::on_audio_sample_rate_checkbox_clicked()
{
    bool state=ui->audio_sample_rate_checkbox->isChecked();
    ui->audio_sample_rate_label->setEnabled(state);
    ui->audio_sample_rate_spinBox->setEnabled(state);
}

void Preferences::on_audio_channels_checkbox_clicked()
{
    bool state=ui->audio_channels_checkbox->isChecked();
    ui->audio_channels_label->setEnabled(state);
    ui->audio_channels_spinBox->setEnabled(state);
}

void Preferences::on_audio_volume_checkbox_clicked()
{
    bool state=ui->audio_volume_checkbox->isChecked();
    ui->audio_volume_label->setEnabled(state);
    ui->audio_volume_spinBox->setEnabled(state);
}

void Preferences::on_image_width_height_checkbox_clicked()
{
    bool state=ui->image_width_height_checkbox->isChecked();
    ui->width_label->setEnabled(state);
    ui->height_label->setEnabled(state);
    ui->image_height_spinBox->setEnabled(state);
    ui->image_width_spinBox->setEnabled(state);
    ui->px_label->setEnabled(state);
    if(state)
        ui->image_respect_ratio_checkbox->setEnabled(true);
    else
        ui->image_respect_ratio_checkbox->setEnabled(false);
}

void Preferences::on_image_colors_number_checkbox_clicked()
{
    bool state=ui->image_colors_number_checkbox->isChecked();
    ui->colors_number_label->setEnabled(state);
    ui->image_colors_number_spinBox->setEnabled(state);
}

void Preferences::on_video_resize_checkbox_clicked()
{
    bool state=ui->video_resize_checkbox->isChecked();
    ui->resize_label->setEnabled(state);
    ui->video_resize_combobox->setEnabled(state);
}

void Preferences::on_image_comment_checkbox_clicked()
{
    bool state=ui->image_comment_checkbox->isChecked();
    ui->comment_label->setEnabled(state);
    ui->image_comment_lineedit->setEnabled(state);
    if(state)
        ui->image_comment_lineedit->setFocus();
}

void Preferences::on_video_crop_checkbox_clicked()
{
    bool state=ui->video_crop_checkbox->isChecked();
    ui->crop_label->setEnabled(state);
    ui->video_height_label->setEnabled(state);
    ui->video_width_label->setEnabled(state);
    ui->video_x_label->setEnabled(state);
    ui->video_y_label->setEnabled(state);
    ui->video_height_spinbox->setEnabled(state);
    ui->video_width_spinbox->setEnabled(state);
    ui->video_x_spinbox->setEnabled(state);
    ui->video_y_spinbox->setEnabled(state);
}

void Preferences::on_image_crop_checkbox_clicked()
{
    bool state=ui->image_crop_checkbox->isChecked();
    ui->crop_label_2->setEnabled(state);
    ui->image_height_label->setEnabled(state);
    ui->image_width_label->setEnabled(state);
    ui->image_x_label->setEnabled(state);
    ui->image_y_label->setEnabled(state);
    ui->image_height_spinbox->setEnabled(state);
    ui->image_width_spinbox->setEnabled(state);
    ui->image_x_spinbox->setEnabled(state);
    ui->image_y_spinbox->setEnabled(state);
}

void Preferences::on_general_btn_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void Preferences::on_audio_btn_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void Preferences::on_video_btn_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void Preferences::on_image_btn_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
}

void Preferences::on_horizontalSlider_valueChanged(int value)
{
    ui->label_5->setText(QString::number(value));
}

void Preferences::on_closeButton_clicked()
{
    this->close();
}

void Preferences::on_reset_clicked()
{
    //GENERAL
    ui->deleteOriginalCheckBox->setChecked(false);
    ui->overwriteCheckBox->setChecked(false);
    ui->horizontalSlider->setValue(100);
    ui->UnityProgressBarCheckbox->setChecked(true);
    ui->UnityCountCheckbox->setChecked(true);
    ui->DesktopNotificationsCheckbox->setChecked(true);
    ui->IndicatorCheckbox->setChecked(false);
    //AUDIO
    ui->audio_bitrate_spinBox->setValue(198);
    ui->audio_sample_rate_spinBox->setValue(44100);
    ui->audio_channels_spinBox->setValue(2);
    ui->audio_volume_spinBox->setValue(256);
    ui->audio_bitrate_checkbox->setChecked(false);
    ui->audio_sample_rate_checkbox->setChecked(false);
    ui->audio_channels_checkbox->setChecked(false);
    ui->audio_volume_checkbox->setChecked(false);
    //VIDEO
    ui->video_bitrate_checkbox->setChecked(false);
    ui->video_frame_rate_checkbox->setChecked(false);
    ui->video_crop_checkbox->setChecked(false);
    ui->video_width_spinbox->setValue(100);
    ui->video_height_spinbox->setValue(100);
    ui->video_x_spinbox->setValue(0);
    ui->video_y_spinbox->setValue(0);
    ui->video_resize_checkbox->setChecked(false);
    //IMAGE
    ui->image_width_height_checkbox->setChecked(false);
    ui->image_height_spinBox->setValue(300);
    ui->image_width_spinBox->setValue(300);
    ui->image_respect_ratio_checkbox->setChecked(false);
    ui->image_colors_number_checkbox->setChecked(false);
    ui->image_colors_number_spinBox->setValue(256);
    ui->image_comment_checkbox->setChecked(false);
    ui->image_comment_lineedit->setText("");
    ui->image_crop_checkbox->setChecked(false);
    ui->image_width_spinbox->setValue(100);
    ui->image_height_spinbox->setValue(100);
    ui->image_x_spinbox->setValue(0);
    ui->image_y_spinbox->setValue(0);

    QMessageBox::information(this, "Format Junkie", "Your preferences have been reset!");
    this->close();
}

void Preferences::put_output_folder_at_line(QString output_folder)
{
    ui->lineEdit->setText(output_folder);
}
void Preferences::on_change_output_clicked()
{
    QString qpath;
    if(QDir(ui->lineEdit->text()).exists())
        qpath = QFileDialog::getExistingDirectory(this, "Choose Output Folder", ui->lineEdit->text());
    else
        qpath = QFileDialog::getExistingDirectory(this, "Choose Output Folder", QDir::homePath());
    if(!qpath.count())
        return;
    ui->lineEdit->setText(qpath);
}

void Preferences::on_ubuntu_clicked()
{
    if(ui->DesktopNotificationsCheckbox->isChecked() && ui->IndicatorCheckbox->isChecked() && ui->UnityCountCheckbox->isChecked() && ui->UnityProgressBarCheckbox->isChecked()){
        //if they are all checked, uncheck them all
        ui->DesktopNotificationsCheckbox->setChecked(false);
        ui->IndicatorCheckbox->setChecked(false);
        ui->UnityCountCheckbox->setChecked(false);
        ui->UnityProgressBarCheckbox->setChecked(false);
    }
    else
    {
        //not all are checked, check them all
        ui->DesktopNotificationsCheckbox->setChecked(true);
        ui->IndicatorCheckbox->setChecked(true);
        ui->UnityCountCheckbox->setChecked(true);
        ui->UnityProgressBarCheckbox->setChecked(true);
    }
}

void Preferences::on_video_bitrate_checkbox_clicked()
{
    bool state=ui->video_bitrate_checkbox->isChecked();
    ui->video_bitrate_label->setEnabled(state);
    ui->video_bitrate_spinBox->setEnabled(state);
}

void Preferences::on_video_frame_rate_checkbox_clicked()
{
    bool state=ui->video_frame_rate_checkbox->isChecked();
    ui->video_frame_rate_label->setEnabled(state);
    ui->video_frame_rate_spinBox->setEnabled(state);
}
