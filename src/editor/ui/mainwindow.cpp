#include <mainwindow.h>
#include <ui_mainwindow.h>

#include <e_graphics_opengl.h>
#include <e_map.h>
#include <e_pawn.h>

#include <u_broadcast.h>
#include <u_broadcast_asset.h>
#include <u_creature_asset.h>
#include <u_custom_asset.h>
#include <u_datatable.h>
#include <u_enum.h>
#include <u_graphic_asset.h>
#include <u_inventory.h>
#include <u_inventory_asset.h>
#include <u_language.h>
#include <u_map_asset.h>
#include <u_portal_asset.h>
#include <u_resource.h>
#include <u_scheduler.h>
#include <u_sound_asset.h>
#include <u_strings.h>
#include <u_tile_asset.h>
#include <u_widget_asset.h>

#include <assetmanager.h>
#include <datatablemanager.h>
#include <mapeditor.h>
#include <newproject.h>
#include <output.h>
#include <packagemanager.h>
#include <patchmanager.h>
#include <propertymanager.h>
#include <releasemanager.h>
#include <scriptdialog.h>
#include <scripteditor.h>
#include <sharedglwidget.h>
#include <smTabWidget.h>
#include <smTreeWidget.h>
#include <stringmanager.h>

#include <QCloseEvent>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QTableWidget>
#include <QTableWidgetItem>

MainWindow* MainWindow::m_pcMainWindow{ nullptr };
Sqrat::Object MainWindow::m_sqScriptDialogResult;
rumPropertyContainer::PropertyContainer MainWindow::s_cPropertyClipboard;

QString MainWindow::s_strAudioPathHint;
QString MainWindow::s_strGraphicsPathHint;
QString MainWindow::s_strTitle;
QString MainWindow::s_strUUID;


MainWindow::MainWindow( rumConfig& i_rcConfig, QWidget* i_pcParent )
  : QMainWindow( i_pcParent )
  , m_pcUI( new Ui::MainWindow )
  , m_iSchedulerTimer( -1 )
  , m_rcConfig( i_rcConfig )
  , m_bProjectOpen( false )
{
  m_pcMainWindow = this;

  m_pcUI->setupUi( this );
  setCentralWidget( m_pcUI->splitter );

  setMinimumWidth( 1600 );
  setMinimumHeight( 1200 );

  m_pcUI->treeWidget->setDisabled( true );
  m_pcUI->treeWidget->setMinimumWidth( 200 );
  m_pcUI->treeWidget->setHeaderLabel( "Project" );
  m_pcUI->tabWidget->setContextMenuPolicy( Qt::CustomContextMenu );

  m_pcUI->tabWidget->setDisabled( true );
  m_pcUI->tabWidget->setMinimumWidth( 200 );
  m_pcUI->tabWidget->setTabsClosable( true );
  m_pcUI->tabWidget->clear();
  m_pcUI->tabWidget->setContextMenuPolicy( Qt::CustomContextMenu );

  setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

  // Set icons and disable toolbar actions
  m_pcUI->actionCloseProject->setEnabled( false );

  const QIcon cAssetIcon( ":/ui/resources/asset.png" );
  m_pcUI->actionAsset_Manager->setIcon( cAssetIcon );
  m_pcUI->actionAsset_Manager->setEnabled( false );

  const QIcon cDataTableIcon( ":/ui/resources/datatable.png" );
  m_pcUI->actionDataTable_Manager->setIcon( cDataTableIcon );
  m_pcUI->actionDataTable_Manager->setEnabled( false );

  const QIcon cOpenIcon( ":/ui/resources/open.png" );
  m_pcUI->actionOpenProject->setIcon( cOpenIcon );

  const QIcon cPackageIcon( ":/ui/resources/package.png" );
  m_pcUI->actionPackage_Manager->setIcon( cPackageIcon );
  m_pcUI->actionPackage_Manager->setEnabled( false );

  const QIcon cPatchIcon( ":/ui/resources/patch.png" );
  m_pcUI->actionPatch_Manager->setIcon( cPatchIcon );
  m_pcUI->actionPatch_Manager->setEnabled( false );

  const QIcon cPropertyIcon( ":/ui/resources/property.png" );
  m_pcUI->actionProperty_Manager->setIcon( cPropertyIcon );
  m_pcUI->actionProperty_Manager->setEnabled( false );

  const QIcon cReleaseIcon( ":/ui/resources/release.png" );
  m_pcUI->actionRelease_Manager->setIcon( cReleaseIcon );
  m_pcUI->actionRelease_Manager->setEnabled( false );

  m_pcUI->actionResaveAllMaps->setEnabled( false );

  const QIcon cSettingsIcon( ":/ui/resources/settings.png" );
  m_pcUI->actionProject_Settings->setIcon( cSettingsIcon );
  m_pcUI->actionProject_Settings->setEnabled( false );

  const QIcon cStringIcon( ":/ui/resources/string.png" );
  m_pcUI->actionString_Manager->setIcon( cStringIcon );
  m_pcUI->actionString_Manager->setEnabled( false );

  // Set up the status bar
  m_pcStatusCursorLabel = new QLabel();
  m_pcStatusCursorLabel->setAlignment( Qt::AlignRight );
  m_pcUI->statusBar->addWidget( m_pcStatusCursorLabel );

  // Set the tree view to occupy 200 pixels
  QList<int32_t> cSplitterList;
  cSplitterList.append( 200 );
  cSplitterList.append( 600 );
  m_pcUI->splitter->setSizes( cSplitterList );

  m_pcSharedGLWidget = new SharedGLWidget( this );
  m_pcUI->verticalLayout->addWidget( m_pcSharedGLWidget );

  m_pcOutputDialog = new OutputDialog( this );
  m_pcOutputDialog->setModal( false );
}


MainWindow::~MainWindow()
{
  m_pcMainWindow = nullptr;

  delete m_pcOutputDialog;

  delete m_pcStatusCursorLabel;
  delete m_pcUI;
}


