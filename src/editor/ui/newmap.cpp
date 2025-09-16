#include <newmap.h>
#include <ui_newmap.h>

#undef TRUE
#undef FALSE

#include <e_map.h>
#include <u_map_asset.h>
#include <u_tile_asset.h>

#include <QFileDialog.h>
#include <QMessageBox>
#include <QPushButton>


NewMap::NewMap( QWidget* i_pcParent, rumAssetID i_eExitMapID, const QPoint& i_rcExitPos, bool i_bEditMode )
  : QDialog( i_pcParent )
  , m_pcUI( new Ui::NewMap )
  , m_uiCols( 32 )
  , m_uiRows( 32 )
  , m_uiExitPosX( 0 )
  , m_uiExitPosY( 0 )
  , m_bEditMode( i_bEditMode )
{
  m_pcUI->setupUi( this );

  // Make error labels red
  QPalette cPalette{ m_pcUI->labelFile_Error->palette() };
  cPalette.setColor( m_pcUI->labelFile_Error->foregroundRole(), Qt::red );
  m_pcUI->labelFile_Error->setPalette( cPalette );
  m_pcUI->labelBorderTile_Error->setPalette( cPalette );
  m_pcUI->labelBaseTile_Error->setPalette( cPalette );

  cPalette = m_pcUI->labelMap_Error->palette();
  cPalette.setColor( m_pcUI->labelMap_Error->foregroundRole(), Qt::red );
  m_pcUI->labelMap_Error->setPalette( cPalette );

  // Hide error labels
  m_pcUI->labelFile_Error->setHidden( true );
  m_pcUI->labelBorderTile_Error->setHidden( true );
  m_pcUI->labelBaseTile_Error->setHidden( true );
  m_pcUI->labelMap_Error->setHidden( true );

  SetNumColumns( m_uiCols );
  SetNumRows( m_uiRows );

  SetExitPosX( i_rcExitPos.x() );
  SetExitPosY( i_rcExitPos.y() );

  InitMapList();
  InitTileList();

  if( IsEditing() )
  {
    // We are in edit mode, so change the title to reflect this
    setWindowTitle( "Edit Map" );

    m_pcUI->lineEditFile->setEnabled( false );
    m_pcUI->pushButtonFile->setEnabled( false );

    // Disable the list widget unless the map size grows
    m_pcUI->listWidgetBaseTile->setEnabled( false );
  }
  else
  {
    // New maps do not need an anchor selection
    m_pcUI->comboBoxAnchor->setEnabled( false );
  }

  // Select the previously selected exit map if it was provided
  m_pcUI->listWidgetMap->setCurrentRow( 0 );

  // Find the matching map by walking the list and looking at the UserRole data
  for( int32_t i = 0; i < m_pcUI->listWidgetMap->count(); ++i )
  {
    QListWidgetItem* pcItem{ m_pcUI->listWidgetMap->item( i ) };
    if( pcItem )
    {
      auto cVariant{ pcItem->data( Qt::UserRole ) };
      if( cVariant.isValid() )
      {
        const rumAssetID eMapID{ (rumAssetID)cVariant.toInt() };
        if( eMapID == i_eExitMapID )
        {
          m_pcUI->listWidgetMap->setCurrentItem( pcItem );
          break;
        }
      }
    }
  }
}


NewMap::~NewMap()
{
  delete m_pcUI;
}


void NewMap::InitMapList()
{
  for( const auto& iter : rumMapAsset::GetAssetHash() )
  {
    const rumMapAsset* pcAsset{ iter.second };
    if( pcAsset )
    {
      QListWidgetItem* pcItem{ new QListWidgetItem };
      pcItem->setData( Qt::DisplayRole, pcAsset->GetName().c_str() );
      pcItem->setData( Qt::UserRole, pcAsset->GetAssetID() );

      m_pcUI->listWidgetMap->addItem( pcItem );
    }
  }

  if( m_pcUI->listWidgetMap->count() > 0 )
  {
    // Sort and add items
    m_pcUI->listWidgetMap->sortItems();
    m_pcUI->listWidgetMap->setEnabled( true );
  }
  else
  {
    QMessageBox::information( this, "No Map Assets Defined!",
                              "You must define one or more Map Assets before specifying an exit to another map" );
  }
}


