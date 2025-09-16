#include <e_utility.h>

#include <u_broadcast_asset.h>
#include <u_creature_asset.h>
#include <u_custom_asset.h>
#include <u_graphic_asset.h>
#include <u_inventory_asset.h>
#include <u_map_asset.h>
#include <u_portal_asset.h>
#include <u_sound_asset.h>
#include <u_strings.h>
#include <u_tile_asset.h>
#include <u_widget_asset.h>

#include <QComboBox>


void rumControlUtils::BuildComboFromAssetType( QComboBox& io_rcCombo, PropertyValueType i_eValueType )
{
  // Add the invalid asset entry
  io_rcCombo.addItem( "<Invalid>", INVALID_ASSET_ID );

  switch( i_eValueType )
  {
    // Add every asset with the matching type to the combo
    case PropertyValueType::CreatureRef:
    {
      const auto& rcHash{ rumCreatureAsset::GetAssetHash() };
      for( auto& iter : rcHash )
      {
        io_rcCombo.addItem( iter.second->GetName().c_str(), iter.first );
      }
      break;
    }

    case PropertyValueType::PortalRef:
    {
      const auto& rcHash{ rumPortalAsset::GetAssetHash() };
      for( auto& iter : rcHash )
      {
        io_rcCombo.addItem( iter.second->GetName().c_str(), iter.first );
      }
      break;
    }

    case PropertyValueType::WidgetRef:
    {
      const auto& rcHash{ rumWidgetAsset::GetAssetHash() };
      for( auto& iter : rcHash )
      {
        io_rcCombo.addItem( iter.second->GetName().c_str(), iter.first );
      }
      break;
    }

    case PropertyValueType::BroadcastRef:
    {
      const auto& rcHash{ rumBroadcastAsset::GetAssetHash() };
      for( auto& iter : rcHash )
      {
        io_rcCombo.addItem( iter.second->GetName().c_str(), iter.first );
      }
      break;
    }

    case PropertyValueType::TileRef:
    {
      const auto& rcHash{ rumTileAsset::GetAssetHash() };
      for( auto& iter : rcHash )
      {
        io_rcCombo.addItem( iter.second->GetName().c_str(), iter.first );
      }
      break;
    }

    case PropertyValueType::MapRef:
    {
      const auto& rcHash{ rumMapAsset::GetAssetHash() };
      for( auto& iter : rcHash )
      {
        io_rcCombo.addItem( iter.second->GetName().c_str(), iter.first );
      }
      break;
    }

    case PropertyValueType::GraphicRef:
    {
      const auto& rcHash{ rumGraphicAsset::GetAssetHash() };
      for( auto& iter : rcHash )
      {
        io_rcCombo.addItem( iter.second->GetName().c_str(), iter.first );
      }
      break;
    }

    case PropertyValueType::SoundRef:
    {
      const auto& rcHash{ rumSoundAsset::GetAssetHash() };
      for( auto& iter : rcHash )
      {
        io_rcCombo.addItem( iter.second->GetName().c_str(), iter.first );
      }
      break;
    }

    case PropertyValueType::PropertyRef:
    {
      const auto& rcHash{ rumPropertyAsset::GetAssetHash() };
      for( auto& iter : rcHash )
      {
        io_rcCombo.addItem( iter.second->GetName().c_str(), iter.first );
      }
      break;
    }

    case PropertyValueType::InventoryRef:
    {
      const auto& rcHash{ rumInventoryAsset::GetAssetHash() };
      for( auto& iter : rcHash )
      {
        io_rcCombo.addItem( iter.second->GetName().c_str(), iter.first );
      }
      break;
    }

    case PropertyValueType::CustomRef:
    {
      const auto& rcHash{ rumCustomAsset::GetAssetHash() };
      for( auto& iter : rcHash )
      {
        io_rcCombo.addItem( iter.second->GetName().c_str(), iter.first );
      }
      break;
    }

    default:
      rumAssertMsg( false, "Unsupported AssetRef type" );
      break;
  }

  io_rcCombo.model()->sort( 0, Qt::AscendingOrder );
  io_rcCombo.view()->setMinimumWidth( io_rcCombo.minimumSizeHint().width() );
}


