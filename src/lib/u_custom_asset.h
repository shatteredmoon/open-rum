#ifndef _U_CUSTOM_ASSET_H_
#define _U_CUSTOM_ASSET_H_

#include <u_asset.h>

#include <unordered_map>


class rumCustomAsset : public rumAsset
{
public:

  AssetType GetAssetType() const override
  {
    return GetClassRegistryID();
  }

  const std::string_view GetTypeName() const override;
  const std::string_view GetTypeSuffix() const override;

  static void ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile );
  static void ExportDBTables( ServiceType i_eServiceType );

  static rumCustomAsset* Fetch( rumAssetID i_eAssetID );

  static const std::string GetAssetClassName();

  typedef std::unordered_map< rumAssetID, rumCustomAsset* > AssetHash;
  static const AssetHash& GetAssetHash()
  {
    return s_hashAssets;
  }

  static const std::string GetAssetTypeSuffix();

  static AssetType GetClassRegistryID()
  {
    return Custom_AssetType;
  }

  static const std::string GetNativeClassName();

  static rumAssetID GetNextFreeID()
  {
    return s_eNextFreeID;
  }

  static const std::string GetPropertyTableCreateQuery();
  static const std::string GetPropertyTableSelectQuery();

  static std::string_view GetStorageName();

  static const std::string GetTableCreateQuery();
  static const std::string GetTableSelectQuery();

  static int32_t Init( const std::string& i_strPath );

  static void Remove( rumAssetID i_eAssetID )
  {
    s_hashAssets.erase( i_eAssetID );
  }

  static void ScriptBind();

  static void Shutdown();

  void OnCreated( const std::vector<std::string>& i_rvFields ) override;

private:

  void ExportCSVFile( std::ofstream& o_rcOutfile ) const final;
  void ExportDBTable( std::string& io_strQuery ) const final;

  static rumAssetID s_eNextFreeID;
  static AssetHash s_hashAssets;

  using super = rumAsset;
};

#endif // _U_CUSTOM_ASSET_H_
