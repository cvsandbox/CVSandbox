/*
    Computer Vision Sandbox

    Copyright (C) 2011-2019, cvsandbox
    http://www.cvsandbox.com/contacts.html

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#pragma once
#ifndef CVS_WIZARD_DIALOG_HPP
#define CVS_WIZARD_DIALOG_HPP

#include <QDialog>

namespace Ui
{
    class WizardDialog;
}

namespace Private
{
    class WizardDialogData;
}

class WizardPageFrame;

class WizardDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit WizardDialog( const QString& wizardName, QWidget* parent = 0 );
    ~WizardDialog( );

    // Add page to the wizard
    void AddPage( WizardPageFrame* page );
    // Remove page from the wizard
    void RemovePage( WizardPageFrame* page );

protected:
    virtual void OnChangingPage( int nextPage );

protected:
    virtual void closeEvent( QCloseEvent* event );

public slots:
    virtual void accept( );
    virtual void reject( );

private slots:
    void on_wizardPage_CompleteStatusChanged( bool canGoNext );
    void on_nextButton_clicked( );
    void on_previousButton_clicked( );
    void on_finishButton_clicked( );
    void on_cancelButton_clicked( );
    void on_helpButton_clicked( );

private:
    void ShowNewPage( int newPageIndex );
    void UpdateDialogButtonsStatus( );

private:
    Ui::WizardDialog*           ui;
    Private::WizardDialogData*  mData;
};

#endif // CVS_WIZARD_DIALOG_HPP
