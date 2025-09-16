enum StoryIndex
{
  JourneyBegin = 0,
  JourneyEnd   = 19,
  Gypsy        = 20,
  VirtueTest   = 23,
  Complete     = 25
}


class CharGenSingleton
{
  // State
  m_bQuestionAsked = false;

  m_eGender = GenderType.Unspecified;
  m_eStage = CharGen.Name;

  m_eBlackBeadArray =
  [
    VirtueType.Honesty, VirtueType.Honesty, VirtueType.Honesty, VirtueType.Honesty,
    VirtueType.Honesty, VirtueType.Honesty, VirtueType.Honesty
  ];

  m_eWhiteBeadArray =
  [
    VirtueType.Honesty, VirtueType.Honesty, VirtueType.Honesty, VirtueType.Honesty,
    VirtueType.Honesty, VirtueType.Honesty, VirtueType.Honesty
  ];

  m_eCardVirtueLeft = VirtueType.Honesty;
  m_eCardVirtueRight = VirtueType.Honesty;

  m_iAnimFrame = 0;
  m_iStoryIndex = StoryIndex.JourneyBegin;
  m_iQuestionNum = 0;
  m_iUsedCards = 0;

  // Controls
  m_ciInputControl = null;
  m_ciLabelControl = null;
  m_ciListControl = null;
  m_ciStatusControl = null;
  m_ciTextControl = null;

  m_strCharacterName = "";

  static s_eQuickBeadSelectionArray =
  [
    // Mage/Wizard
    [ VirtueType.Justice, VirtueType.Honesty, VirtueType.Honor, VirtueType.Valor,
      VirtueType.Honesty, VirtueType.Justice, VirtueType.Honesty ],

    // Mage/Alchemist
    [ VirtueType.Justice, VirtueType.Honesty, VirtueType.Honor, VirtueType.Compassion,
      VirtueType.Honesty, VirtueType.Justice, VirtueType.Honesty ],

    // Bard/Thief
    [ VirtueType.Sacrifice, VirtueType.Compassion, VirtueType.Justice, VirtueType.Valor,
      VirtueType.Compassion, VirtueType.Sacrifice, VirtueType.Compassion ],

    // Bard/Lark
    [ VirtueType.Justice, VirtueType.Compassion, VirtueType.Sacrifice, VirtueType.Honesty,
      VirtueType.Compassion, VirtueType.Justice, VirtueType.Compassion ],

    // Fighter
    [ VirtueType.Honor, VirtueType.Valor, VirtueType.Justice, VirtueType.Honesty,
      VirtueType.Valor, VirtueType.Honor, VirtueType.Valor ],

    // Fighter/Barbarian
    [ VirtueType.Sacrifice, VirtueType.Valor, VirtueType.Compassion, VirtueType.Honor,
      VirtueType.Valor, VirtueType.Sacrifice, VirtueType.Valor ],

    // Druid/Druid/Wizard
    [ VirtueType.Honesty, VirtueType.Justice, VirtueType.Compassion, VirtueType.Honor,
      VirtueType.Justice, VirtueType.Honesty, VirtueType.Justice ],

    // Druid/Wizard/Wizard
    [ VirtueType.Honor, VirtueType.Justice, VirtueType.Valor, VirtueType.Honesty,
      VirtueType.Justice, VirtueType.Honor, VirtueType.Justice ],

    // Tinker/Barbarian
    [ VirtueType.Valor, VirtueType.Sacrifice, VirtueType.Compassion, VirtueType.Honor,
      VirtueType.Sacrifice, VirtueType.Valor, VirtueType.Sacrifice ],

    // Tinker/Fighter
    [ VirtueType.Valor, VirtueType.Sacrifice, VirtueType.Compassion, VirtueType.Justice,
      VirtueType.Sacrifice, VirtueType.Valor, VirtueType.Sacrifice ],

    // Paladin/Paladin/Cleric
    [ VirtueType.Spirituality, VirtueType.Honor, VirtueType.Valor, VirtueType.Sacrifice,
      VirtueType.Honor, VirtueType.Spirituality, VirtueType.Honor ],

    // Paladin/Cleric/Cleric
    [ VirtueType.Spirituality, VirtueType.Honor, VirtueType.Compassion, VirtueType.Sacrifice,
      VirtueType.Honor, VirtueType.Spirituality, VirtueType.Honor ],

    // Ranger/Ranger/Thief
    [ VirtueType.Compassion, VirtueType.Spirituality, VirtueType.Sacrifice, VirtueType.Valor,
      VirtueType.Spirituality, VirtueType.Compassion, VirtueType.Spirituality ],

    // Ranger/Thief/Thief
    [ VirtueType.Compassion, VirtueType.Spirituality, VirtueType.Justice, VirtueType.Honesty,
      VirtueType.Spirituality, VirtueType.Compassion, VirtueType.Spirituality ],

    // Shepherd/Cleric
    [ VirtueType.Spirituality, VirtueType.Humility, VirtueType.Valor, VirtueType.Sacrifice,
      VirtueType.Humility, VirtueType.Spirituality, VirtueType.Humility ],

    // Shepherd/Illusionist
    [ VirtueType.Spirituality, VirtueType.Humility, VirtueType.Compassion, VirtueType.Justice,
      VirtueType.Humility, VirtueType.Spirituality, VirtueType.Humility ]
  ]

