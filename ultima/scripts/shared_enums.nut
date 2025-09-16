enum ActionDelay
{
  // Enumeration values can be integers, floats or string literals
  None   = 0,
  Short  = 1,
  Medium = 2,
  Long   = 3
}

enum AlignmentType
{
  Good,
  Neutral,
  Evil
}

// Warning: Serialized to db!
enum ArticleType
{
  none,
  article_indef_sing_cons,
  article_indef_sing_vowel,
  article_definite
}

enum AttackResultType
{
  Hit        = 0,
  Miss       = 1,
  Unaffected = 2
}

enum AttackType
{
  Directional,
  Targeted
}

enum BuySellResponse
{
  Buy  = 1,
  Sell = 2
}

enum ClientEffectType
{
  Damage,
  Cast,
  ScreenShake
}

enum Condition
{
  Good,
  Meditating,
  Resting,
  Sleeping,
  Unconscious,
  Dead,
  Frozen
}

enum DialogueTerminationType
{
  Standard     = 0,
  TurnsAway    = 1,
  Interrupted  = 2,
  Disconnected = 3
}

enum Direction
{
  Start     = 0,
  West      = 0,
  Northwest = 1,
  North     = 2,
  Northeast = 3,
  East      = 4,
  Southeast = 5,
  South     = 6,
  Southwest = 7,
  End       = 8,
  None      = 8,
  Num       = 9
}

enum EffectType
{
  Jinxed,
  Burning,
  Negated,
  Peering,
  Poisoned,
  Protected,
  Quickened,
  DaemonImmune,
  Starving,
  Unconscious,
  Frozen,
  Lighting
}

enum FlyingStateType
{
  Landed,
  Flying
}

enum FountainState
{
  Flowing,
  Dry
}

enum GameType
{
  Invalid = 0,
  Ultima1 = 1,
  Ultima2 = 2,
  Ultima3 = 3,
  Ultima4 = 4
}

enum GemImmortalityState
{
  Invulnerable,
  Vulnerable,
  Destroyed
}

enum GenderType
{
  Male,
  Female,
  Other,
  Unspecified,
  NumGenderTypes
}

enum HorseStateType
{
  FacingLeft,
  FacingRight
}

// Warning: Serialized to db!
enum InventoryType
{
  Standard = 0,
  Weapon   = 1,
  Armour   = 2
}

enum MapType
{
  Invalid = -1,
  World,
  Village,
  Towne,
  Keep,
  Castle,
  Shrine,
  Dungeon,
  Altar,
  Abyss,
  Tower,
  Space,
  SpaceStation,
  Misc,
  Cave,
  Codex
}

enum MerchantArmouryTransaction
{
  Greet,
  Purchase,
  Sell,
  ServerTerminated
}

enum MerchantGuildProducts
{
  Torches,
  Gems,
  Keys,
  Sextant
}

enum MerchantGuildTransaction
{
  Greet,
  Purchase,
  ServerTerminated
}

enum MerchantHealerService
{
  Cure,
  Heal,
  Resurrect,
  Potions
}

enum MerchantHealerTransaction
{
  Greet,
  Purchase,
  DonateBlood,
  ServerTerminated
}


enum MerchantInnTransaction
{
  Greet,
  NoVacancy,
  RoomPurchase,
  ServerTerminated
}

enum MerchantMagicTransaction
{
  Greet,
  Purchase,
  ServerTerminated
}

enum MerchantOracleTransaction
{
  Greet,
  Hint,
  RequestNext,
  ServerTerminated
}

enum MerchantRationTransaction
{
  Greet,
  Purchase,
  ServerTerminated
}

enum MerchantReagentTransaction
{
  Greet,
  Purchase,
  ServerTerminated
}

enum MerchantStableTransaction
{
  Greet,
  Purchase,
  ServerTerminated
}

enum MerchantTavernTransaction
{
  Greet,
  PurchaseFood,
  PurchaseAle,
  Question,
  Tip,
  ServerTerminated
}

