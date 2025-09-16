/*

----------------------------------------------------------------------------

ooooo  oooo o888    o8    o88                                ooooo ooooo  oooo
888    88   888  o888oo  oooo  oo ooo oooo     ooooooo       888   888    88
888    88   888   888     888   888 888 888    ooooo888      888    888  88
888    88   888   888     888   888 888 888  888    888      888     88888
888oo88   o888o   888o  o888o o888o888o888o  88ooo88 8o    o888o     888
                                     _              _
      _/  /  _   _     _   _ _/     (_  _/  /  _   /_|     _ _/  _  _
      /  /) (-  (/ (/ (- _)  /   () /   /  /) (-  (  | \/ (/ /  (/ /
                /
----------------------------------------------------------------------------

Classic Ultima Online (CUO)

MIT License

Copyright 2015 Jonathon Blake Wood-Brooks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Ultima is a registered trademark of Electronic Arts (EA)

Ultima IV: Quest of the Avatar
Developer:  Origin Systems
Publisher:  Origin Systems
Designers:  Richard Garriott, Ken W. Arnold

*/

g_eU4CodexKeyStringArray <-
[
  u4_item_key1_client_StringID,
  u4_item_key2_client_StringID,
  u4_item_key3_client_StringID
]

g_eU4ReagentStringArray <-
[
  u4_reagent_sulphurous_ash_client_StringID,
  u4_reagent_ginseng_client_StringID,
  u4_reagent_garlic_client_StringID,
  u4_reagent_spider_silk_client_StringID,
  u4_reagent_blood_moss_client_StringID,
  u4_reagent_black_pearl_client_StringID,
  u4_reagent_nightshade_client_StringID,
  u4_reagent_mandrake_root_client_StringID
]

g_eU4RuneMaterialStringArray <-
[
  u4_rune_material0_client_StringID,
  u4_rune_material1_client_StringID,
  u4_rune_material2_client_StringID,
  u4_rune_material3_client_StringID,
  u4_rune_material4_client_StringID,
  u4_rune_material5_client_StringID,
  u4_rune_material6_client_StringID,
  u4_rune_material7_client_StringID
]

g_eU4StatPropertyArray <-
[
  U4_Level_PropertyID,
  U4_Experience_PropertyID,
  U4_Strength_PropertyID,
  U4_Dexterity_PropertyID,
  U4_Intelligence_PropertyID
]

g_eU4StoneStringArray <-
[
  u4_stone_color0_client_StringID,
  u4_stone_color1_client_StringID,
  u4_stone_color2_client_StringID,
  u4_stone_color3_client_StringID,
  u4_stone_color4_client_StringID,
  u4_stone_color5_client_StringID,
  u4_stone_color6_client_StringID,
  u4_stone_color7_client_StringID
]


function U4_Animate()
{
  // Convert wind direction to a direction vector
  local eDir;

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer )
  {
    eDir = ciPlayer.GetProperty( U4_Wind_Direction_Override_PropertyID, Direction.None );
  }

  local vDir;
  if( Direction.None != eDir )
  {
    vDir = GetDirectionVector( eDir );
  }
  else
  {
    vDir = GetDirectionVector( g_ciCUO.m_eWindDirection );
  }

  // Shift water in the direction of the wind
  ::rumGetGraphic( U4_Water_GraphicID ).Shift( vDir );
  ::rumGetGraphic( U4_Water_Deep_GraphicID ).Shift( vDir );
  ::rumGetGraphic( U4_Water_Shoals_GraphicID ).Shift( vDir );

  // Shift other scrolling tiles down
  vDir = rumVector( 0, 1 );
  ::rumGetGraphic( U4_Field_Energy_GraphicID ).Shift( vDir );
  ::rumGetGraphic( U4_Field_Fire_GraphicID ).Shift( vDir );
  ::rumGetGraphic( U4_Field_Force_GraphicID ).Shift( vDir );
  ::rumGetGraphic( U4_Field_Lightning_GraphicID ).Shift( vDir );
  ::rumGetGraphic( U4_Field_Poison_GraphicID ).Shift( vDir );
  ::rumGetGraphic( U4_Field_Sleep_GraphicID ).Shift( vDir );
  //::rumGetGraphic( U4_Phantom_Field_GraphicID ).Shift( vDir );
  ::rumGetGraphic( U4_Lava_GraphicID ).Shift( vDir );
  ::rumGetGraphic( U4_Projectile_Lava_GraphicID ).Shift( vDir );
}


// Callback from when a player must type a keyword response to an npc, like when the symbol of a virtue is asked
function U4_CarveSymbolResponse( i_strResponse )
{
  g_ciUI.m_ciGameInputTextBox.Clear();

  if( i_strResponse != null && i_strResponse != "" )
  {
    ShowString( i_strResponse );

    local ciBroadcast = ::rumCreate( Player_Talk_Carve_BroadcastID, CarveState.SymbolAnswer, i_strResponse.tolower() );
    ::rumSendBroadcast( ciBroadcast );
  }
  else
  {
    U4_NPCPromptInterest();
  }
}


function U4_CastSpell()
{
  local strDesc = ::rumGetString( command_cast_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  local ciPlayer = ::rumGetMainPlayer();

  if( ciPlayer.IsNegated() || ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local ePlayerClassID = ciPlayer.GetProperty( U4_PlayerClass_PropertyID, U4_Mage_Class_CustomID );
  local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );

  local bCastsSpells = ciPlayerClass.GetProperty( Class_Casts_Spells_PropertyID, false );
  if( !bCastsSpells )
  {
    local strClass = ::rumGetStringByName( ciPlayerClass.GetName() + "_client_StringID" );
    strDesc = format( ::rumGetString( msg_cast_restricted_client_StringID ), strClass );
    ShowString( strDesc, g_strColorTagArray.Red );
    return;
  }

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.025|0.1" );

  // Build the list of spells
  foreach( iIndex, eSpellID in g_eU4SpellArray )
  {
    local ciSpell = ::rumGetCustomAsset( eSpellID );
    local strSpell = ::rumGetStringByName( ciSpell.GetName() + "_client_StringID" );
    local strAlpha = ::rumGetStringByName( format( "alpha%02d_client_StringID", iIndex ) );
    local strEntry = format( "%s|%s", strAlpha, strSpell );
    g_ciUI.m_ciGameListView.SetEntry( iIndex, strEntry, rumKeypress.KeyA() + iIndex );
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U4_CastSpellSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = U4_CastSpellFinished;
  g_ciUI.m_ciGameListView.m_funcIndexChanged = Ultima_CastSpellSelectionChanged;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  // Show spell mixture quantities
  g_ciUI.m_ePreviousStatPage = g_ciUI.m_eCurrentStatPage;
  U4_Stat_Update( U4_StatPage.Mixtures );
}


function U4_CastSpellFinished( i_eDelay = ActionDelay.Short )
{
  // Restore the player's previous stat page
  U4_Stat_Update( g_ciUI.m_ePreviousStatPage );
  Ultima_ListSelectionEnd( i_eDelay );
}


function U4_CastSpellSelected()
{
  local strText = g_ciUI.m_ciGameListView.GetCurrentEntry();
  local strArray = split( strText, "|" );
  ShowString( format( "%s: %s", ::rumGetString( token_spell_client_StringID ), strArray[1] ) );

  local eKey = g_ciUI.m_ciGameListView.GetSelectedKey();
  local eSpellID = g_eU4SpellArray[eKey];
  local ciPlayer = ::rumGetMainPlayer();
  local bDone = ciPlayer.CastSpell( eSpellID );

  if( bDone )
  {
    U4_CastSpellFinished();
  }
  else if( U4_Energy_Field_Spell_CustomID == eSpellID )
  {
    g_ciUI.m_ciGameListView.SetActive( true );
    g_ciUI.m_ciGameListView.DisableMultiSelect();
    g_ciUI.m_ciGameListView.Focus();
  }
  else
  {
    g_ciUI.m_ciGameListView.SetActive( false );
    g_ciUI.m_ciGameListView.Clear();
    g_ciUI.m_ciGameListView.DisableMultiSelect();
    g_ciUI.m_ciGameListView.m_funcAccept = null;
    g_ciUI.m_ciGameListView.m_funcCancel = null;
    g_ciUI.m_ciGameListView.m_funcIndexChanged = OnGameListViewIndexChanged;

    g_ciUI.m_ciGameInputTextBox.ShowPrompt( true );
    g_ciUI.m_ciGameInputTextBox.Focus();
  }
}


// Callback from when a player enters an amount when giving gold to a beggar
function U4_GiveGold( i_iAmount )
{
  g_ciUI.m_ciGameInputTextBox.Clear();

  if( i_iAmount != null )
  {
    ShowString( i_iAmount.tostring() );

    if( i_iAmount > 0 )
    {
      local ciBroadcast = ::rumCreate( Player_Talk_Give_BroadcastID, GiveState.AmountResponse, i_iAmount );
      ::rumSendBroadcast( ciBroadcast );
      return;
    }
  }

  U4_NPCPromptInterest();
}


function U4_HoleUpAndCamp()
{
  local ciPlayer = ::rumGetMainPlayer();

  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_hole_up_client_StringID ) );
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  if( ciPlayer.GetTransportID() != rumInvalidGameID )
  {
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_hole_up_client_StringID ) );
    ShowString( ::rumGetString( msg_only_on_foot_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local ciMap = ciPlayer.GetMap();

  // The player can't be on the same space as another pawn
  local ciPosData = ciMap.GetPositionData( ciPlayer.GetPosition() );
  if( ciPosData.GetNumObjects() > 1 )
  {
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_hole_up_client_StringID ) );
    ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
  if( MapType.Dungeon != eMapType &&
      MapType.World != eMapType   &&
      MapType.Abyss != eMapType   &&
      MapType.Altar != eMapType   &&
      MapType.Cave != eMapType )
  {
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_hole_up_client_StringID ) );
    ShowString( ::rumGetString( msg_outdoors_only_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction, U4_HoleUpAndCampCallback );

  local strDesc = format( "%s: <%s>",
                          ::rumGetString( command_hole_up_client_StringID ),
                          ::rumGetString( token_direction_client_StringID ) );
  g_ciUI.m_ciGameInputTextBox.SetText( strDesc );
}


