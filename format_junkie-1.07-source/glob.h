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

#ifndef GLOB_H
#define GLOB_H

#include <QString>
#include <QObject>
#include <QRegExp>
#include <QStringList>

#include <iostream>
#include <stdio.h>
using namespace std;

extern float version;
extern int quality;
extern bool delete_original;
extern bool open_output;
extern bool overwrite_existing;
extern bool audio_bitrate;
extern bool audio_samplerate;
extern bool audio_channels;
extern bool audio_volume;
extern bool video_bitrate;
extern bool video_framerate;
extern bool image_width_height;
extern bool image_respect_ratio;
extern bool unity_progressbar;
extern bool unity_count;
extern bool add_audio;
extern bool add_video;
extern bool add_image;
extern bool desktop_notifications;
extern bool indicator;
extern int image_width;
extern int image_height;
extern int audio_bitrate_value;
extern int audio_samplerate_value;
extern int audio_channels_value;
extern int audio_volume_value;
extern bool image_colors_number;
extern int image_colors_number_value;
extern bool video_crop;
extern int video_bitrate_value;
extern int video_framerate_value;
extern int video_width_value;
extern int video_height_value;
extern int video_x_value;
extern int video_y_value;
extern bool video_resize;
extern bool image_comment;
extern bool image_crop;
extern int image_width_value;
extern int image_height_value;
extern int image_x_value;
extern int image_y_value;
extern QString video_resize_value;
extern QString image_comment_value;

class Global : public QObject
{
    Q_OBJECT
public:
    static float duration(QString file){
        /*
          getting duration for music and video files. This can be done live,
          while the conversion of the file starts, but it failed once
          in a while. This way is more trustful. Getting the duration
          is important so as to calculate the percentage, because ffmpeg/avconv
          doesn't provide any percentage info in its output.
          The function is global because it is used in the properties dialogs
          as well.
        */
        file.replace("\"", "\\\"");
        file.replace( "`", "\\`" );
        QString cmd="avconv -i \""+file+"\" 2>&1 | grep \"  Duration: \"";
        FILE* pipe = popen(cmd.toLocal8Bit().data(), "r");
        char buffer[128];
        string result = "";
        while(!feof(pipe)) {
            if(fgets(buffer, 128, pipe) != NULL)
                result += buffer;
            }
        pclose(pipe);
        QString res = QString::fromStdString(result);
        QRegExp regex("  Duration: .*,");
        regex.setMinimal(true);

        QStringList list;
        int pos = 0;

        while ((pos = regex.indexIn(res, pos)) != -1)
        {
            list << regex.cap(0).remove(QChar(','),Qt::CaseInsensitive);
            pos += regex.matchedLength();
        }
        if(!list.count()){
            //error
            cerr << QString("Couldn't get the duration of the file '"+file+"'\nPlease, unless the file is problematic, report this bug and attach the file that it caused it!\n").toLocal8Bit().data();
            //return a small duration so as to instantly go to 100%, although it isn't...
            return 0.0001;
        }
        QString duration=list.at(0);
        duration.replace(" ","");
        duration.replace("Duration:","");
        QString msecs=duration.right(2);
        duration.chop(3);
        QStringList time=duration.split(":");
        time << msecs;
        return time.at(0).toInt()*3600+time.at(1).toInt()*60+time.at(2).toInt()+QString("0."+time.at(3)).toFloat();
    }
};
#endif // GLOB_H
