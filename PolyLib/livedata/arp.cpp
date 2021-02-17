#ifdef POLYCONTROL
#include "arp.hpp"

// functions
void Arpeggiator::keyPressed(Key &key) {
    allKeysReleased = 1;
    if (!inputKeys.empty()) {                                                                // inputKey list empty?
        for (std::list<Key>::iterator it = inputKeys.begin(); it != inputKeys.end(); it++) { // all Keys released ?
            if (!it->released) {
                allKeysReleased = 0;
            }
        }

        if (allKeysReleased) { // clear list

            inputKeys.clear();
            allKeysReleased = 0;
        }
        else { // check already existing Keys

            for (std::list<Key>::iterator it = inputKeys.begin(); it != inputKeys.end(); it++) { // Key already exist?
                if (it->note == key.note) {

                    return;
                }
            }
        }
    }

    inputKeys.push_back(key);
    orderKeys();
}
void Arpeggiator::keyReleased(Key &key) {

    if (inputKeys.empty()) {
        return;
    }
    for (std::list<Key>::iterator it = inputKeys.begin(); it != inputKeys.end(); it++) {
        if (it->note == key.note) {
            if (arpLatch.value) { // latch on?
                it->released = 1; // mark key as released
            }
            else {

                inputKeys.erase(it); // delete key
            }
            return;
        }
    }
    orderKeys();
}

void Arpeggiator::pressKey(Key key) {

    lifetime(key); // calculate lifespan

    voiceHandler->playNote(key); // play new note
    pressedKeys.push_back(key);  // add to pressed List
}

void Arpeggiator::lifetime(Key &key) {

    // polyrythm next step precalculation

    uint32_t ticksToNextStep;

    if (arpPolyrythm.value == 0) {
        ticksToNextStep = clockTicksPerStep[arpStepsA.value];
    }
    else {
        uint32_t moduloA = clock.counter % clockTicksPerStep[arpStepsA.value];
        uint32_t moduloB = clock.counter % clockTicksPerStep[arpStepsB.value];

        if ((clockTicksPerStep[arpStepsA.value] - moduloA) > (clockTicksPerStep[arpStepsB.value] - moduloB)) {
            ticksToNextStep = clockTicksPerStep[arpStepsB.value] - moduloB;
        }
        else {
            ticksToNextStep = clockTicksPerStep[arpStepsA.value] - moduloA;
        }
    }

    uint32_t lifespan = (60000000 / (clock.bpm * 24)) * ticksToNextStep * arpPresseKeysParallel.value; // in micros

    key.born = micros();
    key.lifespan = lifespan;
}

void Arpeggiator::serviceRoutine() {
    checkLatch();
    release();
    ratched();
}
void Arpeggiator::ratched() {}
void Arpeggiator::release() {

    for (std::list<Key>::iterator it = pressedKeys.begin(); it != pressedKeys.end();) {
        // find keys to be released , check sustain-> half lifespan
        if (((micros() - it->born) > it->lifespan && arpSustain) ||
            (((micros() - it->born) > (it->lifespan / 2)) && !arpSustain)) {
            voiceHandler->freeNote(*it); // free Note
            it = pressedKeys.erase(it);  // delete key
        }
        else {
            it++;
        }
    }
}

void Arpeggiator::setSustain(uint8_t sustain) {
    if (sustain >= 64) {
        arpSustain = 1;
    }
    if (sustain < 64) {
        arpSustain = 0;
    }
}

void Arpeggiator::checkLatch() {
    if (arpLatch.value == 0) {
        for (std::list<Key>::iterator it = inputKeys.begin(); it != inputKeys.end();) {
            if (it->released) {
                it = inputKeys.erase(it); // delete key
            }
            else {
                it++;
            }
        }
        orderKeys();
    }
}

