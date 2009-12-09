#pragma once
#include <vector>
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h>
#include "../../pyconsole/pyconsole.h"

class TextBuffer {
public:
    TextBuffer(unsigned int l, unsigned int t, unsigned int w, unsigned int h);
    void render();
    void push_line(const std::string & s);
private:
    unsigned int left;
    unsigned int top;
    unsigned int width;
    unsigned int height;
    std::vector<std::string> buffer;
    unsigned int oldest;
};


class BwPyConsole: public BWAPI::AIModule
{
public:
  BwPyConsole();
  virtual void onStart();
  virtual void onEnd(bool isWinner);
  virtual void onFrame();
  virtual bool onSendText(std::string text);
  virtual void onPlayerLeft(BWAPI::Player* player);
  virtual void onNukeDetect(BWAPI::Position target);
  virtual void onUnitCreate(BWAPI::Unit* unit);
  virtual void onUnitDestroy(BWAPI::Unit* unit);
  virtual void onUnitMorph(BWAPI::Unit* unit);
  virtual void onUnitShow(BWAPI::Unit* unit);
  virtual void onUnitHide(BWAPI::Unit* unit);
  virtual void onUnitRenegade(BWAPI::Unit* unit);
private:
    TextBuffer console_buffer;
    pyconsole::PyConsole * console;
};