void NewMap::InitTileList()
{
  // Populate the Tiles list widget
  for( const auto& iter : rumTileAsset::GetAssetHash() )
  {
    const rumTileAsset* pcAsset{ iter.second };
    if( pcAsset )
    {
      QListWidgetItem* pcItem{ new QListWidgetItem };
      pcItem->setData( Qt::DisplayRole, pcAsset->GetName().c_str() );
      pcItem->setData( Qt::UserRole, pcAsset->GetAssetID() );

      m_pcUI->listWidgetBaseTile->addItem( pcItem );
      m_pcUI->listWidgetBorderTile->addItem( new QListWidgetItem( *pcItem ) );
    }
  }

  if( m_pcUI->listWidgetBaseTile->count() > 0 || m_pcUI->listWidgetBorderTile->count() > 0 )
  {
    m_pcUI->listWidgetBaseTile->sortItems();
    m_pcUI->listWidgetBaseTile->setEnabled( true );

    m_pcUI->listWidgetBorderTile->sortItems();
    m_pcUI->listWidgetBorderTile->setEnabled( true );
  }
  else
  {
    QMessageBox::information( this, "No Tiles Assets Defined!",
                              "You must define one or more Tile Assets before you can create a map." );
  }
}


void NewMap::on_buttonBox_accepted()
{
  if( !ValidateInput() )
  {
    return;
  }

  const uint32_t uiCols{ (uint32_t)m_pcUI->spinBoxCols->value() };
  const uint32_t uiRows{ (uint32_t)m_pcUI->spinBoxRows->value() };

  const uint32_t uiPosX{ (uint32_t)m_pcUI->spinBoxPosX->value() };
  const uint32_t uiPosY{ (uint32_t)m_pcUI->spinBoxPosY->value() };

  rumAssetID eBaseTileID{ INVALID_ASSET_ID };
  rumAssetID eBorderTileID{ INVALID_ASSET_ID };
  rumAssetID eMapID{ INVALID_ASSET_ID };

  QListWidgetItem* pcItem{ m_pcUI->listWidgetMap->currentItem() };
  if( pcItem )
  {
    const auto cVariant{ pcItem->data( Qt::UserRole ) };
    eMapID = (rumAssetID)cVariant.toInt();
    emit mapExitChanged( eMapID, QPoint( uiPosX, uiPosY ) );
  }

  pcItem = m_pcUI->listWidgetBaseTile->currentItem();
  if( pcItem )
  {
    const auto cVariant{ pcItem->data( Qt::UserRole ) };
    eBaseTileID = (rumAssetID)cVariant.toInt();
  }

  pcItem = m_pcUI->listWidgetBorderTile->currentItem();
  if( pcItem )
  {
    const auto cVariant{ pcItem->data( Qt::UserRole ) };
    eBorderTileID = (rumAssetID)cVariant.toInt();
  }

  if( IsEditing() )
  {
    if( ( uiCols != m_uiCols ) || ( uiRows != m_uiRows ) )
    {
      const int32_t eAnchor{ m_pcUI->comboBoxAnchor->currentIndex() };

      // Warn user if they are about to shrink the map in any way
      if( uiCols < m_uiCols || uiRows < m_uiRows )
      {
        const QString strQuestion{ "WARNING: By shrinking the map, you will lose existing map data!\n"
                                   "Are you sure you want to reduce the map size?" };
        // Verify
        if( QMessageBox::No ==  QMessageBox::question( this, "Verify Resize", strQuestion,
                                                       QMessageBox::Yes | QMessageBox::No,
                                                       QMessageBox::No ) )
        {
          // Resize canceled
          return;
        }
      }

      emit mapResized( uiCols, uiRows, eAnchor, eBaseTileID );
    }

    emit mapBorderChanged( eBorderTileID );
  }
  else
  {
    // Create, resize, and save the map
    Sqrat::Object sqObject{ rumGameObject::Create( eMapID ) };
    if( sqObject.GetType() == OT_INSTANCE )
    {
      EditorMap* pcEditorMap{ sqObject.Cast<EditorMap*>() };
      if( pcEditorMap )
      {
        pcEditorMap->ResizeMap( uiCols, uiRows, eBaseTileID );
        pcEditorMap->SetBorderTile( eBorderTileID );
        pcEditorMap->Save( qPrintable( m_strAbsoluteFilePath ) );

        emit mapCreated( m_strAbsoluteFilePath );

        const QString strRelFilePath{ MainWindow::GetProjectMapDir().relativeFilePath( m_strAbsoluteFilePath ) };
        QMessageBox::information( this, "Map Created", "Successfully created " + strRelFilePath );
      }

      // Free the map
      //pEditorMap->Free();
    }
  }

  accept();
}