void Arpeggiator::orderKeys() {
    orderedKeys.clear();

    for (std::list<Key>::iterator it = inputKeys.begin(); it != inputKeys.end(); it++) {
        orderedKeys.push_back(*it);
    }
    std::sort(orderedKeys.begin(), orderedKeys.end(), compareByNote);
}
void Arpeggiator::nextStep() {

    if (!arpEnable.value || orderedKeys.empty()) {
        return;
    }

    switch (arpMode.value) {
        case ARP_DN: // down
            mode_down();
            break;
        case ARP_DN2:
            mode_down2(); // down2
            break;
        case ARP_DN3: // down3
            mode_down3();
            break;
        case ARP_UP:   // up
        case ARP_ORDR: // order
            mode_ordr();
            break;
        case ARP_UP2: // arpup2
            mode_up2();
            break;
        case ARP_UP3: // arpup3
            mode_up3();
            break;
        case ARP_RND: // arprnd
            mode_rnd();
            break;
        case ARP_UD: // updown
            mode_updown();
            break;
        case ARP_URDR: // upRdownR
            mode_uprdownr();
            break;
        case ARP_DU: // downup
            mode_downup();
            break;
        case ARP_DRUR: // downRupR
            mode_downrupr();
            break;
        default:;
    }

    Key key;

    key = orderedKeys[stepArp];

    // lower octaves
    if (currentOctave < 0) {
        for (int x = currentOctave; x < 0; x++) {
            key.note = changeInt(key.note, -12, 0, NOTERANGE - 1, 0);
        }
    }
    // raise octaves
    else if (currentOctave > 0) {
        for (int x = currentOctave; x > 0; x--) {
            key.note = changeInt(key.note, 12, 0, NOTERANGE - 1, 0);
        }
    }

    pressKey(key);
}

void Arpeggiator::increaseArpOct() {
    // change octave for next round
    // no octaving
    if (arpOctave.value == 0) {
        currentOctave = 0;
    }
    else {

        // eval new min and max
        int newOctMin;
        int newOctMax;

        if (arpOctave.value < 0) {
            newOctMin = arpOctave.value;
            newOctMax = 0;
        }
        else {
            newOctMin = 0;
            newOctMax = arpOctave.value;
        }
        if (restarted) {
            currentOctave = newOctMin;
        }
        else {

            if (arpMode.value == ARP_UD || arpMode.value == ARP_DU || arpMode.value == ARP_URDR ||
                arpMode.value == ARP_DRUR) {
                currentOctave = changeInt(currentOctave, 1, newOctMin, newOctMax, true);
                if (currentOctave == newOctMax)
                    octaveDirection = 0;
            }
            else {
                currentOctave = changeIntLoop(currentOctave, 1, newOctMin, newOctMax);
            }
        }
    }
}

void Arpeggiator::decreaseArpOct() {
    // no octaving
    if (arpOctave.value == 0) {
        currentOctave = 0;
    }
    else {
        // eval new min and max
        int newOctMin;
        int newOctMax;

        if (arpOctave.value < 0) {
            newOctMin = arpOctave.value;
            newOctMax = 0;
        }
        else {
            newOctMin = 0;
            newOctMax = arpOctave.value;
        }

        if (restarted) {
            currentOctave = newOctMax;
        }
        else {
            if (arpMode.value == ARP_UD || arpMode.value == ARP_DU || arpMode.value == ARP_URDR ||
                arpMode.value == ARP_DRUR) {
                currentOctave = changeInt(currentOctave, -1, newOctMin, newOctMax, true);
                if (currentOctave == newOctMin) {
                    octaveDirection = 1;
                }
            }
            else {
                currentOctave = changeIntLoop(currentOctave, -1, newOctMin, newOctMax);
            }
        }
    }
}

void Arpeggiator::mode_down() {

    if (stepArp == 0) {
        decreaseArpOct();
    }
    if (restarted) {
        decreaseArpOct();
        stepArp = orderedKeys.size() - 1;
        restarted = 0;
    }
    else {
        // if arp size increased by one, a note would be repeated, so instead, we increase further
        do {
            stepArp = changeIntLoop(stepArp, -1, 0, orderedKeys.size() - 1);
        } while (orderedKeys[stepArp].note == arpKey.note && orderedKeys.size() > 1);
    }
}

void Arpeggiator::mode_down2() {

    if (stepArp == 0 && direction == 0) {
        decreaseArpOct();
    }
    if (restarted) {
        decreaseArpOct();
        stepArp = orderedKeys.size() - 1;
        restarted = 0;
        direction = 0;
    }
    else {
        // if arp size increased by one, a note would be repeated, so instead, we increase further
        do {
            if (direction == 0) {
                int16_t nextArpStep = (int16_t)stepArp - 2;
                if (nextArpStep == -1) {
                    stepArp = 0;
                }
                else if (nextArpStep < -1) {
                    stepArp = orderedKeys.size() - 1;
                }
                else {
                    stepArp = nextArpStep;
                    if (!(nextArpStep == 0 && orderedKeys.size() % 2 == 0))
                        direction = 1;
                }
            }
            else {
                stepArp = changeInt(stepArp, 1, 0, orderedKeys.size() - 1, true);

                direction = 0;
            }
        } while (orderedKeys[stepArp].note == arpKey.note && orderedKeys.size() > 1);
    }
}