  // Intro story graphic frames
  static s_eIntroStoryGraphicIndicesArray =
  [
    Intro.CircleFar, Intro.CircleFar, Intro.CircleFar, Intro.Moongate, Intro.Moongate, Intro.CircleFar,
    Intro.Circle, Intro.Circle, Intro.Circle, Intro.Circle, Intro.Circle, Intro.CircleFar,
    Intro.CircleFar, Intro.CircleFar, Intro.FaireFar, Intro.FaireFar, Intro.Faire, Intro.Faire,
    Intro.Faire, Intro.Wagon, Intro.Gypsy, Intro.Gypsy, Intro.Abacus, Intro.Abacus, Intro.Abacus
  ]

  m_ciSettingsTable = { bNeedsRenderInit = true };


  constructor()
  {
    local ciGraphic = ::rumGetGraphic( U4_Intro_GraphicID );

    m_ciInputControl = ::rumCreateControl( TextBox );
    m_ciInputControl.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
    m_ciInputControl.SetWidth( 128 );
    m_ciInputControl.SetHeight( g_ciUI.s_iBorderPixelWidth );
    m_ciInputControl.SetCursor( Cursor_GraphicID );
    m_ciInputControl.ShowCursor( true );
    m_ciInputControl.AlignCenter();

    m_ciLabelControl = ::rumCreateControl( Label );
    m_ciLabelControl.SetWidth( ::rumGetScreenWidth() );
    m_ciLabelControl.SetHeight( g_ciUI.s_iBorderPixelWidth );
    //m_ciLabelControl.ShowCursor(true);
    //m_ciLabelControl.SetCursor( Cursor_GraphicID );
    m_ciLabelControl.AlignCenter();

    m_ciListControl = ::rumCreateControl( ListView );
    m_ciListControl.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
    m_ciListControl.SetWidth( 256 );
    m_ciListControl.SetHeight( g_ciUI.s_iBorderPixelWidth * 4 );
    m_ciListControl.ShowScrollbar( false );
    m_ciListControl.SetFormat( "1.0" );
    m_ciListControl.AlignCenter();
    m_ciListControl.Clear();

    m_ciStatusControl = ::rumCreateControl( Label );
    m_ciStatusControl.SetBackgroundColor( ::rumColorBlack );
    m_ciStatusControl.SetWidth( ::rumGetScreenWidth() );
    m_ciStatusControl.SetHeight( g_ciUI.s_iBorderPixelWidth );
    m_ciStatusControl.AlignCenter();

    m_ciTextControl = ::rumCreateControl( TextView );
    m_ciTextControl.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
    m_ciTextControl.SetWidth( ::rumGetScreenWidth() * 0.95 );
    m_ciTextControl.SetHeight( ( ::rumGetScreenHeight() - ciGraphic.GetFrameHeight() ) * 0.80 );
    m_ciTextControl.SetBufferHeight( m_ciTextControl.GetHeight() );
    m_ciTextControl.ShowScrollbar( false );
    m_ciTextControl.SetCursor( Cursor_GraphicID );
    m_ciTextControl.ShowCursor( true );
    m_ciTextControl.SetNewestFirst( true );
    m_ciTextControl.AlignLeft();
  }


  function AdvanceStory()
  {
    ++m_iStoryIndex;

    if( StoryIndex.VirtueTest == m_iStoryIndex )
    {
      InitStage( CharGen.Gypsy );
    }
    else
    {
      // See if the shrine music should play
      if( StoryIndex.Gypsy == m_iStoryIndex )
      {
        PlayMusic( U4_Shrines_SoundID );
      }

      m_iAnimFrame = s_eIntroStoryGraphicIndicesArray[m_iStoryIndex];
      local strDesc = format( "chargen_story%02d_client_StringID", m_iStoryIndex + 1 );
      m_ciTextControl.Clear();
      m_ciTextControl.PushText( ::rumGetStringByName( strDesc ) );
    }
  }


