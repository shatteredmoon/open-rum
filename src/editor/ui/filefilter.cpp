#include <filefilter.h>

#include <QFileSystemModel>


void FileFilterProxyModel::AddDirFilter( const QString& i_strFilter )
{
  QDir cDir( i_strFilter );
  m_strDirFilterList << cDir.absolutePath();
}


bool FileFilterProxyModel::filterAcceptsRow( int32_t i_iRow, const QModelIndex& i_rcIndex ) const
{
  bool bAccept = true;

  if( i_rcIndex.isValid() )
  {
    const QModelIndex cModelIndex{ sourceModel()->index( i_iRow, 0, i_rcIndex ) };
    const QFileSystemModel* pcModel{ qobject_cast<QFileSystemModel*>( sourceModel() ) };

    const QFileInfo cFileInfo{ pcModel->fileInfo( cModelIndex ) };
    const QString strFilePath{ cFileInfo.absoluteFilePath() };

    const bool bIsFile{ cFileInfo.isFile() };

    // Apply the file filter if this is checking a file
    if( bIsFile && !m_strFileFilterList.isEmpty() )
    {
      bAccept = false;

      // Visit each file filter
      foreach( const QString& strFilter, m_strFileFilterList )
      {
        // The file filter must occur somewhere in the file path
        if( strFilePath.contains( strFilter ) )
        {
          bAccept = true;
        }
      }
    }

    // Apply the directory filter
    if( bAccept && !m_strDirFilterList.isEmpty() )
    {
      const QString strPath{ cFileInfo.absolutePath() };
      if( bIsFile && m_strDirFilterList.filter( strPath ).count() > 0 )
      {
        bAccept = true;
      }
      else if( cFileInfo.isDir() && m_strDirFilterList.filter( strFilePath ).count() > 0 )
      {
        bAccept = true;
      }
      else
      {
        bAccept = false;
      }
    }
  }

  return bAccept;
}


void FileFilterProxyModel::FilterUpdated( const QString& i_strFilter )
{
  m_strFileFilterList.clear();

  if( !( i_strFilter.isEmpty() || i_strFilter.isNull() ) )
  {
    m_strFileFilterList << i_strFilter;
  }

  invalidateFilter();
}
