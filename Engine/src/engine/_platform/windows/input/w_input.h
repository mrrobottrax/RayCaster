#pragma once

enum KeyCode;

KeyCode W_TranslateToKeyCode(WPARAM code);

void W_StartRawinput();
void W_EndRawinput();
void W_RawInputBuffer();