  function AnswerQuestion( i_eCardChosen, i_eCardUnchosen)
  {
    m_eWhiteBeadArray[m_iQuestionNum] = i_eCardChosen;
    m_eBlackBeadArray[m_iQuestionNum] = i_eCardUnchosen;

    // Prevent the cards from being picked again
    m_iUsedCards = m_iUsedCards | ( 1 << m_eCardVirtueLeft );
    m_iUsedCards = m_iUsedCards | ( 1 << m_eCardVirtueRight );

    m_bQuestionAsked = false;
    ++m_iQuestionNum;

    // On the 4th and 6th questions, we must redetermine what the available cards are
    if( ( m_iQuestionNum == 4 ) || ( m_iQuestionNum == 6 ) )
    {
      m_iUsedCards = 0;
      for( local i = 0; i < m_iQuestionNum; ++i )
      {
        m_iUsedCards = m_iUsedCards | ( 1 << m_eBlackBeadArray[i] );
      }
    }

    if( m_iQuestionNum < 7 )
    {
      // Pick 2 random virtues
      GetNewCards();

      local str1 = ::rumGetString( chargen_gypsycard1_client_StringID );
      local str2 = ::rumGetString( chargen_gypsycards_client_StringID );

      local strVirtue1 = ::rumGetString( g_eU4VirtueStringArray[m_eCardVirtueLeft] );
      local strVirtue2 = ::rumGetString( g_eU4VirtueStringArray[m_eCardVirtueRight] );

      str2 = format( str2, strVirtue1, strVirtue2 );
      m_ciTextControl.Clear();
      m_ciTextControl.PushText( str1 + " " + str2 );
    }
    else
    {
      InitStage( CharGen.TestDone );
    }
  }


  function AskQuestion()
  {
    // Get the gypsy question
    local strDesc = ::rumGetStringByName( "chargen_gypsy" + m_eCardVirtueLeft + m_eCardVirtueRight +
                                          "_client_StringID" );
    m_ciTextControl.Clear();
    m_ciTextControl.PushText( strDesc );

    m_bQuestionAsked = true;
  }


  function CardAvailable( i_eVirtue )
  {
    local bAvailable = true;

    if( m_iUsedCards & ( 1 << i_eVirtue ) )
    {
      bAvailable = false;
    }

    return bAvailable;
  }


  function ClearControls()
  {
    m_ciTextControl.SetActive( false );
    m_ciTextControl.Clear();
    m_ciTextControl.ClearHandlers();

    m_ciLabelControl.SetActive( false );
    m_ciLabelControl.Clear();

    m_ciInputControl.SetActive( false );
    m_ciInputControl.Clear();
    m_ciInputControl.ClearHandlers();

    m_ciStatusControl.SetActive( false );

    m_ciListControl.SetActive( false );
    m_ciListControl.Clear();
    m_ciListControl.ClearHandlers();
    m_ciListControl.Home();
    m_ciListControl.ShowScrollbar( false );

    g_ciUI.m_ciScreenRegion.SetActive( false );
    g_ciUI.m_ciScreenRegion.ClearHandlers();
  }


  function CreateCharacter()
  {
    m_ciListControl.SelectCurrent();
    if( CreateMode.Standard == m_ciListControl.GetSelectedKey() )
    {
      InitStage( CharGen.Story );
    }
    else
    {
      InitStage( CharGen.SelectChar );
    }
  }


  function CreateCharacterQuick()
  {
    m_ciListControl.SelectCurrent();
    local eQuickChar = m_ciListControl.GetSelectedKey();

    // The sixth selected white bead determines the player's class
    m_eWhiteBeadArray = s_eQuickBeadSelectionArray[eQuickChar];

    g_ciCUO.m_bWaitingForResponse = true;

    local ciBroadcast = ::rumCreate( Player_Initialize_BroadcastID, m_strCharacterName, m_eGender,
                                     m_eWhiteBeadArray );
    ::rumSendBroadcast( ciBroadcast );

    InitStage( CharGen.Create );
  }


  function GenderSelected()
  {
    m_ciListControl.SelectCurrent();
    m_eGender = m_ciListControl.GetSelectedKey();
    InitStage( CharGen.CreateMode );
  }


  function GetNewCards()
  {
    // Get first random card
    local eVirtue1 = rand() % VirtueType.NumVirtues;

    // Make sure card is available
    while( !CardAvailable( eVirtue1 ) )
    {
      ++eVirtue1;
      if( eVirtue1 >= VirtueType.NumVirtues )
      {
        eVirtue1 = VirtueType.Honesty;
      }
    }

    // Get second random card
    local eVirtue2 = rand() % VirtueType.NumVirtues;

    // Make sure card is available
    while( ( eVirtue2 == eVirtue1 ) || !CardAvailable( eVirtue2 ) )
    {
      ++eVirtue2;
      if( eVirtue2 >= VirtueType.NumVirtues )
      {
        eVirtue2 = 0;
      }
    }

    // Set cards
    m_eCardVirtueLeft = min( eVirtue1, eVirtue2 );
    m_eCardVirtueRight = max( eVirtue1, eVirtue2 );
  }


