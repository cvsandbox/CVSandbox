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

#include "MorphologyStructuringElementDialog.hpp"
#include "ui_MorphologyStructuringElementDialog.h"
#include "GlobalServiceManager.hpp"

#include <QItemDelegate>
#include <QLineEdit>
#include <QKeyEvent>

using namespace std;
using namespace CVSandbox;

#define TABLE_COLUMN_WIDTH (40)

namespace Private
{
    class MorphologyStructuringElementDialogData
    {
    private:
        MorphologyStructuringElementDialog::Mode Mode;

    public:
        XVariantArray2d StructuringElement;

    public:
        MorphologyStructuringElementDialogData( MorphologyStructuringElementDialog::Mode mode ) :
            Mode( mode ),
            StructuringElement( XVT_I1, 3, 3, XVariant( 1 ) )
        {
        }

        void RefreshTable( QTableWidget* table );
        void ResizeStructuringElement( uint32_t size );
        QColor GetValueColor( int value );
    };

    class TableValidationDelegate : public QItemDelegate
    {
    private:
        int Min;
        int Max;

    public:
        TableValidationDelegate( int min, int max )
        {
            Min = min;
            Max = max;
        }

        QWidget* createEditor( QWidget* parent, const QStyleOptionViewItem& /* option */,
                               const QModelIndex& /* index */ ) const
        {
            QLineEdit*     lineEdit  = new QLineEdit( parent );
            QIntValidator* validator = new QIntValidator( Min, Max, lineEdit );

            lineEdit->setValidator( validator );
            return lineEdit;
        }
    };
}

MorphologyStructuringElementDialog::MorphologyStructuringElementDialog( Mode mode, QWidget* parent ) :
    QDialog( parent ),
    ui( new Ui::MorphologyStructuringElementDialog ),
    mData( new Private::MorphologyStructuringElementDialogData( mode ) )
{
    ui->setupUi( this );

    // disable "?" button on title bar
    setWindowFlags( windowFlags( ) & ~Qt::WindowContextHelpButtonHint );

    // set delegate to make sure only numeric input is allowed in certain range
    ui->elementTable->setItemDelegate( new Private::TableValidationDelegate( ( mode == Mode::SimpleOperator ) ? 0 : -1, 1 ) );

    // don't allow resizing columns and rows
#if QT_VERSION >= 0x050000
    ui->elementTable->horizontalHeader( )->setSectionResizeMode( QHeaderView::Fixed );
    ui->elementTable->verticalHeader( )->setSectionResizeMode( QHeaderView::Fixed );
#else
    ui->elementTable->horizontalHeader( )->setResizeMode( QHeaderView::Fixed );
    ui->elementTable->verticalHeader( )->setResizeMode( QHeaderView::Fixed );
#endif

    ui->sizeBox->installEventFilter( this );

    // set-up legend
    ui->legendColor1Label->setStyleSheet( "QLabel { background-color : #FFD0D0; }" );
    if ( mode == Mode::SimpleOperator )
    {
        ui->legend1Label->setText( "1 - Process" );

        ui->legendColor2Label->setStyleSheet( "QLabel { background-color : #FFFFFF; }" );
        ui->legend2Label->setText( "0 - Ignore" );

        ui->legendColor3Label->setVisible( false );
        ui->legend3Label->setVisible( false );
    }
    else
    {
        ui->legendColor1Label->setStyleSheet( "QLabel { background-color : #FFD0D0; }" );
        ui->legend1Label->setText( "1 - Foreground" );

        ui->legendColor2Label->setStyleSheet( "QLabel { background-color : #FFFFD0; }" );
        ui->legend2Label->setText( "0 - Background" );

        ui->legendColor3Label->setStyleSheet( "QLabel { background-color : #FFFFFF; }" );
        ui->legend3Label->setText( "-1 - Don't care" );

        setWindowTitle( "Hit-and-Miss Structuring Element" );
    }

    SetStructuringElement( mData->StructuringElement );

    // restore size/position
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->RestoreWidget( this );
}

MorphologyStructuringElementDialog::~MorphologyStructuringElementDialog( )
{
    delete mData;
    delete ui;
}

