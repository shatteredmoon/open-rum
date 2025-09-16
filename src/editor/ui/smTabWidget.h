#ifndef SM_TABWIDGET_H
#define SM_TABWIDGET_H

#include <QTabWidget>
#include <QMap>

class smTabWidget : public QTabWidget
{
  Q_OBJECT

public:

  smTabWidget( QWidget* i_pcParent = 0 );
  ~smTabWidget();

  void DoContextMenu( const QPoint& i_rcPos );

  int32_t GetTabIndex( const QString& i_strPath ) const
  {
    return m_cOpenTabsVector.indexOf( i_strPath );
  }

  const QString& GetTabPath( int32_t i_iIndex ) const;

  void Reset();

  int32_t TabAt( const QPoint& i_rcPos ) const;

  void TabClose( int32_t i_iIndex );

  // Debug list of tab names and extra tab data
  void ToString() const;

  // Updates an existing tab with new info
  void UpdateTabInfo( int32_t i_iIndex, const QString& i_strFilePath );

public slots:

  void onClose()
  {
    TabClose( currentIndex() );
  }

  void onCloseAllButSelected();
  void onCopyFilename() const;
  void onCopyFullPath() const;
  void onExplore() const;

protected:

  void tabInserted( int32_t i_iIndex ) override;
  void tabRemoved( int32_t i_iIndex ) override;

private:

  typedef QVector<QString> OpenTabContainer;
  OpenTabContainer m_cOpenTabsVector;
};

#endif // SM_TABWIDGET_H
