#include <smTabWidget.h>

#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QMenu>
#include <QTabBar>

#ifdef WIN32
#include <shellapi.h>
#include <windows.h>
#endif // WIN32


smTabWidget::smTabWidget( QWidget* i_pcParent )
  : QTabWidget( i_pcParent )
{}


smTabWidget::~smTabWidget()
{
  m_cOpenTabsVector.clear();
}


void smTabWidget::DoContextMenu( const QPoint& i_rcPos )
{
  if( i_rcPos.isNull() )
  {
    return;
  }

  // Get the tab that is requesting the context menu
  const int32_t iTabIndex{ TabAt( i_rcPos ) };

  // Make sure right-click occurred on a tab
  if( iTabIndex == -1 )
  {
    return;
  }

  if( currentIndex() != iTabIndex )
  {
    // Switch to the tab
    setCurrentIndex( iTabIndex );
  }

  QMenu* pcMenu{ new QMenu() };

  QAction* pcCloseAction{ new QAction( "Close", this ) };
  pcMenu->addAction( pcCloseAction );
  connect( pcCloseAction, SIGNAL( triggered() ), this, SLOT( onClose() ) );

  QAction* pcCloseAllAction{ new QAction( "Close All But This", this ) };
  pcMenu->addAction( pcCloseAllAction );
  connect( pcCloseAllAction, SIGNAL( triggered() ), this, SLOT( onCloseAllButSelected() ) );

  pcMenu->addSeparator();

  QAction* pcCopyFilenameAction{ new QAction( "Copy Filename", this ) };
  pcMenu->addAction( pcCopyFilenameAction );
  connect( pcCopyFilenameAction, SIGNAL( triggered() ), this, SLOT( onCopyFilename() ) );

  QAction* pcCopyFullPathAction{ new QAction( "Copy Full Path", this ) };
  pcMenu->addAction( pcCopyFullPathAction );
  connect( pcCopyFullPathAction, SIGNAL( triggered() ), this, SLOT( onCopyFullPath() ) );

  pcMenu->addSeparator();

  QAction* pcExploreAction{ new QAction( "Open Containing Folder", this ) };
  pcMenu->addAction( pcExploreAction );
  connect( pcExploreAction, SIGNAL( triggered() ), this, SLOT( onExplore() ) );

  const QPoint cPos{ mapToGlobal( i_rcPos ) };
  pcMenu->exec( cPos );
}


const QString& smTabWidget::GetTabPath( int32_t i_iIndex ) const
{
  static const QString nullString;

  if( i_iIndex < m_cOpenTabsVector.size() )
  {
    return m_cOpenTabsVector.at( i_iIndex );
  }

  return nullString;
}


void smTabWidget::onCloseAllButSelected()
{
  // Note: Because Qt's tab widgets act much like vectors, we cannot remove the separate tab data during an iteration
  // because the indices will fail to match
  while( count() > 1 )
  {
    if( currentIndex() == 0 )
    {
      TabClose( 1 );
    }
    else
    {
      TabClose( 0 );
    }
  }
}


void smTabWidget::onCopyFilename() const
{
  QClipboard* pcClipBoard{ QApplication::clipboard() };
  if( pcClipBoard )
  {
    QString strFile{ tabText( currentIndex() ) };

    // Database tables contain a '|' symbol, strip it
    const int32_t iIndex{ strFile.indexOf( '|' ) };
    if( iIndex >= 0 )
    {
      strFile = strFile.left( iIndex );
    }

    // Copy the tab filename to the clipboard
    pcClipBoard->setText( strFile );
  }
}


void smTabWidget::onCopyFullPath() const
{
  QClipboard* pcClipBoard{ QApplication::clipboard() };
  if( pcClipBoard )
  {
    QString strPath{ m_cOpenTabsVector.at( currentIndex() ) };

    // Database tables contain a '|' symbol, strip it
    const int32_t iIndex{ strPath.indexOf( '|' ) };
    if( iIndex >= 0 )
    {
      strPath = strPath.left( iIndex );
    }

    // Copy the full path of the tab to the clipboard
    pcClipBoard->setText( strPath );
  }
}


