class LoginTitleStage
{
  m_ciSettingsTable = { bNeedsRenderInit = true };


  function BeginStage()
  {
    if( ::rumIsAccountLoggedIn() )
    {
      EndStage( TitleStages.MainMenu );
    }
    else
    {
      g_ciUI.m_ciInfoLabel.SetActive( true );
      g_ciUI.m_ciTitleLabel.SetActive( true );

      g_ciUI.m_ciTitleListMenu.SetActive( true );
      g_ciUI.m_ciTitleListMenu.Focus();
      g_ciUI.m_ciTitleListMenu.Clear();
      g_ciUI.m_ciTitleListMenu.SetWidth( g_ciUI.s_iMenuWidth );

      g_ciUI.m_ciInfoLabel.SetText( "" );

      if( !g_ciCUO.m_bAccountCreate )
      {
        g_ciUI.m_ciTitleListMenu.SetEntry( LoginMenu.Name, ::rumGetString( account_name_client_StringID ) );
        g_ciUI.m_ciTitleListMenu.SetEntry( LoginMenu.Password, ::rumGetString( account_password_client_StringID ) );
        g_ciUI.m_ciTitleListMenu.SetEntry( LoginMenu.Login, ::rumGetString( account_login_client_StringID ) );
        g_ciUI.m_ciTitleListMenu.SetEntry( LoginMenu.Cancel, ::rumGetString( token_cancel_client_StringID ) );

        CalculateListViewSize( g_ciUI.m_ciTitleListMenu, "default", 4 );

        g_ciUI.m_ciTextBoxArray[LoginMenu.Name].SetText( g_ciCUO.m_ciGameConfigTable["cuo:username"] );
        g_ciUI.m_ciTextBoxArray[LoginMenu.Name].ObscureText( false );
        g_ciUI.m_ciTextBoxArray[LoginMenu.Name].SetActive( true );
        g_ciUI.m_ciTextBoxArray[LoginMenu.Name].SetInputRegEx( "[A-Za-z]+[A-Za-z0-9_]*" );
        g_ciUI.m_ciTextBoxArray[LoginMenu.Name].m_funcAccept = OnTextBoxDone.bindenv( LoginTitleStage );
        g_ciUI.m_ciTextBoxArray[LoginMenu.Name].m_funcCancel = OnTextBoxDone.bindenv( LoginTitleStage );
        g_ciUI.m_ciTextBoxArray[LoginMenu.Name].m_funcKeyPress = OnTextBoxKeyPressed.bindenv( LoginTitleStage );

        g_ciUI.m_ciTextBoxArray[LoginMenu.Password].SetText( "" );
        g_ciUI.m_ciTextBoxArray[LoginMenu.Password].ObscureText( true );
        g_ciUI.m_ciTextBoxArray[LoginMenu.Password].SetActive( true );
        g_ciUI.m_ciTextBoxArray[LoginMenu.Password].m_funcAccept = OnTextBoxDone.bindenv( LoginTitleStage );
        g_ciUI.m_ciTextBoxArray[LoginMenu.Password].m_funcCancel = OnTextBoxDone.bindenv( LoginTitleStage );
        g_ciUI.m_ciTextBoxArray[LoginMenu.Password].m_funcKeyPress = OnTextBoxKeyPressed.bindenv( LoginTitleStage );
      }
      else
      {
        g_ciUI.m_ciTitleListMenu.SetEntry( NewAccountMenu.Name,
                                           ::rumGetString( account_name_client_StringID ) );
        g_ciUI.m_ciTitleListMenu.SetEntry( NewAccountMenu.Email,
                                           ::rumGetString( account_email_client_StringID ) );
        g_ciUI.m_ciTitleListMenu.SetEntry( NewAccountMenu.Password,
                                           ::rumGetString( account_password_client_StringID ) );
        g_ciUI.m_ciTitleListMenu.SetEntry( NewAccountMenu.Verify,
                                           ::rumGetString( account_password_verify_client_StringID ) );
        g_ciUI.m_ciTitleListMenu.SetEntry( NewAccountMenu.Create,
                                           ::rumGetString( account_create_client_StringID ) );
        g_ciUI.m_ciTitleListMenu.SetEntry( NewAccountMenu.Cancel,
                                           ::rumGetString( token_cancel_client_StringID ) );

        CalculateListViewSize( g_ciUI.m_ciTitleListMenu, "default", 6 );

        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Name].SetText( "" );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Name].ObscureText( false );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Name].SetActive( true );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Name].SetInputRegEx( "[a-zA-Z]+[A-Za-z0-9_]*" );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Name].m_funcAccept = OnTextBoxDone.bindenv( LoginTitleStage );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Name].m_funcCancel = OnTextBoxDone.bindenv( LoginTitleStage );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Name].m_funcKeyPress = OnTextBoxKeyPressed.bindenv( LoginTitleStage );

        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Email].SetText( "" );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Email].ObscureText( false );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Email].SetActive( true );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Email].SetInputRegEx( "[a-zA-Z0-9._%+-]+@*[a-zA-Z0-9.-]*[.]*[a-zA-Z]*" );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Email].m_funcAccept = OnTextBoxDone.bindenv( LoginTitleStage );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Email].m_funcCancel = OnTextBoxDone.bindenv( LoginTitleStage );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Email].m_funcKeyPress = OnTextBoxKeyPressed.bindenv( LoginTitleStage );

        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Password].SetText( "" );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Password].ObscureText( true );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Password].SetActive( true );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Password].m_funcAccept = OnTextBoxDone.bindenv( LoginTitleStage );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Password].m_funcCancel = OnTextBoxDone.bindenv( LoginTitleStage );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Password].m_funcKeyPress = OnTextBoxKeyPressed.bindenv( LoginTitleStage );

        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Verify].SetText( "" );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Verify].ObscureText( true );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Verify].SetActive( true );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Verify].m_funcAccept = OnTextBoxDone.bindenv( LoginTitleStage );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Verify].m_funcCancel = OnTextBoxDone.bindenv( LoginTitleStage );
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Verify].m_funcKeyPress = OnTextBoxKeyPressed.bindenv( LoginTitleStage );
      }

      g_ciUI.m_ciTitleListMenu.m_funcAccept = OnListMenuAccepted.bindenv( LoginTitleStage );
      g_ciUI.m_ciTitleListMenu.m_funcCancel = OnListMenuCanceled.bindenv( LoginTitleStage );
      g_ciUI.m_ciTitleListMenu.m_funcKeyPress = OnListMenuKeyPressed.bindenv( LoginTitleStage );

      g_ciUI.m_ciTitleListMenu.Focus();
    }
  }


  function EndStage( i_eNextStage )
  {
    m_ciSettingsTable.bNeedsRenderInit = true;

    g_ciUI.m_ciInfoLabel.SetActive( false );
    g_ciUI.m_ciInfoLabel.SetText( "" );

    if( g_ciCUO.m_bAccountCreate )
    {
      g_ciUI.m_ciTextBoxArray[NewAccountMenu.Name].SetActive( false );
      g_ciUI.m_ciTextBoxArray[NewAccountMenu.Name].ClearHandlers();
      g_ciUI.m_ciTextBoxArray[NewAccountMenu.Email].SetActive( false );
      g_ciUI.m_ciTextBoxArray[NewAccountMenu.Email].ClearHandlers();
      g_ciUI.m_ciTextBoxArray[NewAccountMenu.Password].SetActive( false );
      g_ciUI.m_ciTextBoxArray[NewAccountMenu.Password].ClearHandlers();
      g_ciUI.m_ciTextBoxArray[NewAccountMenu.Verify].SetActive( false );
      g_ciUI.m_ciTextBoxArray[NewAccountMenu.Verify].ClearHandlers();
    }
    else
    {
      g_ciUI.m_ciTextBoxArray[LoginMenu.Name].SetActive( false );
      g_ciUI.m_ciTextBoxArray[LoginMenu.Name].ClearHandlers();
      g_ciUI.m_ciTextBoxArray[LoginMenu.Password].SetActive( false );
      g_ciUI.m_ciTextBoxArray[LoginMenu.Password].ClearHandlers();

      g_ciCUO.m_bAccountCreate = false;
    }

    g_ciUI.m_ciTitleLabel.SetActive( false );
    g_ciUI.m_ciTitleListMenu.SetActive( false );
    g_ciUI.m_ciTitleListMenu.ClearHandlers();

    InitTitleStage( i_eNextStage );
  }


  function OnListMenuAccepted()
  {
    g_ciUI.m_ciInfoLabel.SetText( "" );

    local iSelectedKey = g_ciUI.m_ciTitleListMenu.GetSelectedKey();

    if( g_ciCUO.m_bAccountCreate )
    {
      if( NewAccountMenu.Name == iSelectedKey )
      {
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Name].Focus();
      }
      else if( NewAccountMenu.Email == iSelectedKey )
      {
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Email].Focus();
      }
      else if( NewAccountMenu.Password == iSelectedKey )
      {
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Password].Focus();
      }
      else if( NewAccountMenu.Verify == iSelectedKey )
      {
        g_ciUI.m_ciTextBoxArray[NewAccountMenu.Verify].Focus();
      }
      else if( NewAccountMenu.Create == iSelectedKey )
      {
        local bUpdate = false;

        local strName = g_ciUI.m_ciTextBoxArray[NewAccountMenu.Name].GetText();
        if( g_ciCUO.m_ciGameConfigTable["cuo:username"] != strName )
        {
          g_ciCUO.m_ciGameConfigTable["cuo:username"] = strName;
          bUpdate = true;
        }

        if( bUpdate )
        {
          ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );
        }

        if( g_ciUI.m_ciTextBoxArray[NewAccountMenu.Password].GetText() !=
            g_ciUI.m_ciTextBoxArray[NewAccountMenu.Verify].GetText() )
        {
          g_ciUI.m_ciInfoLabel.SetText( ::rumGetString( rum_account_create_password_mismatch_shared_StringID ) );
        }
        else
        {
          if( ::rumCreateAccount( strName, g_ciUI.m_ciTextBoxArray[NewAccountMenu.Email].GetText(),
                                  g_ciUI.m_ciTextBoxArray[NewAccountMenu.Password].GetText() ) == 0 )
          {
            g_ciCUO.m_bWaitingForResponse = true;
          }
          else
          {
            local strError = ::rumGetLastErrorString();
            if( strError != null )
            {
              g_ciUI.m_ciInfoLabel.SetText( ::rumGetStringByName( strError + "_shared_StringID" ) );
            }
          }
        }
      }
      else if( NewAccountMenu.Cancel == iSelectedKey )
      {
        EndStage( TitleStages.AccountMenu );
      }
    }
    else
    {
      if( LoginMenu.Name == iSelectedKey )
      {
        g_ciUI.m_ciTextBoxArray[LoginMenu.Name].Focus();
      }
      else if( LoginMenu.Password == iSelectedKey )
      {
        g_ciUI.m_ciTextBoxArray[LoginMenu.Password].Focus();
      }
      else if( LoginMenu.Login == iSelectedKey )
      {
        local bUpdate = false;

        local strName = g_ciUI.m_ciTextBoxArray[LoginMenu.Name].GetText();
        if( g_ciCUO.m_ciGameConfigTable["cuo:username"] != strName )
        {
          g_ciCUO.m_ciGameConfigTable["cuo:username"] = strName;
          bUpdate = true;
        }

        if( bUpdate )
        {
          ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );
        }

        if( ::rumAccountLogin( strName, g_ciUI.m_ciTextBoxArray[LoginMenu.Password].GetText() ) == 0 )
        {
          g_ciCUO.m_bWaitingForResponse = true;
        }
        else
        {
          local strError = ::rumGetLastErrorString();
          if( strError != null )
          {
            g_ciUI.m_ciInfoLabel.SetText( ::rumGetStringByName( strError + "_shared_StringID" ) );
          }
        }
      }
      else if( LoginMenu.Cancel == iSelectedKey )
      {
        EndStage( TitleStages.AccountMenu );
      }
    }
  }


  function OnListMenuCanceled()
  {
    g_ciUI.m_ciInfoLabel.SetText( "" );
    EndStage( TitleStages.AccountMenu );
  }


  function OnListMenuKeyPressed( i_ciKeyInput )
  {
    g_ciUI.m_ciInfoLabel.SetText( "" );

    local eKey = i_ciKeyInput.GetKey();
    local bEchoInput = false;

    if( eKey >= rumKeypress.KeyA() && eKey <= rumKeypress.KeyZ() )
    {
      bEchoInput = true;
    }

    if( bEchoInput || ( ( rumKeypress.KeyRight() == eKey ) || ( rumKeypress.KeyTab() == eKey ) ) )
    {
      local iSelectedKey = g_ciUI.m_ciTitleListMenu.GetSelectedKey();

      g_ciUI.m_ciTextBoxArray[iSelectedKey].Focus();
      if( bEchoInput )
      {
        g_ciUI.m_ciTextBoxArray[iSelectedKey].CharacterAdd( i_ciKeyInput.GetAscii() );
      }
    }
  }


  function OnTextBoxDone()
  {
    if( g_ciCUO.m_bAccountCreate )
    {
      if( g_ciUI.m_ciTextBoxArray[NewAccountMenu.Name].HasInputFocus()  ||
          g_ciUI.m_ciTextBoxArray[NewAccountMenu.Email].HasInputFocus() ||
          g_ciUI.m_ciTextBoxArray[NewAccountMenu.Password].HasInputFocus() )
      {
        g_ciUI.m_ciTitleListMenu.MoveNext();
      }
    }
    else
    {
      if( g_ciUI.m_ciTextBoxArray[LoginMenu.Name].HasInputFocus() ||
          g_ciUI.m_ciTextBoxArray[LoginMenu.Password].HasInputFocus() )
      {
        g_ciUI.m_ciTitleListMenu.MoveNext();
      }
    }

    g_ciUI.m_ciTitleListMenu.Focus();
  }


  function OnTextBoxKeyPressed( i_ciKeyInput )
  {
    local eKey = i_ciKeyInput.GetKey();

    if( rumKeypress.KeyDown() == eKey )
    {
      g_ciUI.m_ciTitleListMenu.Focus();
      g_ciUI.m_ciTitleListMenu.MoveNext();
    }
    else if( rumKeypress.KeyUp() == eKey )
    {
      g_ciUI.m_ciTitleListMenu.Focus();
      g_ciUI.m_ciTitleListMenu.MovePrev();
    }
  }


  function Render( i_iOffsetY )
  {
    if( !m_ciSettingsTable.bNeedsRenderInit )
    {
      return;
    }

    local iScreenWidth = ::rumGetScreenWidth();
    local iScreenCenterX = iScreenWidth / 2;

    local iWidth = g_ciUI.s_iTextBoxWidth + g_ciUI.s_iMenuWidth;
    local iMenuOffsetX = iScreenCenterX - ( iWidth / 2 );
    local iTextOffsetX = iMenuOffsetX + g_ciUI.m_ciTitleListMenu.GetWidth();

    i_iOffsetY += g_ciUI.s_iDefaultLabelHeight * 2;

    // Display menu title
    g_ciUI.m_ciTitleLabel.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciTitleLabel.GetWidth() / 2 ), i_iOffsetY ) );
    i_iOffsetY += g_ciUI.m_ciTitleLabel.GetHeight() + ( g_ciUI.s_iDefaultLabelHeight * 2 );

    // Display menu
    g_ciUI.m_ciTitleListMenu.SetPos( rumPoint( iMenuOffsetX, i_iOffsetY ) );

    if( g_ciCUO.m_bAccountCreate )
    {
      foreach( ciTextBox in g_ciUI.m_ciTextBoxArray )
      {
        ciTextBox.SetPos( rumPoint( iTextOffsetX, i_iOffsetY ) );
        i_iOffsetY += ciTextBox.GetHeight();
      }
    }
    else
    {
      g_ciUI.m_ciTextBoxArray[LoginMenu.Name].SetPos( rumPoint( iTextOffsetX, i_iOffsetY ) );
      i_iOffsetY += g_ciUI.m_ciTextBoxArray[LoginMenu.Name].GetHeight();

      g_ciUI.m_ciTextBoxArray[LoginMenu.Password].SetPos( rumPoint( iTextOffsetX, i_iOffsetY ) );
      i_iOffsetY += g_ciUI.m_ciTextBoxArray[LoginMenu.Password].GetHeight();
    }

    i_iOffsetY += g_ciUI.s_iDefaultLabelHeight * 3;

    g_ciUI.m_ciInfoLabel.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciInfoLabel.GetWidth() / 2 ), i_iOffsetY ) );

    m_ciSettingsTable.bNeedsRenderInit = false;
  }
}
