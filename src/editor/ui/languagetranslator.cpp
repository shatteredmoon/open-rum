#include <languagetranslator.h>
#include <ui_languagetranslator.h>

#include <mainwindow.h>
#include <stringmanager.h>

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>


LanguageTranslator::LanguageTranslator( QWidget* i_pcParent )
  : QDialog( i_pcParent )
  , m_pcUI( new Ui::LanguageTranslator )
  , m_iTranslateCount( 0 )
{
  m_pcUI->setupUi( this );

  /*StringManager& rcManager{GetStringManager()};

  // Fill out the drop-down with any currently unselected languages
  QString strLangID{ rcManager.GetSelectedLanguageID() };
  QString strLangName{ rcManager.GetLanguageName() };

  QMenu* pcMenu{ new QMenu( m_pcUI->toolButton ) };

  // Access the shared database for language info
  QSqlDatabase& cDatabase{ MainWindow::GetSqlDatabase( "STRINGS_SHARED" ) };
  Q_ASSERT( cDatabase.isValid() );
  QSqlQuery cQuery( cDatabase );

  if( cQuery.exec( "SELECT name,code,lang_id FROM language WHERE lang_id!=" + strLangID ) )
  {
    while( cQuery.next() )
    {
      const QString strLanguage{ cQuery.value( 0 ).toString() };
      const QString strCode{ cQuery.value( 1 ).toString() };
      const QString strLangID{ cQuery.value( 2 ).toString() };

      // Add a new action for the translation button
      QAction* pcAction{ new QAction( strLanguage, pcMenu ) };
      pcAction->setData( strCode + "," + strLangID );
      pcMenu->addAction( pcAction );

      // Add this language to the language map
      m_cLanguageMap.insert( strLangID, strLanguage );
    }
  }

  // Attach the menu to the tool button
  m_pcUI->toolButton->setMenu( pcMenu );
  m_pcUI->toolButton->setPopupMode( QToolButton::InstantPopup );

  connect( m_pcUI->toolButton, SIGNAL( triggered( QAction* ) ),
           this, SLOT( onTranslate( QAction* ) ) );

  // Disable the preview, and hide the OK button and status bar
  m_pcUI->buttonBox->button( QDialogButtonBox::Ok )->setText( "Save" );
  m_pcUI->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  m_pcUI->textEdit->setEnabled( false );
  m_pcUI->progressBar->setHidden( true );
  m_pcUI->progressBar->setMinimum( 0 );

  // For handling a finished translation and moving on to the next
  connect( &rcManager, SIGNAL( MSTranslateTextDone() ),
           this, SLOT( onMSTranslateTextDone() ) );*/
}


LanguageTranslator::~LanguageTranslator()
{
  delete m_pcUI;
}


void LanguageTranslator::DoTranslation()
{
  /*if( m_TranslationSourceMap.empty() )
  {
    DoneTranslating();
    return;
  }

  TranslationMap::iterator iter( m_TranslationSourceMap.begin() );
  Q_ASSERT( iter != m_TranslationSourceMap.end() );

  // Perform the actual translation
  StringManager& rcManager{ GetStringManager() };
  const QString strDestCode{ rcManager.GetSelectedLanguageCode() };
  rcManager.RequestMSTranslateText( iter.value(),
                                   m_strSrcCode, strDestCode );*/
}


void LanguageTranslator::DoneTranslating()
{
  // Enable the OK/Save button
  m_pcUI->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );

  m_pcUI->toolButton->setEnabled( true );

  m_pcUI->progressBar->setValue( 0 );
  m_pcUI->progressBar->setHidden( true );

  m_pcUI->checkBox->setEnabled( true );
}


StringManager& LanguageTranslator::GetStringManager() const
{
  return *( qobject_cast<StringManager*>( parentWidget() ) );
}


// slot
void LanguageTranslator::on_buttonBox_accepted()
{
  // Save the translations to the database
  /*StringManager& rcManager{GetStringManager()};

  const QString& strDatabase{ rcManager.GetSelectedDatabaseName() };

  // Fetch the currently selected string database
  QSqlDatabase& cDatabase{ MainWindow::GetSqlDatabase( strDatabase ) };
  Q_ASSERT( cDatabase.isValid() );

  QSqlQuery cQuery( cDatabase );
  cQuery.exec( "BEGIN TRANSACTION" );

  QMapIterator<QString, QString> iter( m_TranslationDestMap );
  while( iter.hasNext() )
  {
    iter.next();
    cQuery.prepare( "INSERT INTO localization (lang_id,string_id,translation) VALUES (:lang_id,:string_id,:translation)" );
    cQuery.bindValue( ":lang_id", rcManager.GetSelectedLanguageID() );
    cQuery.bindValue( ":string_id", iter.key() );
    cQuery.bindValue( ":translation", iter.value() );
    cQuery.exec();
  }

  // Finish the transaction, failure here should mean that nothing is
  // committed to the database
  if( !cQuery.exec( "COMMIT" ) )
  {
    cQuery.exec( "ROLLBACK" );
    QMessageBox::critical( this, tr( "Error" ), "Error writing to database " + strDatabase );
    return;
  }

  emit MSTranslationsAdded( m_TranslationDestMap );

  m_cTranslationSourceMap.clear();
  m_cTranslationDestMap.clear();*/
}


