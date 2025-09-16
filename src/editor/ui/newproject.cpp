#include <newproject.h>
#include <ui_newproject.h>

#undef TRUE
#undef FALSE

#include <mainwindow.h>

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QUuid>


NewProject::NewProject( bool i_bEditMode, QWidget* i_pcParent )
  : QDialog( i_pcParent )
  , m_pcUI( new Ui::NewProject )
  , m_bEditMode( i_bEditMode )
{
  m_pcUI->setupUi( this );

  const QIcon cSettingsIcon( ":/ui/resources/settings.png" );
  setWindowIcon( cSettingsIcon );

  // Make error labels red
  QPalette palette{ m_pcUI->label_Title_Error->palette() };
  palette.setColor( m_pcUI->label_Title_Error->foregroundRole(), Qt::red );
  m_pcUI->label_Title_Error->setPalette( palette );
  m_pcUI->label_Uuid_Error->setPalette( palette );
  m_pcUI->label_Project_Error->setPalette( palette );

  // Hide error labels
  m_pcUI->label_Title_Error->setHidden( true );
  m_pcUI->label_Uuid_Error->setHidden( true );
  m_pcUI->label_Project_Error->setHidden( true );

  if( IsEditing() )
  {
    // We are in edit mode, so change the title to reflect this
    setWindowTitle( "Edit Project Settings" );

    // Populate the form with settings stored in the database
    m_pcUI->lineEdit_Title->setText( MainWindow::GetProjectTitle() );
    m_pcUI->lineEdit_Uuid->setText( MainWindow::GetProjectUUID() );

#pragma message("TODO - move these to QSettings")

    m_pcUI->lineEdit_Audio_Path_Hint->setText( MainWindow::GetProjectAudioPathHint() );
    m_pcUI->lineEdit_Graphics_Path_Hint->setText( MainWindow::GetProjectGraphicsPathHint() );

    // In edit mode, show the absolute path to the project folder
    const QString strProjectPath{ QDir::toNativeSeparators( MainWindow::GetProjectDir().canonicalPath() ) };
    m_pcUI->lineEdit_Project_Folder->setText( strProjectPath );
    m_pcUI->lineEdit_Project_Folder->setEnabled( false );
  }
  else
  {
    // Generate a default uuid
    m_pcUI->lineEdit_Uuid->setText( QUuid::createUuid().toString() );
  }
}


NewProject::~NewProject()
{
  delete m_pcUI;
}


bool NewProject::UpdateDatabase()
{
  const QString& strTitle{ m_pcUI->lineEdit_Title->text() };
  MainWindow::SetProjectTitle( strTitle );

  const QString& strUUID{ m_pcUI->lineEdit_Uuid->text().toLower() };
  MainWindow::SetProjectUUID( strUUID );

  const QString& strAudioPathHint{ m_pcUI->lineEdit_Audio_Path_Hint->text() };
  MainWindow::SetProjectAudioPathHint( strAudioPathHint );

  const QString& strGraphicsPathHint{ m_pcUI->lineEdit_Graphics_Path_Hint->text() };
  MainWindow::SetProjectGraphicsPathHint( strGraphicsPathHint );

  // Remove any previous entries
  rumDatabase::Query( rumDatabase::Game_DatabaseID, "DELETE FROM settings WHERE 1" );

  // Update the database
  std::string strQuery{ "INSERT INTO settings (key,value) VALUES ('title','" };
  strQuery += qPrintable( strTitle );
  strQuery += "');INSERT INTO settings (key,value) VALUES ('uuid','";
  strQuery += qPrintable( strUUID );
  strQuery += "');";

  if( !strAudioPathHint.isEmpty() )
  {
    strQuery += "INSERT INTO settings (key,value) VALUES ('audio','";
    strQuery += qPrintable( strAudioPathHint );
    strQuery += "');";
  }

  if( !strGraphicsPathHint.isEmpty() )
  {
    strQuery += "INSERT INTO settings (key,value) VALUES ('graphics','";
    strQuery += qPrintable( strGraphicsPathHint );
    strQuery += "');";
  }

  const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::Game_DatabaseID, strQuery ) };
  return pcQuery && !pcQuery->IsError();
}