void rumControlUtils::BuildComboFromEnumType( QComboBox& io_rcCombo, std::string_view i_strEnum )
{
  // The enum field to use as selection data
  Sqrat::Object sqEnumObj{ Sqrat::ConstTable().GetSlot( i_strEnum.data() ) };

  Sqrat::Object::iterator iter;
  while( sqEnumObj.Next( iter ) )
  {
    // Add the enum field to the combo
    Sqrat::Object sqKey{ iter.getKey() };
    Sqrat::Object sqValue{ iter.getValue() };
    const int32_t eValue{ sqValue.Cast<int32_t>() };

    io_rcCombo.addItem( sqKey.Cast<std::string>().c_str(), eValue );
  }

  io_rcCombo.view()->setMinimumWidth( io_rcCombo.minimumSizeHint().width() );
}


void rumControlUtils::BuildComboFromStringTable( QComboBox& io_rcCombo, std::string_view i_strTableName )
{
  const auto* pcStringTable{ rumStringTable::GetStringTable( i_strTableName ) };
  if( nullptr == pcStringTable )
  {
    return;
  }

  const auto& rcTokenHash{ pcStringTable->GetTokenHash() };
  for( const auto& token : rcTokenHash )
  {
    io_rcCombo.addItem( token.second.c_str(), token.first );
  }

  io_rcCombo.model()->sort( 0, Qt::AscendingOrder );
  io_rcCombo.view()->setMinimumWidth( io_rcCombo.minimumSizeHint().width() );
}


void rumControlUtils::UpdateEditableState( QTableWidgetItem& io_rcCell, PropertyValueType i_eValueType )
{
  switch( i_eValueType )
  {
    case PropertyValueType::Null:
    case PropertyValueType::Float:
    case PropertyValueType::Bool:
    case PropertyValueType::String:
      io_rcCell.setFlags( io_rcCell.flags() | Qt::ItemIsEditable );
      break;

    case PropertyValueType::Integer:
      if( io_rcCell.data( Qt::UserRole ).toBool() )
      {
        // Enums are not directly modifiable
        io_rcCell.setFlags( io_rcCell.flags() ^ Qt::ItemIsEditable );
      }
      else
      {
        io_rcCell.setFlags( io_rcCell.flags() | Qt::ItemIsEditable );
      }
      break;

    default:
      io_rcCell.setFlags( io_rcCell.flags() ^ Qt::ItemIsEditable );
      break;
  }
}


