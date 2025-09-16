#include <assetmanager.h>

// static
template< typename T >
T* AssetManager::CreateAsset()
{
  const rumAssetID eAssetID{ T::GetNextFreeID() };

  std::string strName{ T::GetStorageName() };
  strName += '_';
  strName += rumStringUtils::ToHexString( eAssetID );

  strName[0] = toupper( strName[0] );

  const std::string strBaseClass{ rumStringUtils::NullString() };

  // Create the new asset
  auto* pcAsset{ rumAsset::CreateAsset<T>( eAssetID, strName, strBaseClass ) };
  if( pcAsset )
  {
    rumAsset::RegisterClass<T>( pcAsset );
  }

  return pcAsset;
}