void NewProject::on_buttonBox_accepted()
{
  if( !ValidateInput() )
  {
    return;
  }

  if( IsEditing() )
  {
    if( UpdateDatabase() )
    {
      accept();
    }
    else
    {
      const QString& strTitle{ m_pcUI->lineEdit_Title->text() };
      QMessageBox::critical( this, tr( "Error" ), "Failed to update database for project " + strTitle );
    }
  }
  else
  {
    // TODO - attach to database?
    //QSqlDatabase cDatabase{ QSqlDatabase::addDatabase( "QSQLITE", "GAME" ) };
    //cDatabase.setDatabaseName( strDatabase );

    // Create a new folder for the project
    const QString strProject{ QDir::currentPath() + "/" + m_pcUI->lineEdit_Project_Folder->text() };
    const QDir cDirProject( strProject );
    if( cDirProject.mkpath( strProject ) )
    {
      // Create all subdirectories
      const QString strAudio{ "audio" };
      const QString strFonts{ "fonts" };
      const QString strGraphics{ "graphics" };
      const QString strMaps{ "maps" };
      const QString strScripts{ "scripts" };

      const QString strClientScripts{ strScripts + "/client" };
      const QString strServerScripts{ strScripts + "/server" };

      cDirProject.mkpath( strProject + "/" + strAudio );
      cDirProject.mkpath( strProject + "/" + strFonts );
      cDirProject.mkpath( strProject + "/" + strGraphics );
      cDirProject.mkpath( strProject + "/" + strMaps );
      cDirProject.mkpath( strProject + "/" + strClientScripts );
      cDirProject.mkpath( strProject + "/" + strServerScripts );

      const QString strResource{ ":/ui/resources/project" };
      const QString strDatabase{ strProject + "/" + "game.rum" };

      QVector<QPair<QString, QString>> cFileVector;

      // Databases ------------------------------------------------------------------------------------------
      cFileVector.append( qMakePair( QString(), QString( "game.rum" ) ) );
      cFileVector.append( qMakePair( QString(), QString( "patch.db" ) ) );
      cFileVector.append( qMakePair( QString(), QString( "player.db" ) ) );
      cFileVector.append( qMakePair( QString(), QString( "strings_c.db" ) ) );
      cFileVector.append( qMakePair( QString(), QString( "strings_s.db" ) ) );
      cFileVector.append( qMakePair( QString(), QString( "strings_u.db" ) ) );
      cFileVector.append( qMakePair( QString(), QString( "types.db" ) ) );

      // Fonts ----------------------------------------------------------------------------------------------
      cFileVector.append( qMakePair( strFonts, QString( "tahoma.ttf" ) ) );

      // Graphics -------------------------------------------------------------------------------------------
      cFileVector.append( qMakePair( strGraphics, QString( "floor_brick.png" ) ) );
      cFileVector.append( qMakePair( strGraphics, QString( "floor_rock.png" ) ) );
      cFileVector.append( qMakePair( strGraphics, QString( "floor_tile_a.png" ) ) );
      cFileVector.append( qMakePair( strGraphics, QString( "floor_tile_b.png" ) ) );
      cFileVector.append( qMakePair( strGraphics, QString( "widget_bush.png" ) ) );
      cFileVector.append( qMakePair( strGraphics, QString( "widget_rock.png" ) ) );

      // Audio ----------------------------------------------------------------------------------------------
      cFileVector.append( qMakePair( strAudio, QString( "fx_intro.wav" ) ) );
      cFileVector.append( qMakePair( strAudio, QString( "fx_step.wav" ) ) );
      cFileVector.append( qMakePair( strAudio, QString( "fx_thunder_a.wav" ) ) );
      cFileVector.append( qMakePair( strAudio, QString( "fx_thunder_b.wav" ) ) );
      cFileVector.append( qMakePair( strAudio, QString( "audio.pkg" ) ) );

      // Maps -----------------------------------------------------------------------------------------------
      cFileVector.append( qMakePair( strMaps, QString( "world.map" ) ) );
      cFileVector.append( qMakePair( strMaps, QString( "map.pkg" ) ) );

      // Shared scripts -------------------------------------------------------------------------------------
      cFileVector.append( qMakePair( strScripts, QString( "shared.nut" ) ) );

      // Client Scripts -------------------------------------------------------------------------------------
      cFileVector.append( qMakePair( strClientScripts, QString( "broadcast.nut" ) ) );
      cFileVector.append( qMakePair( strClientScripts, QString( "creature.nut" ) ) );
      cFileVector.append( qMakePair( strClientScripts, QString( "graphic.nut" ) ) );
      cFileVector.append( qMakePair( strClientScripts, QString( "game.nut" ) ) );
      cFileVector.append( qMakePair( strClientScripts, QString( "inventory.nut" ) ) );
      cFileVector.append( qMakePair( strClientScripts, QString( "map.nut" ) ) );
      cFileVector.append( qMakePair( strClientScripts, QString( "npc.nut" ) ) );
      cFileVector.append( qMakePair( strClientScripts, QString( "player.nut" ) ) );
      cFileVector.append( qMakePair( strClientScripts, QString( "portal.nut" ) ) );
      cFileVector.append( qMakePair( strClientScripts, QString( "property.nut" ) ) );
      cFileVector.append( qMakePair( strClientScripts, QString( "sound.nut" ) ) );
      cFileVector.append( qMakePair( strClientScripts, QString( "tile.nut" ) ) );
      cFileVector.append( qMakePair( strClientScripts, QString( "utility.nut" ) ) );
      cFileVector.append( qMakePair( strClientScripts, QString( "widget.nut" ) ) );

      // Server scripts -------------------------------------------------------------------------------------
      cFileVector.append( qMakePair( strServerScripts, QString( "broadcast.nut" ) ) );
      cFileVector.append( qMakePair( strServerScripts, QString( "creature.nut" ) ) );
      cFileVector.append( qMakePair( strServerScripts, QString( "game.nut" ) ) );
      cFileVector.append( qMakePair( strServerScripts, QString( "inventory.nut" ) ) );
      cFileVector.append( qMakePair( strServerScripts, QString( "map.nut" ) ) );
      cFileVector.append( qMakePair( strServerScripts, QString( "npc.nut" ) ) );
      cFileVector.append( qMakePair( strServerScripts, QString( "player.nut" ) ) );
      cFileVector.append( qMakePair( strServerScripts, QString( "portal.nut" ) ) );
      cFileVector.append( qMakePair( strServerScripts, QString( "property.nut" ) ) );
      cFileVector.append( qMakePair( strServerScripts, QString( "server.nut" ) ) );
      cFileVector.append( qMakePair( strServerScripts, QString( "tile.nut" ) ) );
      cFileVector.append( qMakePair( strServerScripts, QString( "widget.nut" ) ) );

      // Copy the resource files
      QFile cFile;
      for( int32_t i = 0; i < cFileVector.size(); ++i )
      {
        cFile.setFileName( strProject + "/" + cFileVector.at( i ).first + "/" + cFileVector.at( i ).second );
        QFile::copy( strResource + "/" + cFileVector.at( i ).first + "/" + cFileVector.at( i ).second, cFile.fileName() );
        cFile.setPermissions( QFile::ReadUser | QFile::WriteUser );
      }

      const QString& strTitle{ m_pcUI->lineEdit_Title->text() };

      // Update the game database with the user-provided settings
      if( UpdateDatabase() )
      {
        QMessageBox::information( this, tr( "Project Created" ), "New project created: " + strTitle );
        accept();
      }
      else
      {
        QMessageBox::critical( this, tr( "Error" ), "Failed to update project database for " + strTitle );
      }

      // TODO
      //QSqlDatabase::removeDatabase( "GAME" );
    }
  }
}