function U4_HoleUpAndCampCallback( i_eDir )
{
  local ciPlayer = ::rumGetMainPlayer();
  local ciPos = ciPlayer.GetPosition() + GetDirectionVector( i_eDir );
  local ciMap = ciPlayer.GetMap();

  local eDelay = ActionDelay.Short;

  // Does a campfire already exist?
  local bSuccess = false;
  local ciCampfire;
  local ciPosData = ciMap.GetPositionData( ciPos );
  while( ciCampfire = ciPosData.GetNext( rumWidgetPawnType, U4_Spit_WidgetID ) )
  {
    if( ciCampfire.IsVisible() )
    {
      bSuccess = true;
      eDelay = ActionDelay.Long;
      ciPosData.Stop();
    }
  }

  if( !bSuccess )
  {
    // Verify that nothing is blocking the new campfire
    if( ( ciPosData.GetNumObjects() == 0 ) &&
        ( ciMap.MovePawn( ciPlayer, ciPos, rumTestMoveFlag ) == rumSuccessMoveResultType ) )
    {
      // If the player isn't on the world map, there's a cost to building a fire
      local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
      if( ( MapType.World == eMapType ) || ciPlayer.GetProperty( U4_Torches_PropertyID, 0 ) >= g_iTorchesForCampfire )
      {
        bSuccess = true;
        eDelay = ActionDelay.Long;
      }
      else
      {
        local strDesc = format( ::rumGetString( msg_camp_req_torches_client_StringID ), g_iTorchesForCampfire );
        ShowString( strDesc, g_strColorTagArray.Red );
      }
    }
    else
    {
      ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
    }
  }

  if( bSuccess )
  {
    local ciBroadcast = ::rumCreate( Player_Camp_BroadcastID, i_eDir );
    ::rumSendBroadcast( ciBroadcast );
  }

  BlockInput( ciPlayer.GetActionDelay( eDelay ) );
}


function U4_Init()
{}


function U4_KeyPressedGame( i_ciKeyboard )
{
  local eKey = i_ciKeyboard.GetKey();

  if( rumKeypress.KeyA() == eKey )
  {
    local bForceRanged = i_ciKeyboard.ShiftPressed();
    Ultima_Attack( bForceRanged );
  }
  else if( rumKeypress.KeyB() == eKey )
  {
    Ultima_Board();
  }
  else if( rumKeypress.KeyC() == eKey )
  {
    U4_CastSpell();
  }
  else if( rumKeypress.KeyD() == eKey )
  {
    Ultima_Descend();
  }
  else if( rumKeypress.KeyE() == eKey )
  {
    local ciPlayer = ::rumGetMainPlayer();
    if( ciPlayer.IsFlying() )
    {
      ShowString( ::rumGetString( msg_must_land_client_StringID ), g_strColorTagArray.Red );
    }
    else
    {
      UsePortal( PortalUsageType.Enter );
    }
  }
  else if( rumKeypress.KeyF() == eKey )
  {
    Ultima_FireTransportWeapon();
  }
  else if( rumKeypress.KeyG() == eKey )
  {
    Ultima_Get( U4_Chest_WidgetID );
  }
  else if( rumKeypress.KeyH() == eKey )
  {
    U4_HoleUpAndCamp();
  }
  else if( rumKeypress.KeyI() == eKey )
  {
    if( i_ciKeyboard.AltPressed() )
    {
      Ultima_ExtinguishLight();
    }
    else
    {
      Ultima_IgniteTorch( U4_Torches_PropertyID );
    }
  }
  else if( rumKeypress.KeyJ() == eKey )
  {
    Ultima_JimmyLock( U4_Door_Widget, U4_Keys_PropertyID );
  }
  else if( rumKeypress.KeyK() == eKey )
  {
    Ultima_Klimb();
  }
  else if( rumKeypress.KeyL() == eKey )
  {
    if( i_ciKeyboard.AltPressed() )
    {
      Ultima_AdvanceMapLabel();
    }
    else
    {
      Ultima_Look();
    }
  }
  else if( rumKeypress.KeyM() == eKey )
  {
    if( i_ciKeyboard.AltPressed() )
    {
      local ciPlayer = ::rumGetMainPlayer();
      if( !( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() ) )
      {
        g_ciUI.m_eOverlayGraphicID = U4_Map_GraphicID;
        g_ciUI.m_ciOverlayGraphicOffset = rumPoint( 0, 0 );
        g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, OnOverlayGraphicDone );
      }
    }
    else
    {
      // Mix Reagents
      U4_MixSpell();
    }
  }
  else if( rumKeypress.KeyN() == eKey )
  {
    local bOnSelf = true;
    if( i_ciKeyboard.AltPressed() )
    {
      bOnSelf = false;
    }

    Ultima_NipPotion( U4_Potions_PropertyID, bOnSelf );
  }
  else if( rumKeypress.KeyO() == eKey )
  {
    Ultima_Open( U4_Door_Widget );
  }
  else if( rumKeypress.KeyP() == eKey )
  {
    Ultima_Peer( U4_Gems_PropertyID );
  }
  else if( rumKeypress.KeyQ() == eKey )
  {
    Ultima_Quit();
  }
  else if( rumKeypress.KeyR() == eKey )
  {
    Ultima_ReadyWeapon();
  }
  else if( rumKeypress.KeyS() == eKey )
  {
    Ultima_Search();
  }
  else if( rumKeypress.KeyT() == eKey )
  {
    Ultima_Talk( command_talk_client_StringID );
  }
  else if( rumKeypress.KeyU() == eKey )
  {
    U4_UseItem();
  }
  else if( rumKeypress.KeyV() == eKey )
  {
    Ultima_Volume();
  }
  else if( rumKeypress.KeyW() == eKey )
  {
    Ultima_WearArmour();
  }
  else if( rumKeypress.KeyX() == eKey )
  {
    Ultima_Xit();
  }
  else if( rumKeypress.KeyY() == eKey )
  {
    Ultima_Yell();
  }
  else if( rumKeypress.KeyZ() == eKey )
  {
    if( i_ciKeyboard.AltPressed() || i_ciKeyboard.CtrlPressed() )
    {
      // Alt-Z always shows the main stat panel
      U4_Stat_Update( U4_StatPage.Main );
    }
    else
    {
      U4_Stat();
    }
  }
  else if( rumKeypress.KeyTab() == eKey )
  {
    Ultima_AdvanceMapLabel();
  }
  else if( ( rumKeypress.KeyPadSubtract() == eKey ) || ( rumKeypress.KeyDash() == eKey ) )
  {
    Ultima_Resurrect( ResurrectionType.Void, GameType.Ultima4 );
  }
  else if( ( rumKeypress.KeyPadAdd() == eKey ) || ( rumKeypress.KeyEquals() == eKey ) )
  {
    Ultima_Resurrect( ResurrectionType.Body, GameType.Ultima4 );
  }
}


