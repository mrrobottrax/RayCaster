#pragma once

enum KeyCode;

KeyCode W_TranslateToKeyCode(WPARAM code);

void W_HideCursor();
void W_ShowCursor();
void W_RawInputBuffer();