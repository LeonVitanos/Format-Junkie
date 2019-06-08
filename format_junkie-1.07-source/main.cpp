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

#include "mainwindow.h"
#include "glob.h"

#include <iostream>

#include <QApplication>
#include <QtDBus/QtDBus>
#include <QtDBus/QDBusMessage>
#include <QSettings>

using namespace std;

//command line options variables...
bool add_audio=false;
bool add_video=false;
bool add_image=false;

float version=1.07;//current program version, if changed from here, it is globally changed!

bool already_runs(){
    return !system("pidof formatjunkie | grep \" \" > /dev/null");
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //searching for arguments that apply to the user interface and applying them!
    QStringList arguments=QApplication::arguments();
    if(arguments.count()>1){//there are command-line arguments!
        if(arguments.contains("--version")){
            cout << "Format Junkie, version " << version << ".\nLicensed under GPL 3.0\n";
        }
        else if(arguments.contains("--help") || arguments.contains("-h")){
            if(arguments.count()>2){
                cerr << "The option --help/-h doesn't need any other arguments! Give formatjunkie --help/-h for the available options!\nAborting...\n";
                exit(1);
            }
            cout << "Format Junkie Version " << version << "\nBrought to you by Alex Solanos <alexsol.developer@gmail.com> and Leon Vytanos <leon.check.me@gmail.com>\n"\
                    "Format Junkie is a GUI program that can convert your media files to all the popular formats.\n"\
                    "This program is licensed under GPL 3.0 and it is completely free!\n\n"\
                    "Available Options:\n"\
                    "--add-audio\tOpens a dialog so as to add audio files for convertion. Sends dbus signal if the application already runs!\n"\
                    "--add-video\tOpens a dialog so as to add video files for convertion. Sends dbus signal if the application already runs!\n"\
                    "--add-image\tOpens a dialog so as to add image files for convertion. Sends dbus signal if the application already runs!\n"\
                    "--open-output\tOpens the folder that the converted files result to. Sends dbus signal if the application already runs!\n"\
                    "--help/-h\tShows this helpful info!\n";
            exit(0);
        }
        if(!already_runs()){
            if(arguments.contains("--open-output")){
                if(arguments.count()>2){
                    cerr << "The option --open-output doesn't need any other arguments! Give formatjunkie --help/-h for the available options!\nAborting...\n";
                    exit(1);
                }
                QSettings settings("Format Junkie", "MainWindow");
                QString output_dir=settings.value("output",QDir::homePath()+"/Documents/FJOutput").toString();
                if(!QDir(output_dir).exists()){
                    QDir dir;
                    if(!dir.mkpath(output_dir)){
                        cerr << QString("Error! Folder '"+output_dir+"' didn't exist and I couldn't create it!\n").toLocal8Bit().data();
                        exit(1);
                    }
                }
                if(system(QString("xdg-open \""+output_dir+"\"&").toLocal8Bit().data())){
                    cerr << QString("Error opening folder '"+output_dir+"'\n").toLocal8Bit().data();
                    exit(1);
                }
                exit(0);
            }
            else if(arguments.contains("--add-audio")){
                if(arguments.count()>2){
                    cerr << "The option --add-audio doesn't need any other arguments! Give formatjunkie --help/-h for the available options!\nAborting...\n";
                    exit(1);
                }
                add_audio=true;
            }
            else if(arguments.contains("--add-video")){
                if(arguments.count()>2){
                    cerr << "The option --add-video doesn't need any other arguments! Give formatjunkie --help/-h for the available options!\nAborting...\n";
                    exit(1);
                }
                add_video=true;
            }
            else if(arguments.contains("--add-image")){
                if(arguments.count()>2){
                    cerr << "The option --add-image doesn't need any other arguments! Give formatjunkie --help/-h for the available options!\nAborting...\n";
                    exit(1);
                }
                add_image=true;
            }
        }
        else
        {//the program has command line arguments while it's open, send all the command line arguments there...
            QDBusMessage msg = QDBusMessage::createSignal("/", "do.action", "formatjunkie_message");
            msg << arguments.at(1);
            QDBusConnection::sessionBus().send(msg);
            cout << "Format Junkie is already running. The argument that you've specified has been sent to the already running application!\n";
            exit(0);
        }
    }
    else
    {//there aren't arguments, if the application is still running, just raise it...
        if(already_runs()){
            //there aren't arguments and the application already runs; just focus to it.
            QDBusMessage msg = QDBusMessage::createSignal("/", "do.action", "formatjunkie_message");
            msg << "focus";
            QDBusConnection::sessionBus().send(msg);
            cout << "Format Junkie is already running. Focus has been given to the already running instance!\n";
            exit(0);
        }
    }
    MainWindow w;
    w.show();
    
    return a.exec();
}