QWidget* MainWindow::AddOrOpenTab( const QTreeWidgetItem* i_pcItem )
{
  if( i_pcItem )
  {
    // Column 1 of each tree widget item stores the matching tab info
    return AddOrOpenTab( i_pcItem->text( COL_FILE_PATH ), i_pcItem->type() );
  }

  return nullptr;
}


QWidget* MainWindow::AddOrOpenTab( const QString& i_strFile, int32_t i_eItemType, const QString& i_strText )
{
  QWidget* pcWidget{ nullptr };

  int32_t iIndex{ -1 };
  if( !i_strFile.isEmpty() )
  {
    // Get the index of the tab if it already exists
    iIndex = m_pcUI->tabWidget->GetTabIndex( i_strFile );
    if( iIndex == -1 )
    {
      // A tab for this item does not appear to be opened, determine the item
      // type and open a new tab for the item
      if( TYPE_SCRIPT == i_eItemType )
      {
        ScriptEditor* pcEditor{ new ScriptEditor( i_strFile, m_pcUI->tabWidget ) };
        pcWidget = pcEditor;
      }
      else if( TYPE_MAP == i_eItemType )
      {
        MapEditor* pcMapEditor{ new MapEditor( i_strFile, m_pcUI->tabWidget ) };
        if( !pcMapEditor->FailedLoad() )
        {
          pcWidget = pcMapEditor;
        }
      }

      if( pcWidget )
      {
        // Tab doesn't already exist, add it
        iIndex = m_pcUI->tabWidget->addTab( pcWidget, i_strFile );
      }
    }
    else
    {
      //qDebug() << "File already opened at tab index " << index ;
      pcWidget = m_pcUI->tabWidget->widget( iIndex );
    }
  }
  else
  {
    ScriptEditor* pcEditor{ new ScriptEditor( i_strText, m_pcUI->tabWidget ) };
    pcWidget = pcEditor;

    const QString strTemp{ QDir::tempPath() + "/temp_script.nut" };
    iIndex = m_pcUI->tabWidget->addTab( pcWidget, strTemp );

    // Since this is a temp file, mark this document as dirty
    pcEditor->ForceModification();
  }

  if( iIndex != -1 )
  {
    // Switch to the tab
    m_pcUI->tabWidget->setCurrentIndex( iIndex );
  }

  return pcWidget;
}


// static
uint32_t MainWindow::FindAssetReferences( const rumAsset& i_rcAsset, bool i_bLog )
{
  MainWindow* pcWindow{ MainWindow::GetMainWindow() };
  rumAssert( pcWindow != nullptr );

  if( i_bLog )
  {
    pcWindow->ShowOutputDialog();
    pcWindow->ShowOnOutputDialog( QString( "Finding references for asset: " ) + i_rcAsset.GetName().c_str() + " " +
                                  std::string( i_rcAsset.GetTypeName() ).c_str() );
  }

  uint32_t uiNumRefs{ 0 };

  // TODO - if a map is already loaded, use that data instead

  const auto eAssetID{ i_rcAsset.GetAssetID() };

  // Determine the asset type
  switch( i_rcAsset.GetAssetType() )
  {
    case Creature_AssetType:
    case Portal_AssetType:
    case Widget_AssetType:
    case Broadcast_AssetType:
    case Tile_AssetType:
    case Map_AssetType:
    case Sound_AssetType:
    case Property_AssetType:
    case Inventory_AssetType:
    case Custom_AssetType:
    {
      uiNumRefs += FindPropertyReferences( i_rcAsset, i_bLog );
      uiNumRefs += FindMapReferences( i_rcAsset, i_bLog);
      uiNumRefs += FindScriptReferences( i_rcAsset, i_bLog );
    }
    break;

    case Graphic_AssetType:
    {
      uiNumRefs += FindGraphicReferences( i_rcAsset, i_bLog );
      uiNumRefs += FindPropertyReferences( i_rcAsset, i_bLog );
      uiNumRefs += FindMapReferences( i_rcAsset, i_bLog );
      uiNumRefs += FindScriptReferences( i_rcAsset, i_bLog );
    }
    break;

    default:
      rumAssertArgs( false, "Unhandled type: ", rumStringUtils::ToHexString( i_rcAsset.GetAssetType() ) );
      break;
  }

  QString strOutput{ "Find references complete, found " };
  strOutput += rumStringUtils::ToString( uiNumRefs );
  strOutput += " references";

  SetStatusBarText( strOutput );
  
  if( i_bLog )
  {
    pcWindow->ShowOnOutputDialog( strOutput );
  }

  return uiNumRefs;
}


// static
uint32_t MainWindow::FindGraphicReferences( const rumAsset& i_rcAsset, bool i_bLog )
{
  uint32_t uiNumRefs{ 0 };

  uiNumRefs += FindGraphicReferences<rumCreatureAsset>( i_rcAsset, i_bLog );
  uiNumRefs += FindGraphicReferences<rumPortalAsset>( i_rcAsset, i_bLog );
  uiNumRefs += FindGraphicReferences<rumWidgetAsset>( i_rcAsset, i_bLog );
  uiNumRefs += FindGraphicReferences<rumTileAsset>( i_rcAsset, i_bLog );

  return uiNumRefs;
}