  function InitStage( i_eStage )
  {
    m_ciSettingsTable.bNeedsRenderInit = true;
    m_eStage = i_eStage;

    ClearControls();

    if( CharGen.Name == i_eStage )
    {
      // Collect player name
      m_ciLabelControl.SetActive( true );
      m_ciLabelControl.SetText( ::rumGetString( chargen_name_client_StringID ) );

      m_ciStatusControl.SetActive( true );

      m_ciInputControl.SetActive( true );
      m_ciInputControl.SetText( m_strCharacterName );
      m_ciInputControl.SetInputRegEx( "[A-Za-z]+[A-Za-z0-9_]*" );
      m_ciInputControl.ObscureText( false );
      m_ciInputControl.Focus();
      m_ciInputControl.m_funcAccept = OnTextBoxAccepted.bindenv( this );
      m_ciInputControl.m_funcCancel = OnTextBoxCanceled.bindenv( this );
    }
    else if( CharGen.Gender == i_eStage )
    {
      // Collect player gender
      m_ciLabelControl.SetActive( true );
      m_ciLabelControl.SetText( ::rumGetString( chargen_gender_client_StringID ) );

      m_ciStatusControl.SetActive( false );
      m_ciStatusControl.Clear();

      m_ciListControl.SetActive( true );
      m_ciListControl.Clear();
      m_ciListControl.SetEntry( 0, ::rumGetString( token_male_client_StringID ) );
      m_ciListControl.SetEntry( 1, ::rumGetString( token_female_client_StringID ) );
      m_ciListControl.SetEntry( 2, ::rumGetString( token_other_client_StringID ) );
      m_ciListControl.SetEntry( 3, ::rumGetString( token_unspecified_client_StringID ) );
      m_ciListControl.m_funcAccept = OnListViewAccepted.bindenv( this );
      m_ciListControl.m_funcCancel = OnListViewCanceled.bindenv( this );
      m_ciListControl.Focus();

      CalculateListViewSize( m_ciListControl, "default", 4 );
    }
    else if( CharGen.CreateMode == i_eStage )
    {
      m_iStoryIndex = StoryIndex.JourneyBegin;
      m_iAnimFrame = s_eIntroStoryGraphicIndicesArray[m_iStoryIndex];

      // Request character creation method - standard or quick
      m_ciLabelControl.SetActive( true );
      m_ciLabelControl.SetText( ::rumGetString( chargen_createmode_client_StringID ) );

      m_ciListControl.SetActive( true );
      m_ciListControl.Clear();
      m_ciListControl.SetEntry( CreateMode.Standard, ::rumGetString( token_yes_client_StringID ) );
      m_ciListControl.SetEntry( CreateMode.Quick, ::rumGetString( token_no_client_StringID ) );
      m_ciListControl.m_funcAccept = OnListViewAccepted.bindenv( this );
      m_ciListControl.m_funcCancel = OnListViewCanceled.bindenv( this );
      m_ciListControl.Focus();

      CalculateListViewSize( m_ciListControl, "default", 2 );
    }
    else if( CharGen.SelectChar == i_eStage )
    {
      // Build a list of all selectable character combinations spanning U1-U4
      m_ciLabelControl.SetActive( true );
      m_ciLabelControl.SetText( ::rumGetString( chargen_quickchar_client_StringID ) );

      m_ciListControl.SetActive( true );
      m_ciListControl.Clear();
      m_ciListControl.ShowScrollbar( true );

      for( local i = 0; i < QuickChar.NumQuickChars; ++i )
      {
        local strEntry = format( "chargen_quickchar%02d_client_StringID", i );
        m_ciListControl.SetEntry( i, ::rumGetStringByName( strEntry ) );
      }

      m_ciListControl.m_funcAccept = OnListViewAccepted.bindenv( this );
      m_ciListControl.m_funcCancel = OnListViewCanceled.bindenv( this );
      m_ciListControl.Focus();

      CalculateListViewSize( m_ciListControl, "default", 5 );
    }
    else if( CharGen.Story == i_eStage )
    {
      // Present Ultima 4 intro story
      if( m_iStoryIndex <= StoryIndex.JourneyEnd )
      {
        PlayMusic( U4_Towns_SoundID );
      }
      else
      {
        PlayMusic( U4_Shrines_SoundID );
      }

      local strDesc = format( "chargen_story%02d_client_StringID", m_iStoryIndex + 1 );

      g_ciUI.m_ciScreenRegion.SetActive( true );
      g_ciUI.m_ciScreenRegion.m_funcKeyPress = OnStoryKeyPress.bindenv( this );
      g_ciUI.m_ciScreenRegion.m_funcMouseButtonPress = OnStoryMouseButtonPress.bindenv( this );

      m_ciTextControl.SetActive( true );
      m_ciTextControl.Clear();
      m_ciTextControl.PushText( ::rumGetStringByName( strDesc ) );
      m_ciTextControl.m_funcAccept = StoryAccepted.bindenv( this );
      m_ciTextControl.m_funcCancel = StoryRejected.bindenv( this );
      m_ciTextControl.m_funcKeyPress = OnStoryKeyPress.bindenv( this );
      m_ciTextControl.Focus();
    }
    else if( CharGen.Gypsy == i_eStage )
    {
      // Virtue questionnaire
      m_bQuestionAsked = false;
      m_iQuestionNum = 0;
      m_iUsedCards = 0;

      // Pick 2 random virtues
      GetNewCards();

      local strCard1 = ::rumGetString( chargen_gypsycard0_client_StringID );
      local strCard2 = ::rumGetString( chargen_gypsycards_client_StringID );

      local strVirtue1 = ::rumGetString( g_eU4VirtueStringArray[m_eCardVirtueLeft] );
      local strVirtue2 = ::rumGetString( g_eU4VirtueStringArray[m_eCardVirtueRight] );

      strCard2 = format( strCard2, strVirtue1, strVirtue2 );

      g_ciUI.m_ciScreenRegion.SetActive( true );
      g_ciUI.m_ciScreenRegion.m_funcKeyPress = OnStoryKeyPress.bindenv( this );
      g_ciUI.m_ciScreenRegion.m_funcMouseButtonPress = OnStoryMouseButtonPress.bindenv( this );

      m_ciTextControl.SetActive( true );
      m_ciTextControl.Clear();
      m_ciTextControl.PushText( strCard1 + " " + strCard2 );
      m_ciTextControl.m_funcAccept = StoryAccepted.bindenv( this );
      m_ciTextControl.m_funcCancel = StoryRejected.bindenv( this );
      m_ciTextControl.m_funcKeyPress = OnStoryKeyPress.bindenv( this );
      m_ciTextControl.Focus();
    }
    else if( CharGen.TestDone == i_eStage )
    {
      local strDesc = format( "chargen_story%02d_client_StringID", m_iStoryIndex + 1 );

      g_ciUI.m_ciScreenRegion.SetActive( true );
      g_ciUI.m_ciScreenRegion.m_funcKeyPress = OnStoryKeyPress.bindenv( this );
      g_ciUI.m_ciScreenRegion.m_funcMouseButtonPress = OnStoryMouseButtonPress.bindenv( this );

      m_ciTextControl.SetActive( true );
      m_ciTextControl.Clear();
      m_ciTextControl.PushText( ::rumGetStringByName( strDesc ) );
      m_ciTextControl.m_funcAccept = StoryAccepted.bindenv( this );
      m_ciTextControl.m_funcCancel = StoryRejected.bindenv( this );
      m_ciTextControl.m_funcKeyPress = OnStoryKeyPress.bindenv( this );
      m_ciTextControl.Focus();
    }
    else if( CharGen.Create == i_eStage )
    {
      // Reset
      m_iStoryIndex = StoryIndex.JourneyBegin;
      m_iAnimFrame = s_eIntroStoryGraphicIndicesArray[m_iStoryIndex];

      // Wait for character creation pass/fail
      g_ciCUO.m_eCurrentGameMode = GameMode.Transition;

      // Position the daemon and dragon completely on the title screen
      g_ciUI.m_iTitleAnimationOffset = 0;

      // Clear pre-existing game activity for the new player
      g_ciUI.m_ciChatTextView.Clear();
      g_ciUI.m_ciGameTextView.Clear();
    }
  }