function U4_MixAmount()
{
  g_ciUI.m_ciGameInputTextBox.Clear();

  local eReagentArray = g_ciUI.m_ciGameListView.GetSelectedKeys();
  local eSpellID =  g_ciUI.m_ciGameInputTextBox.m_vPayload["slot1"]

  Ultima_ListSelectionEnd();

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    U4_MixSpellEnd();
    return;
  }

  // Convert the reagent array to bitflags
  local iFlags = 0;
  foreach( eReagentID in eReagentArray )
  {
    iFlags = iFlags | ( 1 << eReagentID );
  }

  // Save the flags for later retrieval
  g_ciUI.m_ciGameInputTextBox.m_vPayload = { slot1 = eSpellID, slot2 = iFlags };

  local bCancel = false;

  if( eReagentArray.len() > 0 )
  {
    // Switch back to the mixtures stat page
    U4_Stat_Update( U4_StatPage.Mixtures );

    for( local i = 0; i < eReagentArray.len() && !bCancel; ++i )
    {
      local ePropertyID = g_eU4ReagentPropertyArray[eReagentArray[i]];
      local ciProperty = ::rumGetPropertyAsset( ePropertyID );
      local strDesc = ::rumGetStringByName( ciProperty.GetName() + "_Property_client_StringID" );
      ShowString( strDesc );

      local iNumOwned = ciPlayer.GetProperty( ePropertyID, 0 );
      if( 0 == iNumOwned )
      {
        ShowString( ::rumGetString( msg_none_owned_client_StringID ), g_strColorTagArray.Red );
        bCancel = true;
      }
    }

    if( !bCancel )
    {
      // Get how many mixtures the player wants to mix
      ShowString( "" );
      ShowString( ::rumGetString( msg_mix_amount_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, U4_MixFinalize );
      g_ciUI.m_ciGameInputTextBox.Focus();
    }
  }

  if( bCancel )
  {
    U4_MixSpellEnd();
  }
}


function U4_MixFinalize( i_iAmount )
{
  local eSpellID = g_ciUI.m_ciGameInputTextBox.m_vPayload["slot1"];
  local iReagentFlags = g_ciUI.m_ciGameInputTextBox.m_vPayload["slot2"];

  if( ( null == i_iAmount ) || ( 0 == i_iAmount ) )
  {
    U4_MixSpellEnd();
    return;
  }

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    U4_MixSpellEnd();
    return;
  }

  // Echo the amount
  ShowString( format( "%d<b>%s", i_iAmount, ::rumGetString( msg_mix_start_client_StringID ) ) );

  local ciBroadcast = ::rumCreate( Player_Mix_BroadcastID, eSpellID, iReagentFlags, i_iAmount );
  ::rumSendBroadcast( ciBroadcast );

  U4_MixSpellEnd( ActionDelay.Long );
}


function U4_MixReagents()
{
  // Push the selection
  local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();
  ShowString( strDesc );

  local iIndex = g_ciUI.m_ciGameListView.GetSelectedIndex();

  Ultima_ListSelectionEnd();

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    U4_MixSpellEnd();
    return;
  }

  local eSpellID = g_eU4SpellArray[iIndex];
  local ePropertyID = g_eU4SpellPropertyArray[iIndex];
  local ciProperty = ::rumGetPropertyAsset( ePropertyID );
  local iMixCount = ciPlayer.GetProperty( ePropertyID, 0 );
  if( iMixCount >= ciProperty.GetMaxValue() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    U4_MixSpellEnd();
    return;
  }

  g_ciUI.m_ciGameInputTextBox.m_vPayload = { slot1 = eSpellID };

  // Switch to the reagents stat page
  U4_Stat_Update( U4_StatPage.Reagents );

  strDesc = format( "%s:", ::rumGetString( token_reagents_client_StringID ) );
  ShowString( "" );
  ShowString( strDesc );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.EnableMultiSelect( 1, Reagents.NumReagents );
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );

  // Build the list of reagents
  foreach( i, ePropertyID in g_eU4ReagentPropertyArray )
  {
    local ciProperty = ::rumGetPropertyAsset( ePropertyID );
    local strEntry = ::rumGetStringByName( ciProperty.GetName() + "_Property_client_StringID" );
    g_ciUI.m_ciGameListView.SetEntry( i, strEntry, rumKeypress.KeyA() + i );
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U4_MixAmount;
  g_ciUI.m_ciGameListView.m_funcCancel = U4_MixSpellEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();
}


function U4_MixSpell()
{
  local strDesc = ::rumGetString( command_mix_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  Ultima_ListSelectionEnd();

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    return;
  }

  local ePlayerClassID = ciPlayer.GetProperty( U4_PlayerClass_PropertyID, U4_Mage_Class_CustomID );

  local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );
  local bCastsSpells = ciPlayerClass.GetProperty( Class_Casts_Spells_PropertyID, false );
  if( !bCastsSpells )
  {
    local strClass = ::rumGetStringByName( ciPlayerClass.GetName() + "_client_StringID" );
    strDesc = format( ::rumGetString( msg_mix_restricted_client_StringID ), strClass );
    ShowString( strDesc, g_strColorTagArray.Red );
    Ultima_ListSelectionEnd();
    return;
  }

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );

  // Build the list of spells
  foreach( iIndex, ePropertyID in g_eU4SpellPropertyArray )
  {
    local ciProperty = ::rumGetPropertyAsset( ePropertyID );
    local strEntry = ::rumGetStringByName( ciProperty.GetName() + "_Property_client_StringID" );
    g_ciUI.m_ciGameListView.SetEntry( iIndex, strEntry, rumKeypress.KeyA() + iIndex );
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U4_MixReagents;
  g_ciUI.m_ciGameListView.m_funcCancel = U4_MixSpellEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  strDesc = format( "%s:", ::rumGetString( msg_mix_spell_client_StringID ) );
  ShowString( strDesc );

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );

  // Switch to the mixtures stat page
  g_ciUI.m_ePreviousStatPage = g_ciUI.m_eCurrentStatPage;
  U4_Stat_Update( U4_StatPage.Mixtures );
}


function U4_MixSpellEnd( i_eDelay = ActionDelay.Short )
{
  // Restore the player's previous stat page
  U4_Stat_Update( g_ciUI.m_ePreviousStatPage );
  Ultima_ListSelectionEnd( i_eDelay );
}


function U4_NPCPromptInterest()
{
  if( g_ciUI.m_ciGameInputTextBox.m_eInputMode != InputMode.Yes_No_Question )
  {
    // Prompt player for more input
    if( 0 == g_ciCUO.m_uiCurrentTalkID )
    {
      // Lord British prompt
      local strDesc = format( "<b>%s", ::rumGetString( u4_lord_british_prompt3_client_StringID ) );
      ShowString( strDesc, g_strColorTagArray.Cyan );
    }
    else
    {
      local strDesc = format( "<b>%s:", ::rumGetString( talk_your_interest_client_StringID ) );
      ShowString( strDesc, g_strColorTagArray.Cyan );
    }

    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.NPC_Chat );
  }
}


function U4_NPCQuestion( i_strResponse, i_iQuestionIndex )
{
  ShowString( i_strResponse.slice( 0, i_iQuestionIndex ), g_strColorTagArray.Cyan );

  // Todo - u4 requires a keypress here

  local iYesIndex = i_strResponse.find( "<y>" );
  local iNoIndex = i_strResponse.find( "<n>" );

  ShowString( format( "<b>%s", i_strResponse.slice( i_iQuestionIndex + 3, iYesIndex ) ), g_strColorTagArray.Cyan );

  local strResponseArray = array( 2 );
  strResponseArray[0] = i_strResponse.slice( iYesIndex + 3, iNoIndex );
  strResponseArray[1] = i_strResponse.slice( iNoIndex + 3 );

  // Prompt for y/n question
  local strDesc = ::rumGetString( talk_you_say_client_StringID ) + ":";
  ShowString( strDesc );

  g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Yes_No_Question, U4_NPCQuestionCallback, strResponseArray );
}