enum MerchantTransportTransaction
{
  Greet,
  Purchase,
  ServerTerminated
}

enum MerchantWeaponryTransaction
{
  Greet,
  Purchase,
  Sell,
  ServerTerminated
}

enum MondainState
{
  Aggressive,
  Passive,
  Transformed,
  Corpse
}

enum MoongateState
{
  Closed,
  Open
}

// Warning: These are serialized to maps!
enum MoonPhase
{
  New            = 0,
  CrescentWaxing = 1,
  FirstQuarter   = 2,
  GibbousWaxing  = 3,
  Full           = 4,
  GibbousWaning  = 5,
  LastQuarter    = 6,
  CrescentWaning = 7,
  NumPhases      = 8
}

// This table must exist
enum MoveType
{
  Terrestrial = 0x1,
  Aquatic     = 0x2,
  Sails       = 0x4,
  Ballistic   = 0x8,
  Aerial      = 0x10,
  Drifts      = 0x20,
  Incorporeal = 0x40,
  Mutable     = 0x80,
  Stationary  = 0x100,
  Celestial   = 0x200,
  Hovers      = 0x400,
  Flies       = 0x800,
  Blinks      = 0x1000
}

enum ObjectState
{
  Unused   = 0,
  Used     = 1,
  Unopened = 0,
  Opened   = 1,
  Off      = 0,
  On       = 1,
  False    = 0,
  True     = 1,
  Disabled = 0,
  Enabled  = 1
}

enum PartyBroadcastType
{
  Invitation,
  InvitationAccepted,
  InvitationDeclined,
  InvitationInvalid,
  InvitationExpired
  PlayerJoined,
  PlayerLeft,
  PlayerDismissed,
  RosterUpdate,
  NewLeader,
  IDMessage,
  StringMessage
}

enum PlayerTransactionType
{
  Tell,
  Give,
  PartyInvite,
  PartyDismiss,
  PartyPromote,
  NumTransactionTypes
}

enum PortalUsageResultType
{
  Success,
  NoTransports,
  EightPartAvatar,
  NotFound,
  Invalid
}

enum PortalUsageType
{
  Undef   = 0x0,
  Enter   = 0x1,
  Klimb   = 0x2,
  Descend = 0x4
}

enum PostureType
{
  // Warning: These are serialized to maps!
  Idle,
  Wandering,
  Attack,
  Flee
}

enum PronounType
{
  He,
  She,
  It,
  Child
}

enum PropertyType
{
  Int     = 0,
  Integer = 0,
  Bool    = 1,
  Boolean = 1,
  Float   = 2,
  Double  = 2,
  String  = 3,
  Class   = 4
}

enum ResurrectionType
{
  Void,
  Body,
  EnergyField,
  Spell,
  Other
}

enum SharedTradeType
{
  Property,
  Inventory
}

enum ShipStateType
{
  FacingWest,
  FacingNorth,
  FacingEast,
  FacingSouth
}

enum SpellSchool
{
  Divine,
  Arcane,
  Both
}

enum TradeBroadcastType
{
  GiveNotify,
  Give,
  GiveSent,
  GiveReceived
}

enum TransportCommandType
{
  Movement,
  ShuttleLaunch,
  ShuttleLaunchAbort,
  TimeMachineLaunchMondain,
  TimeMachineLaunchEnd,
  TimeMachineLaunchFailed,
  HyperJump
}

// Warning: These are serialized to players in the database! Do not change their order!
enum TransportType
{
  None,
  Horse,
  Cart,
  Skiff, // or Raft
  Ship,  // or Frigate
  Aircar,
  SpaceShip,
  TimeMachine,
  Plane,
  Rocket,
  Balloon
}

enum VerticalDirectionType
{
  None,
  Up,
  Down
}

enum YesNoResponse
{
  Yes = 1,
  No  = 2
}