void Arpeggiator::mode_down3() {
    if (stepArp == 0) {
        decreaseArpOct();
    }
    if (restarted) {
        decreaseArpOct();
        stepArp = orderedKeys.size() - 1;
        restarted = 0;
        direction = 0;
    }
    else {
        // if arp size increased by one, a note would be repeated, so instead, we increase further
        if (direction < 2) {
            do {
                if (stepArp == 0) {
                    stepArp = orderedKeys.size() - 1;
                    direction = 0 - 1;
                }
                else {
                    stepArp = changeInt(stepArp, -1, 0, orderedKeys.size() - 1, true);
                }
            } while (orderedKeys[stepArp].note == arpKey.note && orderedKeys.size() > 1);
            direction++;
        }
        else {
            do {
                if (stepArp == 0) {
                    stepArp = orderedKeys.size() - 1;
                }
                else {
                    if (stepArp == orderedKeys.size() - 1) {
                        stepArp = 0;
                    }
                    else {
                        stepArp = changeInt(stepArp, 1, 0, orderedKeys.size() - 1, true);
                    }
                }
            } while (orderedKeys[stepArp].note == arpKey.note && orderedKeys.size() > 1);
            direction = 0;
        }
    }
}
void Arpeggiator::mode_ordr() {
    if (stepArp == orderedKeys.size() - 1) {
        increaseArpOct();
    }
    if (restarted) {
        increaseArpOct();
        stepArp = 0;
        restarted = 0;
    }
    else {
        // if arp size increased by one, a note would be repeated, so instead, we increase further
        do {
            stepArp = changeIntLoop(stepArp, 1, 0, orderedKeys.size() - 1);
        } while (orderedKeys[stepArp].note == arpKey.note && orderedKeys.size() > 1);
    }
}
void Arpeggiator::mode_up2() {
    if (stepArp == orderedKeys.size() - 1 && direction > 0) {
        increaseArpOct();
    }
    if (restarted) {
        increaseArpOct();
        stepArp = 0;
        restarted = 0;
        direction = 1;
    }
    else {
        // if arp size increased by one, a note would be repeated, so instead, we increase further
        do {
            if (direction > 0) {
                int16_t nextArpStep = (int16_t)stepArp + 2;
                if (nextArpStep == (int16_t)orderedKeys.size()) {
                    stepArp = orderedKeys.size() - 1;
                    if (orderedKeys[stepArp].note == arpKey.note)
                        stepArp = 0;
                }
                else if (nextArpStep > (int16_t)orderedKeys.size()) {
                    stepArp = 0;
                }
                else {
                    stepArp = nextArpStep;
                    if (!(nextArpStep == (int16_t)orderedKeys.size() - 1 && orderedKeys.size() % 2 == 0))
                        direction = 0;
                }
            }
            else {
                stepArp = changeInt(stepArp, -1, 0, orderedKeys.size() - 1, true);
                direction = 1;
            }
        } while (orderedKeys[stepArp].note == arpKey.note && orderedKeys.size() > 1);
    }
}
void Arpeggiator::mode_up3() {
    if (stepArp == orderedKeys.size() - 1) {
        increaseArpOct();
    }
    if (restarted) {
        increaseArpOct();
        stepArp = 0;
        restarted = 0;
        direction = 2;
    }
    else {
        // if arp size increased by one, a note would be repeated, so instead, we increase further
        if (direction > 0) {
            do {
                if (stepArp == orderedKeys.size() - 1) {
                    stepArp = 0;
                    direction = 2 + 1;
                }
                else {
                    stepArp = changeInt(stepArp, 1, 0, orderedKeys.size() - 1, true);
                }
            } while (orderedKeys[stepArp].note == arpKey.note && orderedKeys.size() > 1);
            direction--;
        }
        else {
            do {

                if (stepArp == orderedKeys.size() - 1) {
                    stepArp = 0;
                }
                else {
                    if (stepArp == 0)
                        stepArp = orderedKeys.size() - 1;
                    else
                        stepArp = changeInt(stepArp, -1, 0, orderedKeys.size() - 1, true);
                }
            } while (orderedKeys[stepArp].note == arpKey.note && orderedKeys.size() > 1);
            direction = 2;
        }
    }
}
void Arpeggiator::mode_rnd() {
    if (randomCounter == orderedKeys.size() - 1) {
        increaseArpOct();
    }
    if (restarted) {
        increaseArpOct();
        randomCounter = 0;
        restarted = 0;
    }
    else {
        randomCounter = changeIntLoop(randomCounter, 1, 0, orderedKeys.size() - 1);
    }
    stepArp = std::rand() * orderedKeys.size() / RAND_MAX;
}
void Arpeggiator::mode_updown() {
    if (restarted) {
        increaseArpOct();
        direction = 1;
        octaveDirection = 1;
        stepArp = 0;
        restarted = 0;
    }
    else {
        // if arp size increased by one, a note would be repeated, so instead, we increase further

        do {

            // going up
            if (direction > 0) {
                stepArp = changeInt(stepArp, 1, 0, orderedKeys.size() - 1);
                if (stepArp == orderedKeys.size() - 1) {
                    direction = 0;
                }
            }
            // going down
            else {
                stepArp = changeInt(stepArp, -1, 0, orderedKeys.size() - 1);
                if (stepArp == 0) {
                    direction = 1;
                    if (octaveDirection == 1) {
                        increaseArpOct();
                    }
                    else {
                        decreaseArpOct();
                    }
                }
            }
        } while (orderedKeys[stepArp].note == arpKey.note && orderedKeys.size() > 1);
    }
}
void Arpeggiator::mode_uprdownr() {
    if (restarted) {
        increaseArpOct();
        direction = 1;
        octaveDirection = 1;
        stepArp = 0;
        restarted = 0;
        stepRepeat = 1;
    }
    else {

        do {

            // going up
            if (direction > 0) {
                stepArp = changeInt(stepArp, 1, 0, orderedKeys.size() - 1);
                if (stepArp == orderedKeys.size() - 1) {
                    if (!stepRepeat) {
                        stepRepeat = 1;
                        direction = 0;
                    }
                    else {
                        stepRepeat = 0;
                    }
                }
            }
            // going down
            else {
                stepArp = changeInt(stepArp, -1, 0, orderedKeys.size() - 1);
                if (stepArp == 0) {
                    if (!stepRepeat) {
                        stepRepeat = 1;
                        direction = 1;
                        if (octaveDirection == 1) {
                            increaseArpOct();
                        }
                        else {
                            decreaseArpOct();
                        }
                    }
                    else {
                        stepRepeat = 0;
                    }
                }
            }
        } while (orderedKeys[stepArp].note == arpKey.note && orderedKeys.size() > 1 && !stepRepeat);
    }
}
void Arpeggiator::mode_downup() {
    if (restarted) {
        decreaseArpOct();
        direction = 0;
        octaveDirection = 0;
        stepArp = orderedKeys.size() - 1;
        restarted = 0;
    }
    else {

        do {

            // going up
            if (direction > 0) {
                stepArp = changeInt(stepArp, 1, 0, orderedKeys.size() - 1);
                if (stepArp == orderedKeys.size() - 1) {

                    direction = 0;
                    if (octaveDirection == 1) {
                        increaseArpOct();
                    }
                    else {
                        decreaseArpOct();
                    }
                }
            }
            // going down
            else {
                stepArp = changeInt(stepArp, -1, 0, orderedKeys.size() - 1);
                if (stepArp == 0) {
                    direction = 1;
                }
            }
        } while (orderedKeys[stepArp].note == arpKey.note && orderedKeys.size() > 1);
    }
}

void Arpeggiator::mode_downrupr() {
    if (restarted) {
        decreaseArpOct();
        direction = 0;
        octaveDirection = 0;
        stepArp = orderedKeys.size() - 1;
        restarted = 0;
        stepRepeat = 1;
    }
    else {

        do {

            // going up
            if (direction > 0) {
                stepArp = changeInt(stepArp, 1, 0, orderedKeys.size() - 1);
                if (stepArp == orderedKeys.size() - 1) {
                    if (!stepRepeat) {
                        stepRepeat = 1;

                        direction = 0;
                        if (octaveDirection == 1) {
                            increaseArpOct();
                        }
                        else {
                            decreaseArpOct();
                        }
                    }
                    else {
                        stepRepeat = 0;
                    }
                }
            }
            // going down
            else {
                stepArp = changeInt(stepArp, -1, 0, orderedKeys.size() - 1);
                if (stepArp == 0) {
                    if (!stepRepeat) {
                        stepRepeat = 1;
                        direction = 1;
                    }
                    else {
                        stepRepeat = 0;
                    }
                }
            }
        } while (orderedKeys[stepArp].note == arpKey.note && orderedKeys.size() > 1 && !stepRepeat);
    }
}
#endif
