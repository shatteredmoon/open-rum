#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QMainWindow>
#include <QMap>

#undef TRUE
#undef FALSE

#include <u_asset.h>
#include <u_script.h>

class QFileInfo;
class QLabel;
class QTableWidgetItem;
class QTreeWidgetItem;

class AssetManager;
class DataTableManager;
class OutputDialog;
class PackageManager;
class PatchManager;
class PropertyManager;
class ReleaseManager;
class SharedGLWidget;
class StringManager;

namespace Ui
{
  class MainWindow;
}


class MainWindow : public QMainWindow
{
  Q_OBJECT

  using PropertyContainer = rumPropertyContainer::PropertyContainer;

public:
  explicit MainWindow( rumConfig& i_rcConfig, QWidget* i_pcParent = 0 );
  ~MainWindow();

  // Adds or selects an existing tab in the tab view
  // Returns a pointer to the existing or newly created tab widget
  QWidget* AddOrOpenTab( const QTreeWidgetItem* i_pcItem );
  QWidget* AddOrOpenTab( const QString& i_strFile, int32_t i_iItemType, const QString& i_strText = "" );

  int32_t GetCurrentTabIndex();

  const rumConfig& GetConfig() const
  {
    return m_rcConfig;
  }

  void ShowOnOutputDialog( const QString& i_strOutput ) const;
  void ShowOutputDialog() const;

  static uint32_t FindAssetReferences( const rumAsset& i_rcAsset, bool i_bLog );

  static QDir GetProjectCSVDir();
  static QDir GetProjectDir();

  static QDir GetProjectAudioDir();
  static const QString& GetProjectAudioPathHint()
  {
    return s_strAudioPathHint;
  }
  static void SetProjectAudioPathHint( const QString& i_strPath );

  static QDir GetProjectFontDir();

  static QDir GetProjectGraphicDir();
  static const QString& GetProjectGraphicsPathHint()
  {
    return s_strGraphicsPathHint;
  }
  static void SetProjectGraphicsPathHint( const QString& i_strPath );

  static QDir GetProjectMapDir();
  static QDir GetProjectScriptDir();

  static const QString& GetProjectTitle()
  {
    return s_strTitle;
  }

  static void SetProjectTitle( const QString& i_strTitle )
  {
    s_strTitle = i_strTitle;
  }

  static const QString& GetProjectUUID()
  {
    return s_strUUID;
  }

  static void SetProjectUUID( const QString& i_strUUID )
  {
    s_strUUID = i_strUUID;
  }

  static const PropertyContainer& GetPropertyClipboard()
  {
    return s_cPropertyClipboard;
  }

  static void SetPropertyClipboard( const PropertyContainer& i_rcProperties )
  {
    s_cPropertyClipboard = i_rcProperties;
  }

  static void SetPropertyClipboard( PropertyContainer&& i_rcProperties )
  {
    s_cPropertyClipboard = i_rcProperties;
  }

  static QString GetFileHash( const QString& i_strFilePath );

  static uint32_t GetTileHeight()
  {
    return s_uiTileHeight;
  }
  static uint32_t GetTileWidth()
  {
    return s_uiTileWidth;
  }

  void Init( int32_t i_iArgc, char* i_pcArgv[] );

  void ProjectClose();

  // Loads a project database into the editor
  void ProjectOpen( const QString& i_strFilePath );

  void RefreshTreeWidget();

  void SetTabIcon( int32_t i_iIndex, const QIcon& i_rcIcon );
  void SetTabInfo( int32_t i_iIndex, const QString& i_strFilePath );

  // Recurses object parents until it gets to the main window, or returns NULL
  // if no main window was found
  static MainWindow* GetMainWindow( QObject* i_pcObject );

  static MainWindow* GetMainWindow()
  {
    return m_pcMainWindow;
  }

  static void SetStatusBarText( const QString& i_strText );

  // Prints command-line usage directions
  static void ShowUsage();

  // Modal dialog box request evoked from script
  // Results are stored in static m_sqScriptDialogResult
  static Sqrat::Object ScriptModalDialog( Sqrat::Array i_sqArray );

  // Used by ScriptModal dialogs to return the dialog results
  static void SetScriptModalDialogResult( Sqrat::Object i_sqObj )
  {
    m_sqScriptDialogResult = i_sqObj;
  }

private slots:

  void on_actionAsset_Manager_triggered();
  void on_actionCloseProject_triggered();
  void on_actionDataTable_Manager_triggered();
  void on_actionNewProject_triggered();
  void on_actionOpenProject_triggered();
  void on_actionPackage_Manager_triggered();
  void on_actionPatch_Manager_triggered();
  void on_actionProject_Settings_triggered();
  void on_actionProperty_Manager_triggered();
  void on_actionRelease_Manager_triggered();
  void on_actionResaveAllMaps_triggered();
  void on_actionString_Manager_triggered();

  void on_tabWidget_customContextMenuRequested( const QPoint& i_rcPos );
  void on_tabWidget_tabCloseRequested( int32_t i_iIndex );

  void on_treeWidget_customContextMenuRequested( const QPoint& i_rcPos );
  void on_treeWidget_itemDoubleClicked( QTreeWidgetItem* i_pcItem, int32_t i_iColumn );

  void onAssetManagerClosed();
  void onDataTableManagerClosed();
  void onPackageManagerClosed();
  void onPatchManagerClosed();
  void onProjectSettingsClosed( int32_t i_iResult );
  void onPropertyManagerClosed();
  void onReleaseManagerClosed();
  void onStringManagerClosed();

  void OnOpenRecentFile();

private:

  void closeEvent( QCloseEvent* i_pcEvent ) override;

  void timerEvent( QTimerEvent* i_pcEvent ) override;

  void InitRecentFilesArray();
  void UpdateRecentFilesArray( const QString& i_strFilePath );

  void RemoveRecentFiles();
  void UpdateRecentFiles();

// static
  template<typename AssetClass>
  static uint32_t FindGraphicReferences( const rumAsset& i_rcGraphic, bool i_bLog );
  static uint32_t FindGraphicReferences( const rumAsset& i_rcGraphic, bool i_bLog );

  static uint32_t FindMapReferences( const rumAsset& i_rcAsset, bool i_bLog );

  template<typename AssetClass>
  static uint32_t FindPropertyReferences( const rumAsset& i_rcAsset, bool i_bLog );
  static uint32_t FindPropertyReferences( const rumAsset& i_rcAsset, bool i_bLog );
  static uint32_t FindPropertyReferences( const PropertyContainer& i_rcProperties,
                                          const rumAsset& i_rcCurrentAsset,
                                          const rumAsset& i_rcAssetToFind,
                                          bool i_bLog );

  static uint32_t FindScriptReferences( const rumAsset& i_rcAsset, bool i_bLog );

  Ui::MainWindow* m_pcUI;

  QLabel* m_pcStatusCursorLabel;

  static QString s_strAudioPathHint;
  static QString s_strGraphicsPathHint;
  static QString s_strTitle;
  static QString s_strUUID;

  // Data returned from a modal ScriptDialog
  static Sqrat::Object m_sqScriptDialogResult;

  static PropertyContainer s_cPropertyClipboard;

  static MainWindow* m_pcMainWindow;

  // The project's tile dimensions, set by the user in the project settings
  constexpr static uint32_t s_uiTileHeight{ 32 };
  constexpr static uint32_t s_uiTileWidth{ 32 };

  constexpr static int32_t s_iMaxRecentFiles{ 5 };

  // The id of the timer that invokes scheduler events
  int32_t m_iSchedulerTimer;

  // The id of the misc animation timer
  int32_t m_iAnimTimer;

  AssetManager* m_pcAssetManager{ nullptr };
  DataTableManager* m_pcDataTableManager{ nullptr };
  OutputDialog* m_pcOutputDialog{ nullptr };
  PackageManager* m_pcPackageManager{ nullptr };
  PatchManager* m_pcPatchManager{ nullptr };
  PropertyManager* m_pcPropertyManager{ nullptr };
  ReleaseManager* m_pcReleaseManager{ nullptr };
  SharedGLWidget* m_pcSharedGLWidget{ nullptr };
  StringManager* m_pcStringManager{ nullptr };

  rumConfig& m_rcConfig;

  std::vector<QAction*> m_cRecentActionsVector;

  // Returns true if a project is actively loaded
  bool m_bProjectOpen;
};

#include <mainwindow.inl>

#endif // MAINWINDOW_H