// static
uint32_t MainWindow::FindMapReferences( const rumAsset& i_rcAsset, bool i_bLog )
{
  MainWindow* pcWindow{ MainWindow::GetMainWindow() };
  rumAssert( pcWindow != nullptr );

  if( i_bLog )
  {
    pcWindow->ShowOutputDialog();
  }

  uint32_t uiNumRefs{ 0 };

  for( const auto& iter : rumMapAsset::GetAssetHash() )
  {
    const rumMapAsset* pcMap{ iter.second };
    Q_ASSERT( pcMap );
    if( !pcMap )
    {
      continue;
    }

    std::string strPath;
    const std::string& strFile{ pcMap->GetFilename() };

    const QString strStatus( "Loading " );
    SetStatusBarText( strStatus + strFile.c_str() + "... " );

    // Allow Qt to update
    QApplication::processEvents();

    if( !rumResource::FileExists( strFile ) )
    {
      if( !rumResource::FindFile( strFile, strPath ) )
      {
        continue;
      }
    }
    else
    {
      strPath = strFile;
    }

    Sqrat::Object sqObject{ rumGameObject::Create( pcMap->GetAssetID() ) };
    if( sqObject.GetType() == OT_INSTANCE )
    {
      EditorMap* pcEditorMap{ sqObject.Cast<EditorMap*>() };
      pcEditorMap->Load();

      // Tiles get special search behavior
      if( i_rcAsset.GetAssetType() == Tile_AssetType )
      {
        if( pcEditorMap->GetBorderTile() == i_rcAsset.GetAssetID() )
        {
          if( i_bLog )
          {
            pcWindow->ShowOnOutputDialog( QString( "Found: Map " ) + pcEditorMap->GetName().c_str() + " border tile" );
          }

          ++uiNumRefs;
        }

        // Visit each map cell and look for a tile match
        for( uint32_t iCol{ 0 }; iCol < pcEditorMap->GetCols(); ++iCol )
        {
          for( uint32_t iRow{ 0 }; iRow < pcEditorMap->GetRows(); ++iRow )
          {
            const auto& rcData{ pcEditorMap->GetPositionData( {static_cast<int32_t>( iCol ),
                                                               static_cast<int32_t>( iRow )} ) };
            if( rcData->m_eTileID == i_rcAsset.GetAssetID() )
            {
              if( i_bLog )
              {
                QString strOutput{ QString( "Found: Map " ) + pcEditorMap->GetName().c_str() + " tile pos " };
                strOutput += rumStringUtils::ToString( iCol );
                strOutput += ", ";
                strOutput += rumStringUtils::ToString( iRow );

                pcWindow->ShowOnOutputDialog( strOutput );
              }

              ++uiNumRefs;
            }
          }
        }
      }

      // Search each pawn on the map
      for( const auto uiPawnID : pcEditorMap->GetPawnHash() )
      {
        const auto pcPawn{ rumPawn::Fetch( uiPawnID ) };
        rumAssert( pcPawn != nullptr );
        if( pcPawn == nullptr )
        {
          continue;
        }

        // Check the pawn itself for a match
        if( pcPawn->GetAssetID() == i_rcAsset.GetAssetID() )
        {
          if( i_bLog )
          {
            QString strOutput{ QString( "Found: Map " ) + pcEditorMap->GetName().c_str() };
            strOutput += " ";
            strOutput += pcPawn->GetName().c_str();
            strOutput += " ";
            strOutput += std::string( i_rcAsset.GetTypeName() ).c_str();
            strOutput += " pos ";
            strOutput += rumStringUtils::ToString( pcPawn->GetPosX() );
            strOutput += ", ";
            strOutput += rumStringUtils::ToString( pcPawn->GetPosY() );

            pcWindow->ShowOnOutputDialog( strOutput );
          }

          ++uiNumRefs;
        }

        // Check the pawn's properties for a match
        const auto& rcProperties{ pcPawn->GetInstanceProperties() };
        uint32_t uiNumPropertyRefs{ FindPropertyReferences( rcProperties, *pcPawn->GetAsset(), i_rcAsset, i_bLog ) };
        if( uiNumPropertyRefs > 0 )
        {
          uiNumRefs += uiNumPropertyRefs;

          QString strOutput{ QString( "  on map " ) + pcEditorMap->GetName().c_str() };
          strOutput += " pos ";
          strOutput += rumStringUtils::ToString( pcPawn->GetPosX() );
          strOutput += ", ";
          strOutput += rumStringUtils::ToString( pcPawn->GetPosY() );

          pcWindow->ShowOnOutputDialog( strOutput );
        }
      }
    }
  }

  return uiNumRefs;
}


// static
uint32_t MainWindow::FindPropertyReferences( const rumAsset& i_rcAsset, bool i_bLog )
{
  uint32_t uiNumRefs{ 0 };

  uiNumRefs += FindPropertyReferences<rumCreatureAsset>( i_rcAsset, i_bLog );
  uiNumRefs += FindPropertyReferences<rumPortalAsset>( i_rcAsset, i_bLog );
  uiNumRefs += FindPropertyReferences<rumWidgetAsset>( i_rcAsset, i_bLog );
  uiNumRefs += FindPropertyReferences<rumBroadcastAsset>( i_rcAsset, i_bLog );
  uiNumRefs += FindPropertyReferences<rumTileAsset>( i_rcAsset, i_bLog );
  uiNumRefs += FindPropertyReferences<rumMapAsset>( i_rcAsset, i_bLog );
  uiNumRefs += FindPropertyReferences<rumGraphicAsset>( i_rcAsset, i_bLog );
  uiNumRefs += FindPropertyReferences<rumSoundAsset>( i_rcAsset, i_bLog );
  uiNumRefs += FindPropertyReferences<rumInventoryAsset>( i_rcAsset, i_bLog );
  uiNumRefs += FindPropertyReferences<rumCustomAsset>( i_rcAsset, i_bLog );

  return uiNumRefs;
}


// static
uint32_t MainWindow::FindPropertyReferences( const PropertyContainer& i_rcProperties,
                                             const rumAsset& i_rcCurrentAsset,
                                             const rumAsset& i_rcAssetToFind,
                                             bool i_bLog )
{
  MainWindow* pcWindow{ MainWindow::GetMainWindow() };
  rumAssert( pcWindow != nullptr );

  if( i_bLog )
  {
    pcWindow->ShowOutputDialog();
  }

  uint32_t uiNumRefs{ 0 };

  for( auto& iterProperty : i_rcProperties )
  {
    Sqrat::Object sqProperty{ iterProperty.second };
    if( sqProperty.GetType() == OT_NULL )
    {
      continue;
    }

    // Does the property itself match?
    if( ( i_rcAssetToFind.GetAssetType() == Property_AssetType ) &&
        ( iterProperty.first == i_rcAssetToFind.GetAssetID() ) )
    {
      if( i_bLog )
      {
        pcWindow->ShowOnOutputDialog( QString( "Found: Asset Property set on " ) + i_rcCurrentAsset.GetName().c_str() +
                                      " " + std::string( i_rcCurrentAsset.GetTypeName() ).c_str() );
      }

      ++uiNumRefs;
    }

    // Does the property's value match?
    if( sqProperty.Cast<rumAssetID>() == i_rcAssetToFind.GetAssetID() )
    {
      // Make sure this is actually an asset reference and not just a coincidence with a random value
      const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( iterProperty.first ) };
      rumAssert( pcProperty );
      if( pcProperty != nullptr &&
          ( ( pcProperty->GetValueType() == PropertyValueType::AssetRef ) ||
            ( pcProperty->GetValueType() >= PropertyValueType::FirstAssetRef &&
              pcProperty->GetValueType() <= PropertyValueType::LastAssetRef ) ) )
      {
        if( i_bLog )
        {
          QString strOutput{ QString( "Found: Asset Property Value set on " ) + i_rcCurrentAsset.GetName().c_str() };
          strOutput += " ";
          strOutput += std::string( i_rcCurrentAsset.GetTypeName() ).c_str();

          pcWindow->ShowOnOutputDialog( strOutput );
        }

        ++uiNumRefs;
      }
    }
  }

  return uiNumRefs;
}


