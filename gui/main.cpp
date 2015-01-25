/*
 * Qt GUI for XWAHacker
 * Copyright (C) 2015 Reimar Döffinger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <QApplication>
#include <QFileDialog>

#include "xwahacker-qt.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    char *filename = NULL;
    if (argc > 1) filename = strdup(argv[1]);
    else
    {
        QString name = QFileDialog::getOpenFileName(0, QObject::tr("Select xwingalliance.exe to edit"), QString(), "xwingalliance.exe;;*.exe;;*.*");
        if (name.isNull())
            return 1;
        QByteArray tmp = name.toLocal8Bit();
        filename = strdup(tmp.data());
    }

    XWAHacker window;
    if (!window.openBinary(filename))
        return 1;
    free(filename);
    filename = NULL;
    window.show();

    return app.exec();
}