function U4_NPCQuestionCallback( i_strResponseArray, i_strAnswer )
{
  g_ciUI.m_ciGameInputTextBox.ResetInputMode( false );

  local bValid = false;
  local bPrompt = true;

  if( i_strAnswer.len() > 0 )
  {
    // TODO: localize
    local strAnswer = i_strAnswer.tolower();
    if( "yes" == strAnswer )
    {
      ShowString( ::rumGetString( talk_yes_client_StringID ) );

      // Currently, the <bye> tag is only embedded in yes reponses, so it is only checked here!
      local strYes = i_strResponseArray[0];
      if( strYes.find( "<bye>" ) != null )
      {
        strYes = StripTags( strYes );
        bPrompt = false;
      }

      ShowString( format( "<b>%s", strYes ), g_strColorTagArray.Cyan );

      bValid = true;
    }
    else if( "no" == strAnswer )
    {
      ShowString( ::rumGetString( talk_no_client_StringID ) );
      local strNo = i_strResponseArray[1];
      ShowString( format( "<b>%s", strNo ), g_strColorTagArray.Cyan );

      bValid = true;
    }
  }

  if( bValid )
  {
    local ciPlayer = ::rumGetMainPlayer();

    local ciBroadcast = ::rumCreate( Player_Talk_Answer_BroadcastID, ciPlayer.lastKeyword, i_strAnswer );
    ::rumSendBroadcast( ciBroadcast );

    if( bPrompt )
    {
      U4_NPCPromptInterest();
    }
  }
  else
  {
    ShowString( ::rumGetString( talk_yes_or_no_client_StringID ), g_strColorTagArray.Red );
    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Yes_No_Question, U4_NPCQuestionCallback, i_strResponseArray );
  }
}


function U4_PlayerTransaction( i_ciTarget )
{
  if( ( null == i_ciTarget ) || !( i_ciTarget instanceof Player ) )
  {
    return;
  }

  local ciPlayer = ::rumGetMainPlayer();
  local bInParty = ciPlayer.PlayerInParty( i_ciTarget ) || ( ciPlayer == i_ciTarget );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );
  g_ciUI.m_ciGameListView.SetEntry( PlayerTransactionType.Tell, ::rumGetString( talk_tell_client_StringID ) );
  g_ciUI.m_ciGameListView.SetEntry( PlayerTransactionType.Give, ::rumGetString( talk_give_client_StringID ) );

  if( !bInParty )
  {
    g_ciUI.m_ciGameListView.SetEntry( PlayerTransactionType.PartyInvite,
                                      ::rumGetString( talk_invite_client_StringID ) );
  }
  else if( ciPlayer.GetID() == g_ciCUO.m_uiPartyLeaderID )
  {
    g_ciUI.m_ciGameListView.SetEntry( PlayerTransactionType.PartyDismiss,
                                      ::rumGetString( talk_dismiss_client_StringID ) );
    g_ciUI.m_ciGameListView.SetEntry( PlayerTransactionType.PartyPromote,
                                      ::rumGetString( talk_promote_client_StringID ) );
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U4_PlayerTransactionTypeSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
  g_ciUI.m_ciGameInputTextBox.m_vPayload = i_ciTarget.GetID();
}


function U4_PlayerTransactionGive( i_ciTarget )
{
  local ciBroadcast = ::rumCreate( Player_Give_BroadcastID, TradeBroadcastType.GiveNotify, i_ciTarget.GetID() );
  ::rumSendBroadcast( ciBroadcast );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );

  for( local i = 0; i < U4_ClientTradeType.NumTradeTypes; ++i )
  {
    local strEntry = format( "u4_give%d_client_StringID", i );
    g_ciUI.m_ciGameListView.SetEntry( i, ::rumGetStringByName( strEntry ) );
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U4_PlayerTransactionGiveTypeSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  ciBroadcast = ::rumCreate( Player_Give_BroadcastID, TradeBroadcastType.Give, i_ciTarget.GetID() );
  ::rumSendBroadcast( ciBroadcast );

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
  g_ciUI.m_ciGameInputTextBox.m_vPayload = ciBroadcast;
}


function U4_PlayerTransactionGiveEquipment( i_ciBroadcast )
{
  i_ciBroadcast.SetItemSubtype( U4_ClientTradeType.Equipment );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );

  for( local i = 0; i < g_eU4EquipmentPropertyArray.len(); ++i )
  {
    local ciAsset = ::rumGetPropertyAsset( g_eU4EquipmentPropertyArray[i] );
    local strName = ::rumGetStringByName( ciAsset.GetName() + "_Property_client_StringID" );
    g_ciUI.m_ciGameListView.SetEntry( i, strName );
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U4_PlayerTransactionGiveSubtypeSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
  g_ciUI.m_ciGameInputTextBox.m_vPayload = i_ciBroadcast;

  U4_Stat_Update( U4_StatPage.Equipment );
}


function U4_PlayerTransactionGiveMixture( i_ciBroadcast )
{
  i_ciBroadcast.SetItemSubtype( U4_ClientTradeType.Mixture );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05|0.12" );

  local ciPlayer = ::rumGetMainPlayer();

  // Build the list of spells
  foreach( iIndex, ePropertyID in g_eU4SpellPropertyArray )
  {
    local iAmount = ciPlayer.GetProperty( ePropertyID, 0 );
    if( iAmount )
    {
      local ciProperty = ::rumGetPropertyAsset( ePropertyID );
      local strEntry = iAmount + "| - " + ::rumGetStringByName( ciProperty.GetName() + "_Property_client_StringID" );
      g_ciUI.m_ciGameListView.SetEntry( iIndex, strEntry );
    }
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U4_PlayerTransactionGiveSubtypeSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  /*local strDesc = format("%s:", ::rumGetString( msg_mix_spell_client_StringID ));
  ShowString(strDesc);*/

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
  g_ciUI.m_ciGameInputTextBox.m_vPayload = i_ciBroadcast;

  U4_Stat_Update( U4_StatPage.Mixtures );
}


function U4_PlayerTransactionGiveReagent( i_ciBroadcast )
{
  i_ciBroadcast.SetItemSubtype( U4_ClientTradeType.Reagent );

  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05|0.12" );

  local ciPlayer = ::rumGetMainPlayer();

  // Build the list of reagents
  foreach( iIndex, ePropertyID in g_eU4ReagentPropertyArray )
  {
    local iAmount = ciPlayer.GetProperty( ePropertyID, 0 );
    if( iAmount )
    {
      local ciProperty = ::rumGetPropertyAsset( ePropertyID );
      local strEntry = iAmount + "| - " + ::rumGetStringByName( ciProperty.GetName() + "_Property_client_StringID" );
      g_ciUI.m_ciGameListView.SetEntry( iIndex, strEntry );
    }
  }

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U4_PlayerTransactionGiveSubtypeSelected;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
  g_ciUI.m_ciGameInputTextBox.m_vPayload = i_ciBroadcast;

  U4_Stat_Update( U4_StatPage.Reagents );
}


function U4_PlayerTransactionGiveSubtypeSelected()
{
  local ciBroadcast = g_ciUI.m_ciGameInputTextBox.m_vPayload;
  local iIndex = g_ciUI.m_ciGameListView.GetSelectedIndex();

  Ultima_ListSelectionEnd();

  // Reset the subtype to a class based on the current subtype enum
  local ePropertyID = rumInvalidAssetID;
  switch( ciBroadcast.GetItemSubtype() )
  {
    case U4_ClientTradeType.Equipment:
      ePropertyID = g_eU4EquipmentPropertyArray[iIndex];
      break;

    case U4_ClientTradeType.Reagent:
      ePropertyID = g_eU4ReagentPropertyArray[iIndex];
      break;

    case U4_ClientTradeType.Mixture:
      ePropertyID = g_eU4SpellPropertyArray[iIndex];
      break;
  }

  if( ePropertyID != rumInvalidAssetID )
  {
    local ciProperty = ::rumGetPropertyAsset( ePropertyID );
    local strDesc = ::rumGetStringByName( ciProperty.GetName() + "_Property_client_StringID" );
    ShowString( strDesc );
    g_ciUI.m_ciGameInputTextBox.Clear();

    ciBroadcast.SetItemSubtype( ePropertyID );

    ShowString( ::rumGetString( talk_how_much_client_StringID ) );
    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, Ultima_PlayerTransactionGiveAmount, ciBroadcast );
    g_ciUI.m_ciGameInputTextBox.Focus();
  }
  else
  {
    ShowString( ::rumGetString( msg_none_owned_client_StringID ), g_strColorTagArray.Red );
  }
}


