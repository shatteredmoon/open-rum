#ifndef SHAREDGLWIDGET_H
#define SHAREDGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFUnctions>

// This widget exists so that a single OpenGL context can be shared across all QOpenGLWidgets. Textures loaded during
// the initialization of this widget can be reused by anything that uses this shared widget.

class SharedGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

public:
  SharedGLWidget( QWidget* i_pcParent = 0 )
    : QOpenGLWidget( i_pcParent )
  {}

  void initializeGL() override
  {
    initializeOpenGLFunctions();
  }
};

#endif // SHAREDGLWIDGET_H
