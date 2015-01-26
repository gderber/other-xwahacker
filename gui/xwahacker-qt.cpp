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
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

#include "xwahacker-qt.h"

#define GUI 1
#include "../xwahacker.c"

static void addResHeading(QGridLayout *grid)
{
    grid->addWidget(new QLabel(QObject::tr("Original")), 0, 0);
    grid->addWidget(new QLabel(QObject::tr("New resolution")), 0, 1, 1, 2);
    grid->addWidget(new QLabel(QObject::tr("FOV")), 0, 3);
    grid->addWidget(new QLabel(QObject::tr("HUD scale")), 0, 4);
}

static void addResNames(QGridLayout *grid)
{
    grid->addWidget(new QLabel("640x480"), 1, 0);
    grid->addWidget(new QLabel("800x600"), 2, 0);
    grid->addWidget(new QLabel("1152x864"), 3, 0);
    grid->addWidget(new QLabel("1600x1200"), 4, 0);
}

static void addSpinBoxes(QGridLayout *grid, QSpinBox *(&sb)[4][2], QDoubleSpinBox *(dsb)[4][2])
{
    for (int i = 0; i < 4; ++i)
    {
        sb[i][0] = new QSpinBox();
        sb[i][0]->setMinimum(640);
        sb[i][0]->setMaximum(8192);
        grid->addWidget(sb[i][0], i+1, 1);
        sb[i][1] = new QSpinBox();
        sb[i][1]->setMinimum(480);
        sb[i][1]->setMaximum(8192);
        grid->addWidget(sb[i][1], i+1, 2);
        dsb[i][0] = new QDoubleSpinBox();
        dsb[i][0]->setMinimum(10);
        dsb[i][0]->setMaximum(170);
        grid->addWidget(dsb[i][0], i+1, 3);
        dsb[i][1] = new QDoubleSpinBox();
        dsb[i][1]->setMinimum(0.1);
        dsb[i][1]->setMaximum(10);
        dsb[i][1]->setSingleStep(0.1);
        grid->addWidget(dsb[i][1], i+1, 4);
    }
}

static const char *opt_names[NUM_OPTS] = {
    [OPT_FIXED_CLEAR] = "fixed Z clear",
    [OPT_FORCE_800] = "force 800x600",
    [OPT_USE_32BIT] = "32 bit (buggy)",
    [OPT_NOCD] = "no CD check",
    [OPT_HDVOICE] = "voice from disk",
    [OPT_NOSTARS] = "starfield off",
    [OPT_MSGLOOP] = "message loop in hangar (Linux/WINE fix)",
};

XWAHacker::XWAHacker()
{
    QGridLayout *res_layout = new QGridLayout();
    addResHeading(res_layout);
    addResNames(res_layout);
    addSpinBoxes(res_layout, res_spinboxes, fov_hud_spinboxes);

    QGroupBox *res_group = new QGroupBox(tr("Resolutions"));
    res_group->setLayout(res_layout);

    QGridLayout *opt_layout = new QGridLayout();
    for (int i = 0; i < NUM_OPTS; ++i)
    {
        opts[i] = new QCheckBox(tr(opt_names[i]));
        opt_layout->addWidget(opts[i]);
    }

    QGroupBox *opt_group = new QGroupBox(tr("Options"));
    opt_group->setLayout(opt_layout);

    QHBoxLayout *button_layout = new QHBoxLayout();
    QPushButton *exit = new QPushButton(tr("Exit"));
    button_layout->addWidget(exit);
    QPushButton *save = new QPushButton(tr("Save and exit"));
    button_layout->addWidget(save);

    QVBoxLayout *main_layout = new QVBoxLayout();
    main_layout->addWidget(res_group);
    main_layout->addWidget(opt_group);
    main_layout->addLayout(button_layout);

    QWidget *central_widget = new QWidget();
    central_widget->setLayout(main_layout);
    setCentralWidget(central_widget);

    connect(res_spinboxes[0][0], SIGNAL(valueChanged(int)), this, SLOT(res0_change()));
    connect(res_spinboxes[0][1], SIGNAL(valueChanged(int)), this, SLOT(res0_change()));
    connect(res_spinboxes[1][0], SIGNAL(valueChanged(int)), this, SLOT(res1_change()));
    connect(res_spinboxes[1][1], SIGNAL(valueChanged(int)), this, SLOT(res1_change()));
    connect(res_spinboxes[2][0], SIGNAL(valueChanged(int)), this, SLOT(res2_change()));
    connect(res_spinboxes[2][1], SIGNAL(valueChanged(int)), this, SLOT(res2_change()));
    connect(res_spinboxes[3][0], SIGNAL(valueChanged(int)), this, SLOT(res3_change()));
    connect(res_spinboxes[3][1], SIGNAL(valueChanged(int)), this, SLOT(res3_change()));

    connect(exit, SIGNAL(clicked()), qApp, SLOT(quit()));
    connect(save, SIGNAL(clicked()), this, SLOT(save()));
}