function U4_PlayerTransactionGiveTypeSelected()
{
  local ciBroadcast = g_ciUI.m_ciGameInputTextBox.m_vPayload;

  // Push the selection
  local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();
  ShowString( strDesc );
  g_ciUI.m_ciGameInputTextBox.Clear();

  local eType = g_ciUI.m_ciGameListView.GetSelectedKey();

  Ultima_ListSelectionEnd();

  switch( eType )
  {
    case U4_ClientTradeType.Food:
      ciBroadcast.SetItemType( SharedTradeType.Property );
      ciBroadcast.SetItemSubtype( U4_Food_PropertyID );
      ShowString( ::rumGetString( talk_how_much_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, Ultima_PlayerTransactionGiveAmount, ciBroadcast );
      g_ciUI.m_ciGameInputTextBox.Focus();
      break;

    case U4_ClientTradeType.Gold:
      ciBroadcast.SetItemType( SharedTradeType.Property );
      ciBroadcast.SetItemSubtype( U4_Gold_PropertyID );
      ShowString( ::rumGetString( talk_how_much_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, Ultima_PlayerTransactionGiveAmount, ciBroadcast );
      g_ciUI.m_ciGameInputTextBox.Focus();
      break;

    case U4_ClientTradeType.Equipment:
      ciBroadcast.SetItemType( SharedTradeType.Property );
      U4_PlayerTransactionGiveEquipment( ciBroadcast );
      break;

    case U4_ClientTradeType.Reagent:
      ciBroadcast.SetItemType( SharedTradeType.Property );
      U4_PlayerTransactionGiveReagent( ciBroadcast );
      break;

    case U4_ClientTradeType.Mixture:
      ciBroadcast.SetItemType( SharedTradeType.Property );
      U4_PlayerTransactionGiveMixture( ciBroadcast );
      break;

    case U4_ClientTradeType.Weapon:
      ciBroadcast.SetItemType( SharedTradeType.Inventory );
      Ultima_PlayerTransactionGiveWeapon( GameType.Ultima4, ciBroadcast );
      break;

    case U4_ClientTradeType.Armour:
      ciBroadcast.SetItemType( SharedTradeType.Inventory );
      Ultima_PlayerTransactionGiveArmour( GameType.Ultima4, ciBroadcast );
      break;
  }
}


function U4_PlayerTransactionTypeSelected()
{
  // Push the selection
  local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();
  ShowString( strDesc );
  g_ciUI.m_ciGameInputTextBox.Clear();

  local eType = g_ciUI.m_ciGameListView.GetSelectedKey();
  local uiPlayerID = g_ciUI.m_ciGameInputTextBox.m_vPayload;

  Ultima_ListSelectionEnd();

  if( rumInvalidGameID == uiPlayerID )
  {
    return;
  }

  local ciTarget = ::rumFetchPawn( uiPlayerID );
  if( null == ciTarget )
  {
    return;
  }

  switch( eType )
  {
    case PlayerTransactionType.Tell:
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Player_Chat );
      g_ciUI.m_ciGameInputTextBox.SetText( format( "/tell %s ", ciTarget.GetPlayerName() ) );
      g_ciUI.m_ciGameInputTextBox.Focus();
      break;

    case PlayerTransactionType.Give:
      U4_PlayerTransactionGive( ciTarget );
      break;

    case PlayerTransactionType.PartyInvite:
      local strCommand = "/party invite " + ciTarget.GetPlayerName();
      local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strCommand );
      ::rumSendBroadcast( ciBroadcast );
      Ultima_ListSelectionEnd();
      break;

    case PlayerTransactionType.PartyDismiss:
      local strCommand = "/party dismiss " + ciTarget.GetPlayerName();
      local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strCommand );
      ::rumSendBroadcast( ciBroadcast );
      Ultima_ListSelectionEnd();
      break;

    case PlayerTransactionType.PartyPromote:
      local strCommand = "/party promote " + ciTarget.GetPlayerName();
      local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strCommand );
      ::rumSendBroadcast( ciBroadcast );
      Ultima_ListSelectionEnd();
      break;
  }
}


function U4_RenderGame()
{
  local ciPlayer = ::rumGetMainPlayer();
  if( null == ciPlayer )
  {
    return;
  }

  ::rumGetGraphic( Border_GraphicID ).Draw( rumPoint() );

  local ciMap = ciPlayer.GetMap();
  if( null == ciMap )
  {
    return;
  }

  local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
  if( eMapType <= MapType.Shrine )
  {
    // Show moons
    local ciMoonPhase = ::rumGetGraphic( Moon_Phases_GraphicID );
    ciMoonPhase.DrawAnimation( g_ciUI.m_ciMoonDisplayPos, 0, g_ciCUO.m_eTrammelPhase );
    ciMoonPhase.DrawAnimation( rumPoint( g_ciUI.m_ciMoonDisplayPos.x + g_ciUI.s_iBorderPixelWidth,
                                         g_ciUI.m_ciMoonDisplayPos.y ),
                               0, g_ciCUO.m_eFeluccaPhase );
  }

  if( ( GameMode.Game == g_ciCUO.m_eCurrentGameMode ) && !RenderGameOverride() )
  {
    local bPeering = ( g_ciCUO.m_bAloft || g_ciCUO.m_bPeering );
    local ciPos = ciPlayer.GetPosition();
    local iIntensity = ciPlayer.GetScreenShakeIntensity().tointeger();

    // Determine pixel offset coordinate to where the map is drawn on screen
    local iOffsetX = g_ciUI.m_ciMapRect.p1.x;
    local iOffsetY = g_ciUI.m_ciMapRect.p1.y;
    local iRadius = g_ciCUO.s_iLOSRadius;

    if( bPeering )
    {
      iOffsetX = g_ciUI.m_ciPeerMapRect.p1.x;
      iOffsetY = g_ciUI.m_ciPeerMapRect.p1.y;
      iRadius = g_ciCUO.s_iLOSRadiusPeering;
    }

    if( iIntensity > 0 )
    {
      // Determine a random offset within the +/- iIntensity range
      iOffsetX += ( ( rand() % ( iIntensity * 2 + 1 ) ) - iIntensity );
      iOffsetY += ( ( rand() % ( iIntensity * 2 + 1 ) ) - iIntensity );
    }

    ciMap.Draw( rumPoint( iOffsetX, iOffsetY ), ciPos );

    RenderNamesAndVitals( ciPlayer, U4_Hitpoints_PropertyID, U4_Mana_PropertyID, bPeering, iRadius );
    RenderOffscreenMemberIcons( ciPlayer, U4_Graphic_ID_PropertyID, iRadius );
    RenderAttacks( ciPos, bPeering );
    RenderEffects( ciPlayer );
  }

  if( ciPlayer.IsDead() )
  {
    // All the rest of the rendering applies only to living players
    return;
  }

  local ciAnkhGraphic = ::rumGetGraphic( U4_Ankh_UI_GraphicID );
  local eVirtue = ciPlayer.GetProperty( U4_Virtue_Elevation_PropertyID, 0 );
  if( ::rumBitAllOn( eVirtue, 0xff ) )
  {
    // Draw the entire ankh graphic
    ciAnkhGraphic.Draw( g_ciUI.m_ciAnkhPos );
  }
  else if( eVirtue > 0 )
  {
    local iAnkhX = g_ciUI.m_ciAnkhPos.x;
    local iAnkhY = g_ciUI.m_ciAnkhPos.y;

    for( local i = 0; i < VirtueType.NumVirtues; ++i )
    {
      if( ::rumBitOn( eVirtue, i ) )
      {
        // Draw only the portion of the ankh for each attained virtue
        ciAnkhGraphic.DrawAnimation( rumPoint( iAnkhX, iAnkhY ), 0, i );
      }

      iAnkhY += 2;
    }
  }
}


