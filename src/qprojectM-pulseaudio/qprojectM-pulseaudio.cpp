
/**
 * projectM -- Milkdrop-esque visualisation SDK
 * Copyright (C)2003-2004 projectM Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * See 'LICENSE.txt' included within this release
 *
 */

/* $Id: parec-simple.c 1418 2007-01-04 13:43:45Z ossman $ */

/***
  This file was copied from PulseAudio.

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#include <fcntl.h>
#include <stdlib.h>
#include <math.h>
#include <libprojectM/projectM.hpp>
#include <libprojectM/QProjectM_MainWindow.hpp>
#include <QApplication>
#include <QtDebug>

#include "ConfigFile.h"
#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <QAction>
#include <QThread>
#include <QTimer>
#define CONFIG_FILE "/share/projectM/config.inp"

std::string read_config();

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <string.h>
#include <errno.h>

//#include <pulsecore/gccmacro.h>

#include "QPulseAudioThread.hpp"
#include "QPulseAudioDevChooser.hpp"

int dumpFrame = 0;
int frameNumber = 0;

int texsize=512;
int gx=32,gy=24;
int wvw=512,wvh=512;
int fvw=1024,fvh=768;
int fps=30, fullscreen=0;

int main ( int argc, char*argv[] )
{
	int i;
	char projectM_data[1024];

// Start a new qapplication
	QApplication app ( argc, argv );

	std::string config_file;
	config_file = read_config();

	QProjectM_MainWindow * mainWindow = new QProjectM_MainWindow ( config_file );
	
	QPulseAudioDevChooser devChooser;

	QAction pulseAction("Pulse audio settings...", mainWindow);
	
	devChooser.setupUi(&devChooser);

	QApplication::connect(&pulseAction, SIGNAL(triggered()), &devChooser, SLOT(show())); 
      	mainWindow->registerSettingsAction(&pulseAction);
	mainWindow->show();

	QPulseAudioThread * pulseThread = new QPulseAudioThread(argc, argv, mainWindow->getQProjectM(), mainWindow);
	
	pulseThread->start();
	
	//qDebug() << "app exec";
 	int ret = app.exec();

	pulseThread->exit();
	pulseThread->cleanup();

        mainWindow->unregisterSettingsAction(&pulseAction);

	delete(mainWindow);
	return ret;
}


std::string read_config()
{

	int n;

	char num[512];
	FILE *in;
	FILE *out;

	char* home;
	char projectM_home[1024];
	char projectM_config[1024];

	strcpy ( projectM_config, PROJECTM_PREFIX );
	strcpy ( projectM_config+strlen ( PROJECTM_PREFIX ), CONFIG_FILE );
	projectM_config[strlen ( PROJECTM_PREFIX ) +strlen ( CONFIG_FILE ) ]='\0';
	printf ( "dir:%s \n",projectM_config );
	home=getenv ( "HOME" );
	strcpy ( projectM_home, home );
	strcpy ( projectM_home+strlen ( home ), "/.projectM/config.inp" );
	projectM_home[strlen ( home ) +strlen ( "/.projectM/config.inp" ) ]='\0';


	if ( ( in = fopen ( projectM_home, "r" ) ) != 0 )
	{
		printf ( "reading ~/.projectM/config.inp \n" );
		fclose ( in );
		return std::string ( projectM_home );
	}
	else
	{
		printf ( "trying to create ~/.projectM/config.inp \n" );

		strcpy ( projectM_home, home );
		strcpy ( projectM_home+strlen ( home ), "/.projectM" );
		projectM_home[strlen ( home ) +strlen ( "/.projectM" ) ]='\0';
		mkdir ( projectM_home, 0755 );

		strcpy ( projectM_home, home );
		strcpy ( projectM_home+strlen ( home ), "/.projectM/config.inp" );
		projectM_home[strlen ( home ) +strlen ( "/.projectM/config.inp" ) ]='\0';

		if ( ( out = fopen ( projectM_home,"w" ) ) !=0 )
		{

			if ( ( in = fopen ( projectM_config, "r" ) ) != 0 )
			{

				while ( fgets ( num,80,in ) !=NULL )
				{
					fputs ( num,out );
				}
				fclose ( in );
				fclose ( out );


				if ( ( in = fopen ( projectM_home, "r" ) ) != 0 )
				{
					printf ( "created ~/.projectM/config.inp successfully\n" );
					fclose ( in );
					return std::string ( projectM_home );
				}
				else{printf ( "This shouldn't happen, using implementation defualts\n" );abort();}
			}
			else{printf ( "Cannot find projectM default config, using implementation defaults\n" );abort();}
		}
		else
		{
			printf ( "Cannot create ~/.projectM/config.inp, using default config file\n" );
			if ( ( in = fopen ( projectM_config, "r" ) ) != 0 )
			{
				printf ( "Successfully opened default config file\n" );
				fclose ( in );
				return std::string ( projectM_config );
			}
			else{ printf ( "Using implementation defaults, your system is really messed up, I'm suprised we even got this far\n" );  abort();}
			
		}

	}


	abort();
}