void smTabWidget::onExplore() const
{
  QString strPath{ m_cOpenTabsVector.at( currentIndex() ) };

  // Database tables contain a '|' symbol, strip it
  const int32_t iIndex{ strPath.indexOf( '|' ) };
  if( iIndex >= 0 )
  {
    strPath = strPath.left( iIndex );
  }

  QFileInfo cFileInfo( strPath );
  if( cFileInfo.isFile() )
  {
    // Get the parent folder
    strPath = cFileInfo.path();

    // Update the file info
    cFileInfo.setFile( strPath );
  }

  // Only continue if we have a directory
  if( cFileInfo.isDir() )
  {
#ifdef WIN32
    ShellExecute( NULL, "explore", strPath.toLocal8Bit().data(), NULL, NULL, SW_SHOWNORMAL );
#else
#error Platform not supported!
#endif
  }
}


void smTabWidget::Reset()
{
  while( count() )
  {
    TabClose( 0 );
  }

  clear();

  Q_ASSERT( m_cOpenTabsVector.size() == 0 );
}


int32_t smTabWidget::TabAt( const QPoint& i_rcPos ) const
{
  QTabBar* pcTabBar{ this->tabBar() };
  if( pcTabBar )
  {
    return pcTabBar->tabAt( i_rcPos );
  }

  return -1;
}


void smTabWidget::TabClose( int32_t i_iIndex )
{
  // TODO - request file save if tab is dirty
  QWidget* pcTab{ widget( i_iIndex ) };
  Q_ASSERT( pcTab );

  removeTab( i_iIndex );

  // Free this tab's memory
  delete pcTab;
}


void smTabWidget::tabInserted( int32_t i_iIndex )
{
  // This assertion makes sure tabs are only ever appended
  Q_ASSERT( m_cOpenTabsVector.count() <= i_iIndex );

  // Get the tab "path" which is currently the tab's name. For database tables, we have to strip off the table name.
  const QString strFullPath{ tabText( i_iIndex ) };
  QString strTabName;

  QFileInfo cFileInfo;

  if( strFullPath.contains( '|' ) )
  {
    // This is a database table, strip the table name
    const QStringList strList{ strFullPath.split( '|' ) };

    cFileInfo.setFile( strList[0] );
    strTabName = cFileInfo.fileName() + "|" + strList[1];
  }
  else
  {
    // Convert the full path on the tab to a filename and store the full path for later retrieval
    cFileInfo.setFile( strFullPath );
    strTabName = cFileInfo.fileName();
  }

  setTabText( i_iIndex, strTabName );
  m_cOpenTabsVector.insert( i_iIndex, strFullPath );
}


void smTabWidget::tabRemoved( int32_t i_iIndex )
{
  // This function will be called if a tab failed to be fully created so that the extra tab data can be cleaned up. A
  // failed tab create should have an index higher than expected, so that information can be used to know whether or
  // not the extra data should get cleaned.
  if( i_iIndex < m_cOpenTabsVector.count() )
  {
    // Clean up the separate tab data
    m_cOpenTabsVector.remove( i_iIndex );
  }
}


void smTabWidget::ToString() const
{
  /*qDebug() << "Showing" << count() << "tabs:";

  for( int32_t i = 0; i < count(); ++i )
  {
      qDebug() << "  " << i << tabText(i);
      qDebug() << "  " << OpenTabs.at(i);
  }*/
}


void smTabWidget::UpdateTabInfo( int32_t i_iIndex, const QString& i_strFilePath )
{
  if( i_iIndex >= 0 && i_iIndex < m_cOpenTabsVector.size() )
  {
    m_cOpenTabsVector[i_iIndex] = i_strFilePath;

    const QFileInfo cFileInfo( i_strFilePath );
    setTabText( i_iIndex, cFileInfo.fileName() );
  }
}