// Widget is closed
void MorphologyStructuringElementDialog::closeEvent( QCloseEvent* /*closeEvet*/ )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
}

// Dialog is acceept
void MorphologyStructuringElementDialog::accept( )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::accept( );
}

// Dialog is rejected
void MorphologyStructuringElementDialog::reject( )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::reject( );
}

// Filter some events to ignore Enter key
bool MorphologyStructuringElementDialog::eventFilter( QObject* obj, QEvent* ev )
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
void MorphologyStructuringElementDialog::on_elementTable_cellChanged( int row, int column )
{
    QTableWidgetItem* item = ui->elementTable->item( row, column );
    XVariant          var( item->text( ).toStdString( ) );

    mData->StructuringElement.Set( row, column, var );
    item->setBackground( mData->GetValueColor( var.ToInt( ) ) );
}

// Get current value of the structuring element
XVariantArray2d MorphologyStructuringElementDialog::StructuringElement( ) const
{
    return mData->StructuringElement;
}

// Set new value of the structuring element
void MorphologyStructuringElementDialog::SetStructuringElement( const XVariantArray2d& element )
{
    mData->StructuringElement = element;
    mData->RefreshTable( ui->elementTable );

    if ( mData->StructuringElement.Rows( ) != (uint32_t) ui->sizeBox->value( ) )
    {
        ui->sizeBox->setValue( mData->StructuringElement.Rows( ) );
    }
}

// Size spin box has changed - resize structuring element
void MorphologyStructuringElementDialog::on_sizeBox_valueChanged( int size )
{
    if ( ( size % 2 ) == 0 )
    {
        size++;
    }

    if ( (uint32_t) size != mData->StructuringElement.Rows( ) )
    {
        mData->ResizeStructuringElement( (uint32_t) size );
        mData->RefreshTable( ui->elementTable );
    }
}

namespace Private
{

// Refresh table displaying current value of structuring element
void MorphologyStructuringElementDialogData::RefreshTable( QTableWidget* table )
{
    uint32_t rowsCount    = StructuringElement.Rows( );
    uint32_t columnsCount = StructuringElement.Columns( );

    table->clear( );
    table->setRowCount( rowsCount );
    table->setColumnCount( columnsCount );

    for ( uint32_t column = 0; column < columnsCount; column++ )
    {
        table->setColumnWidth( column, TABLE_COLUMN_WIDTH );

        for ( uint32_t row = 0; row < rowsCount; row++ )
        {
            XVariant value;

            if ( StructuringElement.Get( row, column, value ) == SuccessCode )
            {
                QTableWidgetItem* tableItem = new QTableWidgetItem( QString::fromStdString( value.ToString( ) ) );

                tableItem->setTextAlignment( Qt::AlignCenter );
                tableItem->setBackgroundColor( GetValueColor( value.ToInt( ) ) );
                table->setItem( row, column, tableItem );
            }
        }
    }
}

// Resize structiring element keeping old one centered
void MorphologyStructuringElementDialogData::ResizeStructuringElement( uint32_t size )
{
    XVariantArray2d newStructuringElement( XVT_I1, size, size, XVariant( 0 ) );

    uint32_t start    = 0;
    uint32_t end      = size;
    int32_t  halfDiff = ( (int32_t) size - (int32_t) StructuringElement.Rows( ) ) / 2;

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

            StructuringElement.Get( row - halfDiff, column - halfDiff, element );
            newStructuringElement.Set( row, column, element );
        }
    }

    StructuringElement = newStructuringElement;
}

// Get color to use for highlighting cell with the specified value
QColor MorphologyStructuringElementDialogData::GetValueColor( int value )
{
    QColor color( Qt::white );

    if ( value == 1 )
    {
        color = QColor::fromRgb( 0xFF, 0xD0, 0xD0 );
    }
    else if ( ( value == 0 ) && ( Mode == MorphologyStructuringElementDialog::Mode::HitAndMiss ) )
    {
        color = QColor::fromRgb( 0xFF, 0xFF, 0xD0 );
    }

    return color;
}

} // Private
