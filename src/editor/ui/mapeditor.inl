#include <QListWidget>

template<typename T>
void MapEditor::InitListWidget( QListWidget* i_pcListWidget )
{
  Q_ASSERT( i_pcListWidget );

  i_pcListWidget->setSortingEnabled( false );

  // Populate the appropriate pawn list widget
  const auto& hashAssets{ T::GetAssetHash() };
  for( const auto& iter : hashAssets )
  {
    const T* pcAsset{ iter.second };
    Q_ASSERT( pcAsset );
    if( pcAsset )
    {
      QListWidgetItem* pcItem{ new QListWidgetItem };
      pcItem->setData( Qt::DisplayRole, pcAsset->GetName().c_str() );
      pcItem->setData( Qt::UserRole, pcAsset->GetAssetID() );
      i_pcListWidget->addItem( pcItem );
    }
  }

  i_pcListWidget->setSortingEnabled( true );
  i_pcListWidget->sortItems();
}
