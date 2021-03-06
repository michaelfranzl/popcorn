/*
 * popcorn (c) 2016 Michael Franzl
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include <QCheckBox>

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::loadSettings() {
    ui->labelVersion->setText(m_version);
    ui->jailWorkingInput->setText(jail_working_path);
    ui->limitFileReadingCheckBox->setChecked(settings->value("fileread_jailed").toString() == "true");
    ui->labelIniFile->setText(settings->fileName());
    ui->urlInput->setText(settings->value("url").toString());
}

void OptionsDialog::accept() {
    qDebug() << "[OptionsDialog::accept]";
    settings->setValue("jail_working", ui->jailWorkingInput->text());
    settings->setValue("fileread_jailed", ui->limitFileReadingCheckBox->isChecked());
    settings->setValue("url", ui->urlInput->text());

    emit accepting();
    close();
}
