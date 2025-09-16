/*

----------------------------------------------------------------------------

ooooo  oooo o888    o8    o88
888    88   888  o888oo  oooo  oo ooo oooo     ooooooo
888    88   888   888     888   888 888 888    ooooo888
888    88   888   888     888   888 888 888  888    888
888oo88   o888o   888o  o888o o888o888o888o  88ooo88 8o

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

Ultima I: The First Age of Darkness
Developers: Richard Garriott, Origin Systems
Publishers: California Pacific Computer Co., Origin Systems, Electronic Arts
Designers:  Richard Garriott, Ken W. Arnold

Ultima II: The Revenge of the Enchantress
Developer:  Richard Garriott
Publishers: Sierra On-Line, Origin Systems (re-release)
Designer:   Richard Garriott

Ultima III: Exodus
Developer:  Richard Garriott
Publisher:  Origin Systems
Designers:  Richard Garriott, Ken W. Arnold

Ultima IV: Quest of the Avatar
Developer:  Origin Systems
Publisher:  Origin Systems
Designers:  Richard Garriott, Ken W. Arnold

*/

g_eArticleStringArray <-
[
  null,
  token_article_indef_sing_cons_client_StringID,
  token_article_indef_sing_vowel_client_StringID,
  token_article_definite_client_StringID,
]


function CastSpellFinished( i_eDelay = ActionDelay.Short )
{
  switch( g_ciCUO.m_eVersion )
  {
    case GameType.Ultima4: U4_CastSpellFinished( i_eDelay ); break;
    case GameType.Ultima3: U3_CastSpellFinished( i_eDelay ); break;
    case GameType.Ultima2: U2_CastSpellFinished( i_eDelay ); break;
    case GameType.Ultima1: U1_CastSpellFinished( i_eDelay ); break;
  }
}


function CommandHelp( i_strParamArray )
{
  local strParams = "";

  foreach( strParam in i_strParamArray )
  {
    strParams += strParam + " ";
  }

  // Show what was typed
  ShowString( "<G#Prompt:vcenter>" + strParams );

  if( i_strParamArray.len() < 2 )
  {
    // Show the standard help
    ShowString( "<b>" + ::rumGetString( game_help_general_client_StringID ) + "<b>" );
    return;
  }

  local ciPlayer = ::rumGetMainPlayer();
  local bAdmin = ciPlayer.IsAdmin();

  if( "commands" == i_strParamArray[1] )
  {
    CommandHelpCallback( 0 );

    /*local strMatch = null;

    if( i_strParamArray.len() > 2 )
    {
      strMatch = i_strParamArray[2].tolower();
    }

    local strCommands = "";
    local iNumAdded = 0;

    foreach( strCommand in g_strPlayerCommandsArray )
    {
      local bMatch = true;

      if( strMatch != null )
      {
        if( strCommand.tolower().find( strMatch ) == null )
        {
          bMatch = false;
        }
      }

      if( bMatch )
      {
        if( iNumAdded > 0 )
        {
          strCommands += ", ";
        }

        strCommands += strCommand;
        ++iNumAdded;
      }
    }

    if( bAdmin )
    {
      foreach( strCommand in g_strAdminCommandsArray )
      {
        local bMatch = true;

        if( strMatch != null )
        {
          if( strCommand.find( strMatch ) == null )
          {
            bMatch = false;
          }
        }

        if( bMatch )
        {
          if( iNumAdded > 0 )
          {
            strCommands += ", ";
          }

          strCommands += strCommand;
          ++iNumAdded;
        }
      }
    }

    local strMsg = format( "<b>%s: %s<b>", ::rumGetString( msg_available_commands_client_StringID ), strCommands );
    ShowString( strMsg );*/
  }
  else
  {
    // Unknown sub command
  }
}


function CommandHelpCallback( i_iNextIndex )
{
  if( rumGetLastKeyPressed() == rumKeypress.KeyEscape() )
  {
    return;
  }

  local strHelpCommands = format( "game_help_commands%d_client_StringID", g_ciCUO.m_eVersion );
  local strCommands = ::rumGetStringByName( strHelpCommands );
  local strArray = split( strCommands, "|" );

  local iTotalHeight = 0;
  local iControlHeight = g_ciUI.m_ciGameTextView.GetHeight();

  if( i_iNextIndex < strArray.len() )
  {
    foreach( iIndex, strCommand in strArray )
    {
      if( iIndex >= i_iNextIndex )
      {
        local iEntryHeight = g_ciUI.m_ciGameTextView.CalculateEntryHeight( strCommand );
        if( iTotalHeight + iEntryHeight <= iControlHeight )
        {
          ShowString( strCommand );
          i_iNextIndex = iIndex + 1;
          iTotalHeight += iEntryHeight;
        }
        else
        {
          // Encountered an entry that won't fit, so stop trying
          break;
        }
      }
    }
  }

  if( i_iNextIndex < strArray.len() )
  {
    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, CommandHelpCallback, i_iNextIndex );
  }
}


function CommandWho( i_strMatch )
{
  local strPlayers = "";
  local iNumAdded = 0;

  local ciPlayerArray = ::rumGetAllPlayers();
  foreach( i, ciPlayer in ciPlayerArray )
  {
    local strName = ciPlayer.GetPlayerName();
    local bMatch = true;

    if( i_strMatch != null )
    {
      if( strName.tolower().find( i_strMatch ) == null )
      {
        bMatch = false;
      }
    }

    if( bMatch )
    {
      if( iNumAdded > 0 )
      {
        strPlayers += ", ";
      }

      strPlayers += strName;
      ++iNumAdded;
    }
  }

  local strMsg = format( "%s: %s", ::rumGetString( msg_online_players_client_StringID ), strPlayers );
  g_ciUI.m_ciChatTextView.PushText( "<G#Prompt:vcenter>" + strMsg );
}


function ComparatorEnemy( i_ciSource, i_ciTarget )
{
  if( null == i_ciTarget )
  {
    return false;
  }

  local bEnemy = false;

  if( i_ciTarget instanceof Creature )
  {
    bEnemy = !i_ciTarget.IsCamouflaged() &&
             ( i_ciTarget.GetAlignment() != AlignmentType.Good || i_ciTarget.IsCriminal() );
  }
  else if( i_ciTarget instanceof Transport_Widget )
  {
    local ciCommander = i_ciTarget.GetCommander();
    if( ciCommander != null )
    {
      bEnemy = ciCommander.IsCriminal();
    }
  }

  return bEnemy;
}


function ComparatorFriendly( i_ciSource, i_ciTarget )
{
  if( null == i_ciTarget )
  {
    return false;
  }

  local bFriendly = false;

  if( i_ciTarget instanceof Creature )
  {
    bFriendly = ( i_ciTarget.GetAlignment() == AlignmentType.Good ) && !i_ciTarget.IsCriminal();
  }
  else if( i_ciTarget instanceof Transport_Widget )
  {
    local ciCommander = i_ciTarget.GetCommander();
    if( ciCommander != null )
    {
      bFriendly = !ciCommander.IsCriminal();
    }
  }

  return bFriendly;
}


function DoDamageEffect( i_ciPawn, i_bPlaySound = true )
{
  if( ( i_ciPawn instanceof Creature ) || ( i_ciPawn instanceof Widget ) )
  {
    // Create the effect
    local ciClientEffect = ManagedObjectClientEffect( ClientEffectType.Damage );

    local ciMap = i_ciPawn.GetMap();
    local ciPos = i_ciPawn.GetPosition();

    // Create a damage widget that will be drawn
    local ciDamage = ::rumCreate( Damage_Red_WidgetID );
    if( ciMap.AddPawn( ciDamage, ciPos ) )
    {
      // Save a reference to the pawn
      ciClientEffect.m_uiPawnID = i_ciPawn.GetID();
      ciClientEffect.m_ciObject = ciDamage;

      local fDuration = ciDamage.GetProperty( Expiration_Interval_PropertyID, ciClientEffect.s_fInterval );
      ::rumSchedule( ciClientEffect, ciClientEffect.Expire, fDuration );

      i_ciPawn.AddClientEffect( ciClientEffect );

      if( i_bPlaySound )
      {
        local ciPlayer = ::rumGetMainPlayer();
        if( i_ciPawn != ciPlayer )
        {
          PlaySound( Player_Hit_SoundID );
        }
        else
        {
          PlaySound3D( Creature_Hit_SoundID, ciPos );
        }
      }
    }
  }
}


function DoScreenShakeEffect( i_bPlaySound = true )
{
  local ciPlayer = ::rumGetMainPlayer();

  // Create the effect
  local ciClientEffect = ScreenShakeClientEffect();
  ciClientEffect.m_uiPawnID = ciPlayer.GetID();

  ciPlayer.AddClientEffect( ciClientEffect );

  ciClientEffect.Update();

  if( i_bPlaySound )
  {
    PlaySound( Tremor_SoundID );
  }
}


function DoSpellEffect( i_ciPawn, i_bPlaySound = true )
{
  local ciClientEffect = null;

  // Does the target already have this effect?
  foreach( ciEffect in i_ciPawn.m_ciEffectsTable )
  {
    if( ciEffect instanceof CastSpellClientEffect )
    {
      // Reset the effect if it's already ramping down
      ciEffect.m_bBrighten = true;
      ciClientEffect = ciEffect;
      break;
    }
  }

  if( null == ciClientEffect )
  {
    // Create the effect
    ciClientEffect = CastSpellClientEffect();
    ciClientEffect.m_uiPawnID = i_ciPawn.GetID();

    i_ciPawn.AddClientEffect( ciClientEffect );
  }

  ciClientEffect.Update();

  if( i_bPlaySound )
  {
    // Play a spell sound effect only if this isn't the main player
    local ciPlayer = ::rumGetMainPlayer();
    if( i_ciPawn != ciPlayer )
    {
      PlaySound3D( Creature_Spell_SoundID, i_ciPawn.GetPosition() );
    }
  }
}


function GetMusicForMap( i_ciMap )
{
  local eMapID = i_ciMap.GetAssetID();
  local eMusicID = U4_Wander_SoundID;

  if( U3_Ambrosia_MapID == eMapID )
  {
    eMusicID = U3_Alive_SoundID;
  }
  else if( U3_Castle_Fire_MapID == eMapID )
  {
    eMusicID = U3_Exodus_SoundID;
  }
  else
  {
    local eMapType = i_ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
    switch( eMapType )
    {
      case MapType.World:
      case MapType.Space:
        switch( rand() % 4 )
        {
          case 0: eMusicID = U3_Wander_SoundID;            break;
          case 1: eMusicID = U4_Wander_SoundID;            break;
          case 2: eMusicID = U5_Britannic_Lands_SoundID;   break;
          case 4: eMusicID = U5_Dream_Of_Lady_Nan_SoundID; break;
        }
        break;

      case MapType.Village:
        switch( rand() % 3 )
        {
          case 0: eMusicID = U3_Towns_SoundID;               break;
          case 1: eMusicID = U4_Towns_SoundID;               break;
          case 2: eMusicID = U5_Villager_Tarantella_SoundID; break;
        }
        break;

      case MapType.Towne:
        switch( rand() % 3 )
        {
          case 0: eMusicID = U3_Towns_SoundID;         break;
          case 1: eMusicID = U4_Towns_SoundID;         break;
          case 2: eMusicID = U5_Greysons_Tale_SoundID; break;
        }
        break;

      case MapType.Keep:
        switch( rand() % 3 )
        {
          case 0: eMusicID = U3_Castles_SoundID;                  break;
          case 1: eMusicID = U4_Castles_SoundID;                  break;
          case 2: eMusicID = U5_Fanfare_For_The_Virtuous_SoundID; break;
        }
        break;

      case MapType.Castle:
        switch( rand() % 6 )
        {
          case 0: eMusicID = U3_Castles_SoundID;                  break;
          case 1: eMusicID = U4_Castles_SoundID;                  break;
          case 2: eMusicID = U5_Fanfare_For_The_Virtuous_SoundID; break;
          case 3: eMusicID = U3_Rule_Britannia_SoundID;           break;
          case 4: eMusicID = U4_Rule_Britannia_SoundID;           break;
          case 5: eMusicID = U5_Rule_Britannia_SoundID;           break;
        }
        break;

      case MapType.Shrine:
      case MapType.Codex:
        switch( rand() % 3 )
        {
          case 0: eMusicID = U3_Alive_SoundID;   break;
          case 1: eMusicID = U4_Shrines_SoundID; break;
          case 2: eMusicID = U5_Stones_SoundID;  break;
        }
        break;

      case MapType.Dungeon:
      case MapType.Tower:
      case MapType.Cave:
        switch( rand() % 3 )
        {
          case 0: eMusicID = U3_Dungeon_SoundID;       break;
          case 1: eMusicID = U4_Dungeon_SoundID;       break;
          case 2: eMusicID = U5_Halls_Of_Doom_SoundID; break;
        }
        break;

      case MapType.Altar:
        eMusicID = U5_Worlds_Below_SoundID;
        break;

      case MapType.Abyss:
      case MapType.SpaceStation:
      case MapType.Misc:
        switch( rand() % 3 )
        {
          case 0: eMusicID = U3_Exodus_SoundID;          break;
          case 1: eMusicID = U5_Lord_Blackthorn_SoundID; break;
          case 2: eMusicID = U5_Worlds_Below_SoundID;    break;
        }
        break;
    }
  }

  return eMusicID;
}


function InitTarget( i_iRange, i_funcComparator = null )
{
  local bHasTarget = false;
  local ciPlayer = ::rumGetMainPlayer();
  local ciPlayerPos = ciPlayer.GetPosition();

  g_ciCUO.m_funcLastComparator = i_funcComparator;
  g_ciCUO.m_uiTargetRange = i_iRange;

  if( g_ciCUO.m_uiLockedTargetID != rumInvalidGameID &&
      ( g_ciCUO.m_uiLockedTargetID in g_ciCUO.m_uiVisibleTargetIDsTable ) )
  {
    local ciTarget = ::rumFetchPawn( g_ciCUO.m_uiLockedTargetID );
    if( ciTarget != null )
    {
      local ciMap = ciPlayer.GetMap();
      local bMapRequiresLight = MapRequiresLight( ciMap );

      bHasTarget = i_funcComparator != null ? i_funcComparator( ciPlayer, ciTarget ) : true;
      if( bHasTarget )
      {
        local ciTargetPos = ciTarget.GetPosition();

        local ciScreenPos = g_ciUI.s_ciCenterTilePos;
        ciScreenPos.x += ciTargetPos.x - ciPlayerPos.x;
        ciScreenPos.y += ciTargetPos.y - ciPlayerPos.y;

        if( !bMapRequiresLight || ciMap.IsPositionLit( ciScreenPos ) )
        {
          if( ciMap.IsPositionWithinRadius( ciPlayerPos, ciTargetPos, i_iRange ) )
          {
            if( ciMap.TestLOS( ciPlayerPos, ciTargetPos, g_ciCUO.s_iLOSRadius ) )
            {
              // Old target is still visible, be sure the target reticle has its updated position
              g_ciCUO.m_ciTargetPos = ciTargetPos;
            }
          }
        }
      }
    }
  }

  if( !bHasTarget )
  {
    g_ciCUO.m_ciTargetPos = ciPlayer.GetPosition();
  }
}


function KeyPressedGame( i_ciKeyboard )
{
  local ciPlayer = ::rumGetMainPlayer();
  if( null == ciPlayer )
  {
    return;
  }

  local eKey = i_ciKeyboard.GetKey();

  if( rumKeypress.KeySpace() == eKey )
  {
    if( i_ciKeyboard.CtrlPressed() )
    {
      Ultima_Attack_Target( g_ciCUO.m_uiLockedTargetID );
    }
  }
  else if( rumKeypress.KeyPageUp() == eKey )
  {
    if( i_ciKeyboard.CtrlPressed() )
    {
      g_ciUI.m_ciChatTextView.PageUp();
    }
    else
    {
      g_ciUI.m_ciStatListView.PageUp();
    }
  }
  else if( rumKeypress.KeyPageDown() == eKey )
  {
    if( i_ciKeyboard.CtrlPressed() )
    {
      g_ciUI.m_ciChatTextView.PageDown();
    }
    else
    {
      g_ciUI.m_ciStatListView.PageDown();
    }
  }
  else if( rumKeypress.KeyEnd() == eKey )
  {
    if( i_ciKeyboard.CtrlPressed() )
    {
      g_ciUI.m_ciChatTextView.End();
    }
    else
    {
      g_ciUI.m_ciStatListView.End();
    }
  }
  else if( rumKeypress.KeyHome() == eKey )
  {
    if( i_ciKeyboard.CtrlPressed() )
    {
      g_ciUI.m_ciChatTextView.Home();
    }
    else
    {
      g_ciUI.m_ciStatListView.Home();
    }
  }
  else if( rumKeypress.KeyTab() == eKey )
  {
    Ultima_AdvanceMapLabel();
  }
  else
  {
    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1: U1_KeyPressedGame( i_ciKeyboard ); break;
      case GameType.Ultima2: U2_KeyPressedGame( i_ciKeyboard ); break;
      case GameType.Ultima3: U3_KeyPressedGame( i_ciKeyboard ); break;
      case GameType.Ultima4: U4_KeyPressedGame( i_ciKeyboard ); break;
    }
  }
}


function KeyPressedGameImmediate()
{
  local eDirection = null;

  if( ::rumIsKeyPressed( rumKeypress.KeyUp() ) || ::rumIsKeyPressed( rumKeypress.KeyPad8() ) )
  {
    eDirection = Direction.North;
  }
  else if( ::rumIsKeyPressed( rumKeypress.KeyDown() ) || ::rumIsKeyPressed( rumKeypress.KeyPad2() ) )
  {
    eDirection = Direction.South;
  }
  else if( ::rumIsKeyPressed( rumKeypress.KeyLeft() ) || ::rumIsKeyPressed( rumKeypress.KeyPad4() ) )
  {
    eDirection = Direction.West;
  }
  else if( ::rumIsKeyPressed( rumKeypress.KeyRight() ) || ::rumIsKeyPressed( rumKeypress.KeyPad6() ) )
  {
    eDirection = Direction.East;
  }
  else if( ::rumIsKeyPressed( rumKeypress.KeyPad7() ) )
  {
    eDirection = Direction.Northwest;
  }
  else if( ::rumIsKeyPressed( rumKeypress.KeyPad9() ) )
  {
    eDirection = Direction.Northeast;
  }
  else if( ::rumIsKeyPressed( rumKeypress.KeyPad1() ) )
  {
    eDirection = Direction.Southwest;
  }
  else if( ::rumIsKeyPressed( rumKeypress.KeyPad3() ) )
  {
    eDirection = Direction.Southeast;
  }
  else if( ::rumIsKeyPressed( rumKeypress.KeyPad5() ) )
  {
    eDirection = Direction.None;
  }

  if( eDirection != null )
  {
    local ciPlayer = ::rumGetMainPlayer();
    if( ciPlayer != null && !ciPlayer.IsIncapacitated())
    {
      if( ::rumIsKeyPressed( rumKeypress.KeyLeftControl() ) ||
          ::rumIsKeyPressed( rumKeypress.KeyRightControl() ) )
      {
        // Player is using the fast attack method
        Ultima_Attack_Fast( eDirection );
        return true;
      }
      else
      {
        ciPlayer.Move( eDirection );
        return true;
      }
    }
  }

  return false;
}