  function OnListViewAccepted()
  {
    if( CharGen.Gender == m_eStage )
    {
      GenderSelected();
    }
    else if( CharGen.CreateMode == m_eStage )
    {
      CreateCharacter();
    }
    else if( CharGen.SelectChar == m_eStage )
    {
      CreateCharacterQuick()
    }
  }


  function OnListViewCanceled()
  {
    if( CharGen.Gender == m_eStage )
    {
      InitStage( CharGen.Name );
    }
    else if( CharGen.CreateMode == m_eStage )
    {
      InitStage( CharGen.Gender );
    }
    else if( CharGen.SelectChar == m_eStage )
    {
      InitStage( CharGen.CreateMode );
    }
  }


  function OnStoryKeyPress( i_ciKeyInput )
  {
    local eKey = i_ciKeyInput.GetKey();

    if( CharGen.Gypsy == m_eStage )
    {
      if( m_bQuestionAsked )
      {
        if( ( rumKeypress.Key1() == eKey ) || ( rumKeypress.KeyPad1() == eKey ) )
        {
          AnswerQuestion( m_eCardVirtueLeft, m_eCardVirtueRight );
        }
        else if( ( rumKeypress.Key2() == eKey ) || ( rumKeypress.KeyPad2() == eKey ) )
        {
          AnswerQuestion( m_eCardVirtueRight, m_eCardVirtueLeft );
        }
      }
      else
      {
        AskQuestion();
      }
    }
    else if( rumKeypress.KeyEscape() == eKey )
    {
      StoryRejected();
    }
    else
    {
      StoryAccepted();
    }
  }