function U4_RenderStatItems()
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_items_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local iOffset = 0;

  local ciPlayer = ::rumGetMainPlayer();

  local iFlags = ciPlayer.GetProperty( U4_Item_Rune_Materials_PropertyID, 0 );
  if( iFlags )
  {
    // Build list of rune materials
    for( local eVirtue = 0; eVirtue < VirtueType.NumVirtues; ++eVirtue )
    {
      if( ::rumBitOn( iFlags, eVirtue ) )
      {
        local strEntry = ::rumGetString( g_eU4RuneMaterialStringArray[eVirtue] );
        g_ciUI.m_ciStatListView.SetEntry( iOffset, strEntry );
        ++iOffset
      }
    }
  }

  local eState = ciPlayer.GetProperty( U4_Item_Bell_PropertyID, U4_QuestItemState.NotFound );
  if( eState )
  {
    local strEntry = "";

    if( U4_QuestItemState.Found == eState )
    {
      strEntry = ::rumGetString( u4_item_bell_client_StringID );
    }
    else if( U4_QuestItemState.Imbued == eState || U4_QuestItemState.Abyss_Used == eState )
    {
      strEntry = ::rumGetString( u4_item_bell_of_courage_client_StringID );
    }

    g_ciUI.m_ciStatListView.SetEntry( iOffset, strEntry );
    ++iOffset;
  }

  eState = ciPlayer.GetProperty( U4_Item_Book_PropertyID, U4_QuestItemState.NotFound );
  if( eState )
  {
    local strEntry = "";

    if( U4_QuestItemState.Found == eState )
    {
      strEntry = ::rumGetString( u4_item_book_client_StringID );
    }
    else if( U4_QuestItemState.Imbued == eState || U4_QuestItemState.Abyss_Used == eState )
    {
      strEntry = ::rumGetString( u4_item_book_of_truth_client_StringID );
    }

    g_ciUI.m_ciStatListView.SetEntry( iOffset, strEntry );
    ++iOffset;
  }

  eState = ciPlayer.GetProperty( U4_Item_Candle_PropertyID, U4_QuestItemState.NotFound );
  if( eState )
  {
    local strEntry = "";

    if( U4_QuestItemState.Found == eState )
    {
      strEntry = ::rumGetString( u4_item_candle_client_StringID );
    }
    else if( U4_QuestItemState.Imbued == eState || U4_QuestItemState.Abyss_Used == eState )
    {
      strEntry = ::rumGetString( u4_item_candle_of_love_client_StringID );
    }

    g_ciUI.m_ciStatListView.SetEntry( iOffset, strEntry );
    ++iOffset;
  }

  iFlags = ciPlayer.GetProperty( U4_Item_Three_Part_Key_PropertyID, 0 );
  if( iFlags > 0 )
  {
    // Bitfield for Truth, Love, and Courage combined, or 0x7
    local iKeyFlags = ( 1 << PrincipleType.Truth ) | ( 1 << PrincipleType.Love ) | ( 1 << PrincipleType.Courage );
    if( iKeyFlags == iFlags )
    {
      local strEntry = ::rumGetString( u4_item_three_part_key_client_StringID );
      g_ciUI.m_ciStatListView.SetEntry( iOffset, strEntry );
      ++iOffset;
    }
    else
    {
      for( local ePrinciple = 0; ePrinciple < PrincipleType.NumPrinciples; ++ePrinciple )
      {
        if( ::rumBitOn( iFlags, ePrinciple ) )
        {
          local strEntry = ::rumGetString( g_eU4CodexKeyStringArray[ePrinciple] );
          g_ciUI.m_ciStatListView.SetEntry( iOffset, strEntry );
          ++iOffset;
        }
      }
    }
  }

  eState = ciPlayer.GetProperty( U4_Item_Silver_Horn_PropertyID, U4_QuestItemState.NotFound );
  if( eState )
  {
    local strEntry = "";

    if( U4_QuestItemState.Found == eState )
    {
      strEntry = ::rumGetString( u4_item_horn_client_StringID );
    }
    else if( U4_QuestItemState.Imbued == eState )
    {
      strEntry = ::rumGetString( u4_item_silver_horn_client_StringID );
    }

    g_ciUI.m_ciStatListView.SetEntry( iOffset, strEntry );
    ++iOffset;
  }

  eState = ciPlayer.GetProperty( U4_Item_Skull_Fragment_PropertyID, U4_QuestItemState.NotFound );
  if( eState )
  {
    local strEntry = ::rumGetString( u4_item_skull_fragment_client_StringID );
    g_ciUI.m_ciStatListView.SetEntry( iOffset, strEntry );
    ++iOffset;
  }

  eState = ciPlayer.GetProperty( U4_Item_Sextant_PropertyID, U4_QuestItemState.NotFound );
  if( eState )
  {
    local strEntry = ::rumGetString( u4_item_sextant_client_StringID );
    g_ciUI.m_ciStatListView.SetEntry( iOffset, strEntry );
    ++iOffset;
  }

  eState = ciPlayer.GetProperty( U4_Horse_PropertyID, 0 );
  if( eState )
  {
    local strEntry = ::rumGetString( U4_Horse_Creature_client_StringID );
    g_ciUI.m_ciStatListView.SetEntry( iOffset, strEntry );
    ++iOffset;
  }
}


function U4_RenderStatMain()
{
  local ciPlayer = ::rumGetMainPlayer();
  g_ciUI.m_ciStatLabel.SetText( ciPlayer.GetPlayerName() );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05|0.5" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local iHitpoints = ciPlayer.GetProperty( U4_Hitpoints_PropertyID, 0 );
  local iMaxHitpoints = ciPlayer.GetMaxHitpoints();

  local ePlayerClassID = ciPlayer.GetProperty( U4_PlayerClass_PropertyID, U4_Mage_Class_CustomID );
  local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );
  local strClass = ::rumGetStringByName( ciPlayerClass.GetName() + "_client_StringID" );

  local iOffset = 0;

  local strEntry = ::rumGetString( U4_PlayerClass_Property_client_StringID );
  g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + strClass );

  strEntry = ::rumGetString( Hitpoints_Property_client_StringID );
  local strColor = g_strColorTagArray.White;
  if( iMaxHitpoints * 0.25 > iHitpoints )
  {
    strColor = g_strColorTagArray.Red;
  }
  else if( iMaxHitpoints * 0.5 > iHitpoints )
  {
    strColor = g_strColorTagArray.Yellow;
  }

  local strHitpoints = format( "%s%d%s/ %d", strColor, iHitpoints, g_strColorTagArray.White, iMaxHitpoints );
  g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + strHitpoints );

  local iMaxMana = ciPlayer.GetMaxMana();
  if( iMaxMana > 0 )
  {
    local iMana = ciPlayer.GetProperty( U4_Mana_PropertyID, 0 );

    strEntry = ::rumGetString( Mana_Property_client_StringID );
    strColor = g_strColorTagArray.White;
    if( iMaxMana * 0.25 > iMana )
    {
      strColor = g_strColorTagArray.Red;
    }
    else if( iMaxMana * 0.5 > iMana )
    {
      strColor = g_strColorTagArray.Yellow;
    }

    local strMana = format( "%s%d%s/ %d", strColor, iMana, g_strColorTagArray.White, iMaxMana );
    g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + strMana );
  }

  foreach( ePropertyID in g_eU4StatPropertyArray )
  {
    local ciAsset = ::rumGetPropertyAsset( ePropertyID );
    local iValue = ciPlayer.GetProperty( ePropertyID, 0 );
    strEntry = ::rumGetStringByName( ciAsset.GetName() + "_Property_client_StringID" );
    g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + iValue );
  }
}


function U4_RenderStatReagents()
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_reagents_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();
  g_ciUI.m_ciStatListView.SetFormat( "2.0|0.12" );

  local ciPlayer = ::rumGetMainPlayer();

  // Build the list of reagents
  foreach( iIndex, ePropertyID in g_eU4ReagentPropertyArray )
  {
    local iNum = ciPlayer.GetProperty( ePropertyID, 0 );
    if( iNum )
    {
      local ciProperty = ::rumGetPropertyAsset( ePropertyID );
      local strEntry = iNum + "| - " + ::rumGetStringByName( ciProperty.GetName() + "_Property_client_StringID" );
      g_ciUI.m_ciStatListView.SetEntry( iIndex, strEntry );
    }
  }
}