// static
uint32_t MainWindow::FindScriptReferences( const rumAsset& i_rcAsset, bool i_bLog )
{
  MainWindow* pcWindow{ MainWindow::GetMainWindow() };
  rumAssert( pcWindow != nullptr );

  if( i_bLog )
  {
    pcWindow->ShowOutputDialog();
  }

  uint32_t uiNumRefs{ 0 };
  std::filesystem::path fsScriptPath( std::filesystem::path( GetProjectPath() ) / rumScript::GetFolderName() );

  QDir cScriptPath{ fsScriptPath.generic_string().c_str() };
  cScriptPath.setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks );
  cScriptPath.setNameFilters( QStringList() << "*.nut" );

  QDirIterator iter( cScriptPath, QDirIterator::Subdirectories );
  while( iter.hasNext() )
  {
    const QFileInfo cFileInfo{ iter.fileInfo() };
    if( cFileInfo.isFile() )
    {
      const QString strStatus( "Searching " );
      SetStatusBarText( strStatus + cFileInfo.fileName() + "..." );

      QFile cFile( cFileInfo.absoluteFilePath() );
      if( cFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
        QTextStream cTextStream( &cFile );
        const auto strContents{ cTextStream.readAll() };
        const auto strList{ strContents.split( '\n' ) };
        uint32_t uiLine{ 1 };
        for( const auto& strLine : strList )
        {
          auto strSearchText{ i_rcAsset.GetName() };
          strSearchText += i_rcAsset.GetTypeSuffix();
          if( strLine.contains( strSearchText.c_str() ) )
          {
            ++uiNumRefs;

            if( i_bLog )
            {
              QString strOutput{ "Found: Script " };
              strOutput += cFileInfo.absoluteFilePath();
              strOutput += " line ";
              strOutput += rumStringUtils::ToString( uiLine );

              pcWindow->ShowOnOutputDialog( strOutput );
            }
          }

          ++uiLine;
        }

        cFile.close();
      }

      // Allow Qt to update
      QApplication::processEvents();
    }

    iter.next();
  }

  return uiNumRefs;
}


QString MainWindow::GetFileHash( const QString& i_strFilePath )
{
  QFile cFile( i_strFilePath );
  if( cFile.open( QIODevice::ReadOnly ) )
  {
    const QString strHash{ QCryptographicHash::hash( cFile.readAll(), QCryptographicHash::Md5 ).toHex().constData() };
    cFile.close();
    return strHash;
  }

  return "";
}


int MainWindow::GetCurrentTabIndex()
{
  return m_pcUI->tabWidget->currentIndex();
}


// static
MainWindow* MainWindow::GetMainWindow( QObject* i_pcObject )
{
  QObject* pcObject{ i_pcObject };
  MainWindow* pcMainWindow{ nullptr };
  while( pcObject && !pcMainWindow )
  {
    pcMainWindow = qobject_cast<MainWindow*>( pcObject );
    pcObject = pcObject->parent();
  }

  return pcMainWindow;
}


// static
QDir MainWindow::GetProjectCSVDir()
{
  return QDir( GetProjectDir().canonicalPath() + "/csv" );
}


// static
QDir MainWindow::GetProjectDir()
{
  // Get the project path
  const MainWindow* pcMain{ GetMainWindow() };
  const rumConfig& rcConfigStruct{ pcMain->GetConfig() };
  return QDir( rcConfigStruct.m_strProjectPath.c_str() );
}


// static
QDir MainWindow::GetProjectAudioDir()
{
  return QDir( GetProjectDir().canonicalPath() + "/audio" );
}


// static
QDir MainWindow::GetProjectFontDir()
{
  return QDir( GetProjectDir().canonicalPath() + "/fonts" );
}


// static
QDir MainWindow::GetProjectGraphicDir()
{
  return QDir( GetProjectDir().canonicalPath() + "/graphics" );
}


// static
QDir MainWindow::GetProjectMapDir()
{
  return QDir( GetProjectDir().canonicalPath() + "/maps" );
}


// static
QDir MainWindow::GetProjectScriptDir()
{
  return QDir( GetProjectDir().canonicalPath() + "/scripts" );
}


