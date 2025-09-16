/*----------------------------------------------------------------------------

ooooo  oooo o888    o8    o88                                ooooo ooooo  oooo
 888    88   888  o888oo  oooo  oo ooo oooo     ooooooo       888   888    88
 888    88   888   888     888   888 888 888    ooooo888      888    888  88
 888    88   888   888     888   888 888 888  888    888      888     88888
  888oo88   o888o   888o  o888o o888o888o888o  88ooo88 8o    o888o     888
                                   _             _
      _/ /  _   _     _  _ _/     (_  _/ /  _   /_|     _ _/ _  _
      / /) (-  (/ (/ (- _) /   () /   / /) (-  (  | \/ (/ / (/ /
               /
----------------------------------------------------------------------------*/

// Ultima IV shared enums

// The number of torches required to build a campfire (used by Hole Up & Camp command)
const g_iTorchesForCampfire = 5;

enum Reagents
{
  Sulphurous_Ash,
  Ginseng,
  Garlic,
  Spider_Silk,
  Blood_Moss,
  Black_Pearl,
  Nightshade,
  Mandrake_Root,
  NumReagents
}

enum MixtureFlags
{
  None           = 0x0,
  Sulphurous_Ash = 0x1,
  Ginseng        = 0x2,
  Garlic         = 0x4,
  Spider_Silk    = 0x8,
  Blood_Moss     = 0x10,
  Black_Pearl    = 0x20,
  Nightshade     = 0x40,
  Mandrake_Root  = 0x80
}

enum CarveState
{
  Init,
  SymbolPrompt,
  SymbolAnswer,
  SymbolWrong,
  MaterialNeeded,
  Success,
  RuneOwned
}

enum GiveState
{
  Init,
  AmountPrompt,
  AmountResponse,
  Reaction
}

enum OrbState
{
  Unused,
  Used
}

enum OrbType
{
  // Warning: These are serialized to maps!
  Strength = 0x1,
  Dexterity = 0x2,
  Intelligence = 0x4
}

enum RuneMaterialType
{
  Gemstone,    // Honesty
  Bamboo,      // Compassion
  Dragonscale, // Valor
  YewWood,     // Justice
  Soapstone,   // Sacrifice
  Granite,     // Honor
  Ivory,       // Spirituality
  Bone         // Humility
}

enum StoneType
{
  Blue,   // Honesty
  Yellow, // Compassion
  Red,    // Valor
  Green,  // Justice
  Orange, // Sacrifice
  Purple, // Honor
  White,  // Spirituality
  Black   // Humility
}

enum TrapState
{
  Active,
  Disarmed
}

enum PrincipleType
{
  Truth,
  Love
  Courage,
  NumPrinciples
}

enum VirtueType
{
  Honesty,
  Compassion,
  Valor,
  Justice,
  Sacrifice,
  Honor,
  Spirituality,
  Humility,
  NumVirtues
}

enum U4_AbyssAltarPhaseType
{
  Virtue,
  Stone,
  Codex
}

enum U4_AbyssCodexPhaseType
{
  ChamberDoor,
  WordOfPassage,
  PassageNotGranted,
  PassageGranted,
  VirtuesPassed,
  PrinciplesPassed,
  TestFailed,
  AxiomTestFailed,
  Virtue1,
  Virtue2,
  Virtue3,
  Virtue4,
  Virtue5,
  Virtue6,
  Virtue7,
  Virtue8,
  Principle1,
  Principle2,
  Principle3,
  Axiom,
  Enlightenment,
  Endgame1,
  Endgame2,
  Endgame3,
  Endgame4,
  Endgame5,
  Endgame6,
  Endgame7,
  Endgame8,
  Endgame9,
  Endgame10,
  Complete
}

enum U4_SeerTalkType
{
  Greet,
  Response
}

enum U4_LBTalkType
{
  Greet,
  Standard,
  LevelUp
}

// These apply to the properties affecting the bell, book, candle, horn, and skull
enum U4_QuestItemState
{
  NotFound   = 0,
  Found      = 1,
  Imbued     = 2,
  Abyss_Used = 3
}

enum U4_Meditation_Phase
{
  Begin         = 0,
  Cycle1        = 1,
  Cycle2        = 2,
  Cycle3        = 3,
  Vision        = 4,
  Elevation     = 5,
  Done          = 6
}

enum U4_MagicFieldType
{
  Fire,
  Poison,
  Lightning,
  Sleep
}