function MouseMovedGame( i_ciPoint )
{
  if( InputMode.Target == g_ciUI.m_ciGameInputTextBox.m_eInputMode )
  {
    local ciPlayer = ::rumGetMainPlayer();
    g_ciCUO.m_ciTargetPos = GetMapPosFromMousePos( ciPlayer.GetPosition(), i_ciPoint );
  }
}


function MousePressedGame( i_eButton, i_ciPoint )
{
  if( !g_ciUI.m_ciGameInputTextBox.HasInputFocus() )
  {
    return;
  }

  if( InputMode.Game == g_ciUI.m_ciGameInputTextBox.m_eInputMode )
  {
    if( !g_ciUI.m_bInputBlocked && ( rumLeftMouseButton == i_eButton ) )
    {
      local ciPlayer = ::rumGetMainPlayer();
      if( !ciPlayer.IsIncapacitated() )
      {
        local ciVector = GetVectorFromMousePos( i_ciPoint );
        ciVector.x = clamp( ciVector.x, -1, 1 );
        ciVector.y = clamp( ciVector.y, -1, 1 );

        local eDir = GetDirectionFromVector( ciVector );
        ciPlayer.Move( eDir );
      }
    }
  }
  else if( InputMode.Direction == g_ciUI.m_ciGameInputTextBox.m_eInputMode )
  {
    if( !g_ciUI.m_bInputBlocked && ( rumLeftMouseButton == i_eButton ) )
    {
      local ciVector = GetVectorFromMousePos( i_ciPoint );
      ciVector.x = clamp( ciVector.x, -1, 1 );
      ciVector.y = clamp( ciVector.y, -1, 1 );

      local eDir = GetDirectionFromVector( ciVector );
      local strDir = g_ciUI.m_ciGameInputTextBox.GetText() + GetDirectionString( eDir );
      ShowString( "<G#Prompt:vcenter>" + strDir );

      g_ciUI.m_ciGameInputTextBox.Clear();
      g_ciUI.m_ciGameInputTextBox.EndInputMode( true, eDir );
    }
  }
  else if( InputMode.Target == g_ciUI.m_ciGameInputTextBox.m_eInputMode )
  {
    g_ciUI.m_ciGameInputTextBox.TargetSelected();
  }
}


function BlockInput( i_fDuration )
{
  g_ciUI.m_bInputBlocked = true;
  g_ciUI.m_fBlockTimer = max( g_ciUI.m_fBlockTimer, g_ciCUO.m_fMainTimer + i_fDuration );

  g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
  g_ciUI.m_ciGameInputTextBox.ShowCursor( false );
}


function OnFrameStart( i_fElapsedTime )
{
  // Update the game timer
  g_ciCUO.m_fMainTimer = i_fElapsedTime;

  if( g_ciUI.m_bInputBlocked && g_ciCUO.m_fMainTimer >= g_ciUI.m_fBlockTimer )
  {
    // Input is no longer blocked
    g_ciUI.m_ciGameInputTextBox.ShowPrompt( true );
    g_ciUI.m_ciGameInputTextBox.ShowCursor( true );

    g_ciUI.m_bInputBlocked = false;
  }

  if( ( GameMode.Title == g_ciCUO.m_eCurrentGameMode ) || ( GameMode.CharGen == g_ciCUO.m_eCurrentGameMode ) )
  {
    UpdateTitleAnimations( i_fElapsedTime );
  }

  // Handle all tile animations
  if( g_ciCUO.m_fMainTimer - g_ciCUO.m_fTileAnimationTimer > 0.5 )
  {
    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1: U1_Animate(); break;
      case GameType.Ultima2: U2_Animate(); break;
      case GameType.Ultima3: U3_Animate(); break;
      case GameType.Ultima4: U4_Animate(); break;
    }

    g_ciCUO.m_fTileAnimationTimer = g_ciCUO.m_fMainTimer;
  }

  if( ::rumIsKeyPressed( rumKeypress.KeyF1() ) )
  {
    OpenSettingsMenu();
  }
  else if( ::rumIsKeyPressed( rumKeypress.KeyEnter() ) &&
           ( ::rumIsKeyPressed( rumKeypress.KeyLeftAlt() ) || ::rumIsKeyPressed( rumKeypress.KeyRightAlt() ) ) )
  {
    // Toggle fullscreen/windowed
    if( ::rumIsFullscreen() )
    {
      ::rumSetWindowedMode( ::rumGetWindowedScreenWidth(), ::rumGetWindowedScreenHeight() );
    }
    else
    {
      ::rumSetFullscreenMode( ::rumGetFullScreenWidth(), ::rumGetFullScreenHeight() );
    }
  }
  else if( ::rumIsKeyPressed( rumKeypress.KeyPause() ) )
  {
    if( ::rumDebuggerAttached() )
    {
      ::rumDebugDetachVM();
      print( "Debugger detached\n" );
    }
    else
    {
      ::rumDebugAttachVM();
      print( "Debugger attached\n" );
    }
  }
  else if( !g_ciUI.m_bInputBlocked )
  {
    if( ( g_ciCUO.m_eCurrentGameMode == GameMode.Game ) &&
        ( g_ciUI.m_ciGameInputTextBox.m_eInputMode == InputMode.Game ) &&
        ( g_ciUI.m_ciGameInputTextBox.HasInputFocus() ) )
    {
      KeyPressedGameImmediate();
    }
  }

  switch( g_ciCUO.m_eCurrentGameMode )
  {
    case GameMode.Game:       Ultima_RenderGame(); break;
    case GameMode.Title:      RenderTitleScreen(); break;
    case GameMode.CharGen:    RenderCharGen();     break;
    case GameMode.Transition: RenderTransition();  break;
  }

  ::rumDisplayActiveControls();
  RenderOverlay();

  // Render the mouse cursor
  local ciGraphic = ::rumGetGraphic( Mouse_Cursor_GraphicID );
  if( ciGraphic != null )
  {
    ciGraphic.DrawAnimation( ::rumGetMousePosition() );
  }
}


function OnGraphicDone()
{
  g_eGraphicID = rumInvalidAssetID;
}


function OnMapChange( i_ciMap )
{
  if( null == i_ciMap )
  {
    return;
  }

  local ciPlayer = ::rumGetMainPlayer();
  if( null == ciPlayer )
  {
    return;
  }

  local eVersion = i_ciMap.GetProperty( Ultima_Version_PropertyID, GameType.Invalid );

  // Update the listener position
  local ciPlayerPos = ciPlayer.GetPosition();
  ::rumSetListenerPosition( ciPlayerPos );

  g_ciUI.m_eMapLabelType = MapLabelType.MapName;
  Ultima_UpdateMapLabel();

  i_ciMap.SetPeeringAttributes();

  PlayMusic( GetMusicForMap( i_ciMap ) );
}


function OnOverlayGraphicDone()
{
  g_ciUI.m_eOverlayGraphicID = rumInvalidAssetID;
}


function OnPlayerLoginFailed( i_strReason )
{
  g_ciCUO.m_bWaitingForResponse = false;
  g_ciUI.m_ciChatTextView.Clear();
  g_ciUI.m_ciChatTextView.PushText( g_strColorTagArray.Red + "Login Failed: " +
                                    ::rumGetStringByName( i_strReason + "_shared_StringID" ) );

  ::rumSchedule( g_ciCUO, g_ciCUO.ReturnToMainMenu, 5.0 );
}


function OnPlayerLoginSuccess()
{
  // Nothing to do on the client
}


function OnPlayerLogout( i_uiPlayerID )
{
  CloseSettingsMenu();
}


function OnSoundStopped( i_ciSound )
{
  if( g_ciCUO.m_ciPlayingMusic != i_ciSound || g_ciCUO.m_bShuttingDown )
  {
    return;
  }

  // Save a ref to the music track that's ending in case we need to unmanage it if it's not going to play again
  local ciPreviousMusic = g_ciCUO.m_ciPlayingMusic;

  // Determine the next music track
  if( GameMode.Game == g_ciCUO.m_eCurrentGameMode )
  {
    local ciPlayer = ::rumGetMainPlayer();
    local bInCombat = ciPlayer.GetProperty( Combat_PropertyID, false );
    if( bInCombat )
    {
      PlayCombatMusic();
    }
    else if( g_ciCUO.m_ciMerchantRef != null )
    {
      PlayMerchantMusic();
    }
    else
    {
      local ciMap = ciPlayer.GetMap();
      PlayMusic( GetMusicForMap( ciMap ) );
    }
  }
  else if( GameMode.Title == g_ciCUO.m_eCurrentGameMode )
  {
    PlayRandomMusic();
  }
}


function PlayCombatMusic()
{
  local eMusicID = U4_Combat_SoundID;

  switch( rand() % 3 )
  {
    case 0: eMusicID = U3_Combat_SoundID;               break;
    case 1: eMusicID = U4_Combat_SoundID;               break;
    case 2: eMusicID = U5_Engagement_And_Melee_SoundID; break;
  }

  PlayMusic( eMusicID );
}


function PlayMerchantMusic()
{
  local eMusicID = U4_Shopping_SoundID;

  switch( rand() % 3 )
  {
    case 0: eMusicID = U3_Shopping_SoundID;             break;
    case 1: eMusicID = U4_Shopping_SoundID;             break;
    case 2: eMusicID = U5_Capn_Johnes_Hornpipe_SoundID; break;
  }

  PlayMusic( eMusicID );
}


// Plays the specified sound only if the sound is not already playing
function PlayMusic( i_eSoundID )
{
  if( g_ciCUO.m_bMusicEnabled && !g_ciCUO.m_bShuttingDown )
  {
    local bPlayNewMusic = true;

    // The currently playing music track
    if( g_ciCUO.m_ciPlayingMusic != null )
    {
      // The current music track is the same as the requested track
      if( g_ciCUO.m_ciPlayingMusic.GetAssetID() == i_eSoundID )
      {
        if( g_ciCUO.m_ciPlayingMusic.IsPlaying() )
        {
          // The requested track is already active and playing
          bPlayNewMusic = false;
        }
      }
      else
      {
        // Allow the new track to play
        bPlayNewMusic = true;

        if( g_ciCUO.m_ciPlayingMusic.IsPlaying() )
        {
          // Fade out the current track
          g_ciCUO.m_ciPlayingMusic.Stop( 2.0 );
        }
      }
    }

    if( bPlayNewMusic )
    {
      // Create and play the new music track
      g_ciCUO.m_ciPlayingMusic = ::rumCreate( i_eSoundID );
      if( g_ciCUO.m_ciPlayingMusic != null )
      {
        g_ciCUO.m_ciPlayingMusic.Play( GetMusicVolume() );
      }
    }
  }
}


function PlayRandomMusic()
{
  local eAssetID = U4_Towns_SoundID;

  switch( rand() % 31 )
  {
    case 0:  eAssetID = U3_Alive_SoundID;          break;
    case 1:  eAssetID = U3_Castles_SoundID;        break;
    case 2:  eAssetID = U3_Combat_SoundID;         break;
    case 3:  eAssetID = U3_Dungeon_SoundID;        break;
    case 4:  eAssetID = U3_Exodus_SoundID;         break;
    case 5:  eAssetID = U3_Fanfare_SoundID;        break;
    case 6:  eAssetID = U3_Rule_Britannia_SoundID; break;
    case 7:  eAssetID = U3_Shopping_SoundID;       break;
    case 8:  eAssetID = U3_Towns_SoundID;          break;
    case 9:  eAssetID = U3_Wander_SoundID;         break;

    case 10: eAssetID = U4_Castles_SoundID;        break;
    case 11: eAssetID = U4_Combat_SoundID;         break;
    case 12: eAssetID = U4_Dungeon_SoundID;        break;
    case 13: eAssetID = U4_Rule_Britannia_SoundID; break;
    case 14: eAssetID = U4_Shopping_SoundID;       break;
    case 15: eAssetID = U4_Shrines_SoundID;        break;
    case 16: eAssetID = U4_Wander_SoundID;         break;
    case 17: eAssetID = U4_Towns_SoundID;          break;

    case 18: eAssetID = U5_Britannic_Lands_SoundID;          break;
    case 19: eAssetID = U5_Capn_Johnes_Hornpipe_SoundID;     break;
    case 20: eAssetID = U5_Dream_Of_Lady_Nan_SoundID;        break;
    case 21: eAssetID = U5_Engagement_And_Melee_SoundID;     break;
    case 22: eAssetID = U5_Fanfare_For_The_Virtuous_SoundID; break;
    case 23: eAssetID = U5_Greysons_Tale_SoundID;            break;
    case 24: eAssetID = U5_Halls_Of_Doom_SoundID;            break;
    case 25: eAssetID = U5_Joyous_Reunion_SoundID;           break;
    case 26: eAssetID = U5_Lord_Blackthorn_SoundID;          break;
    case 27: eAssetID = U5_Rule_Britannia_SoundID;           break;
    case 28: eAssetID = U5_Stones_SoundID;                   break;
    case 29: eAssetID = U5_Villager_Tarantella_SoundID;      break;
    case 30: eAssetID = U5_Worlds_Below_SoundID;             break;
  }

  PlayMusic( eAssetID );
}


function PlaySound( i_eSoundID )
{
  if( g_ciCUO.m_bSFXEnabled && !g_ciCUO.m_bShuttingDown )
  {
    local ciSound = ::rumCreate( i_eSoundID )
    if( ciSound != null )
    {
      return ciSound.Play( GetSFXVolume() );
    }
  }

  return false;
}


function PlaySound3D( i_eSoundID, i_ciPos )
{
  if( g_ciCUO.m_bSFXEnabled && !g_ciCUO.m_bShuttingDown )
  {
    local ciSound = ::rumCreate( i_eSoundID )
    if( ciSound != null )
    {
      return ciSound.Play3D( i_ciPos, GetSFXVolume() );
    }
  }

  return false;
}


function RenderAttacks( i_ciPos, i_bPeering )
{
  if( i_bPeering )
  {
    foreach( ciAttack in g_ciCUO.m_ciAttackEffectsTable )
    {
      local ciGraphic = ::rumGetGraphic( ciAttack.m_eGraphicID );
      if( ciGraphic )
      {
        // Attack effects coordinates are world-coordinates
        local x = ( ciAttack.m_ciPos.x - ( i_ciPos.x * g_ciUI.s_iTilePixelWidth ) ) / 2; // Scale down to 16-pixel
        local y = ( ciAttack.m_ciPos.y - ( i_ciPos.y * g_ciUI.s_iTilePixelWidth ) ) / 2;

        local ciAttackPos = rumPoint( g_ciUI.m_ciPlayerOffsetPos.x + x + g_ciUI.m_ciPeerMapRect.p1.x,
                                      g_ciUI.m_ciPlayerOffsetPos.y + y + g_ciUI.m_ciPeerMapRect.p1.y );

        if( g_ciUI.m_ciPeerMapRect.ContainsPoint( ciAttackPos ) )
        {
          local ciGdp = ciGraphic.GetAttributes();
          ciGdp.DrawScaled = true;
          ciGdp.HorizontalScale = 0.5;
          ciGdp.VerticalScale = 0.5;
          ciGraphic.SetAttributes( ciGdp );

          // Attack is on-screen
          ciGraphic.Draw( ciAttackPos );

          ciGdp.DrawScaled = false;
          ciGdp.HorizontalScale = 1.0;
          ciGdp.VerticalScale = 1.0;
          ciGraphic.SetAttributes( ciGdp );
        }
      }
    }

    if( InputMode.Target == g_ciUI.m_ciGameInputTextBox.m_eInputMode )
    {
      local x = g_ciCUO.m_ciTargetPos.x - i_ciPos.x;
      local y = g_ciCUO.m_ciTargetPos.y - i_ciPos.y;

      local ciTargetPos = rumPoint( g_ciUI.m_ciPlayerOffsetPos.x + ( x * g_ciUI.s_iTileHalfPixelWidth ) +
                                    g_ciUI.m_ciPeerMapRect.p1.x,
                                    g_ciUI.m_ciPlayerOffsetPos.y + ( y * g_ciUI.s_iTileHalfPixelWidth ) +
                                    g_ciUI.m_ciPeerMapRect.p1.y );

      if( g_ciUI.m_ciPeerMapRect.ContainsPoint( ciTargetPos ) )
      {
          local ciGraphic = ::rumGetGraphic( Target_GraphicID );

          local ciGdp = ciGraphic.GetAttributes();
          ciGdp.DrawScaled = true;
          ciGdp.HorizontalScale = 0.5;
          ciGdp.VerticalScale = 0.5;
          ciGraphic.SetAttributes( ciGdp );

          // Target is on-screen
          ciGraphic.Draw( ciTargetPos );

          ciGdp.DrawScaled = false;
          ciGdp.HorizontalScale = 1.0;
          ciGdp.VerticalScale = 1.0;
          ciGraphic.SetAttributes( ciGdp );
      }
    }
  }
  else
  {
    foreach( ciAttack in g_ciCUO.m_ciAttackEffectsTable )
    {
      local ciGraphic = ::rumGetGraphic( ciAttack.m_eGraphicID );
      if( ciGraphic )
      {
        // Attack effects coordinates are world-coordinates
        local x = ciAttack.m_ciPos.x - ( i_ciPos.x * g_ciUI.s_iTilePixelWidth );
        local y = ciAttack.m_ciPos.y - ( i_ciPos.y * g_ciUI.s_iTilePixelWidth );

        local ciAttackPos = rumPoint( g_ciUI.m_ciPlayerOffsetPos.x + x + g_ciUI.s_iBorderPixelWidth,
                                      g_ciUI.m_ciPlayerOffsetPos.y + y + g_ciUI.s_iBorderPixelWidth );

        if( g_ciUI.m_ciMapRect.ContainsPoint( ciAttackPos ) )
        {
          // Attack is on-screen
          ciGraphic.Draw( ciAttackPos );
        }
      }
    }

    if( InputMode.Target == g_ciUI.m_ciGameInputTextBox.m_eInputMode )
    {
      local x = g_ciCUO.m_ciTargetPos.x - i_ciPos.x;
      local y = g_ciCUO.m_ciTargetPos.y - i_ciPos.y;

      local ciTargetPos =
        rumPoint( g_ciUI.m_ciPlayerOffsetPos.x + ( x * g_ciUI.s_iTilePixelWidth ) + g_ciUI.s_iBorderPixelWidth,
                  g_ciUI.m_ciPlayerOffsetPos.y + ( y * g_ciUI.s_iTilePixelWidth ) + g_ciUI.s_iBorderPixelWidth );

      ::rumGetGraphic( Target_GraphicID ).Draw( ciTargetPos );
    }
  }
}


