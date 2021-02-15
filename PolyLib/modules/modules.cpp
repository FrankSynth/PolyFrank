#include "modules.hpp"

// NameLists for switches
const std::vector<std::string> nlOnOff{"OFF", "ON"};
const std::vector<std::string> nlVCFDest{"STEINER", "LADDER", "BOTH", "OFF"};
const std::vector<std::string> nlSteinerModes{"LP", "HP", "BP", "AP"};
const std::vector<std::string> nlLadderSlopes{"6", "12", "18", "24"};
const std::vector<std::string> nlADSRShapes{"Lin", "Log", "AntiLog"};
const std::vector<std::string> nlClockSteps = {"1/64T", "1/32T", "1/32", "1/16T", "1/16", "1/8T", "1/16.", "1/8",
                                               "1/4T",  "1/8.",  "1/4",  "1/2T",  "1/4.", "1/2",  "1/1T",  "1/2.",
                                               "1/1",   "2/1T",  "1/1.", "2/1",   "4/1T", "2/1.", "4/1"};
const std::vector<std::string> nlSubOctaves = {"/2", "/4"};