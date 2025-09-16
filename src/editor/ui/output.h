#ifndef OUTPUT_H
#define OUTPUT_H

#include <QDialog>

class FindDialog;

namespace Ui
{
  class Output;
}

class OutputDialog : public QDialog
{
  Q_OBJECT

public:

  OutputDialog( QWidget* i_pcParent = 0 );
  ~OutputDialog();

  void Append( const QString& i_strText );

private slots:

  void on_actionClear_triggered();
  void on_actionCopy_To_Clipboard_triggered();
  void on_actionFind_triggered();

private:

  void Init();

  FindDialog* m_pcFindDialog;

  Ui::Output* m_pcUI;
};

#endif // OUTPUT_H