// slot
void LanguageTranslator::onMSTranslateTextDone()
{
  /*StringManager& rcManager{GetStringManager()};
  const QString& strTranslated{ rcManager.GetMSTranslateInfo().m_strLastTranslatedText };

  Q_ASSERT( !m_TranslationSourceMap.empty() );

  // Locate the item we just translated
  TranslationMap::iterator iter( m_TranslationSourceMap.begin() );
  Q_ASSERT( iter != m_TranslationSourceMap.end() );

  // Print the translation results and add the new translation to the
  // destination map
  m_pcUI->textEdit->append( iter.value() + " -> " + strTranslated );
  m_cTranslationDestMap.insert( iter.key(), strTranslated );

  // Remove this translation from the source map
  m_cTranslationSourceMap.erase( iter );

  m_pcUI->progressBar->setValue( m_pcUI->progressBar->value() + 1 );

  // Continue translating
  DoTranslation();*/
}


// slot
void LanguageTranslator::onTranslate( QAction* i_pcAction )
{
  /*if( !i_pcAction )
  {
    return;
  }

  StringManager& rcManager{ GetStringManager() };

  // Get our destination language info
  const QString strDestID{ rcManager.GetSelectedLanguageID() };

  QStringList strLangInfoList{ i_pcAction->data().toString().split( "," ) };
  m_strSrcCode = strLangInfoList[0];
  m_strSrcID = strLangInfoList[1];

  QString strQuestion{ "Are you sure you want to translate entries from language " };
  strQuestion += m_cLanguageMap.value( m_strSrcID, "Unknown" );
  strQuestion += " to " + rcManager.GetSelectedLanguageName();
  strQuestion += "?\nWARNING: This will forget any currently pending translations!";

  // Verify
  if( QMessageBox::No ==
      QMessageBox::question( this, "Verify Translate", strQuestion,
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::No ) )
  {
    // Translation canceled
    return;
  }

  m_cTranslationSourceMap.clear();
  m_cTranslationDestMap.clear();

  // Fetch the currently selected string database
  QSqlDatabase& rcDatabase{ MainWindow::GetSqlDatabase( rcManager.GetSelectedDatabaseName() ) };
  Q_ASSERT( rcDatabase.isValid() );

  QSqlQuery sqlQuery( rcDatabase );

  // Clear empty database entries
  if( m_pcUI->checkBox->checkState() == Qt::Checked )
  {
    sqlQuery.prepare( "DELETE from localization WHERE lang_id=:lang_id AND (translation='' OR translation IS NULL)" );
    sqlQuery.bindValue( ":lang_id", strDestID );
    sqlQuery.exec();
  }

  sqlQuery.prepare( "SELECT string_id,translation FROM localization WHERE lang_id=:src_lang_id AND string_id NOT IN (SELECT string_id FROM localization WHERE lang_id=:dest_lang_id)" );
  sqlQuery.bindValue( ":src_lang_id", m_strSrcID );
  sqlQuery.bindValue( ":dest_lang_id", strDestID );

  if( sqlQuery.exec() )
  {
    while( sqlQuery.next() )
    {
      const QString& strTrans{ sqlQuery.value( 1 ).toString() };
      if( !( strTrans.isEmpty() || strTrans.isNull() ) )
      {
        m_cTranslationSourceMap.insert( sqlQuery.value( 0 ).toString(), strTrans );
      }
    }
  }

  if( m_cTranslationSourceMap.empty() )
  {
    QMessageBox::critical( this, "Translation Failed",
                           "Nothing found to translate" );
  }
  else
  {
    m_iTranslateCount = 0;

    m_pcUI->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
    m_pcUI->checkBox->setEnabled( false );
    m_pcUI->toolButton->setEnabled( false );
    m_pcUI->textEdit->clear();
    m_pcUI->textEdit->setEnabled( true );
    m_pcUI->progressBar->setValue( 0 );
    m_pcUI->progressBar->setHidden( false );
    m_pcUI->progressBar->setMaximum( m_cTranslationSourceMap.size() );

    DoTranslation();
  }*/
}
