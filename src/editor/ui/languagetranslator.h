#ifndef LANGUAGETRANSLATOR_H
#define LANGUAGETRANSLATOR_H

#include <QDialog>
#include <QMap>

class StringManager;

namespace Ui
{
  class LanguageTranslator;
}

class LanguageTranslator : public QDialog
{
  Q_OBJECT

    typedef QMap<QString, QString> TranslationMap;

public:

  explicit LanguageTranslator( QWidget* i_pcParent = 0 );
  ~LanguageTranslator();

  StringManager& GetStringManager() const;

signals:

  void MSTranslationsAdded( const TranslationMap& i_rcTranslationMap );

private slots:

  void on_buttonBox_accepted();
  void onMSTranslateTextDone();
  void onTranslate( QAction* i_pcAction );

private:

  void DoTranslation();
  void DoneTranslating();

  Ui::LanguageTranslator* m_pcUI;

  QString m_strSrcCode;
  QString m_strSrcID;

  int32_t m_iTranslateCount;

  TranslationMap m_cTranslationSourceMap;
  TranslationMap m_cTranslationDestMap;
  TranslationMap m_cLanguageMap;
};

#endif // LANGUAGETRANSLATOR_H
