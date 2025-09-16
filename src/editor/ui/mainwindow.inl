// static
template<typename AssetClass>
uint32_t MainWindow::FindGraphicReferences( const rumAsset& i_rcAsset, bool i_bLog )
{
  MainWindow* pcWindow{ MainWindow::GetMainWindow() };
  rumAssert( pcWindow != nullptr );

  if( i_bLog )
  {
    pcWindow->ShowOutputDialog();
  }

  uint32_t uiNumRefs{ 0 };

  const auto& rcHash{ AssetClass::GetAssetHash() };
  for( const auto& iter : rcHash )
  {
    const auto& rcAsset{ iter.second };
    if( rcAsset->GetGraphicID() == i_rcAsset.GetAssetID() )
    {
      if( i_bLog )
      {
        pcWindow->ShowOnOutputDialog( QString( "Found: Asset " ) + rcAsset->GetName().c_str() + " " +
                                      std::string( rcAsset->GetTypeName() ).c_str() );
      }

      ++uiNumRefs;
    }
  }

  return uiNumRefs;
}


// static
template<typename AssetClass>
uint32_t MainWindow::FindPropertyReferences( const rumAsset& i_rcProperty, bool i_bLog )
{
  uint32_t uiNumRefs{ 0 };

  // Visit each asset
  const auto& rcAssetHash{ AssetClass::GetAssetHash() };
  for( const auto iterAsset : rcAssetHash )
  {
    if( iterAsset.second == nullptr )
    {
      continue;
    }

    const auto& rcAsset{ *iterAsset.second };

    // Visit each property in the asset
    const auto& rcProperties{ rcAsset.GetProperties() };
    uiNumRefs += FindPropertyReferences( rcProperties, rcAsset, i_rcProperty, i_bLog );
  }

  return uiNumRefs;
}
