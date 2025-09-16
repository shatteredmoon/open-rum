#pragma once

#include <QDialog>
#include <QSet>

#include <u_enum.h>

namespace Ui
{
  class ComboPicker;
}


class ComboPicker : public QDialog
{
  Q_OBJECT

public:

  using ComboPickerVector = QVector<std::pair<QString, int32_t>>;

  explicit ComboPicker( const QString& i_strLabel,
                        const ComboPickerVector& i_rcEntryVector,
                        QWidget* i_pcParent = 0 );
  ~ComboPicker();

signals:

  void ComboPickerItemSelected( int32_t );

private slots:

  void on_buttonBox_accepted();

private:

  Ui::ComboPicker* m_pcUI;
};
