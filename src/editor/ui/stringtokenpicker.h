#pragma once

#include <QDialog>
#include <QSet>

namespace Ui
{
  class StringTokenPicker;
}


class StringTokenPicker : public QDialog
{
  Q_OBJECT

public:

  explicit StringTokenPicker( rumTokenID i_eTokenID, QWidget* i_pcParent = 0 );
  ~StringTokenPicker();

signals:

  void NewStringTokenSelected( rumTokenID i_eTokenID );

private slots:

  void ItemComboChanged( int32_t i_iIndex );

  void on_buttonBox_accepted();
  void on_lineEdit_Filter_textEdited( const QString& i_strText );
  void on_pushButton_Clear_clicked();

private:

  void RefreshStringTable( rumStringTableID uiStringTableID, rumTokenID i_eTokenID );
  void UpdateFilter();

  Ui::StringTokenPicker* m_pcUI;
};
