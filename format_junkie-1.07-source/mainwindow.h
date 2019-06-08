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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "preferences.h"
#include "error_logs.h"
#include "about.h"
#include "audio_video_properties.h"
#include "image_properties.h"

#include <QMainWindow>
#include <QStringList>
#include <QProgressBar>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QSystemTrayIcon>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static void handle_it();

private Q_SLOTS:
    void on_actionAdd_Audio_Files_triggered();
    void on_actionAdd_Video_Files_triggered();
    void on_actionAdd_Image_Files_triggered();
    void on_actionContents_F1_triggered();
    void on_actionPreferences_triggered();
    void on_musicButton_clicked();
    void on_videoButton_clicked();
    void on_imageButton_clicked();
    void on_advancedButton_clicked();
    void on_rom_deviceButton_clicked();
    void add_files(QString path, int type);
    void audio_read_output();
    void video_read_output();
    void image_read_output();
    void files_to_iso_end(int return_code);
    void iso_to_cso_end(int return_code);
    void cso_to_iso_end(int return_code);
    QStringList list_folders(QString parent_folder);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void on_startBtn_clicked();
    void on_addBtn_clicked();
    void on_addfolder_clicked();
    void on_clearBtn_clicked();
    void on_removeBtn_clicked();
    void on_moveupButton_clicked();
    void on_movedownButton_clicked();
    void audio_conversion(int return_code);
    void video_conversion(int return_code);
    void image_conversion(int return_code);
    void on_stop_clicked();
    void on_output_folder_clicked();
    void setup_shortcuts();
    void crop_output_folder(QString output_folder_location);
    void remove_complete();
    void progressbar_animation(int end_value, int type);
    void actions_after_convert();
    void on_stop_2_clicked();
    void on_stackedWidget_currentChanged(int arg1);
    void on_show_errors_clicked();
    void on_actionAbout_triggered();
    void on_actionQuit_triggered();
    void on_actionGet_Help_Online_triggered();
    void on_actionReport_A_Bug_triggered();
    void dbus_action(QString msg);
    void on_progressBar_valueChanged(int value);
    void update_unity_value(int value);
    void update_unity_count_value(int after);
    void show_desktop_notification(int noti_type);
    void add_audio_files();
    void add_video_files();
    void add_image_files();
    void clear_convertion();
    void process_dir(QString dir);
    void make_indicator();
    void delete_indicator();
    void on_startBtn_3_clicked();
    void on_stop_3_clicked();
    void on_progressBar_3_valueChanged(int value);
    void on_startBtn_2_clicked();
    void set_video_resize_value(int index_value);
    void show_me();
    void on_radioButton_files_to_iso_clicked();
    void on_radioButton_iso_cso_clicked();
    void on_startBtn_8_clicked();
    void on_stop_6_clicked();
    void on_select_iso_clicked();
    void on_select_cso_clicked();
    void on_iso_to_cso_start_clicked();
    void on_iso_to_cso_output_textEdited();
    void on_cso_to_iso_output_textEdited();
    void on_cso_to_iso_start_clicked();
    void on_iso_to_cso_stop_clicked();
    void on_cso_to_iso_stop_clicked();
    void show_audio_video_item_properties();
    void show_image_properties();
    void open_containing_folder();
    void on_tableWidget_customContextMenuRequested();
    void on_tableWidget_2_customContextMenuRequested();
    void on_tableWidget_3_customContextMenuRequested();
    float get_size(QString folder);
    float folder_size(QString folder);
    void on_selectavi_clicked();
    void on_selectsubtitle_clicked();
    void on_after_convertion2_currentIndexChanged(int index);
    void on_encode_start_clicked();
    void sub_encoding_end(int return_code);
    void on_encode_stop_clicked();
    void on_progressBar_2_valueChanged(int value);
    void on_addBtn_2_clicked();
    void on_addfolder_2_clicked();
    void on_clearBtn_2_clicked();
    void on_moveupButton_2_clicked();
    void on_movedownButton_2_clicked();
    void on_addBtn_3_clicked();
    void on_addfolder_3_clicked();
    void on_removeBtn_3_clicked();
    void on_clearBtn_3_clicked();
    void on_moveupButton_3_clicked();
    void on_movedownButton_3_clicked();
    void on_addBtn_4_clicked();
    void on_addfolder_4_clicked();
    void on_removeBtn_4_clicked();
    void on_clearBtn_4_clicked();
    void on_moveupButton_4_clicked();
    void on_movedownButton_4_clicked();
    void timer_to_hide();
    void goto_1();
    void goto_2();
    void goto_3();
    void goto_4();
    void goto_5();
    void show_errors_animation();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    void closeEvent( QCloseEvent * );
    QProgressBar *globpro;
    Preferences *pref;
    error_logs *showlogs;
    about *About;
    audio_video_properties *prop;
    image_properties *improp;
    QSystemTrayIcon *trayIcon;
    QAction *Show_action;
    QAction *Show_audio;
    QAction *Show_video;
    QAction *Show_image;
    QAction *Show_preferences;
    QAction *Show_about;
    QAction *Show_outputfolder;
    QAction *Quit_action;
    QGroupBox *optionsGroupBox;
    QGridLayout *optionsGroupBoxLayout;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonsLayout;
    QTimer *show_errors_timer;
    void createOptionsGroupBox();
    void createButtonsLayout();

Q_SIGNALS:
    void send_output_folder(QString);

};

#endif // MAINWINDOW_H