void MainWindow::ProjectClose()
{
  if( !m_bProjectOpen )
  {
    return;
  }

  // Kill the timer that handles scheduler events
  killTimer( m_iSchedulerTimer );
  m_iSchedulerTimer = -1;

  // Kill the timer that handles misc animation events
  killTimer( m_iAnimTimer );
  m_iAnimTimer = -1;

  if( m_pcAssetManager )
  {
    SAFE_DELETE( m_pcAssetManager );
  }

  if( m_pcDataTableManager )
  {
    SAFE_DELETE( m_pcDataTableManager );
  }

  if( m_pcPackageManager )
  {
    m_pcPackageManager->Close();
    SAFE_DELETE( m_pcPackageManager );
  }

  if( m_pcPatchManager )
  {
    m_pcPatchManager->Close();
    SAFE_DELETE( m_pcPatchManager );
  }

  if( m_pcPropertyManager )
  {
    SAFE_DELETE( m_pcPropertyManager );
  }

  if( m_pcReleaseManager )
  {
    m_pcReleaseManager->Close();
    SAFE_DELETE( m_pcReleaseManager );
  }

  if( m_pcStringManager )
  {
    SAFE_DELETE( m_pcStringManager );
  }

  if( m_pcOutputDialog )
  {
    m_pcOutputDialog->close();
  }

  m_pcUI->tabWidget->Reset();
  m_pcUI->treeWidget->Shutdown();

  rumScheduler::Shutdown();

  EditorGraphic::Shutdown();
  rumPawn::Shutdown();
  rumInventory::Shutdown();
  rumBroadcast::Shutdown();
  EditorMap::Shutdown();

  rumAsset::Shutdown();

  rumDatabase::Shutdown();
  rumScript::Shutdown();

  m_pcUI->actionNewProject->setEnabled( true );
  m_pcUI->actionOpenProject->setEnabled( true );
  m_pcUI->actionCloseProject->setEnabled( false );

  m_pcUI->actionProject_Settings->setEnabled( false );

  m_pcUI->actionAsset_Manager->setEnabled( false );
  m_pcUI->actionDataTable_Manager->setEnabled( false );
  m_pcUI->actionPackage_Manager->setEnabled( false );
  m_pcUI->actionPatch_Manager->setEnabled( false );
  m_pcUI->actionProperty_Manager->setEnabled( false );
  m_pcUI->actionRelease_Manager->setEnabled( false );
  m_pcUI->actionResaveAllMaps->setEnabled( false );
  m_pcUI->actionString_Manager->setEnabled( false );

  m_pcUI->treeWidget->setDisabled( true );
  m_pcUI->tabWidget->setDisabled( true );

  m_bProjectOpen = false;
}


void MainWindow::ProjectOpen( const QString& i_strFilePath )
{
  ProjectClose();

  if( i_strFilePath.isEmpty() )
  {
    return;
  }
  else if( !QFile::exists( i_strFilePath ) )
  {
    QMessageBox::critical( this, "Open Project", "File " + i_strFilePath + " does not exist" );
    return;
  }

  SetProjectPath( qPrintable( i_strFilePath ) );

  // Cache the project file structure
  rumResource::CreateFileCache( GetProjectPath() );

  if( InitDatabase() != RESULT_SUCCESS )
  {
    QMessageBox::critical( this, "Open Project", "Failed to initialize databases. Cannot open project." );
    ProjectClose();
    return;
  }

  // Promote file to last opened in the registry
  RemoveRecentFiles();
  UpdateRecentFilesArray( i_strFilePath );
  UpdateRecentFiles();

  // Grab all project settings
  // A local user db would be best for preferences
  const std::string strQuery{ "SELECT key,value FROM settings" };
  const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::Game_DatabaseID, strQuery ) };
  if( pcQuery && !pcQuery->IsError() )
  {
    enum{ COL_KEY, COL_VALUE };

    for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
    {
      const std::string strKey{ pcQuery->FetchString( i, COL_KEY ) };
      const std::string strValue{ pcQuery->FetchString( i, COL_VALUE ) };

      if( strKey.compare( "title" ) == 0 )
      {
        SetProjectTitle( strValue.c_str() );
      }
      else if( strKey.compare( "uuid" ) == 0 )
      {
        SetProjectUUID( strValue.c_str() );
      }
      else if( strKey.compare( "audio" ) == 0 )
      {
        SetProjectAudioPathHint( strValue.c_str() );
      }
      else if( strKey.compare( "graphics" ) == 0 )
      {
        SetProjectGraphicsPathHint( strValue.c_str() );
      }
    }
  }

  rumPatcher::Init( GetProjectPath() );

  // Initialize the server script virtual machine
  std::filesystem::path fsScriptPath( std::filesystem::path( GetProjectPath() ) / rumScript::GetFolderName() );
  if( rumScript::Init( rumScript::VM_EDITOR, fsScriptPath.generic_string() ) != RESULT_SUCCESS )
  {
    QMessageBox::critical( this, "Open Project", "A fatal error occurred during script initialization." );
    ProjectClose();
    return;
  }

  if( rumAsset::Init( GetProjectPath() ) != RESULT_SUCCESS )
  {
    QMessageBox::critical( this, "Open Project", "Failed to initialize assets." );
    ProjectClose();
    return;
  }

  if( rumStringTable::Init( GetProjectPath(), rumStringUtils::NullString() ) != RESULT_SUCCESS )
  {
    QMessageBox::critical( this, "Open Project", "Failed to initialize string tables." );
    ProjectClose();
    return;
  }

  if( rumDataTable::Init( GetProjectPath() ) != RESULT_SUCCESS )
  {
    QMessageBox::critical( this, "Open Project", "Failed to initialize data tables." );
    ProjectClose();
    return;
  }

  BindScripts();

  if( rumScript::ExecuteStartupScript( rumScript::VM_EDITOR ) != RESULT_SUCCESS )
  {
    QMessageBox::critical( this, "Open Project", "A fatal error occurred during script execution." );
    ProjectClose();
    return;
  }

  rumAsset::RegisterClasses();

  EditorMap::Init();
  EditorPawn::Init();
  EditorGraphic::Init();

  if( !rumScript::CallInitFunction( "OnGameInit" ) )
  {
    QMessageBox::critical( this, "Open Project", "A fatal error occurred during initial script startup." );
    ProjectClose();
    return;
  }

  m_pcUI->tabWidget->setDisabled( false );

  m_pcUI->treeWidget->setDisabled( false );
  m_pcUI->treeWidget->Init();

  // Enable toolbar actions
  m_pcUI->actionProject_Settings->setEnabled( true );
  m_pcUI->actionResaveAllMaps->setEnabled( true );

  m_pcUI->actionAsset_Manager->setEnabled( true );
  m_pcUI->actionDataTable_Manager->setEnabled( true );
  m_pcUI->actionPackage_Manager->setEnabled( true );
  m_pcUI->actionPatch_Manager->setEnabled( true );
  m_pcUI->actionProperty_Manager->setEnabled( true );
  m_pcUI->actionRelease_Manager->setEnabled( true );
  m_pcUI->actionString_Manager->setEnabled( true );

  // Start a timer for invoking scheduler events
  m_iSchedulerTimer = startTimer( 50 );

  // Start a timer for animating misc items
  m_iAnimTimer = startTimer( 50 );

  m_pcUI->actionNewProject->setEnabled( false );
  m_pcUI->actionOpenProject->setEnabled( false );
  m_pcUI->actionCloseProject->setEnabled( true );

  m_bProjectOpen = true;
}


