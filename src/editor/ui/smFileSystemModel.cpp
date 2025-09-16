#include <smFileSystemModel.h>

#include <mainwindow.h>

#include <QMessageBox>

#include <u_utility.h>


smFileSystemModel::smFileSystemModel( QObject* i_pcParent )
  : QFileSystemModel( i_pcParent )
{}


smFileSystemModel::~smFileSystemModel()
{}


QVariant smFileSystemModel::data( const QModelIndex& i_rcIndex, int32_t i_eRole ) const
{
  if( !i_rcIndex.isValid() )
  {
    return QVariant();
  }

  if( ( Qt::CheckStateRole == i_eRole ) && ( i_rcIndex.column() == 0 ) )
  {
    const auto iter( m_cFileMap.find( i_rcIndex ) );
    if( iter != m_cFileMap.end() )
    {
      return iter.value();
    }
    else
    {
      return Qt::Unchecked;
    }
  }

  return QFileSystemModel::data( i_rcIndex, i_eRole );
}


Qt::ItemFlags smFileSystemModel::flags( const QModelIndex& i_rcIndex ) const
{
  // Flag each item as checkable (TODO: Qt::ItemIsTristate?)
  return QFileSystemModel::flags( i_rcIndex ) | Qt::ItemIsUserCheckable;
}


bool smFileSystemModel::setData( const QModelIndex& i_rcIndex, const QVariant& i_rcVariant, int32_t i_eRole)
{
  if( !i_rcIndex.isValid() )
  {
    return false;
  }

  if( i_eRole != Qt::CheckStateRole )
  {
    return QFileSystemModel::setData( i_rcIndex, i_rcVariant, i_eRole );
  }

  if( i_rcVariant == Qt::Checked )
  {
    // Insert acts as "replace" if the item already exists
    m_cFileMap.insert( i_rcIndex, Qt::Checked );
    m_cFileSet.insert( filePath( i_rcIndex ) );

    if( m_bRestoring )
    {
      // CheckState info is being restored from database
      if( !isDir( i_rcIndex ) )
      {
        // Remember that a child element of this directory has been checked so that we can determine the final check
        // state of this directory later rather than wastefully recursing over the tree as child element states are
        // being restored
        m_cRestoreDirSet.insert( parent( i_rcIndex ) );
      }
    }
    else
    {
      // Not a database restore, so determine parent and child checkstates immediately
      if( isDir( i_rcIndex ) )
      {
        // Determine new state of all subitems
        RecursiveCheck( i_rcIndex, Qt::Checked );
      }

      // Determine new state of parent
      if( i_rcIndex != m_cRootIndex )
      {
        ParentCheck( parent( i_rcIndex ), Qt::Checked, true );
      }
    }
  }
  else if( i_rcVariant == Qt::PartiallyChecked )
  {
    // Insert will replace the item if it already exists. Also, do not track partially checked items in the file set.
    m_cFileMap.insert( i_rcIndex, Qt::PartiallyChecked );
  }
  else
  {
    m_cFileMap.remove( i_rcIndex );
    m_cFileSet.remove( filePath( i_rcIndex ) );

    if( isDir( i_rcIndex ) )
    {
      // Determine new state of all subitems
      RecursiveCheck( i_rcIndex, Qt::Unchecked );
    }

    // Determine new state of parent
    if( i_rcIndex != m_cRootIndex )
    {
      ParentCheck( parent( i_rcIndex ), Qt::Unchecked, true );
    }
  }

  if( !m_bRestoring )
  {
    emit onDataChanged( i_rcIndex );
  }

  return true;
}


void smFileSystemModel::ParentCheck( const QModelIndex& i_rcIndex, const Qt::CheckState i_eCheckState,
                                     bool i_bCheckSubdirs )
{
  if( m_bRecursivePass || i_rcIndex == m_cRootIndex || !i_rcIndex.isValid() )
  {
    return;
  }

  Qt::CheckState parentValue{ (Qt::CheckState)data( i_rcIndex, Qt::CheckStateRole ).toUInt() };
  if( Qt::PartiallyChecked == parentValue )
  {
    if( SubitemsMatch( i_rcIndex, i_eCheckState, i_bCheckSubdirs ) )
    {
      // Transition from partially checked to fully checked or unchecked
      setData( i_rcIndex, i_eCheckState, Qt::CheckStateRole );
      emit dataChanged( i_rcIndex, i_rcIndex );

      // Determine new state of parent
      ParentCheck( parent( i_rcIndex ), i_eCheckState, false /*no recurse*/);
    }
  }
  else if( !SubitemsMatch( i_rcIndex, i_eCheckState, i_bCheckSubdirs ) )
  {
    // Transition to partially checked
    setData( i_rcIndex, Qt::PartiallyChecked, Qt::CheckStateRole );
    emit dataChanged( i_rcIndex, i_rcIndex );

    // Determine new state of parent
    ParentCheck( parent( i_rcIndex ), i_eCheckState, false /*no recurse*/);
  }
  else
  {
    // All subitems matched, reflect same for parent
    setData( i_rcIndex, i_eCheckState, Qt::CheckStateRole );
    emit dataChanged( i_rcIndex, i_rcIndex );
  }
}