  function OnStoryMouseButtonPress( i_eButton, i_ciPoint )
  {
    if( rumLeftMouseButton != i_eButton )
    {
      return;
    }

    if( CharGen.Gypsy == m_eStage )
    {
      if( m_bQuestionAsked )
      {
        // Center the card on the left side of the screen
        local ciGraphic = ::rumGetGraphic( U4_Intro_GraphicID );

        // Determine the first card's bounding area
        local iVerticalOffset = ciGraphic.GetFrameHeight() / 2;
        local iHorizOffset = ciGraphic.GetFrameWidth() / 3;

        ciGraphic = ::rumGetGraphic( U4_Intro_Cards_GraphicID );

        local uiHeight = ciGraphic.GetFrameHeight();
        local uiWidth = ciGraphic.GetFrameWidth();

        iVerticalOffset -= uiHeight / 2;
        iHorizOffset -= uiWidth;

        local ciRectangle = rumRect( rumPoint( iHorizOffset, iVerticalOffset ), uiWidth, uiHeight );
        if( ciRectangle.ContainsPoint( i_ciPoint ) )
        {
          AnswerQuestion( m_eCardVirtueLeft, m_eCardVirtueRight );
        }
        else
        {
          // Determine the second card's position
          iHorizOffset = ::rumGetScreenWidth() - ( uiWidth + iHorizOffset );

          ciRectangle = rumRect( rumPoint( iHorizOffset, iVerticalOffset ), uiWidth, uiHeight );
          if( ciRectangle.ContainsPoint( i_ciPoint ) )
          {
            AnswerQuestion( m_eCardVirtueRight, m_eCardVirtueLeft );
          }
        }
      }
      else
      {
        AskQuestion();
      }
    }
    else
    {
      StoryAccepted();
    }
  }


  function OnTextBoxAccepted()
  {
    m_ciStatusControl.Clear();

    if( CharGen.Name == m_eStage )
    {
      local iNumCharacters = ::rumGetNumAccountCharacters();
      if( iNumCharacters < g_ciCUO.s_iMaxCharactersPerAccount )
      {
        m_strCharacterName = m_ciInputControl.GetText();
        if( m_strCharacterName != "" )
        {
          // Attempt to create a player by the specified name
          ::rumCreatePlayer( m_strCharacterName );

          g_ciCUO.m_bWaitingForResponse = true;

          m_ciStatusControl.SetText( ::rumGetString( chargen_name_validate_client_StringID ) );

          // Advance to player name validation
          InitStage( CharGen.Validation );
        }
      }
      else
      {
        m_ciStatusControl.SetText( ::rumGetString( chargen_slots_full_client_StringID ) );
      }
    }
  }


  function OnTextBoxCanceled()
  {
    m_ciStatusControl.Clear();

    if( CharGen.Name == m_eStage )
    {
      ClearControls();

      m_ciStatusControl.SetActive( false );

      g_ciCUO.m_eCurrentGameMode = GameMode.Title;
      InitTitleStage( TitleStages.MainMenu );

      PlayMusic( U4_Towns_SoundID );
    }
  }