function U4_RenderStatRunes()
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_runes_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local ciPlayer = ::rumGetMainPlayer();
  local iFlags = ciPlayer.GetProperty( U4_Item_Runes_PropertyID, 0 );
  if( iFlags )
  {
    for( local eVirtue = 0; eVirtue < VirtueType.NumVirtues; ++eVirtue )
    {
      if( ::rumBitOn( iFlags, eVirtue ) )
      {
        local strEntry = ::rumGetString( g_eU4VirtueStringArray[eVirtue] );
        g_ciUI.m_ciStatListView.SetEntry( eVirtue + 1, strEntry );
      }
    }
  }
}


function U4_RenderStatStones()
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_stones_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local ciPlayer = ::rumGetMainPlayer();
  local iFlags = ciPlayer.GetProperty( U4_Item_Stones_PropertyID, 0 );
  if( iFlags )
  {
    for( local eVirtue = 0; eVirtue < VirtueType.NumVirtues; ++eVirtue )
    {
      if( ::rumBitOn( iFlags, eVirtue ) )
      {
        local strEntry = ::rumGetString( g_eU4StoneStringArray[eVirtue] );
        g_ciUI.m_ciStatListView.SetEntry( eVirtue + 1, strEntry );
      }
    }
  }
}


function U4_SeerEndTransaction()
{
  ShowString( format( "<b>%s<b>", ::rumGetString( u4_seer_bye_client_StringID ) ), g_strColorTagArray.Cyan );

  // We must still send to the keyword to the server so that it can end the conversation server-side
  local ciBroadcast = ::rumCreate( Player_Talk_Seer_BroadcastID, 2 );
  ::rumSendBroadcast( ciBroadcast );

  g_ciUI.m_ciGameInputTextBox.ResetInputMode( true );
  g_ciUI.m_ciGameInputTextBox.ShowPrompt( true );
}


function U4_SeerResponse()
{
  // Grab what was typed
  local strKeyword = strip( g_ciUI.m_ciGameInputTextBox.GetText() );

  if( strKeyword.len() > 0 )
  {
    ShowString( strKeyword );
  }

  strKeyword = strKeyword.tolower();

  // Determine what certain special keyword strings are in the player's language
  local strBye = ::rumGetString( talk_bye_client_StringID );

  // If the query is empty or localized "bye", end the conversation
  if( ( strKeyword.len() == 0 ) || ( strKeyword == strBye.tolower() ) )
  {
    U4_SeerEndTransaction();
  }
  else
  {
    local eVirtue = -1;
    for( local i = 0; eVirtue == -1, i < VirtueLoc.len(); ++i )
    {
      // Does the typed keyword match any virtues in the localized virtue table?
      if( strKeyword == VirtueLoc[i] )
      {
        // The index of the virtue in the table
        eVirtue = i;
      }
    }

    if( eVirtue == -1 )
    {
      // The Seer doesn't recognize the topic
      ShowString( format( "<b>%s", ::rumGetString( u4_seer_no_info_client_StringID ) ), g_strColorTagArray.Cyan );
      ShowString( format( "<b>%s", ::rumGetString( u4_seer_prompt_other_client_StringID ) ), g_strColorTagArray.Cyan );
    }
    else
    {
      // Is the player already a partial avatar for this virtue?
      local ciPlayer = ::rumGetMainPlayer();
      local iFlags = ciPlayer.GetProperty( U4_Virtue_Elevation_PropertyID, 0 );

      if( ::rumBitOn( iFlags, eVirtue ) )
      {
        // The player is already a partial avatar
        ShowString( format( "<b>%s", ::rumGetString( u4_seer_partial_client_StringID ) ), g_strColorTagArray.Cyan );
        ShowString( format( "<b>%s", ::rumGetString( u4_seer_prompt_other_client_StringID ) ),
                    g_strColorTagArray.Cyan );
      }
      else
      {
        // Send the virtue to the seer
        local ciBroadcast = ::rumCreate( Player_Talk_Seer_BroadcastID, 1, eVirtue );
        ::rumSendBroadcast( ciBroadcast );
      }
    }
  }

  g_ciUI.m_ciGameInputTextBox.Clear()
}


