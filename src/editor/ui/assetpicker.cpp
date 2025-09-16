#include <assetpicker.h>
#include <ui_assetpicker.h>

#include <u_property_asset.h>


AssetPicker::AssetPicker( AssetType i_eAssetType,
                          bool i_bPersistentOnly,
                          const QSet<rumAssetID>& i_rcIgnoredAssetsSet,
                          QWidget* i_pcParent )
  : QDialog( i_pcParent )
  , m_pcUI( new Ui::AssetPicker )
{
  m_pcUI->setupUi( this );

  setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

  m_pcUI->listWidget->setSortingEnabled( false );

  // Fill the window with the provided asset
  if( i_eAssetType == AssetType::Property_AssetType )
  {
    // Add every property entry by name and id
    const auto& rcPropertyHash{ rumPropertyAsset::GetAssetHash() };
    for( auto& iter : rcPropertyHash )
    {
      const rumPropertyAsset* pcAsset{ iter.second };
      if( pcAsset && ( !i_bPersistentOnly || pcAsset->IsPersistent() ) )
      {
        if( !i_rcIgnoredAssetsSet.contains( iter.first ) )
        {
          QListWidgetItem* pcItem{ new QListWidgetItem };
          pcItem->setData( Qt::DisplayRole, iter.second->GetName().c_str() );
          pcItem->setData( Qt::UserRole, iter.first );
          m_pcUI->listWidget->addItem( pcItem );
        }
      }
    }
  }
  else
  {
    rumAssertMsg( false, "Unsupported asset type" );
  }

  m_pcUI->listWidget->setSortingEnabled( true );
  m_pcUI->listWidget->sortItems();
}


AssetPicker::~AssetPicker()
{
  delete m_pcUI;
}


// slot
void AssetPicker::on_buttonBox_accepted()
{
  rumAssetID ePropertyID{ INVALID_ASSET_ID };

  const QListWidgetItem* pcItem{ m_pcUI->listWidget->currentItem() };
  if( pcItem )
  {
    const QVariant cVariant{ pcItem->data( Qt::UserRole ) };
    emit( NewPropertySelected( (rumAssetID)cVariant.toInt() ) );
  }

  accept();
}


// slot
void AssetPicker::on_lineEdit_Filter_textEdited( const QString &strText )
{
  UpdateFilter();
}


// slot
void AssetPicker::on_pushButton_Clear_clicked()
{
  m_pcUI->lineEdit_Filter->clear();
  UpdateFilter();
}


void AssetPicker::UpdateFilter()
{
  // Hide anything that doesn't pass the filter
  const QString& strFilter{ m_pcUI->lineEdit_Filter->text() };

  // Visit each item in the table and hide anything that doesn't match the filter settings
  const int32_t iRow{ m_pcUI->listWidget->count() };
  for( int32_t i{ 0 }; i < iRow; ++i )
  {
    bool bShow{ true };

    if( !strFilter.isEmpty() )
    {
      const QListWidgetItem* pcItem{ m_pcUI->listWidget->item( i ) };
      bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );
    }

    m_pcUI->listWidget->setRowHidden( i, !bShow );
  }
}