void rumValueUtils::ConvertPropertyValue( Sqrat::Object& sqValue,
                                          PropertyValueType i_eExistingType,
                                          PropertyValueType i_eNewType )
{
  if( ( i_eExistingType == i_eNewType ) ||
      ( rumEnum::IsIntegerPropertyValueType( i_eExistingType ) && rumEnum::IsIntegerPropertyValueType( i_eNewType ) ) )
  {
    return;
  }

  if( PropertyValueType::Null == i_eExistingType )
  {
    if( i_eNewType >= PropertyValueType::FirstAssetRef && i_eNewType <= PropertyValueType::LastAssetRef )
    {
      rumScript::SetValue( sqValue, INVALID_ASSET_ID );
    }
    else if( i_eNewType >= PropertyValueType::StringToken )
    {
      rumScript::SetValue( sqValue, rumStringTable::INVALID_TOKEN_ID );
    }
    else
    {
      switch( i_eNewType )
      {
        case PropertyValueType::Integer:
        case PropertyValueType::Bitfield:
          rumScript::SetValue( sqValue, 0 );
          break;

        case PropertyValueType::Float:
          rumScript::SetValue( sqValue, 0.0f );
          break;

        case PropertyValueType::Bool:
          rumScript::SetValue( sqValue, false );
          break;

        case PropertyValueType::String:
          rumScript::SetValue( sqValue, "null");
          break;

        default:
          rumAssertMsg( false, "Unsupported value type" );
          break;
      }
    }
  }
  else if( ( PropertyValueType::Integer == i_eExistingType ) ||
           ( PropertyValueType::Bitfield == i_eExistingType ) )
  {
    if( i_eNewType >= PropertyValueType::FirstAssetRef && i_eNewType <= PropertyValueType::LastAssetRef )
    {
      rumAsset* pcAsset{ rumAsset::Fetch( static_cast<rumAssetID>( sqValue.Cast<int64_t>() ) ) };
      if( pcAsset )
      {
        rumScript::SetValue( sqValue, pcAsset->GetAssetID() );
      }
      else
      {
        // Set the first available asset
        const rumAssetID eAssetID{ rumValueUtils::GetFirstAsset( i_eNewType ) };
        rumScript::SetValue( sqValue, eAssetID );
      }
    }
    else
    {
      switch( i_eNewType )
      {
        case PropertyValueType::Null:
          rumScript::SetValue( sqValue, nullptr );
          break;

        case PropertyValueType::Float:
          rumScript::SetValue( sqValue, static_cast<float>( sqValue.Cast<int64_t>() ) );
          break;

        case PropertyValueType::Bool:
          rumScript::SetValue( sqValue, ( sqValue.Cast<int64_t>() == 0 ? false : true ) );
          break;

        case PropertyValueType::String:
          rumScript::SetValue( sqValue, rumStringUtils::ToString64( sqValue.Cast<int64_t>() ) );
          break;

        default:
          rumAssertMsg( false, "Unsupported value type" );
          break;
      }
    }
  }
  else if( PropertyValueType::Float == i_eExistingType )
  {
    if( i_eNewType >= PropertyValueType::FirstAssetRef && i_eNewType <= PropertyValueType::LastAssetRef )
    {
      rumAsset* pcAsset{ rumAsset::Fetch( static_cast<rumAssetID>( sqValue.Cast<float>() ) ) };
      if( pcAsset )
      {
        rumScript::SetValue( sqValue, pcAsset->GetAssetID() );
      }
      else
      {
        rumScript::SetValue( sqValue, INVALID_ASSET_ID );
      }
    }
    else
    {
      switch( i_eNewType )
      {
        case PropertyValueType::Null:
          rumScript::SetValue( sqValue, nullptr );
          break;

        case PropertyValueType::Integer:
        case PropertyValueType::Bitfield:
          rumScript::SetValue( sqValue, static_cast<int64_t>( sqValue.Cast<float>() ) );
          break;

        case PropertyValueType::Bool:
          rumScript::SetValue( sqValue, ( rumNumberUtils::ValueZero( sqValue.Cast<float>() ) ? false : true ) );
          break;

        case PropertyValueType::String:
          rumScript::SetValue( sqValue, rumStringUtils::ToFloatString( sqValue.Cast<float>() ) );
          break;

        default:
          rumAssertMsg( false, "Unsupported value type" );
          break;
      }
    }
  }
  else if( PropertyValueType::Bool == i_eExistingType )
  {
    if( i_eNewType >= PropertyValueType::FirstAssetRef && i_eNewType <= PropertyValueType::LastAssetRef )
    {
      if( sqValue.Cast<bool>() )
      {
        rumScript::SetValue( sqValue, GetFirstAsset( i_eNewType ) );
      }
      else
      {
        rumScript::SetValue( sqValue, INVALID_ASSET_ID );
      }
    }
    else
    {
      switch( i_eNewType )
      {
        case PropertyValueType::Null:
          rumScript::SetValue( sqValue, nullptr );
          break;

        case PropertyValueType::Integer:
        case PropertyValueType::Bitfield:
          rumScript::SetValue( sqValue, ( sqValue.Cast<bool>() ? 1 : 0 ) );
          break;

        case PropertyValueType::Float:
          rumScript::SetValue( sqValue, ( sqValue.Cast<bool>() ? 1.0f : 0.0f ) );
          break;

        case PropertyValueType::String:
          rumScript::SetValue( sqValue, ( sqValue.Cast<bool>() ? "true" : "false" ));
          break;

        default:
          rumAssertMsg( false, "Unsupported value type" );
          break;
      }
    }
  }
  else if( PropertyValueType::String == i_eExistingType )
  {
    if( i_eNewType >= PropertyValueType::FirstAssetRef && i_eNewType <= PropertyValueType::LastAssetRef )
    {
      auto* pcAsset{ rumAsset::FetchByName( sqValue.Cast<std::string>() ) };
      if( nullptr == pcAsset )
      {
        pcAsset = rumAsset::Fetch( static_cast<rumAssetID>( rumStringUtils::ToInt( sqValue.Cast<std::string>() ) ) );
      }

      if( pcAsset != nullptr )
      {
        rumScript::SetValue( sqValue, pcAsset->GetAssetID() );
      }
      else
      {
        rumScript::SetValue( sqValue, INVALID_ASSET_ID );
      }
    }
    else
    {
      switch( i_eNewType )
      {
        case PropertyValueType::Null:
          rumScript::SetValue( sqValue, nullptr );
          break;

        case PropertyValueType::Integer:
        case PropertyValueType::Bitfield:
          rumScript::SetValue( sqValue, ( rumStringUtils::ToInt( sqValue.Cast<std::string>() ) ) );
          break;

        case PropertyValueType::Float:
          rumScript::SetValue( sqValue, ( rumStringUtils::ToFloat( sqValue.Cast<std::string>() ) ) );
          break;

        case PropertyValueType::Bool:
          rumScript::SetValue( sqValue, ( rumStringUtils::ToBool( sqValue.Cast<std::string>() ) ) );
          break;

        default:
          rumAssertMsg( false, "Unsupported value type" );
          break;
      }
    }
  }
}


