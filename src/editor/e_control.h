#pragma once

class rumEditorControl
{
public:

  static void ScriptBind();
};


class rumEditorScrollBuffer : public rumEditorControl
{};


class rumEditorLabel : public rumEditorControl
{};


class rumEditorListView : public rumEditorScrollBuffer
{};


class rumEditorRegion : public rumEditorControl
{};


class rumEditorSlider : public rumEditorLabel
{};


class rumEditorTextBox : public rumEditorControl
{};


class rumEditorTextView : public rumEditorScrollBuffer
{};