  function Render()
  {
    ::rumClearScreen();

    // Rendering during player information gathering
    if( m_eStage < CharGen.Story )
    {
      local iScreenWidth = ::rumGetScreenWidth();
      local iScreenCenter = iScreenWidth / 2;

      // Draw the title
      local ciGraphic = ::rumGetGraphic( U4_Title_GraphicID );
      ciGraphic.Draw( rumPoint( iScreenCenter - ( ciGraphic.GetFrameWidth() / 2 ), 0 ) );

      local iVerticalOffset = ciGraphic.GetFrameHeight();

      // Draw the daemon
      ciGraphic = ::rumGetGraphic( U4_Title_Daemon_GraphicID );
      ciGraphic.DrawAnimation( rumPoint( 0, g_ciUI.m_iTitleAnimationOffset ),
                               0, g_ciUI.s_iDaemonFrameIndicesArray[g_ciUI.m_iDaemonAnimFrame] );

      // Draw the dragon
      ciGraphic = ::rumGetGraphic( U4_Title_Dragon_GraphicID );
      ciGraphic.DrawAnimation( rumPoint( iScreenWidth - ciGraphic.GetFrameWidth(), g_ciUI.m_iTitleAnimationOffset ),
                               0, g_ciUI.s_iDragonFrameIndicesArray[g_ciUI.m_iDragonAnimFrame] );

      local iVerticalOffset = ciGraphic.GetFrameHeight();
      iVerticalOffset += ( ::rumGetScreenHeight() - iVerticalOffset ) / 2;

      if( m_ciSettingsTable.bNeedsRenderInit )
      {
        m_ciLabelControl.SetPos( rumPoint( 0, iVerticalOffset - g_ciUI.s_iBorderPixelWidth ) );

        if( !( ( CharGen.Gender == m_eStage ) ||
               ( CharGen.CreateMode == m_eStage ) ||
               ( CharGen.SelectChar == m_eStage ) ) )
        {
          local iHorizOffset = iScreenCenter - ( m_ciInputControl.GetWidth() / 2 );
          m_ciInputControl.SetPos( rumPoint( iHorizOffset, iVerticalOffset ) );

          iVerticalOffset += g_ciUI.s_iDefaultLabelHeight * 2;
          if( !m_ciStatusControl.IsEmpty() )
          {
            iHorizOffset = iScreenCenter - ( m_ciStatusControl.GetWidth() / 2 );
            m_ciStatusControl.SetPos( rumPoint( iHorizOffset, iVerticalOffset ) );
          }
        }
        else
        {
          local iHorizOffset = iScreenCenter - ( m_ciListControl.GetWidth() / 2 );
          m_ciListControl.SetPos( rumPoint( iHorizOffset, iVerticalOffset ) );
        }

        m_ciSettingsTable.bNeedsRenderInit = false;
      }
    }
    // Rendering during introduction story
    else if( CharGen.Story == m_eStage )
    {
      // Draw the story frames
      local ciGraphic = ::rumGetGraphic( U4_Intro_GraphicID );
      ciGraphic.DrawAnimation( rumPoint(), 0, m_iAnimFrame );

      if( m_ciSettingsTable.bNeedsRenderInit )
      {
        local iX = ( ::rumGetScreenWidth() / 2 ) - ( m_ciTextControl.GetWidth() / 2 );
        local iY = ciGraphic.GetFrameHeight() + ( ::rumGetScreenHeight() - ciGraphic.GetFrameHeight() ) / 2 -
                   ( m_ciTextControl.GetHeight() / 2 );
        m_ciTextControl.SetPos( rumPoint( iX, iY ) );

        m_ciSettingsTable.bNeedsRenderInit = false;
      }
    }
    // Rendering during gypsy questions and final results
    else if( ( CharGen.Gypsy == m_eStage ) || ( CharGen.TestDone == m_eStage ) )
    {
      // Draw the background
      local ciGraphic = ::rumGetGraphic( U4_Intro_GraphicID );
      ciGraphic.DrawAnimation( rumPoint(), 0, m_iAnimFrame );

      RenderAbacus();
      RenderCards();

      if( m_ciSettingsTable.bNeedsRenderInit )
      {
        local iX = ( ::rumGetScreenWidth() / 2 ) - ( m_ciTextControl.GetWidth() / 2 );
        local iY = ciGraphic.GetFrameHeight() + ( ::rumGetScreenHeight() - ciGraphic.GetFrameHeight() ) / 2 -
                   ( m_ciTextControl.GetHeight() / 2 );
        m_ciTextControl.SetPos( rumPoint( iX, iY ) );

        m_ciSettingsTable.bNeedsRenderInit = false;
      }
    }
  }


  function RenderAbacus()
  {
    local ciGraphic = ::rumGetGraphic( U4_Intro_Beads_GraphicID );
    local iBeadWidth = ciGraphic.GetFrameWidth();
    local iBeadHeight = ciGraphic.GetFrameHeight();

    // Determine bead offsets
    local iVerticalOffset = iBeadHeight * 2;
    local iHorizOffset = ( ::rumGetScreenWidth() / 2 ) - ( 4 * iBeadWidth );

    // Display beads
    for( local i = 0; i < m_iQuestionNum; ++i )
    {
      local ciPos = rumPoint( iBeadWidth * m_eWhiteBeadArray[i] + iHorizOffset, iVerticalOffset );
      ciGraphic.DrawAnimation( ciPos, 0, 0 );

      ciPos = rumPoint( iBeadWidth * m_eBlackBeadArray[i] + iHorizOffset, iVerticalOffset );
      ciGraphic.DrawAnimation( ciPos, 0, 1 );

      iVerticalOffset += iBeadHeight + 6;
    }
  }