function RenderEffects( i_ciPlayer )
{
  local ciGraphic = ::rumGetGraphic( Effects_GraphicID );
  if( null == ciGraphic )
  {
    return;
  }

  local ciEffectPos = clone g_ciUI.m_ciMapRect.p1;

  // U4
  if( i_ciPlayer.IsJinxed() )
  {
    ciGraphic.DrawAnimation( ciEffectPos, 0, EffectType.Jinxed );
    ciEffectPos.x += g_ciUI.s_iBorderPixelWidth;
  }

  // U1-U4
  if( i_ciPlayer.GetLightRange() > 0 )
  {
    ciGraphic.DrawAnimation( ciEffectPos, 0, EffectType.Lighting );
    ciEffectPos.x += g_ciUI.s_iBorderPixelWidth;
  }

  // U1-U4
  if( i_ciPlayer.IsNegated() )
  {
    ciGraphic.DrawAnimation( ciEffectPos, 0, EffectType.Negated );
    ciEffectPos.x += g_ciUI.s_iBorderPixelWidth;
  }

  // U2-U4
  if( g_ciCUO.m_bPeering )
  {
    ciGraphic.DrawAnimation( ciEffectPos, 0, EffectType.Peering );
    ciEffectPos.x += g_ciUI.s_iBorderPixelWidth;
  }

  // U2-U4
  if( i_ciPlayer.IsPoisoned() )
  {
    ciGraphic.DrawAnimation( ciEffectPos, 0, EffectType.Poisoned );
    ciEffectPos.x += g_ciUI.s_iBorderPixelWidth;
  }

  // U2-U4
  if( i_ciPlayer.IsProtected() )
  {
    ciGraphic.DrawAnimation( ciEffectPos, 0, EffectType.Protected );
    ciEffectPos.x += g_ciUI.s_iBorderPixelWidth;
  }

  // U4
  if( i_ciPlayer.IsQuickened() )
  {
    ciGraphic.DrawAnimation( ciEffectPos, 0, EffectType.Quickened );
    ciEffectPos.x += g_ciUI.s_iBorderPixelWidth;
  }

  // U3-U4
  if( i_ciPlayer.IsUnconscious() )
  {
    ciGraphic.DrawAnimation( ciEffectPos, 0, EffectType.Unconscious );
    ciEffectPos.x += g_ciUI.s_iBorderPixelWidth;
  }

  // U3-U4
  if( i_ciPlayer.IsBurning() )
  {
    ciGraphic.DrawAnimation( ciEffectPos, 0, EffectType.Burning );
    ciEffectPos.x += g_ciUI.s_iBorderPixelWidth;
  }

  // U3-U4
  if( i_ciPlayer.IsFrozen() )
  {
    ciGraphic.DrawAnimation( ciEffectPos, 0, EffectType.Frozen );
    ciEffectPos.x += g_ciUI.s_iBorderPixelWidth;
  }

  // U4
  if( i_ciPlayer.IsDaemonImmune() )
  {
    ciGraphic.DrawAnimation( ciEffectPos, 0, EffectType.DaemonImmune );
    ciEffectPos.x += g_ciUI.s_iBorderPixelWidth;
  }

  // U1-U4
  if( i_ciPlayer.IsStarving() )
  {
    ciGraphic.DrawAnimation( ciEffectPos, 0, EffectType.Starving );
    ciEffectPos.x += g_ciUI.s_iBorderPixelWidth;
  }
}


function RenderGameOverride()
{
  if( g_ciUI.m_eOverrideGraphicID != rumInvalidAssetID )
  {
    local iIntensity = 0;

    local ciPlayer = ::rumGetMainPlayer();
    if( ciPlayer != null )
    {
      iIntensity = ciPlayer.GetScreenShakeIntensity().tointeger();
    }

    local iOffsetX = g_ciUI.s_iBorderPixelWidth;
    local iOffsetY = g_ciUI.s_iBorderPixelWidth;

    if( iIntensity > 0 )
    {
      // Determine a random offset within the +/- iIntensity range
      iOffsetX += ( ( rand() % ( iIntensity * 2 + 1 ) ) - iIntensity );
      iOffsetY += ( ( rand() % ( iIntensity * 2 + 1 ) ) - iIntensity );
    }

    local ciGraphic = ::rumGetGraphic( g_ciUI.m_eOverrideGraphicID );
    ciGraphic.DrawAnimation( rumPoint( iOffsetX, iOffsetY ),
                             g_ciUI.m_ciOverrideGraphicOffset.x, g_ciUI.m_ciOverrideGraphicOffset.y );

    return true;
  }

  return false;
}


function RenderHitpointsAndMana( i_ciPlayer, i_iX, i_iY, i_eHitpointsPropertyID, i_eManaPropertyID )
{
  local iHitpoints = i_ciPlayer.GetProperty( i_eHitpointsPropertyID, 0 );
  if( iHitpoints <= 0 )
  {
    // Player is dead - don't show hitpoints or mana
    return;
  }

  local iMaxHitpoints = 0;
  local ciTransport = i_ciPlayer.GetTransport();
  if( ciTransport != null && ciTransport.GetProperty( Widget_Destructible_PropertyID, false ) )
  {
    iHitpoints = ciTransport.GetProperty( Hitpoints_PropertyID, 0 );
    iMaxHitpoints = ciTransport.GetProperty( Max_Hitpoints_PropertyID, 1 );
  }
  else
  {
    iMaxHitpoints = i_ciPlayer.GetMaxHitpoints();
  }

  local iMaxMana = i_ciPlayer.GetMaxMana();
  if( iMaxMana > 0 )
  {
    // Draw background
    ::rumDrawRect( rumRect( rumPoint( i_iX - 1, i_iY - 1 ), i_iX + g_ciUI.s_iTilePixelWidth, i_iY + 6 ),
                   g_ciUI.s_ciColorLightBlue );

    local iMana = i_ciPlayer.GetProperty( i_eManaPropertyID, 0 );
    local fManaPercent = iMana / iMaxMana.tofloat();
    fManaPercent = clamp( fManaPercent, 0.0, 1.0 );

    local fWidth = fManaPercent * g_ciUI.s_iTilePixelWidth;
    local iWidth = fWidth.tointeger();

    local xOffset = i_iX + iWidth;

    if( fManaPercent < 1.0 )
    {
      ::rumDrawRect( rumRect( rumPoint( xOffset, i_iY + 3 ),
                              xOffset + ( g_ciUI.s_iTilePixelWidth - iWidth ) - 1, i_iY + 5 ),
                     g_ciUI.s_ciColorMissingMana );
    }

    if( fManaPercent > 0.0 )
    {
      ::rumDrawRect( rumRect( rumPoint( i_iX, i_iY + 3 ), xOffset - 1, i_iY + 5 ), ::rumColorBlue );
    }
  }
  else
  {
    // No mana, so offset hitpoint display location
    i_iY += 3;

    // Draw background
    ::rumDrawRect( rumRect( rumPoint( i_iX - 1, i_iY - 1 ), i_iX + g_ciUI.s_iTilePixelWidth, i_iY + 3 ),
                   g_ciUI.s_ciColorLightBlue );
  }

  local fHitpointPercent = iHitpoints / iMaxHitpoints.tofloat();
  fHitpointPercent = clamp( fHitpointPercent, 0.0, 1.0 );

  local fWidth = fHitpointPercent * g_ciUI.s_iTilePixelWidth;
  local iWidth = fWidth.tointeger();

  local xOffset = i_iX + iWidth;

  if( fHitpointPercent < 1.0 )
  {
    ::rumDrawRect( rumRect( rumPoint( xOffset, i_iY ), xOffset + ( g_ciUI.s_iTilePixelWidth - iWidth ) - 1, i_iY + 2 ),
                   g_ciUI.s_ciColorMissingHitpoints );
  }

  if( fHitpointPercent > 0.0 )
  {
    ::rumDrawRect( rumRect( rumPoint( i_iX, i_iY ), xOffset - 1, i_iY + 2 ),
                   ciTransport != null ? ::rumColorYellow : ::rumColorRed );
  }
}


function RenderOffscreenMemberIcons( i_ciPlayer, i_eGraphicProperty, i_iRadius )
{
  // Show off-screen party members on the edge of the border
  if( g_ciCUO.m_uiPartyIDTable != null && g_ciCUO.m_uiPartyIDTable.len() > 1 )
  {
    local uiPlayerID = i_ciPlayer.GetID();
    local ciMap = i_ciPlayer.GetMap();
    local ciPos = i_ciPlayer.GetPosition();

    foreach( uiMemberID in g_ciCUO.m_uiPartyIDTable )
    {
      if( uiPlayerID != uiMemberID )
      {
        local ciMember = ::rumFetchPawn( uiMemberID );
        if( ciMember != null && ( ciMember.GetMap() == ciMap ) )
        {
          local ciTargetPos = ciMember.GetPosition();

          local x = ciTargetPos.x - ciPos.x;
          local y = ciTargetPos.y - ciPos.y;

          if( abs( x ) <= i_iRadius && abs( y ) <= i_iRadius )
          {
            // Member is on screen
            continue;
          }

          local bIntersectFound = false;
          local ciIntersectPos = null;

          // The line between the player and party member using screen coordinates
          local ciPointB1 = g_ciUI.m_ciPlayerCenterPos;
          local ciPointB2 = rumPoint( ciPointB1.x - ( x * g_ciUI.s_iTilePixelWidth ),
                                      ciPointB1.y - ( y * g_ciUI.s_iTilePixelWidth ) );

          if( ciTargetPos.y < ciPos.y )
          {
            // The line that defines the top edge of the play area, offset inward
            local ciPointA1 = rumPoint( g_ciUI.m_ciMapRect.p1.x,
                                        g_ciUI.m_ciMapRect.p1.y + g_ciUI.s_iTileQuarterPixelWidth );
            local ciPointA2 = rumPoint( g_ciUI.m_ciMapRect.p2.x,
                                        g_ciUI.m_ciMapRect.p1.y + g_ciUI.s_iTileQuarterPixelWidth );

            ciIntersectPos = rumFindLineIntersection( ciPointA1, ciPointA2, ciPointB1, ciPointB2 );
            if( ciIntersectPos.x >= g_ciUI.m_ciMapRect.p1.x && ciIntersectPos.x <= g_ciUI.m_ciMapRect.p2.x )
            {
              bIntersectFound = true;
            }
          }
          else if( ciTargetPos.y > ciPos.y )
          {
            // The line that defines the bottom edge of the play area, offset inward
            local ciPointA1 = rumPoint( g_ciUI.m_ciMapRect.p1.x,
                                        g_ciUI.m_ciMapRect.p2.y - g_ciUI.s_iTileQuarterPixelWidth );
            local ciPointA2 = rumPoint( g_ciUI.m_ciMapRect.p2.x,
                                        g_ciUI.m_ciMapRect.p2.y - g_ciUI.s_iTileQuarterPixelWidth );

            ciIntersectPos = rumFindLineIntersection( ciPointA1, ciPointA2, ciPointB1, ciPointB2 );
            if( ciIntersectPos.x >= g_ciUI.m_ciMapRect.p1.x && ciIntersectPos.x <= g_ciUI.m_ciMapRect.p2.x )
            {
              bIntersectFound = true;
            }
          }

          if( !bIntersectFound )
          {
            if( ciTargetPos.x < ciPos.x )
            {
              // The line that defines the left edge of the play area, offset inward
              local ciPointA1 = rumPoint( g_ciUI.m_ciMapRect.p1.x + g_ciUI.s_iTileQuarterPixelWidth,
                                          g_ciUI.m_ciMapRect.p1.y );
              local ciPointA2 = rumPoint( g_ciUI.m_ciMapRect.p1.x + g_ciUI.s_iTileQuarterPixelWidth,
                                          g_ciUI.m_ciMapRect.p2.y );

              ciIntersectPos = rumFindLineIntersection( ciPointA1, ciPointA2, ciPointB1, ciPointB2 );
              if( ciIntersectPos.y >= g_ciUI.m_ciMapRect.p1.y && ciIntersectPos.y <= g_ciUI.m_ciMapRect.p2.y )
              {
                bIntersectFound = true;
              }
            }
            else if( ciTargetPos.x > ciPos.x )
            {
              // The line that defines the right edge of the play area, offset inward
              local ciPointA1 = rumPoint( g_ciUI.m_ciMapRect.p2.x - g_ciUI.s_iTileQuarterPixelWidth,
                                          g_ciUI.m_ciMapRect.p1.y );
              local ciPointA2 = rumPoint( g_ciUI.m_ciMapRect.p2.x - g_ciUI.s_iTileQuarterPixelWidth,
                                          g_ciUI.m_ciMapRect.p2.y );

              ciIntersectPos = rumFindLineIntersection( ciPointA1, ciPointA2, ciPointB1, ciPointB2 );
              if( ciIntersectPos.y >= g_ciUI.m_ciMapRect.p1.y && ciIntersectPos.y <= g_ciUI.m_ciMapRect.p2.y )
              {
                bIntersectFound = true;
              }
            }
          }

          if( bIntersectFound )
          {
            local eGraphicID = ciMember.GetProperty( i_eGraphicProperty, U4_Adventurer_GraphicID );
            local ciGraphic = ::rumGetGraphic( eGraphicID );

            local ciGdpBackup = ciGraphic.GetAttributes();

            local ciGdp = ciGraphic.GetAttributes();
            ciGdp.DrawScaled = true;
            ciGdp.HorizontalScale = 0.5;
            ciGdp.VerticalScale = 0.5;
            ciGraphic.SetAttributes( ciGdp );

            // Adjust away from the center point to the upper left draw point for the graphic
            ciIntersectPos.x -= g_ciUI.s_iTileQuarterPixelWidth;
            ciIntersectPos.y -= g_ciUI.s_iTileQuarterPixelWidth;

            ciIntersectPos.x = clamp( ciIntersectPos.x,
                                      g_ciUI.m_ciMapRect.p1.x,
                                      g_ciUI.m_ciMapRect.p2.x - g_ciUI.s_iTileHalfPixelWidth );
            ciIntersectPos.y = clamp( ciIntersectPos.y,
                                      g_ciUI.m_ciMapRect.p1.y,
                                      g_ciUI.m_ciMapRect.p2.y - g_ciUI.s_iTileHalfPixelWidth );

            ciGraphic.DrawAnimation( ciIntersectPos );

            ciGraphic.SetAttributes( ciGdpBackup );

            // Draw border around the party member's direction icon
            local ciRect = rumRect( ciIntersectPos,
                                    g_ciUI.s_iTileHalfPixelWidth + 1,
                                    g_ciUI.s_iTileHalfPixelWidth + 1 );
            ::rumDrawRectUnfilled( ciRect, ::rumColorYellow );
          }
        }
      }
    }
  }
}


function RenderOverlay()
{
  if( rumInvalidAssetID == g_ciUI.m_eOverlayGraphicID )
  {
    return;
  }

  local ciGraphic = ::rumGetGraphic( g_ciUI.m_eOverlayGraphicID );
  if( ciGraphic != null )
  {
    local iOffsetX = ::rumGetScreenWidth() / 2 - ciGraphic.GetFrameWidth() / 2;
    local iOffsetY = ::rumGetScreenHeight() / 2 - ciGraphic.GetFrameHeight() / 2;

    ciGraphic.DrawAnimation( rumPoint( iOffsetX, iOffsetY ),
                             g_ciUI.m_ciOverlayGraphicOffset.x,
                             g_ciUI.m_ciOverlayGraphicOffset.y );
  }
}


function RenderNamesAndVitals( i_ciPlayer, i_eHitpointsProperty, i_eManaProperty, i_bPeering, i_iRadius )
{
  if( !( g_ciUI.m_bShowNames || g_ciUI.m_bShowVitals ) )
  {
    return;
  }

  local ciMap = i_ciPlayer.GetMap();
  local ciPos = i_ciPlayer.GetPosition();

  local bCheckLOS = true;
  local iTileSize = g_ciUI.s_iTilePixelWidth;

  if( i_bPeering )
  {
    bCheckLOS = false;
    iTileSize = g_ciUI.s_iTileHalfPixelWidth;
  }

  local ciPlayerArray = ciMap.GetPlayers( ciPos, i_iRadius, bCheckLOS );
  local bCheckLight = !i_bPeering && MapRequiresLight( ciMap );

  local iNameHeight = ::rumGetTextHeight( "small" );

  // Show name label for each visible player, and show hitpoint and mana info for players grouped in the
  // the main player's party
  foreach( ciTargetPlayer in ciPlayerArray )
  {
    local ciTargetPos = ciTargetPlayer.GetPosition();

    local x = ciTargetPos.x - ciPos.x;
    local y = ciTargetPos.y - ciPos.y;

    // Width plus padding
    local iNameWidth = ::rumGetTextWidth( "small", ciTargetPlayer.GetPlayerName() ) +
                       ( g_ciUI.s_iTextPixelPadding * 2 );
    local iNameX = g_ciUI.m_ciPlayerCenterPos.x + ( x * iTileSize ) - iNameWidth / 2;
    local iNameY = g_ciUI.m_ciPlayerCenterPos.y - ( iTileSize / 2 ) + ( y * iTileSize ) - iNameHeight;

    local iStatX = g_ciUI.m_ciPlayerCenterPos.x + ( x * iTileSize ) - g_ciUI.s_iTileHalfPixelWidth;
    local iStatY = iNameY - ( g_ciUI.m_bShowNames ? g_ciUI.s_iTextPixelPadding : 0 );

    // Reposition label if it goes off-screen
    if( iNameX < g_ciUI.m_ciMapRect.p1.x )
    {
      iNameX = g_ciUI.m_ciMapRect.p1.x;
    }
    else if( ( iNameX + iNameWidth ) > g_ciUI.m_ciMapRect.p2.x )
    {
      iNameX = g_ciUI.m_ciMapRect.p2.x - iNameWidth;
    }
    else if( iNameY < g_ciUI.m_ciMapRect.p1.y )
    {
      iNameY = g_ciUI.m_ciMapRect.p1.y;
      iStatY = g_ciUI.m_ciMapRect.p1.y;
    }

    local bVisible = true;
    if( bCheckLight )
    {
      // The environment requires tiles to be lit
      local ciTilePos = rumPos( ciTargetPos.x - ciPos.x + GameSingleton.s_iLOSRadius,
                                ciTargetPos.y - ciPos.y + GameSingleton.s_iLOSRadius );
      if( !ciMap.IsPositionLit( ciTilePos ) )
      {
        bVisible = false;
      }
    }

    if( bVisible )
    {
      if( g_ciUI.m_bShowNames )
      {
        ciTargetPlayer.ShowNameLabel( iNameX, iNameY );
      }

      if( g_ciUI.m_bShowVitals )
      {
        local bInParty = ( i_ciPlayer == ciTargetPlayer ) || i_ciPlayer.PlayerInParty( ciTargetPlayer );
        if( bInParty )
        {
          RenderHitpointsAndMana( ciTargetPlayer, iStatX, iStatY, i_eHitpointsProperty, i_eManaProperty );
        }
      }
    }
  }
}


