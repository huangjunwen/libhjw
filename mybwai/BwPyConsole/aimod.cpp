#include "aimod.h"
#include <sstream>

using namespace std;
using namespace BWAPI;


TextBuffer::TextBuffer(unsigned int l, unsigned int t, 
                       unsigned int w, unsigned int h): left(l), top(t), 
    width(w), height(h), oldest(0), buffer(h)
{
}

void TextBuffer::render()
{
    unsigned int i = oldest;
    unsigned int c = height;
    unsigned int t = top;
    while (c--)
    {
        Broodwar->drawTextScreen(left, t, buffer[i].c_str());
        i = (i + 1) % height;
        t += 10;
    }
}

void TextBuffer::push_line(const std::string & s) 
{
    unsigned int pos = 0;
    while (pos < s.size()) {
        buffer[oldest] = s.substr(pos, width);
        oldest = (oldest + 1) % height;
        pos += width;
    }
}


BwPyConsole::BwPyConsole(): console_buffer(5, 5, 50, 29)
{
}

void BwPyConsole::onStart()
{
    Broodwar->enableFlag(Flag::UserInput);
    console = pyconsole::getConsole();
    if (console->alive())
        Broodwar->sendText("Pyconsole loaded");
    else
        Broodwar->sendText("Failed to load pyconsole");
}

void BwPyConsole::onEnd(bool isWinner)
{
}

void BwPyConsole::onFrame()
{
    console_buffer.render();
}

void BwPyConsole::onUnitCreate(BWAPI::Unit* unit)
{
}

void BwPyConsole::onUnitDestroy(BWAPI::Unit* unit)
{
}

void BwPyConsole::onUnitMorph(BWAPI::Unit* unit)
{
}

void BwPyConsole::onUnitShow(BWAPI::Unit* unit)
{
}

void BwPyConsole::onUnitHide(BWAPI::Unit* unit)
{
}

void BwPyConsole::onUnitRenegade(BWAPI::Unit* unit)
{
}

void BwPyConsole::onPlayerLeft(BWAPI::Player* player)
{
}

void BwPyConsole::onNukeDetect(BWAPI::Position target)
{
}

bool BwPyConsole::onSendText(std::string in)
{
    if (!console->alive())
        return true;

    string out, err;
    console_buffer.push_line(">>> " + in);
    // run it
    console->loop_once(in, out, err);

    // put to console buffer
    string tmp;
    stringstream ssout(out);
    while (getline(ssout, tmp, '\n'))
        console_buffer.push_line(tmp);
    stringstream sserr(err);
    while (getline(sserr, tmp, '\n'))
        console_buffer.push_line(tmp);

    return false;
}