function U4_Stat()
{
  local strDesc = ::rumGetString( command_zstats_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  // Bookmark the current page
  local eCurrentPage = g_ciUI.m_eCurrentStatPage;

  ++g_ciUI.m_eCurrentStatPage;

  local ciPlayer = ::rumGetMainPlayer();

  if( ( U4_StatPage.Party == g_ciUI.m_eCurrentStatPage ) && ( null == g_ciCUO.m_uiPartyIDTable ) )
  {
    // Don't show the party page if the player is not part of a party
    ++g_ciUI.m_eCurrentStatPage;
  }

  if( ( U4_StatPage.Runes == g_ciUI.m_eCurrentStatPage ) &&
      ( ciPlayer.GetProperty( U4_Item_Runes_PropertyID, 0 ) == 0 ) )
  {
    // Don't show the rune page if the player doesn't own any
    ++g_ciUI.m_eCurrentStatPage;
  }

  if( ( U4_StatPage.Stones == g_ciUI.m_eCurrentStatPage ) &&
      ( ciPlayer.GetProperty( U4_Item_Stones_PropertyID, 0 ) == 0 ) )
  {
    // Don't show the rune page if the player doesn't own any
    ++g_ciUI.m_eCurrentStatPage;
  }

  if( g_ciUI.m_eCurrentStatPage >= U4_StatPage.NumPages )
  {
    g_ciUI.m_eCurrentStatPage = U4_StatPage.Main;
  }
  else
  {
    local ciPlayer = ::rumGetMainPlayer();
    if( ciPlayer.IsDead() && g_ciUI.m_eCurrentStatPage >= U4_StatPage.Party )
    {
      g_ciUI.m_eCurrentStatPage = U4_StatPage.Main;
    }
  }

  if( eCurrentPage != g_ciUI.m_eCurrentStatPage )
  {
    U4_Stat_Update( g_ciUI.m_eCurrentStatPage );
  }
}


function U4_Stat_Update( i_eStatPage )
{
  g_ciUI.m_eCurrentStatPage = i_eStatPage;

  switch( i_eStatPage )
  {
    case U4_StatPage.Main:      U4_RenderStatMain(); break;
    case U4_StatPage.Party:     RenderStatParty( U4_Graphic_ID_PropertyID,
                                                 U4_Adventurer_GraphicID,
                                                 U4_PlayerClass_PropertyID,
                                                 U4_Mage_Class_CustomID); break;
    case U4_StatPage.Weapons:   RenderStatWeapons( GameType.Ultima4 ); break;
    case U4_StatPage.Armour:    RenderStatArmour( GameType.Ultima4 ); break;
    case U4_StatPage.Equipment: RenderStatEquipment( g_eU4EquipmentPropertyArray ); break;
    case U4_StatPage.Runes:     U4_RenderStatRunes(); break;
    case U4_StatPage.Stones:    U4_RenderStatStones(); break;
    case U4_StatPage.Items:     U4_RenderStatItems(); break;
    case U4_StatPage.Reagents:  U4_RenderStatReagents(); break;
    case U4_StatPage.Mixtures:  RenderStatSpells( token_mixtures_client_StringID,
                                                  g_eU4SpellArray,
                                                  g_eU4SpellPropertyArray ); break;
    default:                    U4_RenderStatMain(); break;
  }
}


function U4_UseHornCallback( i_eDir )
{
  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local eState = ciPlayer.GetProperty( U4_Item_Silver_Horn_PropertyID, U4_QuestItemState.NotFound );
  if( U4_QuestItemState.Imbued == eState )
  {
    // Send the class id of the item used to the server
    local ciBroadcast = ::rumCreate( Player_Use_BroadcastID, U4_Item_Silver_Horn_PropertyID, i_eDir );
    ::rumSendBroadcast( ciBroadcast );
  }
}


function U4_UseItem()
{
  local strDesc = ::rumGetString( command_use_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  g_ciUI.m_ciGameListView.Clear();

  // The map, which is always available
  g_ciUI.m_ciGameListView.SetEntry( 0, ::rumGetString( token_map_client_StringID ) );

  local eState = ciPlayer.GetProperty( U4_Item_Bell_PropertyID, U4_QuestItemState.NotFound );
  if( eState )
  {
    local strEntry = "";

    if( U4_QuestItemState.Found == eState )
    {
      strEntry = ::rumGetString( u4_item_bell_client_StringID );
    }
    else if( U4_QuestItemState.Imbued == eState || U4_QuestItemState.Abyss_Used == eState )
    {
      strEntry = ::rumGetString( u4_item_bell_of_courage_client_StringID );
    }

    g_ciUI.m_ciGameListView.SetEntry( U4_Item_Bell_PropertyID, strEntry );
  }

  eState = ciPlayer.GetProperty( U4_Item_Book_PropertyID, U4_QuestItemState.NotFound );
  if( eState )
  {
    local strEntry = "";

    if( U4_QuestItemState.Found == eState )
    {
      strEntry = ::rumGetString( u4_item_book_client_StringID );
    }
    else if( U4_QuestItemState.Imbued == eState || U4_QuestItemState.Abyss_Used == eState )
    {
      strEntry = ::rumGetString( u4_item_book_of_truth_client_StringID );
    }

    g_ciUI.m_ciGameListView.SetEntry( U4_Item_Book_PropertyID, strEntry );
  }

  eState = ciPlayer.GetProperty( U4_Item_Candle_PropertyID, U4_QuestItemState.NotFound );
  if( eState )
  {
    local strEntry = "";

    if( U4_QuestItemState.Found == eState )
    {
      strEntry = ::rumGetString( u4_item_candle_client_StringID );
    }
    else if( U4_QuestItemState.Imbued == eState || U4_QuestItemState.Abyss_Used == eState )
    {
      strEntry = ::rumGetString( u4_item_candle_of_love_client_StringID );
    }

    g_ciUI.m_ciGameListView.SetEntry( U4_Item_Candle_PropertyID, strEntry );
  }

  local iFlags = ciPlayer.GetProperty( U4_Item_Three_Part_Key_PropertyID, 0 );
  if( iFlags > 0 )
  {
    // Bitfield for Truth, Love, and Courage combined, or 0x7
    local iKeyFlags = ( 1 << PrincipleType.Truth ) | ( 1 << PrincipleType.Love ) | ( 1 << PrincipleType.Courage );

    // Only show the Three Part Key as a usable item
    if( iKeyFlags == iFlags )
    {
      local strEntry = ::rumGetString( u4_item_three_part_key_client_StringID );
      g_ciUI.m_ciGameListView.SetEntry( U4_Item_Three_Part_Key_PropertyID, strEntry );
    }
  }

  eState = ciPlayer.GetProperty( U4_Item_Silver_Horn_PropertyID, U4_QuestItemState.NotFound );
  if( eState )
  {
    local strEntry = "";

    if( U4_QuestItemState.Found == eState )
    {
      strEntry = ::rumGetString( u4_item_horn_client_StringID );
    }
    else if( U4_QuestItemState.Imbued == eState )
    {
      strEntry = ::rumGetString( u4_item_silver_horn_client_StringID );
    }

    g_ciUI.m_ciGameListView.SetEntry( U4_Item_Silver_Horn_PropertyID, strEntry );
  }

  eState = ciPlayer.GetProperty( U4_Item_Skull_Fragment_PropertyID, U4_QuestItemState.NotFound );
  if( eState )
  {
    local strEntry = ::rumGetString( u4_item_skull_fragment_client_StringID );
    g_ciUI.m_ciGameListView.SetEntry( U4_Item_Skull_Fragment_PropertyID, strEntry );
  }

  eState = ciPlayer.GetProperty( U4_Item_Sextant_PropertyID, U4_QuestItemState.NotFound );
  if( eState )
  {
    local strEntry = ::rumGetString( u4_item_sextant_client_StringID );
    g_ciUI.m_ciGameListView.SetEntry( U4_Item_Sextant_PropertyID, strEntry );
  }

  if( ciPlayer.GetProperty( U4_Horse_PropertyID, rumInvalidAssetID ) != rumInvalidAssetID &&
      ( ciPlayer.GetTransportID() == rumInvalidGameID ) )
  {
    local iLastDismountTime = ciPlayer.GetProperty( U4_Horse_Dismount_Time_PropertyID, 0 );
    if( ::rumGetSecondsSinceEpoch() - iLastDismountTime > Transport_Widget.s_fIdleInterval )
    {
      local strEntry = ::rumGetString( U4_Horse_Creature_client_StringID );
      g_ciUI.m_ciGameListView.SetEntry( U4_Horse_PropertyID, strEntry );
    }
  }

  local eTransportWidgetID = ciPlayer.GetProperty( U4_Transport_Widget_PropertyID, rumInvalidAssetID );
  if( eTransportWidgetID != rumInvalidAssetID )
  {
    local ciMap = ciPlayer.GetMap();

    local eMapID = ciPlayer.GetProperty( U4_Transport_Map_PropertyID, rumInvalidAssetID );
    if( eMapID != rumInvalidAssetID && ( ciMap.GetAssetID() == eMapID ) )
    {
      g_ciUI.m_ciGameListView.SetEntry( U4_Transport_Widget_PropertyID, "Summon Transport" );
    }
  }

  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );
  g_ciUI.m_ciGameListView.ShowPrompt( false );

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = U4_UseItemCallback;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();
}


function U4_UseItemCallback()
{
  g_ciUI.m_ciGameInputTextBox.Clear();

  local ciPlayer = ::rumGetMainPlayer();

  local strSelected = g_ciUI.m_ciGameListView.GetCurrentEntry();
  ShowString( strSelected );

  local ePropertyID = g_ciUI.m_ciGameListView.GetSelectedKey();
  if( 0 == ePropertyID )
  {
    // The map is being used
    if( !( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() ) )
    {
      g_ciUI.m_eOverlayGraphicID = U4_Map_GraphicID;
      g_ciUI.m_ciOverlayGraphicOffset = rumPoint( 0, 0 );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, OnOverlayGraphicDone );
    }
  }
  else if( U4_Item_Sextant_PropertyID == ePropertyID )
  {
    if( MapLabelType.Position == g_ciUI.m_eMapLabelType )
    {
      // Put the sextant away
      g_ciUI.m_eMapLabelType = MapLabelType.First;
    }
    else
    {
      g_ciUI.m_eMapLabelType = MapLabelType.Position;
    }

    Ultima_UpdateMapLabel();
  }
  else if( ( U4_Item_Silver_Horn_PropertyID == ePropertyID ) && ciPlayer.IsFlying() )
  {
    local eState = ciPlayer.GetProperty( U4_Item_Silver_Horn_PropertyID, U4_QuestItemState.NotFound );
    if( U4_QuestItemState.Imbued == eState )
    {
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction, U4_UseHornCallback );

      local strDesc = format( "%s <%s>",
                              ::rumGetString( msg_horn_use_client_StringID ),
                              ::rumGetString( token_direction_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetText( strDesc );
    }
    else
    {
      ShowString( ::rumGetString( msg_horn_use_client_StringID ) );
      ShowString( ::rumGetString( msg_no_effect_client_StringID ), g_strColorTagArray.Red );
    }
  }
  else
  {
    if( U4_Horse_PropertyID == ePropertyID )
    {
      // Is the map compatible with a horse?
      local ciMap = ciPlayer.GetMap();
      local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
      if( ( MapType.Dungeon == eMapType ) ||
          ( MapType.Abyss == eMapType )   ||
          ( MapType.Altar == eMapType )   ||
          ( MapType.Cave == eMapType )    ||
          ( MapType.Codex == eMapType )   ||
          ( MapType.Shrine == eMapType )  ||
          ( ciMap.GetAssetID() == U4_Castle_British_2_MapID ) )
      {
        ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
        Ultima_ListSelectionEnd();
        return;
      }
    }

    // Send the class id of the item used to the server
    local ciBroadcast = ::rumCreate( Player_Use_BroadcastID, ePropertyID );
    ::rumSendBroadcast( ciBroadcast );
  }

  // Special handling for the TLC key because it displays an image on use
  local eDelay = ActionDelay.Short;
  if( U4_Item_Three_Part_Key_PropertyID != ePropertyID )
  {
    eDelay = ActionDelay.Long;
  }

  Ultima_ListSelectionEnd( eDelay );
}