function RenderStatArmour( i_eVersion )
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_armour_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();
  g_ciUI.m_ciStatListView.SetFormat( "0.05" );
  g_ciUI.m_ciStatListView.ShowPrompt( true );

  // Iterate over the player's armour and build the list
  local iIndex = 0;

  local ciPlayer = ::rumGetMainPlayer();
  local ciArmour = ciPlayer.GetEquippedArmour();
  local uiArmourID = rumInvalidGameID;
  if( ciArmour != null )
  {
    uiArmourID = ciArmour.GetID();

    local strName = ciArmour.GetName() + "_client_StringID";
    g_ciUI.m_ciStatListView.SetEntry( iIndex,
                                      ::rumGetStringByName( strName ) + " (" +
                                      ::rumGetString( token_armour_worn_client_StringID ) + ")" );
    ++iIndex;
  }

  local ciInventory = ciPlayer.GetInventory();
  local ciItem;
  while( ciItem = ciInventory.GetNextObject() )
  {
    if( null == ciItem )
    {
      continue;
    }

    if( ciItem.GetID() != uiArmourID )
    {
      // Be sure the item is armour
      local eType = ciItem.GetProperty( Inventory_Type_PropertyID, InventoryType.Standard );
      if( InventoryType.Armour == eType )
      {
        // Be sure the item makes sense for the requested game version
        local eVersion = ciItem.GetProperty( Ultima_Version_PropertyID, GameType.Invalid );
        if( i_eVersion == eVersion )
        {
          local strName = ciItem.GetName() + "_client_StringID";
          g_ciUI.m_ciStatListView.SetEntry( iIndex, ::rumGetStringByName( strName ) );
          ++iIndex;
        }
      }
    }
  }
}


function RenderStatEquipment( i_eEquipmentPropertyArray )
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_equipment_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();

  g_ciUI.m_ciStatListView.SetFormat( "0.05|0.5" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local iOffset = 0;
  local ciPlayer = ::rumGetMainPlayer();

  foreach( ePropertyID in i_eEquipmentPropertyArray )
  {
    local ciAsset = ::rumGetPropertyAsset( ePropertyID );
    local iValue = ciPlayer.GetProperty( ePropertyID, 0 );
    local strEntry = ::rumGetStringByName( ciAsset.GetName() + "_Property_client_StringID" );
    g_ciUI.m_ciStatListView.SetEntry( ++iOffset, strEntry + "|" + iValue );
  }
}


function RenderStatParty( i_eGraphicPropertyID,
                          i_eDefaultGraphicPropertyID,
                          i_eClassPropertyID,
                          i_eDefaultClassPropertyID )
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_party_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();
  g_ciUI.m_ciStatListView.SetFormat( "0.0|0.1|0.4|0.6" );
  g_ciUI.m_ciStatListView.ShowPrompt( false );

  local iIndex = 0;
  foreach( uiMemberID in g_ciCUO.m_uiPartyIDTable )
  {
    local ciMember = ::rumFetchPawn( uiMemberID );
    if( ciMember != null && ( ciMember instanceof Player ) )
    {
      // Determine the player's icon
      local eGraphicID = ciMember.GetProperty( i_eGraphicPropertyID, i_eDefaultGraphicPropertyID );
      local ciGraphic = ::rumGetGraphic( eGraphicID );

      // Determine the player's class
      local ePlayerClassID = ciMember.GetProperty( i_eClassPropertyID, i_eDefaultClassPropertyID );
      local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );

      // What effects are on the player?
      local strEffects = "";
      if( ciMember.IsJinxed() )
      {
        strEffects += format( "<G#Effects:h16:w16:f%d>", EffectType.Jinxed );
      }

      if( ciMember.IsNegated() )
      {
        strEffects += format( "<G#Effects:h16:w16:f%d>", EffectType.Negated );
      }

      if( ciMember.IsPoisoned() )
      {
        strEffects += format( "<G#Effects:h16:w16:f%d>", EffectType.Poisoned );
      }

      if( ciMember.IsProtected() )
      {
        strEffects += format( "<G#Effects:h16:w16:f%d>", EffectType.Protected );
      }

      if( ciMember.IsQuickened() )
      {
        strEffects += format( "<G#Effects:h16:w16:f%d>", EffectType.Quickened );
      }

      if( ciMember.IsUnconscious() )
      {
        strEffects += format( "<G#Effects:h16:w16:f%d>", EffectType.Unconscious );
      }

      if( ciMember.IsBurning() )
      {
        strEffects += format( "<G#Effects:h16:w16:f%d>", EffectType.Burning );
      }

      if( ciMember.IsFrozen() )
      {
        strEffects += format( "<G#Effects:h16:w16:f%d>", EffectType.Frozen );
      }

      if( ciMember.IsStarving() )
      {
        strEffects += format( "<G#Effects:h16:w16:f%d>", EffectType.Starving );
      }

      local strEntry = "";
      if( ciGraphic != null && ciPlayerClass != null )
      {
        local strClass = ::rumGetStringByName( ciPlayerClass.GetName() + "_client_StringID" );
        strEntry = format( "<G#%s:w16:h16>|%s|%s|%s",
                           ciGraphic.GetName(), ciMember.GetPlayerName(), strClass, strEffects );
      }

      g_ciUI.m_ciStatListView.SetEntry( iIndex++, strEntry );
    }
  }
}


function RenderStatSpells( i_eTokenStringID, i_eSpellArray, i_eSpellPropertyArray )
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( i_eTokenStringID ) );

  g_ciUI.m_ciStatListView.Clear();
  g_ciUI.m_ciStatListView.SetFormat( "2.0|0.12" );

  local ciPlayer = ::rumGetMainPlayer();

  // Build the list of spells
  foreach( iIndex, eSpellID in i_eSpellPropertyArray )
  {
    local iNum = ciPlayer.GetProperty( eSpellID );
    if( iNum )
    {
      local ciSpell = ::rumGetCustomAsset( i_eSpellArray[iIndex] );
      local strEntry = iNum + "| - " + ::rumGetStringByName( ciSpell.GetName() + "_client_StringID" );
      g_ciUI.m_ciStatListView.SetEntry( iIndex, strEntry );
    }
  }
}


function RenderStatWeapons( i_eVersion )
{
  g_ciUI.m_ciStatLabel.SetText( ::rumGetString( token_weapons_client_StringID ) );

  g_ciUI.m_ciStatListView.Clear();
  g_ciUI.m_ciStatListView.SetFormat( "0.05" );
  g_ciUI.m_ciStatListView.ShowPrompt( true );

  // Iterate over the player's weapons and build the list
  local iIndex = 0;

  local ciPlayer = ::rumGetMainPlayer();
  local ciWeapon = ciPlayer.GetEquippedWeapon();
  local uiEquippedWeaponID = rumInvalidGameID;
  if( ciWeapon != null )
  {
    uiEquippedWeaponID = ciWeapon.GetID();

    local strName = ciWeapon.GetName() + "_client_StringID";
    strName = ::rumGetStringByName( strName );

    if( ciWeapon.GetProperty( Inventory_Stacks_PropertyID, false ) )
    {
      strName += " (" + ciWeapon.GetProperty( Inventory_Quantity_PropertyID, 1 ) + ")";
    }

    strName += " (" + ::rumGetString( token_equipped_client_StringID ) + ")";
    g_ciUI.m_ciStatListView.SetEntry( iIndex, strName );

    ++iIndex;
  }

  local ciInventory = ciPlayer.GetInventory();
  local ciItem;
  while( ciItem = ciInventory.GetNextObject() )
  {
    if( ( null == ciItem ) || ciItem.GetID() == uiEquippedWeaponID )
    {
      continue;
    }

    local eType = ciItem.GetProperty( Inventory_Type_PropertyID, InventoryType.Standard );
    if( InventoryType.Weapon == eType )
    {
      // Be sure the item makes sense for the requested game version
      local eVersion = ciItem.GetProperty( Ultima_Version_PropertyID, GameType.Invalid );
      if( i_eVersion == eVersion )
      {
        local strName = ciItem.GetName() + "_client_StringID";
        strName = ::rumGetStringByName( strName );

        if( ciItem.GetProperty( Inventory_Stacks_PropertyID, false ) )
        {
          strName += " (" + ciItem.GetProperty( Inventory_Quantity_PropertyID, 1 ) + ")";
        }

        g_ciUI.m_ciStatListView.SetEntry( iIndex, strName );
        ++iIndex;
      }
    }
  }
}


function RenderTransition()
{
  ::rumGetGraphic( Border_GraphicID ).Draw( rumPoint() );
  ::rumGetGraphic( U4_Transfer_GraphicID ).Draw( rumPoint( g_ciUI.s_iBorderPixelWidth, g_ciUI.s_iBorderPixelWidth ) );
  g_ciUI.m_ciChatTextView.Display( rumPoint( g_ciUI.s_iBorderPixelWidth, 400 ) );
}


// Stops the currently playing music track, but only if it's combat music
function StopCombatMusic()
{
  if( g_ciCUO.m_ciPlayingMusic != null )
  {
    switch( g_ciCUO.m_ciPlayingMusic.GetAssetID() )
    {
      case U3_Combat_SoundID:
      case U4_Combat_SoundID:
      case U5_Engagement_And_Melee_SoundID:
        g_ciCUO.m_ciPlayingMusic.Stop( 2.0 );
        break;
    }
  }
}


// Stops the currently playing music track
function StopMusic()
{
  if( g_ciCUO.m_ciPlayingMusic != null )
  {
    g_ciCUO.m_ciPlayingMusic.Stop( 2.0 );
  }
}


function Ultima_AdvanceMapLabel()
{
  ++g_ciUI.m_eMapLabelType;
  Ultima_UpdateMapLabel();
}


function Ultima_Attack( i_bForceRanged )
{
  local ciPlayer = ::rumGetMainPlayer();

  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local eTransportType = TransportType.None;
  local ciTransport = ciPlayer.GetTransport();
  if( ciTransport != null )
  {
    eTransportType = ciTransport.GetType();
  }

  if( eTransportType != TransportType.SpaceShip )
  {
    local ciWeapon = ciPlayer.GetEquippedWeapon();
    local bThrowable = false;
    local iRange = 1;
    if( ciWeapon != null )
    {
      iRange = ciWeapon.GetProperty( Inventory_Weapon_Attack_Range_PropertyID, 1 );
      bThrowable = ciWeapon.GetProperty( Inventory_Weapon_Throwable_PropertyID, false );
    }

    if( i_bForceRanged || ( iRange > 1 && !bThrowable ) )
    {
      // This is a ranged-only weapon, or a melee weapon that can also be thrown
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Target, Ultima_Attack_Target );

      local strDesc = format( "%s: <%s>",
                              ::rumGetString( command_attack_client_StringID ),
                              ::rumGetString( token_target_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetText( strDesc );

      InitTarget( iRange, ComparatorEnemy );
    }
    else
    {
      // Melee weapon
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction, Ultima_Attack_Dir );

      local strDesc = format( "%s: <%s>",
                              ::rumGetString( command_attack_client_StringID ),
                              ::rumGetString( token_direction_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetText( strDesc );
    }
  }
  else
  {
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
  }
}


function Ultima_Attack_Dir( i_eDir )
{
  local ciPlayer = ::rumGetMainPlayer();
  BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );

  local ciBroadcast = ::rumCreate( Player_Attack_BroadcastID, AttackType.Directional, i_eDir );
  ::rumSendBroadcast( ciBroadcast );
}


function Ultima_Attack_Fast( i_eDir )
{
print("===========Ultima_Attack_Fast===========\n");
  if( g_ciUI.m_bShowCombat )
  {
    local strDesc = format( "%s: %s", ::rumGetString( command_attack_client_StringID ), GetDirectionString( i_eDir ) );
    ShowString( "<G#Prompt:vcenter>" + strDesc );
  }

  local ciPlayer = ::rumGetMainPlayer();

  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local eTransportType = TransportType.None;
  local ciTransport = ciPlayer.GetTransport();
  if( ciTransport != null )
  {
    eTransportType = ciTransport.GetType();
  }

  if( eTransportType != TransportType.SpaceShip )
  {
    local ciWeapon = ciPlayer.GetEquippedWeapon();
    local iWeaponRange = 1;
    if( ciWeapon != null )
    {
      iWeaponRange = ciWeapon.GetProperty( Inventory_Weapon_Attack_Range_PropertyID, 1 );
      local bThrowable = ciWeapon.GetProperty( Inventory_Weapon_Throwable_PropertyID, false );
      if( bThrowable )
      {
        // The fast attack method does not perform weapon throws
        iWeaponRange = 1;
      }
      else if( ciWeapon.GetAssetID() == U4_Halberd_Weapon_InventoryID )
      {
        iWeaponRange = 2;
      }
    }

    local iRange = 1;
    local ciPlayerPos = ciPlayer.GetPosition();
    local ciTargetPos = ciPlayerPos + GetDirectionVector( i_eDir );

    if( Direction.None == i_eDir )
    {
      // Player is purposefully attacking their own position
      iRange = 0;
      iWeaponRange = 0;
      ciTargetPos = ciPlayerPos;
    }

    local ciMap = ciPlayer.GetMap();
    local ciTarget = null;

    // TODO - not doing the null == ciTarget check causes the player to target the creature farthest away which
    // could be a cool feature
    while( ( null == ciTarget ) && iRange <= iWeaponRange )
    {
      // Find a target within range and line of sight
      if( ciMap.TestLOS( ciPlayerPos, ciTargetPos, iRange ) )
      {
        local ciPosData = ciMap.GetPositionData( ciTargetPos );
        local ciCreature;
        while( ciCreature = ciPosData.GetNext( rumCreaturePawnType ) )
        {
          if( ciCreature != ciPlayer && ciCreature.IsVisible() && !ciCreature.IsDead() &&
              !ciPlayer.PlayerInParty( ciCreature ) )
          {
            ciTarget = ciCreature;
            ciPosData.Stop();
          }
        }

        if( null == ciTarget )
        {
          // Was the intended target a widget?
          ciPosData.Reset();

          local ciWidget;
          while( ciWidget = ciPosData.GetNext( rumWidgetPawnType ) )
          {
            if( ciWidget.IsVisible() )
            {
              local bDestructible = ciWidget.GetProperty( Widget_Destructible_PropertyID, false );
              if( bDestructible )
              {
                ciTarget = ciWidget;
                ciPosData.Stop();
              }
            }
          }
        }
      }

      ++iRange;
      ciTargetPos = ciTargetPos + GetDirectionVector( i_eDir );
    }

    if( ciTarget != null && ciTarget != ciPlayer )
    {
      local ciBroadcast = ::rumCreate( Player_Attack_BroadcastID, AttackType.Targeted, ciTarget.GetID() );
      ::rumSendBroadcast( ciBroadcast );
    }
    else
    {
      ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
    }
  }
  else
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
  }

  BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
}


function Ultima_Attack_Target( i_uiTargetID )
{
  local ciPlayer = ::rumGetMainPlayer();
  BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );

  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    return;
  }

  local bTargetFound = false;

  if( g_ciUI.m_bShowCombat )
  {
    local strDesc = format( "%s: %s", ::rumGetString( command_attack_client_StringID ), "target" );
    ShowString( "<G#Prompt:vcenter>" + strDesc );
  }

  if( i_uiTargetID != rumInvalidGameID )
  {
    local ciTarget = ::rumFetchPawn( i_uiTargetID );
    if( ciTarget != null && ciTarget != ciPlayer && ( ciTarget instanceof Creature || ciTarget instanceof Widget ) )
    {
      bTargetFound = true;

      local ciBroadcast = ::rumCreate( Player_Attack_BroadcastID, AttackType.Targeted, i_uiTargetID );
      ::rumSendBroadcast( ciBroadcast );
    }
  }

  if( !bTargetFound )
  {
    ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
  }
}