void MainWindow::onAssetManagerClosed()
{
  // Re-enable all related action/buttons
  m_pcUI->actionAsset_Manager->setEnabled( true );
}


void MainWindow::onDataTableManagerClosed()
{
  // Re-enable all related action/buttons
  m_pcUI->actionDataTable_Manager->setEnabled( true );
}


void MainWindow::onPackageManagerClosed()
{
  // Re-enable all related action/buttons
  m_pcUI->actionPackage_Manager->setEnabled( true );
}


void MainWindow::onPatchManagerClosed()
{
  // Re-enable all related action/buttons
  m_pcUI->actionPatch_Manager->setEnabled( true );
}


void MainWindow::onPropertyManagerClosed()
{
  // Re-enable all related action/buttons
  m_pcUI->actionProperty_Manager->setEnabled( true );
}


void MainWindow::onReleaseManagerClosed()
{
  // Re-enable all related action/buttons
  m_pcUI->actionRelease_Manager->setEnabled( true );
}


void MainWindow::onProjectSettingsClosed( int32_t i_eResult )
{
  // Re-enable all related action/buttons
  m_pcUI->actionProject_Settings->setEnabled( true );
}


void MainWindow::onStringManagerClosed()
{
  // Re-enable all related action/buttons
  m_pcUI->actionString_Manager->setEnabled( true );

  //disconnect( m_pcStringManager, SIGNAL( closed() ), this, SLOT( onStringManagerClosed() ) );
  //SAFE_DELETE( m_pcStringManager );
}


void MainWindow::Init( int32_t i_iArgc, char* i_pcArgv[] )
{
  RUM_COUT( "Using Qt version" << qVersion() << "\n" );

  QCoreApplication::setOrganizationName( "ShatteredMoon" );
  QCoreApplication::setOrganizationDomain( "shatteredmoon.com" );
  QCoreApplication::setApplicationName( "RUM" );

  InitRecentFilesArray();
  UpdateRecentFiles();
}


void MainWindow::InitRecentFilesArray()
{
  QSettings cSettings;

  for( int32_t i = 0; i < s_iMaxRecentFiles; ++i )
  {
    const QString strRegistryEntry{ QString( "MainWindow/RecentFile/%1" ).arg( i + 1 ) };
    const QString strRecentFile{ cSettings.value( strRegistryEntry ).toString() };
    if( !strRecentFile.isEmpty() )
    {
      // Only proceed if the file exists and is not already in the recent file list
      const QFileInfo cFileInfo( strRecentFile );
      if( cFileInfo.exists() )
      {
        const auto iter{ std::find_if( m_cRecentActionsVector.begin(), m_cRecentActionsVector.end(),
                                       [&strRecentFile]( const auto& i_rcEntry )
                                       {
                                         return i_rcEntry->text().compare( strRecentFile ) == 0;
                                       } ) };

        if( iter == m_cRecentActionsVector.end() )
        {
          // Create an action for the entry
          QAction* pcAction{ new QAction( this ) };
          pcAction->setText( strRecentFile );
          pcAction->setVisible( true );

          connect( pcAction, SIGNAL( triggered() ),
                   this, SLOT( OnOpenRecentFile() ) );

          m_cRecentActionsVector.push_back( pcAction );
        }
      }
    }
  }
}


// virtual
void MainWindow::closeEvent( QCloseEvent* i_pcEvent )
{
  ProjectClose();

  ::Shutdown();

  // Release the recent file actions
  for( auto& iter : m_cRecentActionsVector )
  {
    SAFE_DELETE( iter );
  }

  m_cRecentActionsVector.clear();

  i_pcEvent->accept();
}


void MainWindow::RefreshTreeWidget()
{
  m_pcUI->treeWidget->Refresh();
}


// static
Sqrat::Object MainWindow::ScriptModalDialog( Sqrat::Array i_sqArray )
{
  // Set the dialog results to null in the event of an error
  m_sqScriptDialogResult = Sqrat::Object();

  if( i_sqArray.GetType() == OT_ARRAY )
  {
    ScriptDialog* pcDialog{ new ScriptDialog( i_sqArray, nullptr ) };
    pcDialog->setModal( true );
    pcDialog->exec();
  }
  else
  {
    QMessageBox::critical( nullptr, tr( "Error" ), "Dialog data must be in the form of an array" );
  }

  return m_sqScriptDialogResult;
}


void MainWindow::ShowOnOutputDialog( const QString& i_strOutput ) const
{
  if( m_pcOutputDialog != nullptr )
  {
    m_pcOutputDialog->Append( i_strOutput );
  }
}


void MainWindow::ShowOutputDialog() const
{
  if( m_pcOutputDialog != nullptr )
  {
    m_pcOutputDialog->show();
  }
}


// static
void MainWindow::ShowUsage()
{
  QMessageBox::information( nullptr,
                            "Usage",
                            "rum_editor<br>"
                            "-log &lt;file&gt; : Output program info to specified file<br>"
                            "-project : Path to project folder containing game.rum<br>"
                            "-help/-h/-? : Access this dialog" );
}


void MainWindow::on_actionCloseProject_triggered()
{
  ProjectClose();
}


void MainWindow::on_actionAsset_Manager_triggered()
{
  if( nullptr == m_pcAssetManager )
  {
    m_pcAssetManager = new AssetManager( this );

    connect( m_pcAssetManager, SIGNAL( closed() ),
             this, SLOT( onAssetManagerClosed() ) );
  }

  m_pcUI->actionAsset_Manager->setEnabled( false );
  m_pcAssetManager->show();
}


