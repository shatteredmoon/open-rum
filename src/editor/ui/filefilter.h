#ifndef FILEFILTER_H
#define FILEFILTER_H

#include <QSortFilterProxyModel>
#include <QStringList>

// Used for preventing certain files from showing up in file views
class FileFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:

  FileFilterProxyModel( QObject* i_pcParent = 0 )
    : QSortFilterProxyModel( i_pcParent )
    , m_eDirFilterType( FILTER_WHITELIST )
    , m_eFileFilterType( FILTER_WHITELIST )
  {}

  enum FilterType
  {
    FILTER_BLACKLIST,
    FILTER_WHITELIST
  };

  void FilterUpdated( const QString& i_strFilter );

  const QStringList& GetFileFilterList() const
  {
    return m_strFileFilterList;
  }

  void AddFileFilter( const QString& i_strFilter )
  {
    m_strFileFilterList << i_strFilter;
  }

  void ClearFileFilter()
  {
    m_strFileFilterList.clear();
  }

  void SetFileFilterType( FilterType i_eType )
  {
    m_eFileFilterType = i_eType;
  }

  const QStringList& GetDirFilterList() const
  {
    return m_strDirFilterList;
  }

  void AddDirFilter( const QString& i_strFilter );

  void ClearDirFilter()
  {
    m_strDirFilterList.clear();
  }

  void SetDirFilterType( FilterType i_eType )
  {
    m_eDirFilterType = i_eType;
  }

protected:

  bool filterAcceptsRow( int32_t i_iRow, const QModelIndex& i_rcIndex ) const override;

private:

  QStringList m_strDirFilterList;
  QStringList m_strFileFilterList;

  FilterType m_eDirFilterType;
  FilterType m_eFileFilterType;
};

#endif // FILEFILTER_H