  function RenderCards()
  {
    // Center the card on the left side of the screen
    local ciGraphic = ::rumGetGraphic( U4_Intro_GraphicID );

    // Determine the first card's position
    local iVerticalOffset = ciGraphic.GetFrameHeight() / 2;
    local iHorizOffset = ciGraphic.GetFrameWidth() / 3;

    ciGraphic = ::rumGetGraphic( U4_Intro_Cards_GraphicID );

    iVerticalOffset -= ciGraphic.GetFrameHeight() / 2;
    iHorizOffset -= ciGraphic.GetFrameWidth();

    ciGraphic.DrawAnimation( rumPoint( iHorizOffset, iVerticalOffset ), 0, m_eCardVirtueLeft );

    // Determine the second card's position
    iHorizOffset = ::rumGetScreenWidth() - ( ciGraphic.GetFrameWidth() + iHorizOffset );

    ciGraphic.DrawAnimation( rumPoint( iHorizOffset, iVerticalOffset ), 0, m_eCardVirtueRight );
  }


  function RewindStory()
  {
    --m_iStoryIndex;

    if( m_iStoryIndex < StoryIndex.JourneyBegin )
    {
      InitStage( CharGen.CreateMode );
    }
    else
    {
      local strDesc = format( "chargen_story%02d_client_StringID", m_iStoryIndex + 1 );
      m_ciTextControl.Clear();
      m_ciTextControl.PushText( ::rumGetStringByName( strDesc ) );

      m_iAnimFrame = s_eIntroStoryGraphicIndicesArray[m_iStoryIndex];

      // Make sure the towns music is playing
      if( StoryIndex.JourneyEnd == m_iStoryIndex )
      {
        PlayMusic( U4_Towns_SoundID );
      }
    }
  }


  function Shutdown()
  {
    m_ciInputControl = null;
    m_ciLabelControl = null;
    m_ciListControl = null;
    m_ciStatusControl = null;
    m_ciTextControl = null;
  }


  function StoryAccepted()
  {
    if( CharGen.Gypsy == m_eStage )
    {
      if( !m_bQuestionAsked )
      {
        AskQuestion();
      }
    }
    else if( CharGen.TestDone == m_eStage )
    {
      TestDone();
    }
    else
    {
      AdvanceStory();
    }
  }


  function StoryRejected()
  {
    if( ( CharGen.Gypsy == m_eStage ) || ( CharGen.TestDone == m_eStage ) )
    {
      // Start the entire test over
      m_iStoryIndex = StoryIndex.VirtueTest - 1;
      InitStage( CharGen.Story );
    }
    else
    {
      RewindStory();
    }
  }


  function TestDone()
  {
    ++m_iStoryIndex;

    if( StoryIndex.Complete == m_iStoryIndex )
    {
      g_ciCUO.m_bWaitingForResponse = true;

      // Send init results to server - the sixth selected white bead determines the player's class
      local ciBroadcast = ::rumCreate( Player_Initialize_BroadcastID, m_strCharacterName, m_eGender,
                                       m_eWhiteBeadArray );
      ::rumSendBroadcast( ciBroadcast );

      InitStage( CharGen.Create );
    }
    else
    {
      m_iAnimFrame = s_eIntroStoryGraphicIndicesArray[m_iStoryIndex];
      local strDesc = format( "chargen_story%02d_client_StringID", m_iStoryIndex + 1 );
      m_ciTextControl.Clear();
      m_ciTextControl.PushText( ::rumGetStringByName( strDesc ) );
    }
  }
}


g_ciCharGen <- null;


function InitCharGen()
{
  // Create the singleton object
  if( null == g_ciCharGen )
  {
    ::g_ciCharGen = CharGenSingleton();
  }

  g_ciCharGen.InitStage( CharGen.Name );

  g_ciCUO.m_bWaitingForResponse = false;
}


function OnPlayerCreationFailed( i_strReason, i_strPlayerName )
{
  g_ciCUO.m_bWaitingForResponse = false;
  g_ciCharGen.m_ciStatusControl.SetText( ::rumGetStringByName( i_strReason + "_shared_StringID" ) );
  g_ciCharGen.InitStage( CharGen.Name );
}


function OnPlayerCreationSuccess( i_strPlayerName )
{
  g_ciCUO.m_bWaitingForResponse = false;
  g_ciCharGen.InitStage( CharGen.Gender );
}


function RenderCharGen()
{
  if( g_ciCharGen != null )
  {
    g_ciCharGen.Render();
  }
}
