#include <u_enum.h>

#include <u_rum.h>
#include <u_script.h>


bool rumEnum::IsIntegerPropertyValueType( PropertyValueType i_eValueType )
{
  return ( ( PropertyValueType::Integer == i_eValueType ) ||
           ( PropertyValueType::Bitfield == i_eValueType ) ||
           ( PropertyValueType::StringToken == i_eValueType ) );
}


void rumEnum::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::ConstTable( pcVM )
    .Const( "rumSuccessReserveType", RESERVE_SUCCESS )
    .Const( "rumFailedReserveType", RESERVE_FAILED )
    .Const( "rumUnavailableReserveType", RESERVE_UNAVAILABLE )
    .Const( "rumRestrictedReserveType", RESERVE_RESTRICTED );

  Sqrat::ConstTable( pcVM )
    .Const( "rumInvalidHandle", RUM_INVALID_NATIVEHANDLE )
    .Const( "rumInvalidAssetID", INVALID_ASSET_ID )
    .Const( "rumInvalidGameID", INVALID_GAME_ID );

  Sqrat::ConstTable( pcVM )
    .Const( "rumCardinalDirectionType", Cardinal_DirectionType )
    .Const( "rumIntercardinalDirectionType", Intercardinal_DirectionType );

  Sqrat::ConstTable( pcVM )
    .Const( "rumSuccessMoveResultType", Success_MoveResultType )
    .Const( "rumFailMoveResultType", Fail_MoveResultType )
    .Const( "rumTileCollisionMoveResultType", TileCollision_MoveResultType )
    .Const( "rumPawnCollisionMoveResultType", PawnCollision_MoveResultType )
    .Const( "rumOffMapMoveResultType", OffMap_MoveResultType )
    .Const( "rumTooFarMoveResultType", TooFar_MoveResultType )
    .Const( "rumErrorMoveResultType", Error_MoveResultType );

  Sqrat::ConstTable( pcVM )
    .Const( "rumIgnoreTileCollisionMoveFlag", IgnoreTileCollision_MoveFlag )
    .Const( "rumIgnorePawnCollisionMoveFlag", IgnorePawnCollision_MoveFlag )
    .Const( "rumIgnoreDistanceMoveFlag", IgnoreDistance_MoveFlag )
    .Const( "rumTestMoveFlag", Test_MoveFlag );

  Sqrat::ConstTable( pcVM )
    .Const( "rumServerOriginType", Server_ObjectCreationType )
    .Const( "rumClientOriginType", Client_ObjectCreationType );

  Sqrat::ConstTable( pcVM )
    .Const( "rumNullPropertyValueType",         rumUtility::ToUnderlyingType( PropertyValueType::Null ) )
    .Const( "rumIntegerPropertyValueType",      rumUtility::ToUnderlyingType( PropertyValueType::Integer ) )
    .Const( "rumFloatPropertyValueType",        rumUtility::ToUnderlyingType( PropertyValueType::Float ) )
    .Const( "rumBoolPropertyValueType",         rumUtility::ToUnderlyingType( PropertyValueType::Bool ) )
    .Const( "rumStringPropertyValueType",       rumUtility::ToUnderlyingType( PropertyValueType::String ) )
    .Const( "rumBitfieldPropertyValueType",     rumUtility::ToUnderlyingType( PropertyValueType::Bitfield ) )
    .Const( "rumAssetRefPropertyValueType",     rumUtility::ToUnderlyingType( PropertyValueType::AssetRef ) )
    .Const( "rumCreatureRefPropertyValueType",  rumUtility::ToUnderlyingType( PropertyValueType::CreatureRef ) )
    .Const( "rumPortalRefPropertyValueType",    rumUtility::ToUnderlyingType( PropertyValueType::PortalRef ) )
    .Const( "rumWidgetRefPropertyValueType",    rumUtility::ToUnderlyingType( PropertyValueType::WidgetRef ) )
    .Const( "rumBroadcastRefPropertyValueType", rumUtility::ToUnderlyingType( PropertyValueType::BroadcastRef ) )
    .Const( "rumTileRefPropertyValueType",      rumUtility::ToUnderlyingType( PropertyValueType::TileRef ) )
    .Const( "rumMapRefPropertyValueType",       rumUtility::ToUnderlyingType( PropertyValueType::MapRef ) )
    .Const( "rumGraphicRefPropertyValueType",   rumUtility::ToUnderlyingType( PropertyValueType::GraphicRef ) )
    .Const( "rumSoundRefPropertyValueType",     rumUtility::ToUnderlyingType( PropertyValueType::SoundRef ) )
    .Const( "rumPropertyRefPropertyValueType",  rumUtility::ToUnderlyingType( PropertyValueType::PropertyRef ) )
    .Const( "rumInventoryRefPropertyValueType", rumUtility::ToUnderlyingType( PropertyValueType::InventoryRef ) )
    .Const( "rumCustomRefPropertyValueType",    rumUtility::ToUnderlyingType( PropertyValueType::CustomRef ) );
}