void NewMap::on_lineEditFile_textEdited( const QString& i_strText )
{
  m_pcUI->labelFile_Error->setHidden( true );
}


void NewMap::on_listWidgetBaseTile_itemSelectionChanged()
{
  m_pcUI->labelBaseTile_Error->setHidden( true );
}


void NewMap::on_listWidgetBorderTile_itemSelectionChanged()
{
  m_pcUI->labelBorderTile_Error->setHidden( true );
}


void NewMap::on_listWidgetMap_itemSelectionChanged()
{
  m_pcUI->labelMap_Error->setHidden( true );
}


void NewMap::on_pushButtonFile_clicked()
{
  m_pcUI->labelFile_Error->setHidden( true );

  // Get the existing file path
  QString strFilePath{ m_pcUI->lineEditFile->text() };
  strFilePath = QDir::fromNativeSeparators( strFilePath );
  const QDir cDirFilePath( strFilePath );

  // Determine the standard path to game maps
  const QDir cDirMapPath{ MainWindow::GetProjectMapDir() };
  const QString strMapsPath{ QDir::fromNativeSeparators( cDirMapPath.canonicalPath() ) };

  // Convert the existing file path to absolute
  QString strAbsFilePath;
  if( cDirFilePath.isRelative() )
  {
    strAbsFilePath = cDirMapPath.absoluteFilePath( strFilePath );
    strAbsFilePath = QDir::cleanPath( strAbsFilePath );
  }
  else
  {
    strAbsFilePath = strFilePath;
  }

  // Make sure file is under the project maps subfolder
  if( !strAbsFilePath.startsWith( strMapsPath ) )
  {
    // Fall back to default maps folder
    strAbsFilePath = strMapsPath;
  }

  const QString strNewFilePath{ QFileDialog::getSaveFileName( this, tr( "File Location" ), strAbsFilePath ) };
  if( !strNewFilePath.isEmpty() )
  {
    // Create a relative file path to the selected file
    const QString strRelFilePath{ cDirMapPath.relativeFilePath( strNewFilePath ) };
    m_pcUI->lineEditFile->setText( strRelFilePath );
  }
}


void NewMap::on_spinBoxCols_valueChanged( int32_t i_uiCols )
{
  if( IsEditing() )
  {
    const uint32_t uiRows{ (uint32_t)m_pcUI->spinBoxRows->value() };
    if( i_uiCols <= (int32_t)m_uiCols && uiRows <= m_uiRows )
    {
      m_pcUI->listWidgetBaseTile->setEnabled( false );
      m_pcUI->labelBaseTile_Error->setHidden( true );
    }
    else
    {
      m_pcUI->listWidgetBaseTile->setEnabled( true );
    }
  }
}


void NewMap::on_spinBoxPosX_valueChanged( int32_t i_uiPosX )
{
  // Todo - verify destination map can take the set value?
}


void NewMap::on_spinBoxPosY_valueChanged( int32_t i_uiPosY )
{
  // Todo - verify destination map can take the set value?
}


void NewMap::on_spinBoxRows_valueChanged( int32_t i_uiRows )
{
  if( IsEditing() )
  {
    const uint32_t uiCols{ (uint32_t)m_pcUI->spinBoxCols->value() };
    if( uiCols <= m_uiCols && i_uiRows <= (int32_t)m_uiRows )
    {
      m_pcUI->listWidgetBaseTile->setEnabled( false );
      m_pcUI->labelBaseTile_Error->setHidden( true );
    }
    else
    {
      m_pcUI->listWidgetBaseTile->setEnabled( true );
    }
  }
}