function Ultima_Board()
{
  local bDelay = true;
  local ciPlayer = ::rumGetMainPlayer();

  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  if( ciPlayer.IsFlying() )
  {
    ShowString( ::rumGetString( msg_must_land_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction, Ultima_BoardCallback );

  local strDesc = format( "%s: <%s>",
                          ::rumGetString( command_board_client_StringID ),
                          ::rumGetString( token_direction_client_StringID ) );
  g_ciUI.m_ciGameInputTextBox.SetText( strDesc );
}


function Ultima_BoardCallback( i_eDir )
{
  local ciPlayer = ::rumGetMainPlayer();
  local ciPos = ciPlayer.GetPosition() + GetDirectionVector( i_eDir );
  local ciMap = ciPlayer.GetMap();

  // First, check for any transports at the target location
  local uiTargetID = rumInvalidGameID;
  local ciWidget;
  local ciPosData = ciMap.GetPositionData( ciPos );
  while( ciWidget = ciPosData.GetNext( rumWidgetPawnType ) )
  {
    if( ciWidget.IsVisible() && ciWidget instanceof Transport_Widget )
    {
      uiTargetID = ciWidget.GetID();
      break;
    }
  }

  if( rumInvalidGameID == uiTargetID )
  {
    // Trying to steal a horse?
    local ciCreature;
    local ciPosData = ciMap.GetPositionData( ciPos );
    while( ciCreature = ciPosData.GetNext( rumCreaturePawnType ) )
    {
      if( ciCreature.IsVisible() && ciCreature.GetProperty( Creature_Is_Horse_PropertyID, false ) )
      {
        local eAssetID = ciCreature.GetAssetID();
        uiTargetID = ciCreature.GetID();
        break;
      }
    }
  }

  local eDelay = ActionDelay.Short;

  if( uiTargetID != rumInvalidGameID )
  {
    local ciBroadcast = ::rumCreate( Player_Board_BroadcastID, uiTargetID );
    ::rumSendBroadcast( ciBroadcast );

    eDelay = ActionDelay.Long;
  }
  else
  {
    ShowString( ::rumGetString( msg_board_what_client_StringID ), g_strColorTagArray.Red );
  }

  BlockInput( ciPlayer.GetActionDelay( eDelay ) );
}


function Ultima_CastSpellSelectionChanged( i_iIndex )
{
  local strSpell = g_ciUI.m_ciGameListView.GetCurrentEntry();
  if( strSpell )
  {
    local strFmt = format( "%s: %s", ::rumGetString( token_spell_client_StringID ), strSpell );
    g_ciUI.m_ciGameInputTextBox.SetText( strFmt );
  }
  else
  {
    g_ciUI.m_ciGameInputTextBox.Clear();
  }
}


function Ultima_Descend()
{
  local ciPlayer = ::rumGetMainPlayer();

  if( ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local ciTransport = ciPlayer.GetTransport();
  local eTransportType = ciTransport != null ? ciTransport.GetType() : TransportType.None;
  if( eTransportType != TransportType.None && !ciPlayer.IsDead() )
  {
    local ciMap = ciPlayer.GetMap();
    local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );

    if( ciPlayer.IsFlying() )
    {
      // Be sure the transport is above a compatible tile
      local ciPosData = ciMap.GetPositionData( ciPlayer.GetPosition() );
      local eTileID = ciPosData.GetTileID();
      if( ciTransport.CanLandOnTile( eTileID ) )
      {
        local ciBroadcast = ::rumCreate( Player_Transport_BroadcastID, TransportCommandType.Movement,
                                         VerticalDirectionType.Down );
        ::rumSendBroadcast( ciBroadcast );
      }
      else
      {
        ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
      }
    }
    else if( TransportType.Balloon == eTransportType )
    {
      ShowString( ::rumGetString( msg_already_landed_client_StringID ), g_strColorTagArray.Red );
    }
    else if( ( TransportType.Rocket == eTransportType ) && ( MapType.Space == eMapType ) )
    {
      local ciBroadcast = ::rumCreate( Player_Transport_BroadcastID, TransportCommandType.Movement,
                                       VerticalDirectionType.Down );
      ::rumSendBroadcast( ciBroadcast );
    }
    else if( TransportType.SpaceShip == eTransportType )
    {
      ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    }
    else
    {
      ShowString( ::rumGetString( msg_only_on_foot_client_StringID ), g_strColorTagArray.Red );
    }

    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
  }
  else
  {
    UsePortal( PortalUsageType.Descend );
  }
}


function Ultima_ExtinguishLight()
{
  local strDesc = ::rumGetString( command_extinguish_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  local ciPlayer = ::rumGetMainPlayer();

  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() || ( ciPlayer.GetLightRange() == 0 ) )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  // Extinguish any current lighting effects (torch or spell)
  local ciBroadcast = ::rumCreate( Player_Ignite_BroadcastID, false );
  ::rumSendBroadcast( ciBroadcast );

  BlockInput( ciPlayer.GetActionDelay( ActionDelay.Long ) );
}


function Ultima_FireTransportWeapon()
{
  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_fire_client_StringID ) );
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local uiStringID = msg_fire_what_client_StringID;
  local bSuccess = false;

  local ciTransport = ciPlayer.GetTransport();
  if( ciTransport != null )
  {
    // Does the player command the transport?
    local uiCommanderID = ciTransport.GetCommanderID();
    if( ciPlayer.GetID() == uiCommanderID )
    {
      local ciWeapon = ciTransport.GetWeapon();
      if( ciWeapon != null )
      {
        bSuccess = true;

        local iRange = ciWeapon.GetProperty( Inventory_Weapon_Attack_Range_PropertyID, 1 );
        if( iRange > 1 )
        {
          // This is a ranged-only weapon, or a melee weapon that can also be thrown
          g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Target, Ultima_FireTransportWeaponTargetCallback );

          local strDesc = format( "%s: <%s>",
                                  ::rumGetString( command_fire_client_StringID ),
                                  ::rumGetString( token_target_client_StringID ) );
          g_ciUI.m_ciGameInputTextBox.SetText( strDesc );

          InitTarget( iRange, ComparatorEnemy );
        }
        else
        {
          // Melee weapon
          g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction, Ultima_FireTransportWeaponDirCallback );

          local strDesc = format( "%s: <%s>",
                                  ::rumGetString( command_Fire_client_StringID ),
                                  ::rumGetString( token_direction_client_StringID ) );
          g_ciUI.m_ciGameInputTextBox.SetText( strDesc );
        }
      }
    }
    else
    {
      uiStringID = msg_not_commander_client_StringID;
    }
  }

  if( !bSuccess )
  {
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_fire_client_StringID ) );
    ShowString( ::rumGetString( uiStringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
  }
}


function Ultima_FireTransportWeaponDirCallback( i_eDir )
{
  local bTargetFound = false;
  local eDelay = ActionDelay.Short;

  local ciPlayer = ::rumGetMainPlayer();
  local ciTransport = ciPlayer.GetTransport();
  if( ciTransport != null && i_uiTargetID != rumInvalidGameID )
  {
    local ciPos = ciPlayer.GetPosition() + GetDirectionVector( i_eDir );
    local ciMap = ciPlayer.GetMap();

    // Does a creature exist in the specified direction?
    local ciTarget;
    local ciPosData = ciMap.GetPositionData( ciPos );
    while( ciTarget = ciPosData.GetNext() )
    {
      if( ciTarget != ciTransport && ciTarget != ciPlayer &&
          ( ( ciTarget instanceof Creature ) || ( ciTarget instanceof Widget ) ) &&
          ciTarget.IsVisible() )
      {
        local ciBroadcast = ::rumCreate( Player_Fire_BroadcastID, AttackType.Directional, i_eDir );
        ::rumSendBroadcast( ciBroadcast );

        bTargetFound = true;
        eDelay = ActionDelay.Long;
        ciPosData.Stop();
      }
    }
  }

  if( bTargetFound )
  {
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_fire_client_StringID ) );
    ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
  }

  BlockInput( ciPlayer.GetActionDelay( eDelay ) );
}


function Ultima_FireTransportWeaponTargetCallback( i_uiTargetID )
{
  local bTargetFound = false;
  local eDelay = ActionDelay.Short;

  local ciPlayer = ::rumGetMainPlayer();
  local ciTransport = ciPlayer.GetTransport();
  if( ciTransport != null && i_uiTargetID != rumInvalidGameID )
  {
    local ciTarget = ::rumFetchPawn( i_uiTargetID );
    if( ciTarget != null && ciTarget != ciTransport && ciTarget != ciPlayer &&
        ( ( ciTarget instanceof Creature ) || ( ciTarget instanceof Widget ) ) )
    {
      local ciBroadcast = ::rumCreate( Player_Fire_BroadcastID, AttackType.Targeted, i_uiTargetID );
      ::rumSendBroadcast( ciBroadcast );
      bTargetFound = true;
      eDelay = ActionDelay.Long;
    }
  }

  if( !bTargetFound )
  {
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_fire_client_StringID ) );
    ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
  }

  BlockInput( ciPlayer.GetActionDelay( eDelay ) );
}


function Ultima_Get( i_eTargetType )
{
  local bMonetary = false;

  local strDesc;
  if( ( U2_Shield_WidgetID == i_eTargetType ) || ( U2_Sword_WidgetID == i_eTargetType ) )
  {
    strDesc = ::rumGetString( command_get_cache_client_StringID );
  }
  else if( U1_Gem_Immortality_WidgetID == i_eTargetType )
  {
    strDesc = ::rumGetString( command_get_gem_client_StringID );
  }
  else
  {
    strDesc = ::rumGetString( command_get_client_StringID );
    bMonetary = true;
  }

  ShowString( "<G#Prompt:vcenter>" + strDesc );

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    return;
  }
  else if( ciPlayer.IsFlying() )
  {
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    ShowString( ::rumGetString( msg_must_land_client_StringID ), g_strColorTagArray.Red );
    return;
  }

  if( bMonetary )
  {
    // Don't let the player get a chest if they're at max gold
    local iMaxGold = ::rumGetMaxPropertyValue( g_eGoldPropertyVersionArray[g_ciCUO.m_eVersion] );
    local iGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
    if( iGold >= iMaxGold )
    {
      BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
      ShowString( ::rumGetString( msg_cant_carry_more_client_StringID ), g_strColorTagArray.Red );
      return;
    }
  }

  local eDelay = ActionDelay.Short;

  local eTransportType = TransportType.None;
  local ciTransport = ciPlayer.GetTransport();
  if( ciTransport != null )
  {
    eTransportType = ciTransport.GetType();
  }

  if( ( eTransportType == TransportType.None ) || ( eTransportType == TransportType.Horse ) )
  {
    local ciWidget;
    local ciMap = ciPlayer.GetMap();
    local bFound = false;
    local ciPosData = ciMap.GetPositionData( ciPlayer.GetPosition() );
    while( ciWidget = ciPosData.GetNext( rumWidgetPawnType, i_eTargetType ) )
    {
      if( ciWidget.IsVisible() )
      {
        bFound = true;

        local ciBroadcast = ::rumCreate( Player_Get_BroadcastID );
        ::rumSendBroadcast( ciBroadcast );

        eDelay = ActionDelay.Long;
        ciPosData.Stop();
      }
    }

    if( !bFound )
    {
      ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
    }
  }
  else
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
  }

  BlockInput( ciPlayer.GetActionDelay( eDelay ) );
}


function Ultima_Hyperjump()
{
  // TODO
}


function Ultima_IgniteTorch( i_uiTorchPropertyID )
{
  local strDesc = ::rumGetString( command_ignite_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local eDelay = ActionDelay.Short;
  local bCanHoldTorch = true;

  local ciWeapon = ciPlayer.GetEquippedWeapon();
  if( ciWeapon != null )
  {
    bCanHoldTorch = ciWeapon.GetProperty( Inventory_Weapon_Num_Hands_PropertyID, 1 ) <= 1;
  }

  if( bCanHoldTorch)
  {
    if( ciPlayer.GetLightRange() == 0 )
    {
      local ciMap = ciPlayer.GetMap();
      if( MapRequiresLight( ciMap ) )
      {
        local iTorches = ciPlayer.GetProperty( i_uiTorchPropertyID, 0 );
        if( iTorches > 0 )
        {
          local ciBroadcast = ::rumCreate( Player_Ignite_BroadcastID, true );
          ::rumSendBroadcast( ciBroadcast );

          eDelay = ActionDelay.Long;
        }
        else
        {
          ShowString( ::rumGetString( msg_none_owned_client_StringID ), g_strColorTagArray.Red );
        }
      }
      else
      {
        ShowString( ::rumGetString( msg_dungeons_only_client_StringID ), g_strColorTagArray.Red );
      }
    }
    else
    {
      ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    }
  }
  else
  {
    ShowString( ::rumGetString( msg_hands_full_client_StringID ), g_strColorTagArray.Red );
  }

  BlockInput( ciPlayer.GetActionDelay( eDelay ) );
}


function Ultima_Init()
{
  U1_Init();
  U2_Init();
  U3_Init();
  U4_Init();
}


function Ultima_JimmyLock( i_eDoorType, i_eKeyPropertyID )
{
  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_jimmy_client_StringID ) );
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    return;
  }

  local uiStringID = msg_cant_client_StringID;
  local bSuccess = false;
  local bDelay = true;

  local eTransportType = TransportType.None;
  local ciTransport = ciPlayer.GetTransport();
  if( ciTransport != null )
  {
    eTransportType = ciTransport.GetType();
  }

  if( ( eTransportType == TransportType.None ) || ( eTransportType == TransportType.Horse ) )
  {
    local iKeys = ciPlayer.GetProperty( i_eKeyPropertyID, 0 );
    if( iKeys > 0 )
    {
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction, Ultima_JimmyLockCallback, i_eDoorType );

      local strDesc = format( "%s: <%s>",
                              ::rumGetString( command_jimmy_client_StringID ),
                              ::rumGetString( token_direction_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetText( strDesc );

      bSuccess = true;
      bDelay = false;
    }
    else
    {
      uiStringID = msg_none_owned_client_StringID;
    }
  }

  if( !bSuccess )
  {
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_jimmy_client_StringID ) );
    ShowString( ::rumGetString( uiStringID ), g_strColorTagArray.Red );
  }

  if( bDelay )
  {
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
  }
}


function Ultima_JimmyLockCallback( i_cDoor, i_eDir )
{
  local ciPlayer = ::rumGetMainPlayer();
  local ciPos = ciPlayer.GetPosition() + GetDirectionVector( i_eDir );
  local ciMap = ciPlayer.GetMap();

  local eDelay = ActionDelay.Short;

  // Does a door widget exist in the specified direction?
  local bFound = false;
  local ciDoor;
  local ciPosData = ciMap.GetPositionData( ciPos );
  while( ciDoor = ciPosData.GetNext( rumWidgetPawnType ) )
  {
    if( ciDoor.IsVisible() && ( ciDoor instanceof i_cDoor ) )
    {
      local ciBroadcast = ::rumCreate( Player_Jimmy_BroadcastID, i_eDir );
      ::rumSendBroadcast( ciBroadcast );

      eDelay = ActionDelay.Long;
      ciPosData.Stop();
      bFound = true;
    }
  }

  if( !bFound )
  {
    ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
  }

  BlockInput( ciPlayer.GetActionDelay( eDelay ) );
}


function Ultima_Klimb()
{
  local ciPlayer = ::rumGetMainPlayer();

  local ciTransport = ciPlayer.GetTransport();
  local eTransportType = ciTransport != null ? ciTransport.GetType() : TransportType.None;

  if( eTransportType != TransportType.None && ( ciTransport.GetMoveType() == MoveType.Stationary ) )
  {
    local eTransportType = ciTransport.GetType();
    if( TransportType.Rocket == eTransportType )
    {
      ShowString( ::rumGetString( command_launch_client_StringID ) );
    }
    else if( TransportType.Plane == eTransportType )
    {
      ShowString( ::rumGetString( command_klimb_altitude_client_StringID ) );
    }
  }

  if( ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  if( !ciPlayer.IsDead() &&
      ( ( TransportType.Balloon == eTransportType )   ||
        ( TransportType.SpaceShip == eTransportType ) ||
        ( TransportType.Plane == eTransportType )     ||
        ( TransportType.Rocket == eTransportType ) ) )
  {
    local bSend = true;

    if( TransportType.SpaceShip == eTransportType )
    {
      local eAssetID = ciTransport.GetAssetID();
      if( ( U1_Space_Fighter1_WidgetID == eAssetID ) || ( U1_Space_Fighter2_WidgetID == eAssetID ) )
      {
        ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
        bSend = false;
      }
      else if( U1_Shuttle_WidgetID == eAssetID )
      {
        local ciMap = ciPlayer.GetMap();
        local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
        if( eMapType != MapType.Towne )
        {
          ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
          bSend = false;
        }
      }
    }

    if( bSend )
    {
      local ciBroadcast = ::rumCreate( Player_Transport_BroadcastID, TransportCommandType.Movement,
                                       VerticalDirectionType.Up );
      ::rumSendBroadcast( ciBroadcast );
      BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    }
  }
  else
  {
    UsePortal( PortalUsageType.Klimb );
  }
}


function Ultima_ListSelectionEnd( i_eDelay = ActionDelay.Short )
{
  g_ciUI.m_ciGameListView.SetActive( false );
  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.m_funcAccept = null;
  g_ciUI.m_ciGameListView.m_funcCancel = null;
  g_ciUI.m_ciGameListView.m_funcIndexChanged = OnGameListViewIndexChanged;

  g_ciUI.m_ciYesNoListView.SetActive( false );
  g_ciUI.m_ciYesNoListView.Clear();
  g_ciUI.m_ciYesNoListView.DisableMultiSelect();
  g_ciUI.m_ciYesNoListView.m_funcAccept = null;
  g_ciUI.m_ciYesNoListView.m_funcCancel = null;
  g_ciUI.m_ciYesNoListView.m_funcIndexChanged = OnYesNoListViewIndexChanged;

  g_ciUI.m_ciGameInputTextBox.Clear();
  g_ciUI.m_ciGameInputTextBox.ShowPrompt( true );
  g_ciUI.m_ciGameInputTextBox.Focus();
  g_ciUI.m_ciGameInputTextBox.m_vPayload = null;

  local ciPlayer = ::rumGetMainPlayer();
  BlockInput( ciPlayer.GetActionDelay( i_eDelay ) );
}


function Ultima_Look()
{
  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction, Ultima_LookCallback );

  local strDesc = format( "%s: <%s>",
                          ::rumGetString( command_look_client_StringID ),
                          ::rumGetString( token_direction_client_StringID ) );
  g_ciUI.m_ciGameInputTextBox.SetText( strDesc );
}


function Ultima_LookCallback( i_eDir )
{
  local ciPlayer = ::rumGetMainPlayer();
  local ciPos = ciPlayer.GetPosition() + GetDirectionVector( i_eDir );
  local ciMap = ciPlayer.GetMap();
  if( MapRequiresLight( ciMap ) )
  {
    local ciScreenPos = g_ciUI.s_ciCenterTilePos + GetDirectionVector( i_eDir );

    local bLit = ciMap.IsPositionLit( ciScreenPos );
    if( !bLit )
    {
      ShowString( ::rumGetString( msg_dark_client_StringID ), g_strColorTagArray.Red );
      BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
      return;
    }
  }

  local strDesc = ::rumGetString( talk_you_see_client_StringID ) + " ";

  local ciPosData = ciMap.GetPositionData( ciPos );
  local eTileID = ciPosData.GetTileID();
  if( rumInvalidAssetID == eTileID )
  {
    // Default to the border tile
    eTileID = ciMap.GetBorderTile();
  }

  local ciTile = ::rumGetTileAsset( eTileID );
  local eArticleType = ciTile.GetProperty( Description_Article_Type_PropertyID, ArticleType.none );
  local strArticle = "";
  if( eArticleType != ArticleType.none )
  {
    strDesc += ::rumGetString( g_eArticleStringArray[eArticleType] ) + " ";
  }

  // The base tile description
  local strTileDesc = ciTile.GetName() + "_Tile_client_StringID";
  strDesc += ::rumGetStringByName( strTileDesc );

  local iIndex = 0;
  local iCount = ciPosData.GetNumObjects();
  local strDescArray = array( iCount, "" );
  local ciPawn;
  local ciWidget = null;
  while( ciPawn = ciPosData.GetNextObject() )
  {
    if( ciPawn.IsVisible() && ciPawn != ciPlayer )
    {
      if( ciPawn instanceof Player )
      {
        strDescArray[iIndex] = ciPawn.GetPlayerName();
        ++iIndex;
      }
      else
      {
        // Some special things like light sources do not have a description
        local bLookIgnore = ciPawn.GetProperty( Look_Ignore_PropertyID, false );
        if( !bLookIgnore )
        {
          local strArticle = "";

          local eArticleType = ciPawn.GetProperty( Description_Article_Type_PropertyID,
                                                   ArticleType.article_indef_sing_cons );
          if( eArticleType != ArticleType.none )
          {
            strArticle = ::rumGetString( g_eArticleStringArray[eArticleType] ) + " ";
          }

          if( ciPawn instanceof Widget )
          {
            local bServerLook = ciPawn.GetProperty( Server_Authoritative_Look_PropertyID, false );
            if( bServerLook )
            {
              ciWidget = ciPawn;
            }
          }

          // Skip any instances of U1_Pond_Widget since they're already stacked on top of a pond tile. Otherwise,
          // players will get a double-description for the pond.
          if( ciPawn.GetAssetID() != U1_Pond_WidgetID )
          {
            // Add the object description to the array
            strDescArray[iIndex] = strArticle + ciPawn.GetDescription();
            ++iIndex;
          }
        }
      }
    }
  }

  if( iIndex > 0 )
  {
    // Walk the array and add the visible object desciptions
    local iEntries = iIndex;
    for( iIndex = 0; iIndex < iEntries; ++iIndex )
    {
      if( iIndex == ( iEntries - 1 ) )
      {
        strDesc += " " + ::rumGetString( token_and_client_StringID ) + " ";
      }
      else
      {
        strDesc += ", ";
      }

      strDesc += strDescArray[iIndex];
    }
  }

  strDesc += ".";
  ShowString( strDesc, g_strColorTagArray.Cyan );

  if( ciWidget != null && !ciPlayer.IsDead() )
  {
    // The player looked at an interactive object - see if the player wants to interact
    ciWidget.Look();
  }

  BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
}