void MainWindow::on_actionDataTable_Manager_triggered()
{
  if( nullptr == m_pcDataTableManager )
  {
    m_pcDataTableManager = new DataTableManager( this );

    connect( m_pcDataTableManager, SIGNAL( closed() ),
             this, SLOT( onDataTableManagerClosed() ) );
  }

  m_pcUI->actionDataTable_Manager->setEnabled( false );
  m_pcDataTableManager->show();
}


void MainWindow::on_actionNewProject_triggered()
{
  NewProject* pcNewProject{ new NewProject( false, this ) };
  pcNewProject->setModal( true );
  pcNewProject->show();
}


void MainWindow::on_actionOpenProject_triggered()
{
  const QString strFile{ QFileDialog::getOpenFileName( this, "Open RUM Project", QString(), "*.rum" ) };
  ProjectOpen( strFile );
}


void MainWindow::on_actionPackage_Manager_triggered()
{
  if( nullptr == m_pcPackageManager )
  {
    m_pcPackageManager = new PackageManager( this );

    connect( m_pcPackageManager, SIGNAL( closed() ), this, SLOT( onPackageManagerClosed() ) );
  }

  m_pcUI->actionPackage_Manager->setEnabled( false );
  m_pcPackageManager->show();
}


void MainWindow::on_actionPatch_Manager_triggered()
{
  if( nullptr == m_pcPatchManager )
  {
    m_pcPatchManager = new PatchManager( this );

    connect( m_pcPatchManager, SIGNAL( closed() ), this, SLOT( onPatchManagerClosed() ) );
  }

  m_pcUI->actionPatch_Manager->setEnabled( false );
  m_pcPatchManager->show();
}


void MainWindow::on_actionRelease_Manager_triggered()
{
  if( nullptr == m_pcReleaseManager )
  {
    m_pcReleaseManager = new ReleaseManager( this );

    connect( m_pcReleaseManager, SIGNAL( closed() ), this, SLOT( onReleaseManagerClosed() ) );
  }

  m_pcUI->actionRelease_Manager->setEnabled( false );
  m_pcReleaseManager->show();
}


void MainWindow::on_actionProject_Settings_triggered()
{
  // Create a new project dialog in edit mode
  NewProject* pcNewProject{ new NewProject( true, this ) };

  connect( pcNewProject, SIGNAL( finished( int32_t ) ), this, SLOT( onProjectSettingsClosed( int32_t ) ) );

  m_pcUI->actionProject_Settings->setEnabled( false );
  pcNewProject->setModal( true );
  pcNewProject->show();
}


void MainWindow::on_actionProperty_Manager_triggered()
{
  if( nullptr == m_pcPropertyManager )
  {
    m_pcPropertyManager = new PropertyManager( this );

    connect( m_pcPropertyManager, SIGNAL( closed() ), this, SLOT( onPropertyManagerClosed() ) );
  }

  m_pcUI->actionProperty_Manager->setEnabled( false );
  m_pcPropertyManager->show();
}


void MainWindow::on_actionResaveAllMaps_triggered()
{
  // TODO - do not proceed if any map files are opened

  if( QMessageBox::Cancel == QMessageBox::question( this, "Resave All Maps", "Are you sure?",
                                                    QMessageBox::Ok | QMessageBox::Cancel,
                                                    QMessageBox::Cancel ) )
  {
    return;
  }

  int32_t iResavedCount{ 0 };

  for( const auto& iter : rumMapAsset::GetAssetHash() )
  {
    const rumMapAsset* pcAsset{ iter.second };
    Q_ASSERT( pcAsset );
    if( !pcAsset )
    {
      continue;
    }

    std::string strPath;
    const std::string& strFile{ pcAsset->GetFilename() };

    const QString strStatus( "Resaving " );
    SetStatusBarText( strStatus + strFile.c_str() + "... " );

    // Allow Qt to update
    QApplication::processEvents();

    if( !rumResource::FileExists( strFile ) )
    {
      if( !rumResource::FindFile( strFile, strPath ) )
      {
        continue;
      }
    }
    else
    {
      strPath = strFile;
    }

    Sqrat::Object sqObject{ rumGameObject::Create( pcAsset->GetAssetID() ) };
    if( sqObject.GetType() == OT_INSTANCE )
    {
      EditorMap* pcEditorMap{ sqObject.Cast<EditorMap*>() };
      if( pcEditorMap->Resave() == RESULT_SUCCESS )
      {
        ++iResavedCount;
      }
    }
  }

  SetStatusBarText( "Resave all maps complete " );
}


void MainWindow::on_actionString_Manager_triggered()
{
  if( nullptr == m_pcStringManager )
  {
    m_pcStringManager = new StringManager( this );

    connect( m_pcStringManager, SIGNAL( closed() ), this, SLOT( onStringManagerClosed() ) );
  }

  m_pcUI->actionString_Manager->setEnabled( false );
  m_pcStringManager->show();
}


void MainWindow::on_tabWidget_tabCloseRequested( int32_t i_iIndex )
{
  QWidget* pcWidget{ m_pcUI->tabWidget->widget( i_iIndex ) };
  Q_ASSERT( pcWidget );

  bool bClose{ true };

  const QString& strText{ m_pcUI->tabWidget->tabText( i_iIndex ) };
  if( strText.contains( ".map", Qt::CaseInsensitive ) )
  {
    MapEditor* pcMapEditor{ qobject_cast<MapEditor*>( pcWidget ) };
    if( pcMapEditor )
    {
      bClose = pcMapEditor->RequestClose();
    }
  }
  else if( strText.contains( ".nut", Qt::CaseInsensitive ) )
  {
    ScriptEditor* pcScriptEditor{ qobject_cast<ScriptEditor*>( pcWidget ) };
    if( pcScriptEditor )
    {
      bClose = pcScriptEditor->RequestClose();
    }
  }

  if( bClose )
  {
    m_pcUI->tabWidget->TabClose( i_iIndex );
  }
}


void MainWindow::on_treeWidget_itemDoubleClicked( QTreeWidgetItem* i_pcItem, int32_t i_iColumn )
{
  m_pcUI->treeWidget->Open( i_pcItem );
}


