/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software  Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include <QApplication>
#include <QFile>
#include <QSplashScreen>

#include <kamikaze/renderbuffer.h>

#include "core/kamikaze_main.h"
#include "ui/mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setOrganizationName("numéro7");
	a.setApplicationName("kamikaze");

	QFile file("styles/main.qss");
	file.open(QFile::ReadOnly);
	QString style_sheet = QLatin1String(file.readAll());

	qApp->setStyleSheet(style_sheet);

	auto pixmap = QPixmap(600, 600);
	pixmap.fill(Qt::gray);

	auto splash = std::make_unique<QSplashScreen>(pixmap);
	splash->show();

	qApp->processEvents(QEventLoop::AllEvents);

	int ret;

	{
		Main main;

		splash->showMessage("Initializing types...");
		main.initialize();

		splash->showMessage("Loading plugins...");
		main.charge_greffons();

		MainWindow w(&main);
		w.setWindowTitle(QCoreApplication::applicationName());
		w.showMaximized();

		splash->finish(&w);

		ret = a.exec();
	}

	/* The objects are freed after the viewer is freed, so make sure all buffers
	 * are freed before quitting. Also make sure Main is deleted before calling
	 * this. */
	purge_all_buffers();

	return ret;
}