function Ultima_NipPotion( i_ePropertyID, i_bUseOnSelf )
{
  local strDesc = ::rumGetString( command_nip_client_StringID );

  local bDelay = true;
  local eDelay = ActionDelay.Short;

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( strDesc );
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  if( ciPlayer.GetProperty( i_ePropertyID, 0 ) > 0 )
  {
    if( i_bUseOnSelf )
    {
      local ciBroadcast = ::rumCreate( Player_Nip_BroadcastID, true /* on self */, i_ePropertyID );
      ::rumSendBroadcast( ciBroadcast );

      ShowString( "<G#Prompt:vcenter>" + strDesc );

      eDelay = ActionDelay.Medium;
    }
    else
    {
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction, Ultima_NipPotionCallback, i_ePropertyID );

      strDesc = format( "%s: <%s>", strDesc, ::rumGetString( token_direction_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetText( strDesc );

      bDelay = false;
    }
  }
  else
  {
    ShowString( strDesc );
    ShowString( ::rumGetString( msg_none_owned_client_StringID ), g_strColorTagArray.Red );
  }

  if( bDelay )
  {
    BlockInput( ciPlayer.GetActionDelay( eDelay ) );
  }
}


function Ultima_NipPotionCallback( i_ePropertyID, i_eDir )
{
  local ciPlayer = ::rumGetMainPlayer();
  local ciPos = ciPlayer.GetPosition() + GetDirectionVector( i_eDir );
  local ciMap = ciPlayer.GetMap();

  local eDelay = ActionDelay.Short;

  // Does a creature exist in the specified direction?
  local bFound = false;
  local ciCreature;
  local ciPosData = ciMap.GetPositionData( ciPos );
  while( ciCreature = ciPosData.GetNext( rumCreaturePawnType ) )
  {
    if( ciCreature.IsVisible() )
    {
      local ciBroadcast = ::rumCreate( Player_Nip_BroadcastID, false /* not on self */, i_ePropertyID, i_eDir );
      ::rumSendBroadcast( ciBroadcast );

      eDelay = ActionDelay.Medium;
      ciPosData.Stop();
      bFound = true;
    }
  }

  if( !bFound )
  {
    ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
  }

  BlockInput( ciPlayer.GetActionDelay( eDelay ) );
}


function Ultima_NPCBye()
{
  if( 0 == g_ciCUO.m_uiCurrentTalkID )
  {
    // Lord British
    ShowString( format( "%s<b>", ::rumGetString( u4_lord_british_bye_client_StringID ) ), g_strColorTagArray.Cyan );
  }
  else
  {
    local strBye = ::rumGetString( talk_bye_client_StringID );
    ShowString( format( "%s.<b>", strBye ), g_strColorTagArray.Cyan );
  }

  // We must still send to the keyword to the server so that it can end the conversation server-side
  local ciBroadcast = ::rumCreate( Player_Talk_Bye_BroadcastID );
  ::rumSendBroadcast( ciBroadcast );
}


function Ultima_Open( i_cDoor )
{
  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_open_client_StringID ) );
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  if( ciPlayer.IsFlying() )
  {
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_open_client_StringID ) );
    ShowString( ::rumGetString( msg_must_land_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction, Ultima_OpenCallback, i_cDoor );

  local strDesc = format( "%s: <%s>",
                          ::rumGetString( command_open_client_StringID ),
                          ::rumGetString( token_direction_client_StringID ) );
  g_ciUI.m_ciGameInputTextBox.SetText( strDesc );
}


function Ultima_OpenCallback( i_cDoor, i_eDir )
{
  local ciPlayer = ::rumGetMainPlayer();
  local ciPos = ciPlayer.GetPosition() + GetDirectionVector( i_eDir );
  local ciMap = ciPlayer.GetMap();

  local eDelay = ActionDelay.Short;

  // Does a door widget exist in the specified direction?
  local bFound = false;
  local ciDoor;
  local ciPosData = ciMap.GetPositionData( ciPos );
  while( ciDoor = ciPosData.GetNext( rumWidgetPawnType ) )
  {
    if( ciDoor.IsVisible() && ( ciDoor instanceof i_cDoor ) )
    {
      local ciBroadcast = ::rumCreate( Player_Open_BroadcastID, i_eDir );
      ::rumSendBroadcast( ciBroadcast );

      eDelay = ActionDelay.Long;
      ciPosData.Stop();
      bFound = true;
    }
  }

  if( !bFound )
  {
    ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
  }

  BlockInput( ciPlayer.GetActionDelay( eDelay ) );
}


function Ultima_Peer( i_iPropertyID )
{
  if( g_ciCUO.m_bPeering )
  {
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_peer_stop_client_StringID ) );
  }
  else if( U2_Helms_PropertyID == i_iPropertyID )
  {
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_peer_helm_client_StringID ) );
  }
  else
  {
    ShowString( "<G#Prompt:vcenter>" + ::rumGetString( command_peer_gem_client_StringID ) );
  }

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsNegated() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  if( g_ciCUO.m_bPeering )
  {
    // Stop peering
    local ciBroadcast = ::rumCreate( Player_Peer_BroadcastID, false );
    ::rumSendBroadcast( ciBroadcast );
  }
  else
  {
    // Does the player own the item that allows them to peer?
    local iAmount = ciPlayer.GetProperty( i_iPropertyID, 0 );
    if( iAmount > 0 )
    {
      // Start peering
      local ciBroadcast = ::rumCreate( Player_Peer_BroadcastID, true );
      ::rumSendBroadcast( ciBroadcast );
    }
    else
    {
      ShowString( ::rumGetString( msg_none_owned_client_StringID ), g_strColorTagArray.Red )
    }
  }

  BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
}


function Ultima_PlayerTransaction( i_ciPlayer )
{
  switch( g_ciCUO.m_eVersion )
  {
    case GameType.Ultima1: U1_PlayerTransaction( i_ciPlayer ); break;
    case GameType.Ultima2: U2_PlayerTransaction( i_ciPlayer ); break;
    case GameType.Ultima3: U3_PlayerTransaction( i_ciPlayer ); break;
    case GameType.Ultima4: U4_PlayerTransaction( i_ciPlayer ); break;
  }
}


function Ultima_PlayerTransactionGiveAmount( i_ciBroadcast, i_iAmount )
{
  // Push the amount
  local strAmount = g_ciUI.m_ciGameInputTextBox.GetText();
  ShowString( strAmount );
  g_ciUI.m_ciGameInputTextBox.Clear();

  if( i_iAmount > 0 )
  {
    i_ciBroadcast.SetAmount( i_iAmount );
    ::rumSendBroadcast( i_ciBroadcast );
  }
}


function Ultima_PlayerTransactionGiveArmour( i_eVersion, i_ciBroadcast )
{
  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );
  g_ciUI.m_ciGameListView.ShowPrompt( false );

  local iNumItems = 0;
  local ciPlayer = ::rumGetMainPlayer();

  local uiEquippedArmourID = ciPlayer.GetEquippedArmourID();

  // Access each item in the player's inventory
  local ciInventory = ciPlayer.GetInventory();
  local ciItem;
  while( ciItem = ciInventory.GetNextObject() )
  {
    if( null == ciItem )
    {
      continue;
    }

    // Do not display the equipped item
    local uiItemID = ciItem.GetID();
    if( uiItemID != uiEquippedArmourID )
    {
      // Players can't trade bound items
      local bBound = ciItem.GetProperty( Inventory_Bound_PropertyID, false );
      if( !bBound )
      {
        // Be sure the item is armour
        local eType = ciItem.GetProperty( Inventory_Type_PropertyID, InventoryType.Standard );
        if( InventoryType.Armour == eType )
        {
          // Be sure the item makes sense for the requested game version
          local eVersion = ciItem.GetProperty( Ultima_Version_PropertyID, GameType.Invalid );
          if( i_eVersion == eVersion )
          {
            // Add the Armour to the UI control
            local strName = ciItem.GetName() + "_client_StringID";
            g_ciUI.m_ciGameListView.SetEntry( uiItemID, ::rumGetStringByName( strName ) );
            ++iNumItems;
          }
        }
      }
    }
  }

  if( iNumItems > 0 )
  {
    CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

    g_ciUI.m_ciGameInputTextBox.m_vPayload = i_ciBroadcast;

    g_ciUI.m_ciGameListView.m_funcAccept = Ultima_PlayerTransactionGiveInventorySelected;
    g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
    g_ciUI.m_ciGameListView.SetActive( true );
    g_ciUI.m_ciGameListView.Focus();
  }
  else
  {
    ShowString( ::rumGetString( msg_none_owned_client_StringID ), g_strColorTagArray.Red );
    Ultima_ListSelectionEnd();
  }
}


function Ultima_PlayerTransactionGiveInventoryQuantity( i_ciBroadcast, i_iQuantity )
{
  if( null == i_ciBroadcast )
  {
    return;
  }

  // Push the amount player wants to trade to output
  ShowString( format("%d<b>", i_iQuantity ) );
  g_ciUI.m_ciGameInputTextBox.Clear();

  i_ciBroadcast.SetAmount( i_iQuantity );
  ::rumSendBroadcast( i_ciBroadcast );
}


function Ultima_PlayerTransactionGiveInventorySelected()
{
  local ciBroadcast = g_ciUI.m_ciGameInputTextBox.m_vPayload;

  // Push the selection
  local strSelection = g_ciUI.m_ciGameInputTextBox.GetText();
  ShowString( strSelection );
  g_ciUI.m_ciGameInputTextBox.Clear();

  local uiItemID = g_ciUI.m_ciGameListView.GetSelectedKey();
  ciBroadcast.SetItemSubtype( uiItemID );

  Ultima_ListSelectionEnd();

  local ciPlayer = ::rumGetMainPlayer();
  local ciInventory = ciPlayer.GetInventory( uiItemID );

  if( ciInventory != null && ciInventory.GetProperty( Inventory_Stacks_PropertyID, false ) )
  {
    // Since these items stack, request the quantity to trade
    ShowString( ::rumGetString( token_how_many_client_StringID ), g_strColorTagArray.Cyan );
    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount,
                                              Ultima_PlayerTransactionGiveInventoryQuantity,
                                              ciBroadcast );
    g_ciUI.m_ciGameInputTextBox.Focus();
  }
  else
  {
    ::rumSendBroadcast( ciBroadcast );
  }
}


function Ultima_PlayerTransactionGiveWeapon( i_eVersion, i_ciBroadcast )
{
  g_ciUI.m_ciGameListView.Clear();
  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );
  g_ciUI.m_ciGameListView.ShowPrompt( false );

  local iNumItems = 0;
  local ciPlayer = ::rumGetMainPlayer();

  local uiEquippedWeaponID = ciPlayer.GetEquippedWeaponID();

  // Access each item in the player's inventory
  local ciInventory = ciPlayer.GetInventory();
  local ciItem;
  while( ciItem = ciInventory.GetNextObject() )
  {
    if( null == ciItem )
    {
      continue;
    }

    // Do not display the equipped item
    local uiItemID = ciItem.GetID();
    if( uiItemID != uiEquippedWeaponID )
    {
      // Players can't trade bound items
      local bBound = ciItem.GetProperty( Inventory_Bound_PropertyID, false );
      if( !bBound )
      {
        // Be sure the item is a weapon
        local eType = ciItem.GetProperty( Inventory_Type_PropertyID, InventoryType.Standard );
        if( InventoryType.Weapon == eType )
        {
          // Be sure the item makes sense for the requested game version
          local eVersion = ciItem.GetProperty( Ultima_Version_PropertyID, GameType.Invalid );
          if( i_eVersion == eVersion )
          {
            // Add the Weapon to the UI control
            local strName = ciItem.GetName() + "_client_StringID";
            if( ciItem.GetProperty( Inventory_Stacks_PropertyID, false ) )
            {
              local iNumOwned = ciItem.GetProperty( Inventory_Quantity_PropertyID, 1 );
              local strQuantity = format( " (%d)", iNumOwned );
              g_ciUI.m_ciGameListView.SetEntry( uiItemID, ::rumGetStringByName( strName ) + strQuantity );
            }
            else
            {
              g_ciUI.m_ciGameListView.SetEntry( uiItemID, ::rumGetStringByName( strName ) );
            }

            ++iNumItems;
          }
        }
      }
    }
  }

  if( iNumItems > 0 )
  {
    CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

    g_ciUI.m_ciGameInputTextBox.m_vPayload = i_ciBroadcast;

    g_ciUI.m_ciGameListView.m_funcAccept = Ultima_PlayerTransactionGiveInventorySelected;
    g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
    g_ciUI.m_ciGameListView.SetActive( true );
    g_ciUI.m_ciGameListView.Focus();
  }
  else
  {
    ShowString( ::rumGetString( msg_none_owned_client_StringID ), g_strColorTagArray.Red );
    Ultima_ListSelectionEnd();
  }
}


function Ultima_Quit()
{
  // Quit & Save
  local strDesc = ::rumGetString( command_quit_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  local strDesc = ::rumGetString( command_quit_confirm_client_StringID );
  ShowString( strDesc, g_strColorTagArray.Yellow );

  g_ciUI.m_ciYesNoListView.Clear();
  g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                     rumKeypress.KeyY() );
  g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                     rumKeypress.KeyN() );

  g_ciUI.m_ciYesNoListView.m_funcAccept = Ultima_Quit_Callback;
  g_ciUI.m_ciYesNoListView.m_funcCancel = Ultima_ListSelectionEnd;
  g_ciUI.m_ciYesNoListView.SetActive( true );
  g_ciUI.m_ciYesNoListView.Focus();
}


function Ultima_Quit_Callback()
{
  local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();

  Ultima_ListSelectionEnd();

  if( YesNoResponse.Yes == eResponse )
  {
    local ciBroadcast = ::rumCreate( Player_Quit_BroadcastID );
    ::rumSendBroadcast( ciBroadcast );
  }
}


function Ultima_ReadyWeapon()
{
  local strDesc = ::rumGetString( command_ready_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  g_ciUI.m_ciGameListView.Clear();

  local uiEquippedWeaponID = ciPlayer.GetEquippedWeaponID();
  if( rumInvalidGameID != uiEquippedWeaponID )
  {
    g_ciUI.m_ciGameListView.SetEntry( 0, ::rumGetString( token_hands_client_StringID ) );
  }

  // Access each item in the player's inventory
  local ciInventory = ciPlayer.GetInventory();
  local ciItem;
  while( ciItem = ciInventory.GetNextObject() )
  {
    if( null == ciItem )
    {
      continue;
    }

    local uiWeaponID = ciItem.GetID();

    // Do not display the equipped item
    if( uiWeaponID != uiEquippedWeaponID )
    {
      // Be sure the item is a weapon
      local eType = ciItem.GetProperty( Inventory_Type_PropertyID, InventoryType.Standard );
      if( InventoryType.Weapon == eType )
      {
        // Be sure the item makes sense for the requested game version
        local eVersion = ciItem.GetProperty( Ultima_Version_PropertyID, GameType.Invalid );
        if( eVersion == g_ciCUO.m_eVersion )
        {
          // Add the weapon to the UI control
          local strName = ciItem.GetName() + "_client_StringID";
          g_ciUI.m_ciGameListView.SetEntry( uiWeaponID, ::rumGetStringByName( strName ) );
        }
      }
    }
  }

  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );
  g_ciUI.m_ciGameListView.ShowPrompt( false );

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = Ultima_ReadyWeaponCallback;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ReadyWearEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  // Show the weapon stat page
  g_ciUI.m_ePreviousStatPage = g_ciUI.m_eCurrentStatPage;

  switch( g_ciCUO.m_eVersion )
  {
    case GameType.Ultima1: U1_Stat_Update( U1_StatPage.Weapons ); break;
    case GameType.Ultima2: U2_Stat_Update( U2_StatPage.Weapons ); break;
    case GameType.Ultima3: U3_Stat_Update( U3_StatPage.Weapons ); break;
    case GameType.Ultima4: U4_Stat_Update( U4_StatPage.Weapons ); break;
  }
}


function Ultima_ReadyWeaponCallback()
{
  local ciPlayer = ::rumGetMainPlayer();

  local bShowError = true;
  local eDelay = ActionDelay.Short;

  // Push the player's selection to output
  local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();
  ShowString( strDesc + "<b>" );
  g_ciUI.m_ciGameInputTextBox.Clear();

  local uiWeaponID = g_ciUI.m_ciGameListView.GetSelectedKey();
  if( rumInvalidGameID == uiWeaponID )
  {
    if( ciPlayer.GetProperty( Equipped_Weapon_PropertyID, 0 ) != 0 )
    {
      // Player is un-equipping
      local ciBroadcast = ::rumCreate( Player_Equip_Weapon_BroadcastID, uiWeaponID );
      ::rumSendBroadcast( ciBroadcast );

      eDelay = ActionDelay.Medium;
      bShowError = false;
    }
  }
  else
  {
    // Can the player's class equip this kind of weapon?
    local ciWeapon = ciPlayer.GetInventory( uiWeaponID );
    if( ciWeapon != null )
    {
      local eWeaponVersion = ciWeapon.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
      if( g_ciCUO.m_eVersion == eWeaponVersion )
      {
        local ciPlayerClass = ciPlayer.GetPlayerClass();
        if( ciPlayerClass != null )
        {
          // Check class compatibility
          local uiClassFlag = 1 << ciPlayerClass.GetProperty( Class_ID_PropertyID, 0 );
          local uiCompatibilityFlags = ciWeapon.GetProperty( Inventory_Class_Compatibility_Flags_PropertyID, 0 );
          if( ( uiClassFlag & uiCompatibilityFlags ) != 0 )
          {
            local uiRequiredAgility = ciWeapon.GetProperty( Inventory_Agility_Min_PropertyID, 0 );
            local uiAgility = uiRequiredAgility;

            // Check stat compatibility
            if( GameType.Ultima1 == eWeaponVersion )
            {
              uiAgility = ciPlayer.GetProperty( U1_Agility_PropertyID, 0 );
            }
            else if( GameType.Ultima2 == eWeaponVersion )
            {
              uiAgility = ciPlayer.GetProperty( U2_Agility_PropertyID, 0 );
            }

            if( uiAgility >= uiRequiredAgility )
            {
              local ciBroadcast = ::rumCreate( Player_Equip_Weapon_BroadcastID, uiWeaponID );
              ::rumSendBroadcast( ciBroadcast );

              eDelay = ActionDelay.Medium;
              bShowError = false;
            }
          }
          else
          {
            local strArticle = ::rumGetString( token_article_indef_sing_cons_client_StringID );
            strArticle = strArticle.slice( 0, 1 ).toupper() + strArticle.slice( 1, strArticle.len() );
            local strWeaponName = ::rumGetStringByName( ciWeapon.GetName() + "_client_StringID" );
            local strPlayerClass = ::rumGetStringByName( ciPlayerClass.GetName() + "_client_StringID" );
            strDesc = format( "%s %s %s %s<b>", strArticle,
                              strPlayerClass.tolower(),
                              ::rumGetString( msg_cannot_equip_weapon_client_StringID ),
                              strWeaponName.tolower() );
            ShowString( strDesc, g_strColorTagArray.Red );

            bShowError = false;
          }
        }
      }
    }
  }

  if( bShowError )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
  }

  Ultima_ReadyWearEnd( eDelay );
}


