/*
    Common tools for Computer Vision Sandbox

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

#include "ConvolutionKernelDialog.hpp"
#include "ui_ConvolutionKernelDialog.h"

#include "GlobalServiceManager.hpp"

#include <QItemDelegate>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QKeyEvent>

using namespace std;
using namespace CVSandbox;

#define TABLE_COLUMN_WIDTH (90)

namespace Private
{
    class ConvolutionKernelDialogData
    {
    public:
        XVariantArray2d Kernel;

    public:
        ConvolutionKernelDialogData( ) :
            Kernel( XVT_R4, 3, 3, XVariant( 1.0f ) )
        {
        }

        void RefreshTable( QTableWidget* table );
        void ResizeStructuringElement( uint32_t size );
    };

    class KernelValidationDelegate : public QItemDelegate
    {
    public:
        QWidget* createEditor( QWidget* parent, const QStyleOptionViewItem& /* option */,
                               const QModelIndex& /* index */ ) const
        {
            QDoubleSpinBox* editor = new QDoubleSpinBox( parent );

            editor->setRange( -10000, 10000 );
            editor->setDecimals( 4 );

            return editor;
        }
    };
}

ConvolutionKernelDialog::ConvolutionKernelDialog( QWidget* parent ) :
    QDialog( parent ),
    ui( new Ui::ConvolutionKernelDialog ),
    mData( new Private::ConvolutionKernelDialogData( ) )
{
    ui->setupUi( this );

    // disable "?" button on title bar
    setWindowFlags( windowFlags( ) & ~Qt::WindowContextHelpButtonHint );

    //ui->kernelTable->set
    // set delegate to make sure only numeric input is allowed in certain range
    ui->kernelTable->setItemDelegate( new Private::KernelValidationDelegate( ) );

    // don't allow resizing columns and rows
#if QT_VERSION >= 0x050000
    ui->kernelTable->horizontalHeader( )->setSectionResizeMode( QHeaderView::Fixed );
    ui->kernelTable->verticalHeader( )->setSectionResizeMode( QHeaderView::Fixed );
#else
    ui->kernelTable->horizontalHeader( )->setResizeMode( QHeaderView::Fixed );
    ui->kernelTable->verticalHeader( )->setResizeMode( QHeaderView::Fixed );
#endif

    ui->sizeBox->installEventFilter( this );

    SetKernel( mData->Kernel );

    // restore size/position
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->RestoreWidget( this );
}

ConvolutionKernelDialog::~ConvolutionKernelDialog( )
{
    delete mData;
    delete ui;
}

// Widget is closed
void ConvolutionKernelDialog::closeEvent( QCloseEvent* /*closeEvet*/ )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
}

// Dialog is acceepted
void ConvolutionKernelDialog::accept( )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::accept( );
}

// Dialog is rejected
void ConvolutionKernelDialog::reject( )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::reject( );
}

// Filter some events to ignore Enter key
bool ConvolutionKernelDialog::eventFilter( QObject* obj, QEvent* ev )
{
    bool ret = false;

    if ( obj == ui->sizeBox )
    {
        if ( ( ev->type( ) == QEvent::KeyPress ) || ( ev->type( ) == QEvent::KeyRelease ) )
        {
            if ( ( static_cast<const QKeyEvent*>( ev )->key( ) == Qt::Key_Enter ) ||
                 ( static_cast<const QKeyEvent*>( ev )->key( ) == Qt::Key_Return ) )
            {
                ui->sizeBox->selectAll( );
                ret = true;
            }
        }
    }

    if ( !ret )
    {
        ret = QWidget::eventFilter( obj, ev );
    }

    return ret;
}

// An element has changed in the table
void ConvolutionKernelDialog::on_kernelTable_cellChanged( int row, int column )
{
    QTableWidgetItem* item = ui->kernelTable->item( row, column );
    XVariant          var( item->text( ).toStdString( ) );

    mData->Kernel.Set( row, column, var );
}

// Get current value of the convolution kernel
XVariantArray2d ConvolutionKernelDialog::Kernel( ) const
{
    return mData->Kernel;
}

// Set new value of the convolution kernel
void ConvolutionKernelDialog::SetKernel( const XVariantArray2d& element )
{
    mData->Kernel = element;
    mData->RefreshTable( ui->kernelTable );

    if ( mData->Kernel.Rows( ) != (uint32_t) ui->sizeBox->value( ) )
    {
        ui->sizeBox->setValue( mData->Kernel.Rows( ) );
    }
}

// Size spin box has changed - resize kernel
void ConvolutionKernelDialog::on_sizeBox_valueChanged( int size )
{
    if ( ( size % 2 ) == 0 )
    {
        size++;
    }

    if ( (uint32_t) size != mData->Kernel.Rows( ) )
    {
        mData->ResizeStructuringElement( (uint32_t) size );
        mData->RefreshTable( ui->kernelTable );
    }
}

namespace Private
{

// Refresh table displaying current value of convolution kernel
void ConvolutionKernelDialogData::RefreshTable( QTableWidget* table )
{
    uint32_t rowsCount    = Kernel.Rows( );
    uint32_t columnsCount = Kernel.Columns( );

    table->clear( );
    table->setRowCount( rowsCount );
    table->setColumnCount( columnsCount );

    for ( uint32_t column = 0; column < columnsCount; column++ )
    {
        table->setColumnWidth( column, TABLE_COLUMN_WIDTH );

        for ( uint32_t row = 0; row < rowsCount; row++ )
        {
            XVariant value;

            if ( Kernel.Get( row, column, value ) == SuccessCode )
            {
                QTableWidgetItem* tableItem = new QTableWidgetItem( QString::fromStdString( value.ToString( ) ) );

                tableItem->setTextAlignment( Qt::AlignCenter );
                table->setItem( row, column, tableItem );
            }
        }
    }
}

// Resize kernel keeping old one centered
void ConvolutionKernelDialogData::ResizeStructuringElement( uint32_t size )
{
    XVariantArray2d newKernel( XVT_R4, size, size, XVariant( 1.0f ) );

    uint32_t start    = 0;
    uint32_t end      = size;
    int32_t  halfDiff = ( (int32_t) size - (int32_t) Kernel.Rows( ) ) / 2;

    if ( halfDiff > 0 )
    {
        start += halfDiff;
        end   -= halfDiff;
    }

    for ( uint32_t row = start; row < end; row++ )
    {
        for ( uint32_t column = start; column < end; column++ )
        {
            XVariant element;

            Kernel.Get( row - halfDiff, column - halfDiff, element );
            newKernel.Set( row, column, element );
        }
    }

    Kernel = newKernel;
}

} // Private
