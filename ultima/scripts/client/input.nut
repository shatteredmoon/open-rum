class InputModeContext
{
  // The function to call when this input mode is done
  m_funcCallback = null;

  // Params to pass to callback function
  m_vParams = null;

  constructor( i_funcCallback, i_vParams )
  {
    m_funcCallback = i_funcCallback;
    m_vParams = i_vParams;
  }
}


class GameInput extends TextBox
{
  m_ciInputModeContext = null;
  m_eInputMode = InputMode.Game;

  m_vPayload = null;


  function ClearHandlers()
  {
    base.ClearHandlers();
  }


  function CreateInputModeContext( i_eMode, ... )
  {
    if( i_eMode > InputMode.Game && i_eMode < InputMode.NumModes )
    {
      local funcCallback = null;
      local varParams = null;

      if( vargv.len() > 0 )
      {
        funcCallback = vargv[0];
        vargv.remove( 0 );

        if( vargv.len() > 0 )
        {
          varParams = clone vargv;
        }
      }

      m_ciInputModeContext = InputModeContext( funcCallback, varParams );
    }
  }


  function EndInputMode( i_bDoCallback, ... )
  {
    switch( vargv.len() )
    {
      case 4: ResetInputMode( i_bDoCallback, vargv[0], vargv[1], vargv[2], vargv[3] ); break;
      case 3: ResetInputMode( i_bDoCallback, vargv[0], vargv[1], vargv[2] ); break;
      case 2: ResetInputMode( i_bDoCallback, vargv[0], vargv[1] ); break;
      case 1: ResetInputMode( i_bDoCallback, vargv[0] ); break;
      case 0:
      default: ResetInputMode( i_bDoCallback ); break;
    }
  }