function Ultima_ReadyWearEnd( i_eDelay = ActionDelay.Short )
{
  Ultima_Stat_Restore();
  Ultima_ListSelectionEnd( i_eDelay );
}


function Ultima_RenderGame()
{
  if( g_ciCUO.m_bReceivingData )
  {
    RenderTransition();
    return;
  }

  switch( g_ciCUO.m_eVersion )
  {
    case GameType.Ultima1: U1_RenderGame(); break;
    case GameType.Ultima2: U2_RenderGame(); break;
    case GameType.Ultima3: U3_RenderGame(); break;
    case GameType.Ultima4:
    default:
      U4_RenderGame();
      break;
  }
}


function Ultima_Resurrect( i_eResurrectionType, i_eGameType )
{
  local ciPlayer = ::rumGetMainPlayer();

  // Is the player really dead?
  if( ciPlayer.IsDead() )
  {
    if( ResurrectionType.Void == i_eResurrectionType )
    {
      local ciBroadcast = ::rumCreate( Player_Resurrect_BroadcastID, ResurrectionType.Void );
      ::rumSendBroadcast( ciBroadcast );

      BlockInput( ciPlayer.GetActionDelay( ActionDelay.Long ) );
    }
    else if( ResurrectionType.Body == i_eResurrectionType )
    {
      local bBound = ciPlayer.GetVersionedProperty( g_eSpiritBoundPropertyVersionArray );
      if( bBound )
      {
        local ciPos = ciPlayer.GetPosition();

        // The Unique ID we expect to find stamped on the corpse
        local uiPlayerID = ciPlayer.GetID();

        // Is the player over a corpse?
        local bFound = false;
        local ciMap = ciPlayer.GetMap();
        local ciPosData = ciMap.GetPositionData( ciPos );
        local ciCorpse;
        while( ciCorpse = ciPosData.GetNext( rumWidgetPawnType, U4_Body_Dead_WidgetID ) )
        {
          if( ciCorpse.IsVisible() )
          {
            local uiCorpseID = ciCorpse.GetProperty( Player_ID_PropertyID, -1 );
            if( uiCorpseID == uiPlayerID )
            {
              local ciBroadcast = ::rumCreate( Player_Resurrect_BroadcastID, ResurrectionType.Body );
              ::rumSendBroadcast( ciBroadcast );

              bFound = true;
              BlockInput( ciPlayer.GetActionDelay( ActionDelay.Long ) );
              ciPosData.Stop();
            }
          }
        }

        if( !bFound )
        {
          // Player not over their corpse
          ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
        }
      }
      else
      {
        // Spirit is not bound
        ShowString( ::rumGetString( msg_no_effect_client_StringID ), g_strColorTagArray.Red );
      }
    }
  }
}


function Ultima_Search( i_strCommand = "command_search" )
{
  local strDesc = ::rumGetStringByName( i_strCommand + "_client_StringID" );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }
  else if( ciPlayer.IsFlying() )
  {
    ShowString( ::rumGetString( msg_must_land_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local ciBroadcast = ::rumCreate( Player_Search_BroadcastID );
  ::rumSendBroadcast( ciBroadcast );

  BlockInput( ciPlayer.GetActionDelay( ActionDelay.Long ) );
}


function Ultima_Stat_Restore()
{
  g_ciUI.m_eCurrentStatPage = g_ciUI.m_ePreviousStatPage;
  Ultima_Stat_Update();
}


function Ultima_Stat_Update()
{
  switch( g_ciCUO.m_eVersion )
  {
    case GameType.Ultima1: U1_Stat_Update( g_ciUI.m_eCurrentStatPage ); break;
    case GameType.Ultima2: U2_Stat_Update( g_ciUI.m_eCurrentStatPage ); break;
    case GameType.Ultima3: U3_Stat_Update( g_ciUI.m_eCurrentStatPage ); break;
    case GameType.Ultima4: U4_Stat_Update( g_ciUI.m_eCurrentStatPage ); break;
  }
}


function Ultima_Steal()
{
  local strDesc = ::rumGetString( command_steal_client_StringID );

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( strDesc );
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  if( ciPlayer.GetTransportID() != rumInvalidGameID )
  {
    ShowString( strDesc );
    ShowString( ::rumGetString( msg_only_on_foot_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction, Ultima_StealCallback );

  strDesc = format( "%s: <%s>", strDesc, ::rumGetString( token_direction_client_StringID ) );
  g_ciUI.m_ciGameInputTextBox.SetText( strDesc );
}


function Ultima_StealCallback( i_eDir )
{
  local ciPlayer = ::rumGetMainPlayer();
  local ciVector = GetDirectionVector( i_eDir );
  local ciPos = ciPlayer.GetPosition() + ciVector;
  local ciMap = ciPlayer.GetMap();

  local bFound = false;
  local ciPosData = ciMap.GetPositionData( ciPos );

  // If the player is stealing a sign, advance to the next tile
  local ciTile = ::rumGetTileAsset( ciPosData.GetTileID() );
  if( IsSignTile( ciTile ) )
  {
    bFound = Ultima_TrySteal( ciPlayer, ciMap, ciPos + ciVector, i_eDir );
    if( !bFound )
    {
      // If the vector was diagonal, check other nearby spaces
      if( ciVector.x != 0 && ciVector.y != 0 )
      {
        // Try a vertical check
        local ciVectorTest = rumVector( 0, ciVector.y );
        bFound = Ultima_TrySteal( ciPlayer, ciMap, ciPos + ciVectorTest, i_eDir );
        if( !bFound )
        {
          // Try a horizontal check
          ciVectorTest = rumVector( ciVector.x, 0 );
          bFound = Ultima_TrySteal( ciPlayer, ciMap, ciPos + ciVectorTest, i_eDir );
        }
      }
    }
  }
  else
  {
    bFound = Ultima_TrySteal( ciPlayer, ciMap, ciPos, i_eDir );
  }

  if( !bFound )
  {
    ShowString( ::rumGetString( msg_no_effect_client_StringID ), g_strColorTagArray.Red );
  }

  BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
}


function Ultima_TrySteal( i_ciPlayer, i_ciMap, i_ciPos, i_eDir )
{
  local bFound = false;
  local ciVector = GetDirectionVector( i_eDir );
  local ciPosData = i_ciMap.GetPositionData( i_ciPos );

  // Skip across widgets that allow it
  local ciWidget = ciPosData.GetNext( rumWidgetPawnType );
  if( ciWidget != null )
  {
    local bForward = ciWidget.GetProperty( Widget_Forwards_Interaction_PropertyID, false );
    if( bForward )
    {
      bFound = Ultima_TrySteal( i_ciPlayer, i_ciMap, i_ciPos + ciVector, i_eDir );
      if( !bFound )
      {
        // If the vector was diagonal, check other nearby spaces
        if( ciVector.x != 0 && ciVector.y != 0 )
        {
          // Try a vertical check
          local ciVectorTest = rumVector( 0, ciVector.y );
          bFound = Ultima_TrySteal( i_ciPlayer, i_ciMap, i_ciPos + ciVectorTest, i_eDir );
          if( !bFound )
          {
            // Try a horizontal check
            ciVectorTest = rumVector( ciVector.x, 0 );
            bFound = Ultima_TrySteal( i_ciPlayer, i_ciMap, i_ciPos + ciVectorTest, i_eDir );
          }
        }
      }
    }
  }

  if( !bFound )
  {
    ciPosData.Reset();

    local ciCreature = null;
    while( ciCreature = ciPosData.GetNext( rumCreaturePawnType ) )
    {
      if( ciCreature.IsVisible() )
      {
        local ciBroadcast = ::rumCreate( Player_Steal_BroadcastID, i_eDir );
        ::rumSendBroadcast( ciBroadcast );

        ciPosData.Stop();
        bFound = true;
      }
    }
  }

  return bFound;
}


function Ultima_Talk( i_iStringID )
{
  local strCommand = ::rumGetString( i_iStringID );

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( strCommand );
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local ciTransport = ciPlayer.GetTransport();
  if( ciTransport != null )
  {
    // The only transport players can talk from are horses
    local eTransportType = ciTransport.GetType();
    if( eTransportType != TransportType.Horse )
    {
      ShowString( strCommand );
      ShowString( ::rumGetString( msg_only_on_foot_client_StringID ), g_strColorTagArray.Red );
      BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
      return;
    }
  }

  g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction, Ultima_TalkCallback );

  strCommand = format( "%s: <%s>", strCommand, ::rumGetString( token_direction_client_StringID ) );
  g_ciUI.m_ciGameInputTextBox.SetText( strCommand );
}


function Ultima_TalkCallback( i_eDir )
{
  local ciPlayer = ::rumGetMainPlayer();
  local ciVector = GetDirectionVector( i_eDir );
  local ciPos = ciPlayer.GetPosition() + ciVector;
  local ciMap = ciPlayer.GetMap();

  local bFound = false;
  local ciPosData = ciMap.GetPositionData( ciPos );

  // If the player is talking to a sign, advance to the next tile
  local ciTile = ::rumGetTileAsset( ciPosData.GetTileID() );
  if( IsSignTile( ciTile ) )
  {
    bFound = Ultima_TryTalk( ciPlayer, ciMap, ciPos + ciVector, i_eDir );
    if( !bFound )
    {
      // If the vector was diagonal, check other nearby spaces
      if( ciVector.x != 0 && ciVector.y != 0 )
      {
        // Try a vertical check
        local ciVectorTest = rumVector( 0, ciVector.y );
        bFound = Ultima_TryTalk( ciPlayer, ciMap, ciPos + ciVectorTest, i_eDir );
        if( !bFound )
        {
          // Try a horizontal check
          ciVectorTest = rumVector( ciVector.x, 0 );
          bFound = Ultima_TryTalk( ciPlayer, ciMap, ciPos + ciVectorTest, i_eDir );
        }
      }
    }
  }
  else
  {
    bFound = Ultima_TryTalk( ciPlayer, ciMap, ciPos, i_eDir );
  }

  if( !bFound )
  {
    ShowString( ::rumGetString( msg_not_here_client_StringID ), g_strColorTagArray.Red );
  }
}


function Ultima_TryTalk( i_ciPlayer, i_ciMap, i_ciPos, i_eDir )
{
  local bFound = false;
  local ciVector = GetDirectionVector( i_eDir );
  local ciPosData = i_ciMap.GetPositionData( i_ciPos );

  // Skip across widgets that allow it
  local ciWidget = ciPosData.GetNext( rumWidgetPawnType );
  if( ciWidget != null)
  {
    local bForward = ciWidget.GetProperty( Widget_Forwards_Interaction_PropertyID, false );
    if( bForward )
    {
      bFound = Ultima_TryTalk( i_ciPlayer, i_ciMap, i_ciPos + ciVector, i_eDir );
      if( !bFound )
      {
        // If the vector was diagonal, check other nearby spaces
        if( ciVector.x != 0 && ciVector.y != 0 )
        {
          // Try a vertical check
          local ciVectorTest = rumVector( 0, ciVector.y );
          bFound = Ultima_TryTalk( i_ciPlayer, i_ciMap, i_ciPos + ciVectorTest, i_eDir );
          if( !bFound )
          {
            // Try a horizontal check
            ciVectorTest = rumVector( ciVector.x, 0 );
            bFound = Ultima_TryTalk( i_ciPlayer, i_ciMap, i_ciPos + ciVectorTest, i_eDir );
          }
        }
      }
    }
  }

  if( !bFound )
  {
    ciPosData.Reset();

    local ciCreature = null;
    while( ciCreature = ciPosData.GetNext( rumCreaturePawnType ) )
    {
      if( ciCreature.IsVisible() )
      {
        if( ciCreature instanceof Player && ciCreature.GetID() != i_ciPlayer.GetID() )
        {
          Ultima_PlayerTransaction( ciCreature );
          ciPosData.Stop();
          bFound = true;
        }
        else
        {
          local ciBroadcast = ::rumCreate( Player_Talk_BroadcastID, i_eDir );
          ::rumSendBroadcast( ciBroadcast );

          ciPosData.Stop();
          bFound = true;
        }
      }
    }
  }

  BlockInput( i_ciPlayer.GetActionDelay( ActionDelay.Short ) );

  return bFound;
}


function Ultima_UpdateMapLabel()
{
  local ciPlayer = ::rumGetMainPlayer();
  if( null == ciPlayer )
  {
    return;
  }

  local ciMap = ciPlayer.GetMap();
  if( null == ciMap )
  {
    return;
  }

  if( g_ciUI.m_eMapLabelType > MapLabelType.Last )
  {
    // Wrap back around to the front of the list
    g_ciUI.m_eMapLabelType = MapLabelType.First;
  }

  if( MapLabelType.WindDir == g_ciUI.m_eMapLabelType )
  {
    local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
    if( ( MapType.World == eMapType )   ||
        ( MapType.Village == eMapType ) ||
        ( MapType.Towne == eMapType )   ||
        ( MapType.Keep == eMapType )    ||
        ( MapType.Castle == eMapType )  ||
        ( MapType.Shrine == eMapType ) )
    {
      local eDir = Direction.None;

      if( GameType.Ultima4 == g_ciCUO.m_eVersion )
      {
        // Does the player have an active wind override?
        eDir = ciPlayer.GetProperty( U4_Wind_Direction_Override_PropertyID, Direction.None );
      }

      if( Direction.None == eDir )
      {
        // No override found, just use the current global wind direction
        eDir = g_ciCUO.m_eWindDirection;
      }

      local strDesc = format( "%s %s", ::rumGetString( token_wind_client_StringID ),
                                       ::rumGetString( g_eDirectionStringArray[eDir] ) );
      g_ciUI.m_ciMapNameLabel.SetText( strDesc );
    }
    else
    {
      ++g_ciUI.m_eMapLabelType;
    }
  }

  if( MapLabelType.Position == g_ciUI.m_eMapLabelType )
  {
    // Verify that the player owns a sextant
    if( GameType.Ultima4 == g_ciCUO.m_eVersion )
    {
      local bHasSextant = ( ciPlayer.GetProperty( U4_Item_Sextant_PropertyID, 0 ) > 0 );
      if( bHasSextant )
      {
        local strDesc = "";

        // TODO - MapIsOutside() instead?
        if( MapRequiresLight( ciMap ) )
        {
          strDesc = format( "%s %s", ::rumGetString( token_location_client_StringID ),
                                     ::rumGetString( token_unknown_client_StringID ) );
        }
        else
        {
          local ciPos = ciPlayer.GetPosition();

          // Sextant latitude
          local strMinute = ( ciPos.y >> 4 ) + 'A';
          local strSecond = ( ciPos.y % 16 ) + 'A';
          local strLatitude = format( ::rumGetString( msg_sextant_lat_client_StringID ), strMinute, strSecond );

          // Sextant longitude
          strMinute = ( ciPos.x >> 4 ) + 'A';
          strSecond = ( ciPos.x % 16 ) + 'A';
          local strLongitude = format( ::rumGetString( msg_sextant_long_client_StringID ), strMinute, strSecond );

          strDesc = format( "%s %s", strLatitude, strLongitude );
        }

        g_ciUI.m_ciMapNameLabel.SetText( strDesc );
      }
      else
      {
        ++g_ciUI.m_eMapLabelType;
      }
    }
    else
    {
      ++g_ciUI.m_eMapLabelType;
    }
  }

  if( g_ciUI.m_eMapLabelType > MapLabelType.Last )
  {
    // Wrap back around to the front of the list
    g_ciUI.m_eMapLabelType = MapLabelType.First;
  }

  if( g_ciUI.m_eMapLabelType == MapLabelType.MapName )
  {
    local strName = null;

    local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
    if( ( MapType.Shrine == eMapType ) && ( GameType.Ultima4 == g_ciCUO.m_eVersion ) )
    {
      // Reveal the shrine name only if the player has previously meditated at the shrine
      local eVirtue = ciMap.GetProperty( U4_Virtue_PropertyID, VirtueType.Honesty );

      local iFlags = ciPlayer.GetProperty( U4_Shrine_Cycle1_PropertyID, 0 );
      if( iFlags && ::rumBitOn( iFlags, eVirtue ) )
      {
        strName = ciMap.GetName();
      }
      else
      {
        iFlags = ciPlayer.GetProperty( U4_Shrine_Cycle2_PropertyID, 0 );
        if( iFlags && ::rumBitOn( iFlags, eVirtue ) )
        {
          strName = ciMap.GetName();
        }
        else
        {
          iFlags = ciPlayer.GetProperty( U4_Shrine_Cycle3_PropertyID, 0 );
          if( iFlags && ::rumBitOn( iFlags, eVirtue ) )
          {
            strName = ciMap.GetName();
          }
        }
      }

      if( strName != null )
      {
        g_ciUI.m_ciMapNameLabel.SetText( ::rumGetStringByName( strName + "_Map_client_StringID" ) );
      }
      else
      {
        g_ciUI.m_ciMapNameLabel.SetText( ::rumGetString( U4_Shrine_Map_client_StringID ) );
      }
    }
    else
    {
      strName = ciMap.GetName();

      // Strip any trailing numbers (so that dungeon maps, etc. do not need a separate string db entry
      local ex = regexp( "[0-9_]+$" );
      local res = ex.search( strName );
      if( res != null )
      {
        strName = strName.slice( 0, res.begin );
      }

      strName = ::rumGetStringByName( strName + "_Map_client_StringID" );

      local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
      if( ( MapType.Dungeon == eMapType ) || ( MapType.Tower == eMapType ) || ( MapType.Abyss == eMapType ) )
      {
        // Update dungeon/tower level
        local iLevel = ciMap.GetProperty( Map_Level_PropertyID, 1 );
        strName = format( "%s L%d", strName, iLevel );
      }

      g_ciUI.m_ciMapNameLabel.SetText( strName );
    }
  }
}


function Ultima_Volume()
{
  if( g_ciCUO.m_bMusicEnabled && g_ciCUO.m_bSFXEnabled )
  {
    g_ciCUO.m_bMusicEnabled = false;
  }
  else if( !g_ciCUO.m_bMusicEnabled && g_ciCUO.m_bSFXEnabled )
  {
    g_ciCUO.m_bMusicEnabled = true;
    g_ciCUO.m_bSFXEnabled = false;
  }
  else if( g_ciCUO.m_bMusicEnabled && !g_ciCUO.m_bSFXEnabled )
  {
    g_ciCUO.m_bMusicEnabled = false;
    g_ciCUO.m_bSFXEnabled = false;
  }
  else
  {
    g_ciCUO.m_bMusicEnabled = true;
    g_ciCUO.m_bSFXEnabled = true;
  }

  local strDesc = ::rumGetString( command_volume_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  // Save this setting
  g_ciCUO.m_ciGameConfigTable["cuo:music_enabled"] <- g_ciCUO.m_bMusicEnabled;
  g_ciCUO.m_ciGameConfigTable["cuo:sfx_enabled"] <- g_ciCUO.m_bSFXEnabled;
  ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );

  // TODO - Show what was just enabled/disabled
  //local strVolumeDesc = format( "command_volume%d_client_StringID", g_eVolumeSetting );
  //ShowString( ::rumGetStringByName( strVolumeDesc ), g_strColorTagArray.Yellow );

  if( !g_ciCUO.m_bMusicEnabled )
  {
    StopMusic();
  }
  else
  {
    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();
    PlayMusic( GetMusicForMap( ciMap ) );
  }
}


function Ultima_WearArmour()
{
  local strDesc = ::rumGetString( command_wear_client_StringID );
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsDead() || ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  g_ciUI.m_ciGameListView.Clear();

  local uiEquippedArmourID = ciPlayer.GetEquippedArmourID();
  if( rumInvalidGameID != uiEquippedArmourID )
  {
    g_ciUI.m_ciGameListView.SetEntry( 0, ::rumGetString( token_armour_none_client_StringID ) );
  }

  // Access each item in the player's inventory
  local ciInventory = ciPlayer.GetInventory();
  local ciItem;
  while( ciItem = ciInventory.GetNextObject() )
  {
    if( null == ciItem )
    {
      continue;
    }

    local uiArmourID = ciItem.GetID();

    // Do not display the equipped item
    if( uiArmourID != uiEquippedArmourID )
    {
      // Be sure the item is armour
      local eType = ciItem.GetProperty( Inventory_Type_PropertyID, InventoryType.Standard );
      if( InventoryType.Armour == eType )
      {
        // Be sure the item makes sense for the requested game version
        local eVersion = ciItem.GetProperty( Ultima_Version_PropertyID, GameType.Invalid );
        if( eVersion == g_ciCUO.m_eVersion )
        {
          // Add the armour to the UI control
          local strName = ciItem.GetName() + "_client_StringID";
          g_ciUI.m_ciGameListView.SetEntry( uiArmourID, ::rumGetStringByName( strName ) );
        }
      }
    }
  }

  g_ciUI.m_ciGameListView.DisableMultiSelect();
  g_ciUI.m_ciGameListView.SetFormat( "0.05" );
  g_ciUI.m_ciGameListView.ShowPrompt( false );

  CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

  g_ciUI.m_ciGameListView.m_funcAccept = Ultima_WearArmourCallback;
  g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ReadyWearEnd;
  g_ciUI.m_ciGameListView.SetActive( true );
  g_ciUI.m_ciGameListView.Focus();

  // Show the weapon stat page
  g_ciUI.m_ePreviousStatPage = g_ciUI.m_eCurrentStatPage;

  switch( g_ciCUO.m_eVersion )
  {
    case GameType.Ultima1: U1_Stat_Update( U1_StatPage.Armour ); break;
    case GameType.Ultima2: U2_Stat_Update( U2_StatPage.Armour ); break;
    case GameType.Ultima3: U3_Stat_Update( U3_StatPage.Armour ); break;
    case GameType.Ultima4: U4_Stat_Update( U4_StatPage.Armour ); break;
  }
}


function Ultima_WearArmourCallback()
{
  local ciPlayer = ::rumGetMainPlayer();

  local bShowError = true;
  local eDelay = ActionDelay.Short;

  // Push the player's selection to output
  local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();
  ShowString( strDesc + "<b>" );
  g_ciUI.m_ciGameInputTextBox.Clear();

  local uiArmourID = g_ciUI.m_ciGameListView.GetSelectedKey();
  if( rumInvalidGameID == uiArmourID )
  {
    if( ciPlayer.GetProperty( Equipped_Armour_PropertyID, 0 ) != 0 )
    {
      // Player is un-equipping
      local ciBroadcast = ::rumCreate( Player_Equip_Armour_BroadcastID, uiArmourID );
      ::rumSendBroadcast( ciBroadcast );

      eDelay = ActionDelay.Long;
      bShowError = false;
    }
  }
  else
  {
    // Can the player's class wear this kind of armour?
    local ciArmour = ciPlayer.GetInventory( uiArmourID );
    if( ciArmour != null )
    {
      local eArmourVersion = ciArmour.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
      if( g_ciCUO.m_eVersion == eArmourVersion )
      {
        local ciPlayerClass = ciPlayer.GetPlayerClass();
        if( ciPlayerClass != null )
        {
          // Check class compatibility
          local uiClassFlag = 1 << ciPlayerClass.GetProperty( Class_ID_PropertyID, 0 );
          local uiCompatibilityFlags = ciArmour.GetProperty( Inventory_Class_Compatibility_Flags_PropertyID, 0 );
          if( ( uiClassFlag & uiCompatibilityFlags ) != 0 )
          {
            local uiRequiredStrength = ciArmour.GetProperty( Inventory_Strength_Min_PropertyID, 0 );
            local uiStrength = uiRequiredStrength;

            // Check stat compatibility
            if( GameType.Ultima2 == eArmourVersion )
            {
              uiStrength = ciPlayer.GetProperty( U2_Strength_PropertyID, 0 );
            }

            if( uiStrength >= uiRequiredStrength )
            {
              local ciBroadcast = ::rumCreate( Player_Equip_Armour_BroadcastID, uiArmourID );
              ::rumSendBroadcast( ciBroadcast );

              eDelay = ActionDelay.Long;
              bShowError = false;
            }
          }
          else
          {
            local strArticle = ::rumGetString( token_article_indef_sing_cons_client_StringID );
            strArticle = strArticle.slice( 0, 1 ).toupper() + strArticle.slice( 1, strArticle.len() );
            local strArmourName = ::rumGetStringByName( ciArmour.GetName() + "_client_StringID" );
            local strPlayerClass = ::rumGetStringByName( ciPlayerClass.GetName() + "_client_StringID" );
            strDesc = format( "%s %s %s %s<b>", strArticle,
                              strPlayerClass.tolower(),
                              ::rumGetString( msg_cannot_equip_armour_client_StringID ),
                              strArmourName.tolower() );
            ShowString( strDesc, g_strColorTagArray.Red );

            bShowError = false;
          }
        }
      }
    }
  }

  if( bShowError )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
  }

  Ultima_ReadyWearEnd( eDelay );
}


function Ultima_Xit()
{
  local ciPlayer = ::rumGetMainPlayer();
  if( ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local bDelay = true;

  local ciTransport = ciPlayer.GetTransport();
  if( ciTransport != null )
  {
    if( !ciPlayer.IsDead() )
    {
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction, Ultima_XitCallback );

      local iExitTokenID = command_xit_client_StringID;
      if( ciTransport.GetType() == TransportType.Horse )
      {
        iExitTokenID = command_dismount_client_StringID;
      }

      local strDesc = format( "%s %s: <%s>",
                              ::rumGetString( iExitTokenID ),
                              ::rumGetStringByName( ciTransport.GetName() ),
                              ::rumGetString( token_direction_client_StringID ) );
      g_ciUI.m_ciGameInputTextBox.SetText( strDesc );

      bDelay = false;
    }
  }
  else
  {
    ShowString( ::rumGetString( msg_xit_what_client_StringID ), g_strColorTagArray.Red );
  }

  if( bDelay )
  {
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
  }
}


function Ultima_XitCallback( i_eDir )
{
  local ciPlayer = ::rumGetMainPlayer();
  local ciPos = ciPlayer.GetPosition() + GetDirectionVector( i_eDir );
  local ciMap = ciPlayer.GetMap();

  local eDelay = ActionDelay.Short;

  local eTransportType = TransportType.None;
  local ciTransport = ciPlayer.GetTransport();
  if( ciTransport != null )
  {
    eTransportType = ciTransport.GetType();
  }

  // Is this a valid move for the player?
  local ciPosData = ciMap.GetPositionData( ciPos );
  local ciTile = ::rumGetTileAsset( ciPosData.GetTileID() );
  if( !ciTile.IsCollision( MoveType.Terrestrial ) )
  {
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Long ) );

    local ciBroadcast = ::rumCreate( Player_Xit_BroadcastID, i_eDir );
    ::rumSendBroadcast( ciBroadcast );
  }
  else if( ( TransportType.Ship == eTransportType ) && IsWaterTile( ciTile ) )
  {
    // It is possible that the player is exiting the ship onto a skiff?
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Long ) );

    local ciBroadcast = ::rumCreate( Player_Xit_BroadcastID, i_eDir );
    ::rumSendBroadcast( ciBroadcast );
  }
  else
  {
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    ShowString( ::rumGetString( msg_blocked_client_StringID ), g_strColorTagArray.Red );
  }
}


