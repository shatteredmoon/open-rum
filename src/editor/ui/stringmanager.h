#ifndef STRINGMANAGER_H
#define STRINGMANAGER_H

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QSet>
#include <QTimer>

#include <platform.h>

namespace Ui
{
  class StringManager;
}

class QItemSelection;
class QNetworkReply;
class QTableWidgetItem;

/*struct MSTranslateInfo
{
  // Information sent
  QString m_strClientID;
  QString m_strClientSecret;

  // Information received
  QString m_strLastTranslatedText;
  QString m_strTokenType;
  QString m_strAccessToken;
  QString m_strScope;
  QTimer  m_tExpireTimer;

  // Translation request
  QString m_strTranslateText;
  QString m_strFromLanguage;
  QString m_strToLanguage;
};*/


class StringManager : public QMainWindow
{
  Q_OBJECT

  typedef QMap<QString, QString> TranslationMap;

public:

  explicit StringManager( QWidget* i_pcParent = 0 );
  ~StringManager();

  bool IsDirty() const
  {
    return m_bDirty;
  }

  QString GetLanguageCode( int32_t i_iRow ) const;
  QString GetLanguageName( int32_t i_iRow ) const;

  rumStringTableID GetSelectedStringTableID() const;
  rumLanguageID GetSelectedLanguageID() const;

  QString GetTableName( int32_t i_iRow ) const;
  QString GetToken( int32_t i_iRow ) const;
  rumTokenID GetTokenID( int32_t i_iRow ) const;

  /*const MSTranslateInfo& GetMSTranslateInfo() const
  {
    return m_cMSTranslateInfo;
  }

  // Sets translate info for actual request, calls RequestMSTranslateText()
  void RequestMSTranslateText( const QString& i_strText,
                               const QString& i_strFrom,
                               const QString& i_strTo );*/

private:

  void SaveTable( rumStringTableID i_eStringTableID );

signals:

  void closed();
  //void MSTranslateTextDone();
  //void MSTranslateTokenUpdated();

private slots:

  void on_actionNew_Language_triggered();
  void on_actionRemove_Language_triggered();

  void on_actionNew_String_Table_triggered();
  void on_actionRemove_String_Table_triggered();

  void on_actionNew_String_triggered();
  void on_actionRemove_String_triggered();

  void on_actionSave_triggered();

  void on_actionTranslate_All_triggered();

  void on_tableWidget_cellDoubleClicked( int32_t i_iRow, int32_t i_iCol );
  void on_tableWidget_customContextMenuRequested( const QPoint& i_rcPos );

  void on_tableWidget_2_cellDoubleClicked( int32_t i_iRow, int32_t i_iCol );
  void on_tableWidget_2_customContextMenuRequested( const QPoint& i_rcPos );

  void on_tableWidget_3_cellDoubleClicked( int32_t i_iRow, int32_t i_iCol );
  void on_tableWidget_3_customContextMenuRequested( const QPoint& i_rcPos );

  // custom
  void LanguageLineEditFinished();
  void TableLineEditFinished();
  void TokenLineEditFinished();

  void onFilterChanged();

  /*void onMSTranslateLanguagesResponse(QNetworkReply* i_pcReply);
  void onMSTranslateTextResponse( QNetworkReply* i_pcReply );
  void onMSTranslateTokenExpired();
  void onMSTranslateTokenResponse( QNetworkReply* );
  void onMSTranslationsAdded( const TranslationMap& i_rcTranslationMap );

  void onResumeMSTranslateLanguages();
  void onResumeMSTranslateText();*/

  void onLanguageItemChanged( QTableWidgetItem* i_pcItem );
  void OnStringTableComboChanged( const QString& i_strText );
  void OnStringTableItemStartEdit( int32_t i_iRow, int32_t i_iCol );
  void onStringTableItemChanged( QTableWidgetItem* i_pcItem );
  void onTranslationTableItemChanged( QTableWidgetItem* i_pcItem );

  void selectionChanged_Language( const QItemSelection& i_rcSelected, const QItemSelection& i_rcDeselected );
  void selectionChanged_String( const QItemSelection& i_rcSelected, const QItemSelection& i_rcDeselected );
  void selectionChanged_StringTable( const QItemSelection& i_rcSelected, const QItemSelection& i_rcDeselected );

private:

  void AddLanguage( const QString& i_strName, const QString& i_strCode, rumLanguageID i_iLanguageID ) const;

  // Called when the string content table is changed or refreshed
  void AddString( int32_t i_iRow,
                  rumTokenID i_iTokenID,
                  const QString& i_strToken,
                  const QString& i_strTranslation ) const;

  void AddStringTable( const rumStringTable& i_rcStringTable ) const;

  void closeEvent( QCloseEvent* i_pcEvent ) override;

  void InitLanguages();
  void InitStringTables();

  bool IsLanguageCodeUnique( const QString& i_strCode ) const;
  bool IsLanguageUnique( const QString& i_strName ) const;
  bool IsTableNameUnique( const QString& i_strName ) const;
  bool IsTokenUnique( const QString& i_strToken ) const;

  // Sends a request to Microsoft for supported languages and matching language codes
  //void RequestMSTranslateLanguages();

  // Sends a request to Microsoft for a new translator API token
  //void RequestMSTranslateToken();

  // Sends a request to Microsft Translator to translate the provided
  // string from the source language to the destination language
  //void RequestMSTranslateText();

  // Called when a new string table is selected
  void RefreshTranslationTable();

  void SetDirty( bool i_bDirty );

  void UpdateFilter();

  QLabel m_cFilterLabel;
  QLineEdit m_cFilterEdit;
  QAction* m_pcFilterAction;

  //MSTranslateInfo m_cMSTranslateInfo;

  Ui::StringManager* m_pcUI;

  bool m_bDirty{ false };
};

#endif // STRINGMANAGER_H