  function HandleAmountInput( i_ciKeyInput )
  {
    local eKey = i_ciKeyInput.GetKey();

    if( ( eKey >= rumKeypress.Key0() && eKey <= rumKeypress.Key9() ) ||
        ( eKey >= rumKeypress.KeyPad0() && eKey <= rumKeypress.KeyPad9() ) )
    {
      // Submit what was typed
      if( i_ciKeyInput.IsPrintable() )
      {
        CharacterAdd( i_ciKeyInput.GetAscii() );
      }
    }
    else if( ( rumKeypress.KeyEnter() == eKey ) || ( rumKeypress.KeyPadEnter() == eKey ) )
    {
      local strInput = GetText();
      if( strInput.len() == 0 )
      {
        strInput = 0;
      }

      local iAmount = 0;

      try
      {
        iAmount = strInput.tointeger();
      }
      catch( eException )
      {
        // Nothing to do
      }

      EndInputMode( true, iAmount );
    }
    else if( i_ciKeyInput.CtrlPressed() )
    {
      if( rumKeypress.KeyC() == eKey )
      {
        CopyTextToClipboard();
      }
      else if( rumKeypress.KeyV() == eKey )
      {
        PasteTextFromClipboard();
      }
      else if( rumKeypress.KeyA() == eKey )
      {
        SelectAll();
      }
    }
    else if( rumKeypress.KeyLeft() == eKey )
    {
      CursorLeft( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyRight() == eKey )
    {
      CursorRight( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyBackspace() == eKey )
    {
      CharacterBackspace();
    }
    else if( rumKeypress.KeyDelete() == eKey )
    {
      CharacterDelete();
    }
    else if( rumKeypress.KeyHome() == eKey )
    {
      CursorHome( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyEnd() == eKey )
    {
      CursorEnd( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyEscape() == eKey )
    {
      if( GetText().len() > 0 )
      {
        Clear();
      }
      else
      {
        // Cancel
        EndInputMode( true, null );
      }
    }
  }


  function HandleAnyKeyInput( i_ciKeyInput )
  {
    Clear();
    EndInputMode( true );
  }


  function HandleChatInput( i_ciKeyInput )
  {
    local eKey = i_ciKeyInput.GetKey();

    if( i_ciKeyInput.CtrlPressed() &&
        ( ( InputMode.Player_Chat == m_eInputMode ) || ( InputMode.Text_Input == m_eInputMode ) ) )
    {
      if( rumKeypress.KeyC() == eKey )
      {
        CopyTextToClipboard();
      }
      else if( rumKeypress.KeyV() == eKey )
      {
        PasteTextFromClipboard();
      }
      else if( rumKeypress.KeyA() == eKey )
      {
        SelectAll();
      }
    }
    else if( ( rumKeypress.KeyEnter() == eKey ) || ( rumKeypress.KeyPadEnter() == eKey ) )
    {
      if( InputMode.NPC_Chat == m_eInputMode )
      {
        // Grab what was typed
        local strKeyword = GetText();

        if( strKeyword.len() > 0 )
        {
          ShowString( strKeyword );
        }

        ShowString( "" );

        // Strip off any whitespace and only take the first 4 characters into account
        strKeyword = strip( GetText() );
        if( strKeyword.len() > 4 )
        {
          strKeyword = strKeyword.slice( 0, 4 );
        }

        strKeyword = strKeyword.tolower();

        // We may need to know what the player typed at a later time
        local ciPlayer = ::rumGetMainPlayer();
        ciPlayer.lastKeyword = strKeyword;

        // Turn the keyword into a hash key for cached respone storage and retrieval
        local strKey = strKeyword + "_" + g_ciCUO.m_uiCurrentTalkID;

        // Determine what certain special keyword strings are in the player's language
        local strBye = ::rumGetString( talk_bye_client_StringID );
        if( strBye.len() > 4 )
        {
          strBye = strBye.slice( 0, 4 );
        }

        local strGive = ::rumGetString( talk_give_client_StringID ).tolower();
        if( strGive.len() > 4 )
        {
          strGive = strGive.slice( 0, 4 );
        }

        local strCarve = ::rumGetString( talk_carve_client_StringID ).tolower();
        if( strCarve.len() > 4 )
        {
          strCarve = strCarve.slice( 0, 4 );
        }

        // If the query is empty or localized "bye", end the conversation
        if( ( strKeyword.len() == 0 ) || ( strKeyword == strBye.tolower() ) )
        {
          Ultima_NPCBye();
        }
        else if( strKeyword == strGive.tolower() )
        {
          local ciBroadcast = ::rumCreate( Player_Talk_Give_BroadcastID, GiveState.Init, 0 );
          ::rumSendBroadcast( ciBroadcast );
        }
        else if( strKeyword == strCarve.tolower() )
        {
          local ciBroadcast = ::rumCreate( Player_Talk_Carve_BroadcastID, CarveState.Init );
          ::rumSendBroadcast( ciBroadcast );
        }
        // See if the query exists locally
        else if( strKey in g_ciCUO.m_strDialogueTable )
        {
          local strDesc = "";

          if( "look" == strKeyword )
          {
            strDesc += ::rumGetString( talk_you_see_client_StringID );
            strDesc += " ";
          }

          strDesc += g_ciCUO.m_strDialogueTable[strKey];

          local questionIndex = strDesc.find( "<q>" );
          if( questionIndex )
          {
            U4_NPCQuestion( strDesc, questionIndex );
          }
          else
          {
            ShowString( strDesc, g_strColorTagArray.Cyan );
          }

          U4_NPCPromptInterest();
        }
        else
        {
          // Send the query to the server
          local ciBroadcast = ::rumCreate( Player_Talk_Keyword_BroadcastID, strKeyword );
          ::rumSendBroadcast( ciBroadcast );
        }

        Clear();
      }
      else if( InputMode.Seer_Chat == m_eInputMode )
      {
        U4_SeerResponse();
      }
      else if( InputMode.Player_Chat == m_eInputMode )
      {
        if( GetText().len() > 0 && GetText() != "/" )
        {
          local strChat = GetText();
          local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strChat );
          if( ciBroadcast.Validate() )
          {
            local bProcessed = false;

            if( "/" == strChat.slice( 0, 1 ) )
            {
              // Tokenize the string
              local strTokenArray = split( strChat, " " );
              if( strTokenArray[0] == "/help" )
              {
                // The /list command does not need to be sent to the server because (currently) clients
                // receive login and logout notifications for all players and contain all map info
                CommandHelp( strTokenArray );
                bProcessed = true;

                if( g_ciUI.m_ciGameInputTextBox.m_eInputMode == InputMode.PressAnyKey )
                {
                  g_ciUI.AppendCommandHistory( strChat );
                  return;
                }
              }
              else if( strTokenArray[0] == "/title" )
              {
                if( strTokenArray.len() > 1 )
                {
                  local strTitle = strChat.slice(strTokenArray[0].len() + 1);
                  ::rumSetWindowTitle( strTitle );
                  bProcessed = true;
                }
              }
              else if( strTokenArray[0] == "/property" )
              {
                // Convert strings to asset IDs - this is necessary because certain assets like sounds and graphics do
                // not exist on the server, so we have to send up the AssetID instead of a string. If we can't find an
                // asset here, that's fine, just go ahead and send up what was typed and hope the server can cope. It's
                // also better to do the conversion here because fetching an asset by name is expensive and we save the
                // server some heavy lifting by paying that cost on the client.
                local strProperty = strTokenArray[4];
                local ePropertyID = ::rumGetAssetID( strProperty + "_PropertyID" );
                local ciProperty = ::rumGetPropertyAsset( ePropertyID );
                if( ciProperty != null && ciProperty.IsAssetRef() )
                {
                  // Convert the final parameter to an integer if necessary by fetching the asset
                  local ciAsset = ::rumGetAssetByName( strTokenArray[5] );
                  if( ciAsset != null )
                  {
                    local strModified = format( "%s %s %s %s %s %d",
                                                strTokenArray[0],
                                                strTokenArray[1],
                                                strTokenArray[2],
                                                strTokenArray[3],
                                                strTokenArray[4],
                                                ciAsset.GetAssetID() );

                    // Recreate the broadcast
                    ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strModified );
                  }
                }
              }
              else if( strTokenArray[0] == "/who" )
              {
                local strMatch = null;

                if( strTokenArray.len() > 1 )
                {
                  strMatch = strTokenArray[1].tolower();
                }

                CommandWho( strMatch );
                bProcessed = true;
              }
            }

            if( !bProcessed )
            {
              ::rumSendBroadcast( ciBroadcast );
            }

            g_ciUI.AppendCommandHistory( strChat );
          }
        }

        // End chat mode
        Clear();
        EndInputMode( false );
      }
      else if( InputMode.Text_Input == m_eInputMode )
      {
        // End the text input mode
        EndInputMode( true, GetText() );
      }
    }
    else if( ( rumKeypress.KeyUp() == eKey ) &&
             ( ( InputMode.Player_Chat == m_eInputMode ) || ( InputMode.Text_Input == m_eInputMode ) ) )
    {
      local iOffset;

      if( g_ciUI.m_iCommandHistoryDisplayIndex != -1 )
      {
        // Up pressed since last insertion, move to previous item
        iOffset = g_ciUI.m_iCommandHistoryDisplayIndex - 1;
      }
      else
      {
        // Up not previously pressed since last insertion, start at previous item
        iOffset = g_ciUI.m_iCommandHistoryInsertIndex - 1;
      }

      if( iOffset < 0 )
      {
        // Wrap around to the end of the array
        iOffset = g_ciUI.s_iCommandHistorySize - 1;
      }

      local strCommand = g_ciUI.m_strCommandHistoryArray[iOffset];
      if( ( null == strCommand ) && g_ciUI.m_iCommandHistoryInsertIndex > 0 )
      {
        // The history array isn't completely full, so items don't exist at the end. Jump to the highest index that
        // has a valid entry in it.
        iOffset = g_ciUI.m_iCommandHistoryInsertIndex - 1;
        strCommand = g_ciUI.m_strCommandHistoryArray[iOffset];
      }

      if( strCommand != null )
      {
        SetText( strCommand );

        // Save the display offset
        g_ciUI.m_iCommandHistoryDisplayIndex = iOffset;
      }
    }
    else if( ( rumKeypress.KeyDown() == eKey ) &&
             ( ( InputMode.Player_Chat == m_eInputMode ) || ( InputMode.Text_Input == m_eInputMode ) ) )
    {
      local iOffset;

      if( g_ciUI.m_iCommandHistoryDisplayIndex != -1 )
      {
        // Down pressed since last insertion, advance to the next item
        iOffset = g_ciUI.m_iCommandHistoryDisplayIndex + 1;
      }
      else
      {
        // Down not previously pressed since last insertion, start at the current item
        iOffset = g_ciUI.m_iCommandHistoryInsertIndex;
      }

      if( iOffset >= g_ciUI.s_iCommandHistorySize )
      {
        // Wrap around to the front of the array
        iOffset = 0;
      }

      local strCommand = g_ciUI.m_strCommandHistoryArray[iOffset];
      if( null == strCommand )
      {
        // Wrap around to the front of the array
        iOffset = 0;
        strCommand = g_ciUI.m_strCommandHistoryArray[iOffset];
      }

      if( strCommand != null )
      {
        SetText( strCommand );

        // Save the display offset
        g_ciUI.m_iCommandHistoryDisplayIndex = iOffset;
      }
    }
    else if( rumKeypress.KeyBackspace() == eKey )
    {
      CharacterBackspace();
    }
    else if( rumKeypress.KeyDelete() == eKey )
    {
      CharacterDelete();
    }
    else if( ( rumKeypress.KeyLeft() == eKey ) &&
             ( ( InputMode.Player_Chat == m_eInputMode ) || ( InputMode.Text_Input == m_eInputMode ) ) )
    {
      CursorLeft( i_ciKeyInput.ShiftPressed() );
    }
    else if( ( rumKeypress.KeyRight() == eKey ) &&
             ( ( InputMode.Player_Chat == m_eInputMode ) || ( InputMode.Text_Input == m_eInputMode ) ) )
    {
      CursorRight( i_ciKeyInput.ShiftPressed() );
    }
    else if( ( rumKeypress.KeyHome() == eKey ) &&
             ( ( InputMode.Player_Chat == m_eInputMode ) || ( InputMode.Text_Input == m_eInputMode ) ) )
    {
      CursorHome( i_ciKeyInput.ShiftPressed() );
    }
    else if( ( rumKeypress.KeyEnd() == eKey ) &&
             ( ( InputMode.Player_Chat == m_eInputMode ) || ( InputMode.Text_Input == m_eInputMode ) ) )
    {
      CursorEnd( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyEscape() == eKey )
    {
      if( GetText().len() > 0 )
      {
        Clear();
        g_ciUI.m_iCommandHistoryDisplayIndex = -1;
      }
      else if( InputMode.NPC_Chat == m_eInputMode )
      {
        ShowString( "" );
        Ultima_NPCBye();
      }
      else if( InputMode.Seer_Chat == m_eInputMode )
      {
        U4_SeerEndTransaction();
      }
      else if( InputMode.Player_Chat == m_eInputMode )
      {
        // End chat mode
        EndInputMode( false );

      }
      else if( InputMode.Text_Input == m_eInputMode )
      {
        // End text input mode
        EndInputMode( true );
      }
      else
      {
        EndInputMode( false );
      }
    }
    else
    {
      if( i_ciKeyInput.IsPrintable() )
      {
        // Append what was typed
        CharacterAdd( i_ciKeyInput.GetAscii() );
      }
    }
  }


  function HandleDirectionInput( i_ciKeyInput )
  {
    local eKey = i_ciKeyInput.GetKey();

    // Is the keypress a direction?
    if( KeyPressIsDirection( eKey ) )
    {
      local eDir = GetDirectionFromInput( eKey );
      local strDir = GetText() + GetDirectionString( eDir );
      ShowString( "<G#Prompt:vcenter>" + strDir );

      Clear();
      EndInputMode( true, eDir );
    }
    else if( rumKeypress.KeyEscape() == eKey )
    {
      Clear();
      EndInputMode( false );
    }
  }


  function HandleGameInput( i_ciKeyInput )
  {
    local eKey = i_ciKeyInput.GetKey();

    if( ( rumKeypress.KeyEnter() == eKey ) || ( rumKeypress.KeySlash() == eKey ) ||
        ( rumKeypress.KeyPadEnter() == eKey ) )
    {
      SetInputMode( InputMode.Player_Chat );

      if( rumKeypress.KeySlash() == eKey )
      {
        SetText( "/" );
        return;
      }
    }

    if( m_funcKeyPress != null )
    {
      m_funcKeyPress.call( this, i_ciKeyInput );
    }
  }


  function HandleTargetInput( i_ciKeyInput )
  {
    local ciPlayer = ::rumGetMainPlayer();
    local eKey = i_ciKeyInput.GetKey();

    // Is the keypress a direction?
    if( KeyPressIsDirection( eKey ) || ( eKey >= rumKeypress.KeyPad1() && eKey <= rumKeypress.KeyPad9() ) )
    {
      local eDir = GetDirectionFromInput( eKey );

      local ciMap = ciPlayer.GetMap();
      if( ciMap )
      {
        // Determine the new position and whether or not it is within range
        local newPos = g_ciCUO.m_ciTargetPos + GetDirectionVector( eDir );
        if( ciMap.IsPositionWithinTileDistance( ciPlayer.GetPosition(), newPos, g_ciCUO.s_iLOSRadius ) )
        {
          g_ciCUO.m_ciTargetPos = newPos;
        }
      }
    }
    else if( rumKeypress.KeyHome() == eKey )
    {
      g_ciCUO.m_iLockedTargetIndex = -1;
      g_ciCUO.m_uiLockedTargetID = rumInvalidGameID;
      g_ciCUO.m_ciTargetPos = ciPlayer.GetPosition();
    }
    else if( rumKeypress.KeyTab() == eKey )
    {
      ciPlayer.GetNewTarget( ++g_ciCUO.m_iLockedTargetIndex, g_ciCUO.m_funcLastComparator );
    }
    else if( rumKeypress.KeyEscape() == eKey )
    {
      Clear();
      EndInputMode( false );
    }
    else if( ( rumKeypress.KeyEnter() == eKey ) || ( rumKeypress.KeySpace() == eKey ) ||
             ( rumKeypress.KeyPadEnter() == eKey ) )
    {
      TargetSelected();
    }
  }


  function HandleTextResponseInput( i_ciKeyInput )
  {
    local eKey = i_ciKeyInput.GetKey();

    if( ( rumKeypress.KeyEnter() == eKey ) || ( rumKeypress.KeyPadEnter() == eKey ) )
    {
      local strInput = strip( GetText() );
      if( strInput.len() > 0 )
      {
        // Pass the given amount through to the pop keyboard callback
        EndInputMode( true, strInput );
      }
    }
    else if( i_ciKeyInput.CtrlPressed() )
    {
      if( rumKeypress.KeyC() == eKey )
      {
        CopyTextToClipboard();
      }
      else if( rumKeypress.KeyV() == eKey )
      {
        PasteTextFromClipboard();
      }
      else if( rumKeypress.KeyA() == eKey )
      {
        SelectAll();
      }
    }
    else if( rumKeypress.KeyLeft() == eKey )
    {
      CursorLeft( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyRight() == eKey )
    {
      CursorRight( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyBackspace() == eKey )
    {
      CharacterBackspace();
    }
    else if( rumKeypress.KeyDelete() == eKey )
    {
      CharacterDelete();
    }
    else if( rumKeypress.KeyHome() == eKey )
    {
      CursorHome( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyEnd() == eKey )
    {
      CursorEnd( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyEscape() == eKey )
    {
      // Cancel the input
      Clear();
      EndInputMode( true, null );
    }
    else if( rumKeypress.KeyA() <= eKey && rumKeypress.KeyZ() >= eKey || ( rumKeypress.KeySpace() == eKey ) )
    {
      if( i_ciKeyInput.IsPrintable() )
      {
        CharacterAdd( i_ciKeyInput.GetAscii() );
      }
    }
  }


  function HandleYesNoQuestionInput( i_ciKeyInput )
  {
    local eKey = i_ciKeyInput.GetKey();

    if( ( rumKeypress.KeyEnter() == eKey ) || ( rumKeypress.KeyPadEnter() == eKey ) )
    {
      local strAnswer = strip( GetText() );
      Clear();
      EndInputMode( true, strAnswer );
    }
    else if( i_ciKeyInput.CtrlPressed() )
    {
      if( rumKeypress.KeyC() == eKey )
      {
        CopyTextToClipboard();
      }
      else if( rumKeypress.KeyV() == eKey )
      {
        PasteTextFromClipboard();
      }
      else if( rumKeypress.KeyA() == eKey )
      {
        SelectAll();
      }
    }
    else if( rumKeypress.KeyLeft() == eKey )
    {
      CursorLeft( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyRight() == eKey )
    {
      CursorRight( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyBackspace() == eKey )
    {
      CharacterBackspace();
    }
    else if( rumKeypress.KeyDelete() == eKey )
    {
      CharacterDelete();
    }
    else if( rumKeypress.KeyHome() == eKey )
    {
      CursorHome( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyEnd() == eKey )
    {
      CursorEnd( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyEscape() == eKey )
    {
      // Cancel the question
      Clear();
      EndInputMode( false );

      U4_NPCPromptInterest();
    }
    else if( i_ciKeyInput.IsPrintable() )
    {
      CharacterAdd( i_ciKeyInput.GetAscii() );
    }
  }


  function KeyPressIsDirection( i_eKey )
  {
    return ( ( rumKeypress.KeyLeft() == i_eKey ) || ( rumKeypress.KeyRight() == i_eKey ) ||
             ( rumKeypress.KeyUp() == i_eKey ) || ( rumKeypress.KeyDown() == i_eKey ) ||
             ( i_eKey >= rumKeypress.KeyPad1() && i_eKey <= rumKeypress.KeyPad9() ) );
  }


  function OnKeyPressed( i_ciKeyInput )
  {
    if( InputMode.Direction == m_eInputMode )
    {
      HandleDirectionInput( i_ciKeyInput );
    }
    else if( InputMode.Game == m_eInputMode )
    {
      HandleGameInput( i_ciKeyInput );
    }
    else if( InputMode.Target == m_eInputMode )
    {
      HandleTargetInput( i_ciKeyInput );
    }
    else if( InputMode.PressAnyKey == m_eInputMode )
    {
      HandleAnyKeyInput( i_ciKeyInput );
    }
    else if( InputMode.Amount == m_eInputMode )
    {
      HandleAmountInput( i_ciKeyInput );
    }
    else if( InputMode.Text_Response == m_eInputMode )
    {
      HandleTextResponseInput( i_ciKeyInput );
    }
    else if( InputMode.Yes_No_Question == m_eInputMode )
    {
      HandleYesNoQuestionInput( i_ciKeyInput );
    }
    else if( ( InputMode.Text_Input == m_eInputMode ) ||
             ( InputMode.NPC_Chat == m_eInputMode )   ||
             ( InputMode.Seer_Chat == m_eInputMode )  ||
             ( InputMode.Player_Chat == m_eInputMode ) )
    {
      HandleChatInput( i_ciKeyInput );
    }
    else
    {
      base.OnKeyPressed( i_ciKeyInput );
    }
  }


  function OnKeyRepeated( i_ciKeyInput )
  {
    OnKeyPressed( i_ciKeyInput );
  }


  function PrintInputMode( i_eMode )
  {
    switch( i_eMode )
    {
      case InputMode.Unset: print( "Unset" ); break;
      case InputMode.Game: print( "Game" ); break;
      case InputMode.Text_Input: print( "Text_Input" ); break;
      case InputMode.NPC_Chat: print( "NPC_Chat" ); break;
      case InputMode.Player_Chat: print( "Player_Chat" ); break;
      case InputMode.Direction: print( "Direction" ); break;
      case InputMode.Yes_No_Question: print( "Yes_No_Question" ); break;
      case InputMode.Amount: print( "Amount" ); break;
      case InputMode.Text_Response: print( "Text_Response" ); break;
      case InputMode.List_Selection: print( "List_Selection" ); break;
      case InputMode.Slider: print( "Slider" ); break;
      case InputMode.Seer_Chat: print( "Seer_Chat" ); break;
      case InputMode.Target: print( "Target" ); break;
      case InputMode.PressAnyKey: print( "PressAnyKey" ); break;
      default: print( "Unknown: " + i_eMode ); break;
    }
  }


  function ResetInputMode( i_bDoCallback, ... )
  {
    if( InputMode.Game == m_eInputMode )
    {
      return;
    }

    if( ( null == m_ciInputModeContext ) || !( m_ciInputModeContext instanceof InputModeContext ) )
    {
      return;
    }

    m_eInputMode = InputMode.Game;

    // Get the callback function of the current mode
    if( i_bDoCallback )
    {
      local funcCallback = m_ciInputModeContext.m_funcCallback;
      if( funcCallback )
      {
        local vParamArray = m_ciInputModeContext.m_vParams;
        if( null == vParamArray )
        {
          // Use incoming parameters only
          vParamArray = vargv;
        }
        else
        {
          // Append the incoming parameters
          vParamArray.extend( vargv );
        }

        switch( vParamArray.len() )
        {
          case 1:
            funcCallback.call( getroottable(), vParamArray[0] );
            break;

          case 2:
            funcCallback.call( getroottable(), vParamArray[0], vParamArray[1] );
            break;

          case 3:
            funcCallback.call( getroottable(), vParamArray[0], vParamArray[1], vParamArray[2] );
            break;

          case 4:
            funcCallback.call( getroottable(), vParamArray[0], vParamArray[1], vParamArray[2], vParamArray[3] );
            break;

          case 5:
            funcCallback.call( getroottable(), vParamArray[0], vParamArray[1], vParamArray[2], vParamArray[3],
                               vParamArray[4] );
            break;

          case 0:
          default:
            funcCallback.call( getroottable() );
            break;
        }
      }
    }

    UpdateBackgroundColor();
  }


  function SetInputMode( i_eMode, ... )
  {
    m_ciInputModeContext = null;

    switch( vargv.len() )
    {
      case 3: CreateInputModeContext( i_eMode, vargv[0], vargv[1], vargv[2] ); break;
      case 2: CreateInputModeContext( i_eMode, vargv[0], vargv[1] ); break;
      case 1: CreateInputModeContext( i_eMode, vargv[0] ); break;
      case 0:
      default: CreateInputModeContext( i_eMode ); break;
    }

    m_eInputMode = i_eMode;

    UpdateBackgroundColor();
    UpdateInputText();
  }


  function TargetSelected()
  {
    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();
    local ciPlayerPos = ciPlayer.GetPosition();

    local ciTarget = null;

    // Determine what the player is trying to select
    local bFound = false;
    local ciPawn = null;
    local ciPosData = ciMap.GetPositionData( g_ciCUO.m_ciTargetPos );
    while( ciPawn = ciPosData.GetNextObject() )
    {
      local uiPawnID = ciPawn.GetID();
      if( ciPawn instanceof Creature )
      {
        if( ( ciPawn == ciPlayer ) ||
            ( uiPawnID in g_ciCUO.m_uiVisibleTargetIDsTable &&
              ciMap.TestLOS( ciPlayerPos, ciPawn.GetPosition(), g_ciCUO.s_iLOSRadius ) ) )
        {
          g_ciCUO.m_uiLockedTargetID = uiPawnID;
          ciTarget = ciPawn;
          ciPosData.Stop();
        }
      }
      else if( ciPawn instanceof Widget )
      {
        if( ciMap.TestLOS( ciPlayerPos, ciPawn.GetPosition(), g_ciCUO.s_iLOSRadius ) )
        {
          g_ciCUO.m_uiLockedTargetID = uiPawnID;
          ciTarget = ciPawn;
          ciPosData.Stop();
        }
      }
    }

    if( g_ciCUO.m_uiLockedTargetID != rumInvalidGameID )
    {
      if( null == ciTarget )
      {
        ciTarget = ::rumFetchPawn( g_ciCUO.m_uiLockedTargetID );
      }

      if( ciTarget != null )
      {
        local strDesc = GetText();
        if( ciTarget == ciPlayer )
        {
          strDesc += ::rumGetString( token_self_client_StringID );
        }
        else if( ciTarget instanceof Player )
        {
          strDesc += ciTarget.GetPlayerName();
        }
        else if( ( ciTarget instanceof NPC ) || ( ciTarget instanceof Widget ) )
        {
          local strName = ciTarget.GetName() + "_client_StringID";
          strDesc += ::rumGetStringByName( strName );
        }

        ShowString( "<G#Prompt:vcenter>" + strDesc );
      }
    }

    Clear();
    EndInputMode( true, g_ciCUO.m_uiLockedTargetID );
  }


  function UpdateBackgroundColor()
  {
    switch( m_eInputMode )
    {
      case InputMode.NPC_Chat:
      case InputMode.Seer_Chat:
        SetBackgroundColor( g_ciUI.s_ciColorLightBlue );
        break;

      case InputMode.Direction:
      case InputMode.PressAnyKey:
      case InputMode.List_Selection:
      case InputMode.Yes_No_Question:
      case InputMode.Amount:
      case InputMode.Player_Chat:
      case InputMode.Text_Input:
      case InputMode.Target:
      case InputMode.Text_Response:
        SetBackgroundColor( g_ciUI.s_ciColorDarkGreen );
        break;

      case InputMode.Game:
      default:
        SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
        break;
    }
  }


  function UpdateInputText()
  {
    switch( m_eInputMode )
    {
      case InputMode.Direction:
        SetText( format( "<%s>", ::rumGetString( token_direction_client_StringID ) ) );
        break;

      case InputMode.PressAnyKey:
        SetText( format( "<%s>", ::rumGetString( token_press_any_key_client_StringID ) ) );
        break;

      case InputMode.Target:
        SetText( format( "<%s>", ::rumGetString( token_target_client_StringID ) ) );
        break;

      //case InputMode.Yes_No_Question:
      //  SetText(format("<%s>", ::rumGetString( msg_yes_no_prompt_client_StringID )));
      //  break;

      //case InputMode.Amount:
      //  SetText(format("<%s>", ::rumGetString( msg_amount_client_StringID )));
      //  break;
    }
  }
}