void NewProject::on_lineEdit_Project_Folder_textEdited( const QString& i_strText )
{
  m_pcUI->label_Project_Error->setHidden( true );
}


void NewProject::on_lineEdit_Title_textEdited( const QString& i_strText )
{
  m_pcUI->label_Title_Error->setHidden( true );
}


void NewProject::on_lineEdit_Uuid_textEdited( const QString& i_strText )
{
  m_pcUI->label_Uuid_Error->setHidden( true );
}


void NewProject::on_pushButton_Audio_Path_Hint_clicked()
{
  const QDir& cDirProjectPath{ MainWindow::GetProjectDir().absolutePath() };
  const QDir& cDirAudioPath{ MainWindow::GetProjectAudioDir().absolutePath() };
  const QString& strFolder{ QFileDialog::getExistingDirectory( this, "Audio Path Hint",
                                                               cDirAudioPath.absolutePath() ) };
  if( !strFolder.isEmpty() )
  {
    const QString& strRelativePath{ cDirProjectPath.relativeFilePath( strFolder ) };
    m_pcUI->lineEdit_Audio_Path_Hint->setText( strRelativePath );
  }
}


void NewProject::on_pushButton_Graphics_Path_Hint_clicked()
{
  const QDir& cDirProjectPath{ MainWindow::GetProjectDir().absolutePath() };
  const QDir& cDirGraphicsPath{ MainWindow::GetProjectGraphicDir().absolutePath() };
  const QString& strFolder{ QFileDialog::getExistingDirectory( this, "Graphics Path Hint",
                                                               cDirGraphicsPath.absolutePath() ) };
  if( !strFolder.isEmpty() )
  {
    const QString& strRelativePath{ cDirProjectPath.relativeFilePath( strFolder ) };
    m_pcUI->lineEdit_Graphics_Path_Hint->setText( strRelativePath );
  }
}


