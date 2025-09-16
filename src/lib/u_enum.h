#ifndef _U_ENUM_H_
#define _U_ENUM_H_

#include <platform.h>

#include <unordered_set>

enum class PropertyValueType : uint16_t;

namespace rumEnum
{
  bool IsIntegerPropertyValueType( PropertyValueType i_eValueType );
  void ScriptBind();
}


// Note: serialized to maps as a byte. Do not change order once set, and do not exceed 0xF!
enum AssetType
{
  Creature_AssetType  = 0x0,
  Portal_AssetType    = 0x1,
  Widget_AssetType    = 0x2,
  Broadcast_AssetType = 0x3,
  Tile_AssetType      = 0x4,
  Map_AssetType       = 0x5,
  Graphic_AssetType   = 0x6,
  Sound_AssetType     = 0x7,
  Property_AssetType  = 0x8,
  Inventory_AssetType = 0x9,
  Custom_AssetType    = 0xA,
  DataTable_AssetType = 0xB,
  NumAssetTypes       = 0xC
};

// An asset ID containing an AssetType and database ID
using rumAssetID = uint32_t;

// Asset ID with the type masked out
#define RAW_ASSET_ID( x ) ( (rumAssetID)( ( x ) & 0x0FFFFFFF ) )

// Asset ID including the asset type. The asset type is shifted to become the highest byte, and that's why AssetType
// shouldn't exceed one byte. If that happens, the asset type will need to occupy the highest word.
#define FULL_ASSET_ID( x, y ) ( (rumAssetID)( ( ( x ) << 28 ) | ( y ) ) )

// Asset type with the ID masked out
#define RAW_ASSET_TYPE( x ) ( (AssetType)( ( ( x ) & 0xF0000000 ) >> 28 ) )

#define INVALID_ASSET_ID 0xFFFFFFFF

// Serialized to CSV. Do not change order once set.
enum class PropertyValueType : uint16_t
{
  Null          = 0,
  Integer       = 0x100,
  Float         = 0x101,
  Bool          = 0x102,
  String        = 0x103,

  // Possibly eventually support these:
  //Table         = 0x104,
  //Array         = 0x105,
  //Closure       = 0x106,
  //NativeClosure = 0x107,
  //Class         = 0x108,
  //Instance      = 0x109,
  //Weakref       = 0x110,

  Bitfield      = 0x111,
  AssetRef      = 0x200,

  FirstAssetRef = 0x300,
  CreatureRef   = 0x300,
  PortalRef     = 0x301,
  WidgetRef     = 0x302,
  BroadcastRef  = 0x303,
  TileRef       = 0x304,
  MapRef        = 0x305,
  GraphicRef    = 0x306,
  SoundRef      = 0x307,
  PropertyRef   = 0x308,
  InventoryRef  = 0x309,
  CustomRef     = 0x30a,
  LastAssetRef  = 0x30a,

  StringToken   = 0x400
};


// A unique ID attached to any instantiated asset
using rumUniqueID = uint64_t;

#define INVALID_GAME_ID 0

typedef std::unordered_set<rumUniqueID> GameIDHash;


enum ObjectCreationType
{
  Neutral_ObjectCreationType = 0,
  Server_ObjectCreationType  = 1,
  Client_ObjectCreationType  = 2
};

// Mask for the creation type
#define OBJECT_CREATION_TYPE( x ) ( (ObjectCreationType)( ( ( x ) & 0x0F000000'00000000 ) >> 56 ) )

// Mask for an object's asset type
#define OBJECT_ASSET_TYPE( x ) ( (AssetType)( ( ( x ) & 0xF0000000'00000000 ) >> 60 ) )


enum PlayerNameReserveType
{
  RESERVE_SUCCESS,
  RESERVE_FAILED,
  RESERVE_UNAVAILABLE,
  RESERVE_RESTRICTED
};


// NOTE: These values are serialized to database entries
enum ServiceType
{
  // Client and Server
  Shared_ServiceType = 0,

  // Server only
  Server_ServiceType = 1,

  // Client only
  Client_ServiceType = 2
};


// NOTE: These values are serialized to database entries
enum ClientReplicationType
{
  // Server-side changes are not replicated to clients
  None_ClientReplicationType = 0,

  // Server-side changes are shared privately with the client controlling the involved player
  Private_ClientReplicationType = 1,

  // Server-side changes are replicated to all nearby clients (all clients on the same map)
  Regional_ClientReplicationType = 2,

  // Server-side changes are replicated to all clients
  Global_ClientReplicationType = 3
};


enum rumAnimationType
{
  StandardLooping_AnimationType = 0, // Standard frame advancement on an interval, automatically restarts
  StandardOnce_AnimationType    = 1, // Standard frame advancement on an interval, no automatic restart
  Random_AnimationType          = 2, // The next frame is randomly chosen on an interval
  Custom_AnimationType          = 3  // A script callback is executed on an interval
};


enum rumCollisionType
{
  None_CollisionType,
  Tile_CollisionType,
  Pawn_CollisionType,
  Error_CollisionType
};


enum rumDirectionType
{
  // Supports North, East, South, and West directions only
  Cardinal_DirectionType,

  // Supports NorthEast, SouthEast, SouthWest, and NorthWest directions (in addition to cardinal directions)
  Intercardinal_DirectionType
};


enum rumMoveFlags
{
  Default_MoveFlag = 0x0,
  IgnoreTileCollision_MoveFlag = 0x1,
  IgnorePawnCollision_MoveFlag = 0x2,
  IgnoreDistance_MoveFlag = 0x4,
  Test_MoveFlag = 0x8
};


enum rumMoveResultType
{
  Success_MoveResultType,
  Fail_MoveResultType,
  TileCollision_MoveResultType,
  PawnCollision_MoveResultType,
  OffMap_MoveResultType,
  TooFar_MoveResultType,
  Error_MoveResultType
};

#endif // _U_ENUM_H_