rumAssetID rumValueUtils::GetFirstAsset( PropertyValueType i_eAssetType )
{
  if( i_eAssetType < PropertyValueType::FirstAssetRef || i_eAssetType > PropertyValueType::LastAssetRef )
  {
    // Not a valid asset ref type
    return INVALID_ASSET_ID;
  }

  switch( i_eAssetType )
  {
    // Add every asset with the matching type to the combo
    case PropertyValueType::CreatureRef:
    {
      const auto& rcHash{ rumCreatureAsset::GetAssetHash() };
      return !rcHash.empty() ? rcHash.begin()->first : INVALID_ASSET_ID;
    }

    case PropertyValueType::PortalRef:
    {
      const auto& rcHash{ rumPortalAsset::GetAssetHash() };
      return !rcHash.empty() ? rcHash.begin()->first : INVALID_ASSET_ID;
    }

    case PropertyValueType::WidgetRef:
    {
      const auto& rcHash{ rumWidgetAsset::GetAssetHash() };
      return !rcHash.empty() ? rcHash.begin()->first : INVALID_ASSET_ID;
    }

    case PropertyValueType::BroadcastRef:
    {
      const auto& rcHash{ rumBroadcastAsset::GetAssetHash() };
      return !rcHash.empty() ? rcHash.begin()->first : INVALID_ASSET_ID;
    }

    case PropertyValueType::TileRef:
    {
      const auto& rcHash{ rumTileAsset::GetAssetHash() };
      return !rcHash.empty() ? rcHash.begin()->first : INVALID_ASSET_ID;
    }

    case PropertyValueType::MapRef:
    {
      const auto& rcHash{ rumMapAsset::GetAssetHash() };
      return !rcHash.empty() ? rcHash.begin()->first : INVALID_ASSET_ID;
    }

    case PropertyValueType::GraphicRef:
    {
      const auto& rcHash{ rumGraphicAsset::GetAssetHash() };
      return !rcHash.empty() ? rcHash.begin()->first : INVALID_ASSET_ID;
    }

    case PropertyValueType::SoundRef:
    {
      const auto& rcHash{ rumSoundAsset::GetAssetHash() };
      return !rcHash.empty() ? rcHash.begin()->first : INVALID_ASSET_ID;
    }

    case PropertyValueType::PropertyRef:
    {
      const auto& rcHash{ rumPropertyAsset::GetAssetHash() };
      return !rcHash.empty() ? rcHash.begin()->first : INVALID_ASSET_ID;
    }

    case PropertyValueType::InventoryRef:
    {
      const auto& rcHash{ rumInventoryAsset::GetAssetHash() };
      return !rcHash.empty() ? rcHash.begin()->first : INVALID_ASSET_ID;
    }

    case PropertyValueType::CustomRef:
    {
      const auto& rcHash{ rumCustomAsset::GetAssetHash() };
      return !rcHash.empty() ? rcHash.begin()->first : INVALID_ASSET_ID;
    }

    default:
      rumAssertMsg( false, "Unsuppoted AssetRef type" );
      break;
  }

  return INVALID_ASSET_ID;
}