function Ultima_Yell()
{
  local strDesc = ::rumGetString( command_yell_client_StringID ) + ":";
  ShowString( "<G#Prompt:vcenter>" + strDesc );

  g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Player_Chat );
}


function UsePortal( i_ePortalUsageType )
{
  local ciPlayer = ::rumGetMainPlayer();

  if( ciPlayer.IsIncapacitated() )
  {
    ShowString( ::rumGetString( msg_cant_client_StringID ), g_strColorTagArray.Red );
    BlockInput( ciPlayer.GetActionDelay( ActionDelay.Short ) );
    return;
  }

  local strDesc;

  switch( i_ePortalUsageType )
  {
    case PortalUsageType.Enter:   strDesc = ::rumGetString( command_enter_client_StringID );   break;
    case PortalUsageType.Klimb:   strDesc = ::rumGetString( command_klimb_client_StringID );   break;
    case PortalUsageType.Descend: strDesc = ::rumGetString( command_descend_client_StringID ); break;
  }

  local eResult = PortalUsageResultType.NotFound;
  local eDelay = ActionDelay.Short;

  local ciMap = ciPlayer.GetMap();
  local bFound = false;
  local ciPosData = ciMap.GetPositionData( ciPlayer.GetPosition() );
  local ciPortal;
  while( ciPortal = ciPosData.GetNext( rumPortalPawnType ) )
  {
    if( ciPortal.IsVisible() )
    {
      // At least one portal was found
      bFound = true;

      eResult = ciPortal.CanBeUsedBy( ciPlayer, i_ePortalUsageType );
      if( PortalUsageResultType.Success == eResult )
      {
        local ciBroadcast = ::rumCreate( Player_Portal_BroadcastID, i_ePortalUsageType );
        ::rumSendBroadcast( ciBroadcast );

        local strName = ::rumGetStringByName( ciPortal.GetName() + "_Portal_client_StringID" );
        strDesc = format( "%s %s", strDesc, strName );
        ShowString( "<G#Prompt:vcenter>" + strDesc );
        ciPosData.Stop();

        eDelay = ActionDelay.Long;
      }
    }
  }

  if( eResult != PortalUsageResultType.Success )
  {
    ShowString( "<G#Prompt:vcenter>" + strDesc );

    switch( eResult )
    {
      case PortalUsageResultType.NoTransports:
        strDesc = ::rumGetString( msg_only_on_foot_client_StringID );
        break;

      case PortalUsageResultType.EightPartAvatar:
        strDesc = ::rumGetString( msg_not_eight_parts_avatar_client_StringID );
        break;

      case PortalUsageResultType.NotFound:

      case PortalUsageResultType.Invalid:
        strDesc = format( "%s %s", strDesc, ::rumGetString( token_what_client_StringID ) );
        break;
    }

    ShowString( strDesc, g_strColorTagArray.Red );
  }

  BlockInput( ciPlayer.GetActionDelay( eDelay ) );
}


function VerifyInstallation( i_eVersion )
{
  g_ciUI.m_bNeedsInstallVerification = true;
  g_ciUI.m_eVerificationVersion = i_eVersion;
  g_ciUI.m_iNumVerificationAttempts = 0;

  if( !VerifyUltima( i_eVersion ) )
  {
    local strGame = g_aUltimaInfoArray[i_eVersion][UltimaInfo.Game];

    // Verification failed, prompt the user for an install location
    local strPrompt = format( ::rumGetString( ultima_install_req_client_StringID ), strGame, strGame );
    ShowString( strPrompt, g_strColorTagArray.Yellow );

    // Show the user the previously saved value
    g_ciUI.m_ciGameInputTextBox.SetText( g_ciCUO.m_ciGameConfigTable[format( "cuo:ultima%d", i_eVersion )] );
    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Text_Input, VerifyInstallationCallback );

    local iChatWidth = g_ciUI.m_ciChatTextView.GetWidth();
    local iChatHeight = g_ciUI.m_ciChatTextView.GetHeight();

    local iFileListWidth = ( iChatWidth * 0.9 ).tointeger();
    local iDriveListWidth = iChatWidth - iFileListWidth;

    // Prepare the gui file explorer
    g_ciUI.m_ciExplorerDriveListView.SetActive( true );
    g_ciUI.m_ciExplorerDriveListView.SetWidth( iDriveListWidth );
    g_ciUI.m_ciExplorerDriveListView.SetHeight( iChatHeight );
    g_ciUI.m_ciExplorerDriveListView.SetFormat( "0.0|0.45" );

    g_ciUI.m_ciExplorerFileListView.SetActive( true );
    g_ciUI.m_ciExplorerFileListView.Clear();
    g_ciUI.m_ciExplorerFileListView.SetWidth( iFileListWidth );
    g_ciUI.m_ciExplorerFileListView.SetHeight( iChatHeight );
    g_ciUI.m_ciExplorerFileListView.SetFormat( "0.0|0.05" );

    local ciPos = g_ciUI.m_ciExplorerDriveListView.GetPos();
    ciPos = rumPoint( ciPos.x + g_ciUI.m_ciExplorerDriveListView.GetWidth(), ciPos.y );
    g_ciUI.m_ciExplorerFileListView.SetPos( ciPos );

    UpdateDriveListView();
    UpdateFileListView();
  }
  else
  {
    VerifyInstallationDone();
  }
}


function VerifyInstallationCallback( i_strPath )
{
  ++g_ciUI.m_iNumVerificationAttempts;

  // The user provided a path, check it
  local strUltima = format( "cuo:ultima%d", g_ciUI.m_eVerificationVersion );
  g_ciCUO.m_ciGameConfigTable[strUltima] <- i_strPath;

  if( !VerifyUltima( g_ciUI.m_eVerificationVersion ) )
  {
    // Verification failed
    ShowString( format( ::rumGetString( msg_location_invalid_client_StringID ), g_ciUI.m_eVerificationVersion ) );

    if( g_ciUI.m_iNumVerificationAttempts < 3 )
    {
      // Verification failed again, keep prompting
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Text_Input, VerifyInstallationCallback );

      UpdateDriveListView();
      UpdateFileListView();

      g_ciUI.m_ciExplorerDriveListView.SetActive( true );
      g_ciUI.m_ciExplorerFileListView.SetActive( true );
    }
    else
    {
      // Failed to resolve the installation, so notify the server. Unfortunately, this is a bit of an honor
      // system, because a hacked client will allow any player to continue to play the content.
      local ciBroadcast = ::rumCreate( Version_Check_BroadcastID );
      ::rumSendBroadcast( ciBroadcast );

      // Verification failed, but we don't want to continue checking
      VerifyInstallationDone();
    }
  }
  else
  {
    // Verification succeeded
    ShowString( ::rumGetString( ultima_install_verified_client_StringID ), g_strColorTagArray.Green );
    VerifyInstallationDone();
  }
}


function VerifyInstallationDone()
{
  g_ciUI.m_bNeedsInstallVerification = false;
  g_ciUI.m_iNumVerificationAttempts = 0;

  g_ciUI.m_ciExplorerDriveListView.SetActive( false );
  g_ciUI.m_ciExplorerFileListView.SetActive( false );

  g_ciUI.m_ciChatTextView.SetActive( true );

  g_ciUI.m_ciGameInputTextBox.Clear();
  g_ciUI.m_ciGameInputTextBox.ResetInputMode( false );
  g_ciUI.m_ciGameInputTextBox.Focus();
}


function VerifyUltima( i_eVersion )
{
  // Verify the Ultima executable hash
  local strIni = format( "cuo:ultima%d", i_eVersion );
  local strFile = g_aUltimaInfoArray[i_eVersion][UltimaInfo.PC_File];
  local strHash = g_aUltimaInfoArray[i_eVersion][UltimaInfo.PC_FileHash];

  local strPath = g_ciCUO.m_ciGameConfigTable[strIni];
  local strAdjustedPath = ::rumAppendPathSeparator( strPath );
  local strFilePath = strAdjustedPath + strFile;

  local bVerified = ::rumVerifyFile( strFilePath, strHash );
  if( !bVerified )
  {
    // In addition to the path specified in the ini, try some well-known default paths
    strPath = format( "C:/GOG Games/Ultima %d/", i_eVersion );
    strFilePath = strPath + strFile;

    bVerified = ::rumVerifyFile( strFilePath, strHash );
    if( !bVerified )
    {
      strPath = format( "C:/Program Files (x86)/GalaxyClient/Games/Ultima %d/", i_eVersion );
      strFilePath = strPath + strFile;

      bVerified = ::rumVerifyFile( strFilePath, strHash );
      if( !bVerified )
      {
        strPath = format( "C:/Program Files (x86)/GOG Galaxy/Games/Ultima %d/", i_eVersion );
        strFilePath = strPath + strFile;

        bVerified = ::rumVerifyFile( strFilePath, strHash );
        if( !bVerified && ( GameType.Ultima4 == i_eVersion ) )
        {
          strPath = format( "C:/Program Files (x86)/GOG Galaxy/Games/Ultima 4 - Quest of the Avatar/", i_eVersion );
          strFilePath = strPath + strFile;

          bVerified = ::rumVerifyFile( strFilePath, strHash );
        }
      }
    }
  }

  if( bVerified )
  {
    g_ciCUO.m_ciGameConfigTable[strIni] = strPath;

    // Save the new path
    ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );
  }

  return bVerified;
}