void NewProject::on_pushButton_Uuid_clicked()
{
  m_pcUI->lineEdit_Uuid->setText( QUuid::createUuid().toString() );
}


bool NewProject::ValidateInput()
{
  bool bValid{ true };

  m_pcUI->label_Title_Error->setHidden( true );
  m_pcUI->label_Uuid_Error->setHidden( true );
  m_pcUI->label_Project_Error->setHidden( true );

  const QString& strTitle{ m_pcUI->lineEdit_Title->text() };
  const QString& strUuid{ m_pcUI->lineEdit_Uuid->text().toLower() };

  // Title is required
  if( strTitle.isNull() || strTitle.isEmpty() )
  {
    bValid = false;
    m_pcUI->label_Title_Error->setText( "Title required" );
    m_pcUI->label_Title_Error->setHidden( false );
  }

  // Uuid format {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
  const QRegExp cUuidRegExp( "^[{]([a-fA-F0-9]){8}[-]((([a-fA-F0-9]){4}[-]){3})([a-fA-F0-9]){12}[}]$" );

  // Uuid must not be a Null Uuid
  const QUuid cUuid( strUuid );

  if( strUuid.isNull() || strUuid.isEmpty() )
  {
    bValid = false;
    m_pcUI->label_Uuid_Error->setText( "UUID required" );
    m_pcUI->label_Uuid_Error->setHidden( false );
  }
  else if( !strUuid.contains( cUuidRegExp ) )
  {
    bValid = false;
    m_pcUI->label_Uuid_Error->setText( "UUID invalid" );
    m_pcUI->label_Uuid_Error->setHidden( false );
  }
  else if( cUuid.isNull() )
  {
    bValid = false;
    m_pcUI->label_Uuid_Error->setText( "UUID invalid or null" );
    m_pcUI->label_Uuid_Error->setHidden( false );
  }

  const QString& strProgramPath{ QDir::currentPath() };

  const QString& strProjectPath{ m_pcUI->lineEdit_Project_Folder->text() };
  QDir cDirProject( strProjectPath );
  if( !cDirProject.isAbsolute() )
  {
    cDirProject.setPath( strProgramPath + "/" + strProjectPath );
  }

  if( IsAdding() )
  {
    // The project path must not contain special symbols, whitespace or slashes
    const QRegExp cProjectPathRegExp( "^(\\w){1,64}$" );

    if( strProjectPath.isEmpty() )
    {
      bValid = false;
      m_pcUI->label_Project_Error->setText( "Folder name required" );
      m_pcUI->label_Project_Error->setHidden( false );
    }
    else if( !strProjectPath.contains( cProjectPathRegExp ) )
    {
      bValid = false;
      m_pcUI->label_Project_Error->setText( "Folder name invalid" );
      m_pcUI->label_Project_Error->setHidden( false );
    }
  }

  return bValid;
}