void NewMap::SetBorderTile( rumAssetID i_eTileID )
{
  m_pcUI->listWidgetBorderTile->setCurrentRow( 0 );

  // Find the matching tile by walking the list and looking at the UserRole data
  for( int32_t i = 0; i < m_pcUI->listWidgetBorderTile->count(); ++i )
  {
    QListWidgetItem* pcItem{ m_pcUI->listWidgetBorderTile->item( i ) };
    if( pcItem )
    {
      const auto cVariant{ pcItem->data( Qt::UserRole ) };
      if( cVariant.isValid() )
      {
        const rumAssetID eTileID{ (rumAssetID)cVariant.toInt() };
        if( eTileID == i_eTileID )
        {
          m_pcUI->listWidgetBorderTile->setCurrentItem( pcItem );
          break;
        }
      }
    }
  }
}


void NewMap::SetExitPosX( uint32_t i_uiPosX )
{
  m_uiExitPosX = i_uiPosX;
  m_pcUI->spinBoxPosX->setValue( i_uiPosX );
}


void NewMap::SetExitPosY( uint32_t i_uiPosY )
{
  m_uiExitPosY = i_uiPosY;
  m_pcUI->spinBoxPosY->setValue( i_uiPosY );
}


void NewMap::SetNumColumns( uint32_t i_uiCols )
{
  m_uiCols = i_uiCols;
  m_pcUI->spinBoxCols->setValue( i_uiCols );
}


void NewMap::SetNumRows( uint32_t i_uiRows )
{
  m_uiRows = i_uiRows;
  m_pcUI->spinBoxRows->setValue( i_uiRows );
}


bool NewMap::ValidateInput()
{
  bool bValid{ true };

  if( IsAdding() )
  {
    m_strAbsoluteFilePath.clear();

    // Get the existing file path
    QString strFilePath{ m_pcUI->lineEditFile->text() };
    strFilePath = QDir::fromNativeSeparators( strFilePath );
    const QDir cDirFilePath( strFilePath );

    // Determine the standard path to game maps
    const QDir cDirMapPath{ MainWindow::GetProjectMapDir() };
    const QString strMapsPath{ QDir::fromNativeSeparators( cDirMapPath.canonicalPath() ) };

    // Convert the existing file path to absolute
    QString strAbsFilePath;
    if( cDirFilePath.isRelative() )
    {
      strAbsFilePath = cDirMapPath.absoluteFilePath( strFilePath );
      strAbsFilePath = QDir::cleanPath( strAbsFilePath );
    }
    else
    {
      strAbsFilePath = strFilePath;
    }

    // Make sure file is provided and that it is under the project maps subfolder
    if( strFilePath.isEmpty() )
    {
      bValid = false;
      m_pcUI->labelFile_Error->setText( "Filename required" );
      m_pcUI->labelFile_Error->setHidden( false );
    }
    else if( !strAbsFilePath.startsWith( strMapsPath ) )
    {
      bValid = false;
      m_pcUI->labelFile_Error->setText( "Invalid Path or Filename" );
      m_pcUI->labelFile_Error->setHidden( false );
    }
    else if( !strAbsFilePath.endsWith( ".map" ) )
    {
      bValid = false;
      m_pcUI->labelFile_Error->setText( "Filename extension must be \".map\"" );
      m_pcUI->labelFile_Error->setHidden( false );
    }
    else
    {
      // Cache the final absolute file path to the new map
      m_strAbsoluteFilePath = strAbsFilePath;
    }

    if( !m_pcUI->listWidgetBaseTile->isEnabled() || ( m_pcUI->listWidgetBaseTile->currentItem() == nullptr ) )
    {
      bValid = false;
      m_pcUI->labelBaseTile_Error->setText( "Selection required" );
      m_pcUI->labelBaseTile_Error->setHidden( false );
    }

    if( !m_pcUI->listWidgetBorderTile->isEnabled() || ( m_pcUI->listWidgetBorderTile->currentItem() == nullptr ) )
    {
      bValid = false;
      m_pcUI->labelBorderTile_Error->setText( "Selection required" );
      m_pcUI->labelBorderTile_Error->setHidden( false );
    }
  }
  else
  {
    if( m_pcUI->listWidgetBaseTile->isEnabled() && ( m_pcUI->listWidgetBaseTile->currentItem() == nullptr ) )
    {
      bValid = false;
      m_pcUI->labelBaseTile_Error->setText( "Selection required" );
      m_pcUI->labelBaseTile_Error->setHidden( false );
    }
  }

  return bValid;
}
