#include "modules.hpp"

// NameLists for switches
const std::vector<const char *> nlOnOff{"OFF", "ON"};
const std::vector<const char *> nlRange{"SHORT", "LONG"};

const std::vector<const char *> nlVCFDest{"STEINER", "LADDER", "BOTH", "OFF"};
const std::vector<const char *> nlSteinerModes{"LP", "HP", "BP", "AP"};
const std::vector<const char *> nlLadderSlopes{"6", "12", "18", "24"};
const std::vector<const char *> nlADSRShapes{"Lin", "Log", "AntiLog"};
const std::vector<const char *> nlClockSteps = {"1/64T", "1/32T", "1/32", "1/16T", "1/16", "1/8T", "1/16.", "1/8",
                                                "1/4T",  "1/8.",  "1/4",  "1/2T",  "1/4.", "1/2",  "1/1T",  "1/2.",
                                                "1/1",   "2/1T",  "1/1.", "2/1",   "4/1T", "2/1.", "4/1"};
const std::vector<const char *> nlClockStepsInv = {"4/1",   "2/1.", "4/1T", "2/1",   "1/1.", "2/1T",  "1/1",  "1/2.",
                                                   "1/1T",  "1/2",  "1/4.", "1/2T",  "1/4",  "1/8.",  "1/4T", "1/8",
                                                   "1/16.", "1/8T", "1/16", "1/16T", "1/32", "1/32T", "1/64T"};
const std::vector<const char *> nlSubOctaves = {"/2", "/4"};
const std::vector<const char *> nlDivSteps = {"/1", "/2", "/4", "/8", "/16"};

const std::vector<const char *> nlWavetableSets = {"SET0", "SET1", "SET2", "SET3", "SET4", "SET5"};
