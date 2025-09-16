enum AccountMenu
{
  Login,
  CreateAccount,
  Back,
  ExitGame
}

enum CharGen
{
  Name,
  Validation,
  Gender,
  CreateMode,
  SelectChar,
  Story,
  Gypsy,
  TestDone,
  Create
}

enum ConnectMenu
{
  Server,
  Port,
  Connect
}

enum CreateMode
{
  Standard,
  Quick
}

enum GameMode
{
  Title,
  CharGen,
  Transition,
  Game,
  Map,
  Vision,
  Settings
}

enum InputMode
{
  Unset,
  Game,
  Text_Input,
  NPC_Chat,
  Player_Chat,
  Direction,
  Yes_No_Question,
  Amount,
  Text_Response,
  List_Selection,
  Slider,
  Seer_Chat,
  Target,
  PressAnyKey,
  NumModes
}

enum Intro
{
  CircleFar,
  Moongate,
  Circle,
  FaireFar,
  Faire,
  Wagon,
  Gypsy,
  Abacus
}

enum LoginMenu
{
  Name,
  Password,
  Login,
  Cancel
}

enum MainMenu
{
  JourneyOnward,
  InitiateGame,
  DeleteChar,
  Logout,
  Quit

  // TODO - Edit character for delete/rename? etc.
}

enum MapLabelType
{
  First    = 0,
  MapName  = 0,
  WindDir  = 1,
  Position = 2,
  Last     = 2
}

enum MoongateAnimFrame
{
  Closed,
  QuarterOpen,
  HalfOpen,
  ThreeQuarterOpen,
  FullyOpen
}

enum NewAccountMenu
{
  Name,
  Email,
  Password,
  Verify,
  Create,
  Cancel
}

enum QuickChar
{
  Mage_Wizard_Wizard,
  Mage_Alchemist_Wizard,
  Bard_Thief_Thief,
  Bard_Lark_Thief,
  Fighter_Fighter_Fighter,
  Fighter_Barbarian_Fighter,
  Druid_Druid_Wizard,
  Druid_Wizard_Wizard,
  Tinker_Barbarian_Fighter,
  Tinker_Fighter_Fighter,
  Paladin_Paladin_Cleric,
  Paladin_Cleric_Cleric,
  Ranger_Ranger_Thief,
  Ranger_Thief_Thief,
  Shepherd_Cleric_Cleric,
  Shepherd_Illusionist_Cleric,
  NumQuickChars
}

enum TitleStages
{
  Splash,
  VerifyClientVersion,
  VerifyUltimaInstall,
  Connect,
  VerifyGameFiles,
  AccountMenu,
  Login,
  MainMenu,
  CharSelect,
  DeleteChar,
  Done
}

enum VerifyUltimaMenu
{
  Path,
  Browse,
  Exit
}