void MainWindow::on_tabWidget_customContextMenuRequested( const QPoint& i_rcPos )
{
  m_pcUI->tabWidget->DoContextMenu( i_rcPos );
}


void MainWindow::on_treeWidget_customContextMenuRequested( const QPoint& i_rcPos )
{
  m_pcUI->treeWidget->DoContextMenu( i_rcPos );
}


void MainWindow::OnOpenRecentFile()
{
  const QAction* pcAction{ qobject_cast<const QAction*>( sender() ) };
  if( pcAction )
  {
    ProjectOpen( pcAction->text() );
  }
}


void MainWindow::RemoveRecentFiles()
{
  const auto& rcMenus{ m_pcUI->menuBar->findChildren<QMenu*>() };
  for( auto& iterMenu : rcMenus )
  {
    const QString& strMenu{ iterMenu->objectName() };
    if( strMenu.compare( "menuFile" ) == 0 )
    {
      auto rcActions{ iterMenu->actions() };

      for( const auto& iterRemove : m_cRecentActionsVector )
      {
        if( iterRemove != nullptr )
        {
          rcActions.removeOne( iterRemove );
        }
      }
    }
  }
}


// static
void MainWindow::SetProjectAudioPathHint( const QString& i_strPath )
{
  s_strAudioPathHint = i_strPath;
  const std::filesystem::path cPath( std::filesystem::path( GetProjectPath() ) / qPrintable( s_strAudioPathHint ) );
  rumSoundAsset::SetPathHint( cPath.generic_string() );
  rumSoundAsset::RefreshFileData();
}


// static
void MainWindow::SetProjectGraphicsPathHint( const QString& i_strPath )
{
  s_strGraphicsPathHint = i_strPath;
  const std::filesystem::path cPath( std::filesystem::path( GetProjectPath() ) / qPrintable( s_strGraphicsPathHint ) );
  rumGraphicAsset::SetPathHint( cPath.generic_string() );
  rumGraphicAsset::RefreshFileData();
}


// static
void MainWindow::SetStatusBarText( const QString& i_strText )
{
  if( m_pcMainWindow != nullptr )
  {
    m_pcMainWindow->m_pcStatusCursorLabel->setText( i_strText );
  }
}


void MainWindow::SetTabIcon( int32_t i_iIndex, const QIcon& i_rcIcon )
{
  m_pcUI->tabWidget->setTabIcon( i_iIndex, i_rcIcon );
}


void MainWindow::SetTabInfo( int32_t i_iIndex, const QString& i_strFilePath )
{
  m_pcUI->tabWidget->UpdateTabInfo( i_iIndex, i_strFilePath );
}


void MainWindow::timerEvent( QTimerEvent* i_pcEvent )
{
  if( i_pcEvent->timerId() == m_iSchedulerTimer )
  {
    rumScheduler::Run();
  }
  else if( i_pcEvent->timerId() == m_iAnimTimer )
  {
    QWidget* pcWidget{ m_pcUI->tabWidget->currentWidget() };
    if( pcWidget )
    {
      MapEditor* pcMapEditor{ qobject_cast<MapEditor*>( pcWidget ) };
      if( pcMapEditor )
      {
        pcMapEditor->OnAnimTimer();
      }
    }
  }
}


void MainWindow::UpdateRecentFiles()
{
  static bool bFirstPass{ true };

  // Find where to add the recent file actions by iterating through the menu actions until we find "actionExit"
  const auto pcMenus{ m_pcUI->menuBar->findChildren<QMenu*>() };
  for( auto& iterMenu : pcMenus )
  {
    //RUM_COUT( "menu = " << qPrintable( iterMenu->objectName() ) << "\n" );
    const QString& strMenu{ iterMenu->objectName() };
    if( strMenu.compare( "menuFile" ) == 0 )
    {
      const auto pcActions{ iterMenu->actions() };

      // Add each action
      for( const auto& iterAction : pcActions )
      {
        const QString& strAction{ iterAction->objectName() };
        if( strAction.compare( "actionExit" ) == 0 )
        {
          for( const auto& iterAdd : m_cRecentActionsVector )
          {
            iterMenu->insertAction( iterAction, iterAdd );
          }

          //if( bFirstPass && m_vRecentFileActions.size() > 0 )
          {
            iterMenu->insertSeparator( iterAction );
            bFirstPass = false;
          }
        }
      }
    }
  }
}


void MainWindow::UpdateRecentFilesArray( const QString& i_strFilePath )
{
  const auto iter{ std::find_if( m_cRecentActionsVector.begin(), m_cRecentActionsVector.end(),
                                 [&i_strFilePath]( const auto& i_rcEntry )
                                 {
                                   return i_rcEntry->text().compare( i_strFilePath ) == 0;
                                 } ) };

  if( iter != m_cRecentActionsVector.end() )
  {
    // Promote the existing entry to the front
    std::rotate( m_cRecentActionsVector.begin(), iter, iter + 1 );
  }
  else
  {
    // Create an action for the entry
    QAction* pcAction{ new QAction( this ) };
    pcAction->setText( i_strFilePath );
    pcAction->setVisible( true );
    connect( pcAction, SIGNAL( triggered() ), this, SLOT( OnOpenRecentFile() ) );

    m_cRecentActionsVector.insert( m_cRecentActionsVector.begin(), pcAction );

    if( m_cRecentActionsVector.size() > s_iMaxRecentFiles )
    {
      // Remove the oldest entry
      pcAction = m_cRecentActionsVector.back();
      disconnect( pcAction, SIGNAL( triggered() ), this, SLOT( OnOpenRecentFile() ) );

      SAFE_DELETE( pcAction );
      m_cRecentActionsVector.pop_back();
    }
  }

  // Save the new order to the registry, and re-index each value
  QSettings cSettings;

  int32_t iIndex = 0;
  for( const auto& iter : m_cRecentActionsVector )
  {
    const QString strEntry{ QString( "MainWindow/RecentFile/%1" ).arg( ++iIndex ) };

    if( iter != nullptr )
    {
      cSettings.setValue( strEntry, iter->text() );
    }
    else
    {
      cSettings.setValue( strEntry, "" );
    }
  }
}