void smFileSystemModel::RecursiveCheck( const QModelIndex& index, const Qt::CheckState value )
{
  const QModelIndex cFirstIndex{ index };
  QModelIndex cLastIndex{ index };

  if( m_bRecursivePass )
  {
    // We are already in a recursive check pass
    return;
  }

  if( !index.isValid() )
  {
    return;
  }

  Q_ASSERT( isDir( index ) );

  // Mark as a recursive pass so that this function isn't called re-entrantly
  m_bRecursivePass = true;

  QDirIterator iter( filePath( index ), QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories );
  while( iter.hasNext() )
  {
    const QModelIndex cIndex{ this->index( iter.next() ) };
    cLastIndex = cIndex;

    if( !m_strFilter.isEmpty() )
    {
      // Don't check filtered out filenames
      const QFileInfo cFileInfo{ iter.fileInfo() };
      if( cFileInfo.isFile() )
      {
        const QString& strFilename{ cFileInfo.fileName() };
        if( strFilename.contains( m_strFilter, Qt::CaseInsensitive ) )
        {
          setData( cIndex, value, Qt::CheckStateRole );
        }
      }
      else
      {
        // Recurse on the directory
        setData( cIndex, value, Qt::CheckStateRole );
      }
    }
    else
    {
      // No filtering, so just check anything encountered
      setData( cIndex, value, Qt::CheckStateRole );
    }
  }

  emit dataChanged( cFirstIndex, cLastIndex );
  m_bRecursivePass = false;
}


bool smFileSystemModel::IsChecked( const QModelIndex& i_rcIndex ) const
{
  return ( Qt::Checked == data( i_rcIndex, Qt::CheckStateRole ) );
}


void smFileSystemModel::ResetCheckedFiles()
{
  // TODO - complete set copy here, possibly slow if these grow very large
  const FileSet cFileSet( m_cFileSet );
  SetCheckedFiles( cFileSet );
}


void smFileSystemModel::SetCheckedFiles( const FileSet& i_rcFileSet )
{
  m_cFileMap.clear();
  m_cFileSet.clear();

  m_bRestoring = true;
  m_cRestoreDirSet.clear();

  QSetIterator<QString> iter( i_rcFileSet );
  while( iter.hasNext() )
  {
    setData( index( iter.next() ), Qt::Checked, Qt::CheckStateRole );
  }

  m_bRestoring = false;

  // Since checkstate info was restored from the database, it is necessary to re-examine the checkstate of any
  // directory parents to files. This prevents wasteful recursion over the tree to determine check state when items are
  // being loaded.
  QSetIterator<QModelIndex> iter2( m_cRestoreDirSet );
  while( iter2.hasNext() )
  {
    const QModelIndex cIndex{ iter2.next() };
    if( cIndex != m_cRootIndex )
    {
      // Check the parent state of this item
      ParentCheck( cIndex, Qt::Checked, true /*recurse*/);
    }
  }

  m_cRestoreDirSet.clear();
}


QModelIndex smFileSystemModel::SetRootPath( const QString& i_strPath )
{
  m_cRootIndex = super::setRootPath( i_strPath );
  return m_cRootIndex;
}


bool smFileSystemModel::SubitemsMatch( const QModelIndex& i_rcIndex, const Qt::CheckState i_eCheckState,
                                       bool i_bCheckSubdirs )
{
  if( !i_rcIndex.isValid() )
  {
    return false;
  }

  if( !isDir( i_rcIndex ) )
  {
    return false;
  }

  bool bMatch{ true };

  QDirIterator iter( filePath( i_rcIndex ), QDir::AllEntries | QDir::NoDotAndDotDot,
                     i_bCheckSubdirs ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags );
  while( iter.hasNext() )
  {
    const QModelIndex cIndex{ this->index( iter.next() ) };

    const Qt::CheckState eCheckState{ (Qt::CheckState)data( cIndex, Qt::CheckStateRole ).toUInt() };
    if( i_eCheckState != eCheckState )
    {
      // Found a sub-item that did not match
      bMatch = false;
    }
  }

  return bMatch;
}