bool XWAHacker::openBinary(const char *filename)
{
    xwa = fopen(filename, "r+b");
    if (!xwa)
    {
        QMessageBox err;
#ifdef __WIN32__
        err.setText(tr("Could not open file.\nTry running this program as administrator."));
#else
        err.setText(tr("Could not open file"));
#endif
        err.exec();
        return false;
    }

    uint8_t buffer[BUFFER_SZ];
    int count = count_patches(buffer, xwa, binaries[0].patchgroups);
    if (count != num_patchgroups(binaries[0].patchgroups))
    {
        QMessageBox err;
        err.setText(tr(count ? "File has unsupported modifications" : "Not a supported XWingAlliance binary"));
        err.exec();
        return false;
    }
    struct resopts resolutions[NUM_RES];
    read_res(buffer, xwa, resolutions);
    for (int i = 0; i < 4; ++i)
    {
        res_spinboxes[i][0]->setValue(resolutions[i].w);
        res_spinboxes[i][1]->setValue(resolutions[i].h);
        fov_hud_spinboxes[i][0]->setValue(fov2deg(resolutions[i].fov, resolutions[i].h));
        fov_hud_spinboxes[i][1]->setValue(resolutions[i].hud_scale.f);
    }

    for (int i = 0; i < NUM_OPTS; ++i)
    {
        enum PATCHES optsmap[NUM_OPTS] = {
            [OPT_FIXED_CLEAR] = PATCH_CLEAR2,
            [OPT_FORCE_800] = PATCH_FORCE_RES,
            [OPT_USE_32BIT] = PATCH_32BIT_FB,
            [OPT_NOCD] = PATCH_NO_CD_CHECK,
            [OPT_HDVOICE] = PATCH_HD_VOICE,
            [OPT_NOSTARS] = PATCH_STARS_OFF,
            [OPT_MSGLOOP] = PATCH_ADD_MSGLOOP,
        };
        opts[i]->setChecked(check_patch(buffer, xwa, optsmap[i], 1));
    }
    return true;
}

void XWAHacker::res_change(int i)
{
    int h = res_spinboxes[i][1]->value();
    fov_hud_spinboxes[i][0]->setValue(fov2deg(default_fov(h), h));
    fov_hud_spinboxes[i][1]->setValue(default_hud_scale(h));
}

void XWAHacker::res0_change()
{
    res_change(0);
}
void XWAHacker::res1_change()
{
    res_change(1);
}
void XWAHacker::res2_change()
{
    res_change(2);
}
void XWAHacker::res3_change()
{
    res_change(3);
}

void XWAHacker::save()
{
    uint8_t buffer[BUFFER_SZ];
    struct resopts resolutions[NUM_RES];
    read_res(buffer, xwa, resolutions);
    for (int i = 0; i < 4; ++i)
    {
        resolutions[i].w = res_spinboxes[i][0]->value();
        resolutions[i].h = res_spinboxes[i][1]->value();
        int h = resolutions[i].h;
        double deffov = fov2deg(default_fov(h), h);
        double fov = fov_hud_spinboxes[i][0]->value();
        if (fov > deffov - 0.015 && fov < deffov + 0.015)
            resolutions[i].fov = default_fov(h);
        else
            resolutions[i].fov = deg2fov(fov, h);
        float defhud = default_hud_scale(h);
        double hud = fov_hud_spinboxes[i][1]->value();
        if (hud > defhud - 0.015 && hud < defhud + 0.015)
            resolutions[i].hud_scale.f = defhud;
        else
            resolutions[i].hud_scale.f = hud;
        if (!write_res(buffer, xwa, resolutions + i, i, 0, 0))
        {
            QMessageBox err;
            err.setText(tr("Failed writing resolution values"));
            err.exec();
            return;
        }
    }
    for (int i = 0; i < NUM_OPTS; ++i)
    {
        bool c = opts[i]->isChecked();
        int opt2collection[NUM_OPTS][2] = {
            [OPT_FIXED_CLEAR] = {2, 3},
            [OPT_FORCE_800] = {4, 5},
            [OPT_USE_32BIT] = {0, 1},
            [OPT_NOCD] = {-PATCH_CD_CHECK, -PATCH_NO_CD_CHECK},
            [OPT_HDVOICE] = {-PATCH_CD_VOICE, -PATCH_HD_VOICE},
            [OPT_NOSTARS] = {-PATCH_STARS_ON, -PATCH_STARS_OFF},
            [OPT_MSGLOOP] = {-PATCH_NO_MSGLOOP, -PATCH_ADD_MSGLOOP},
        };
        int collection = opt2collection[i][c];
        int res = 0;
        if (collection < 0)
        {
            res = apply_patch(buffer, xwa, binaries[0].patchgroups, static_cast<enum PATCHES>(-collection));
        }
        else
        {
            res = apply_collection(buffer, xwa, binaries, collection);
        }
        if (!res)
        {
            QMessageBox err;
            err.setText(tr("Failed setting option") + tr(opt_names[i]));
            err.exec();
            return;
        }
    }

    qApp->quit();
}
