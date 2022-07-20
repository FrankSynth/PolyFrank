#ifdef POLYCONTROL
#include "arp.hpp"

extern Clock clock;

// functions
void Arpeggiator::keyPressed(Key &key) {
    allKeysReleased = 1;
    midiUpdateDelayTimer = 0;
    if (!inputKeys.empty()) {                                            // inputKey list empty?
        for (auto it = inputKeys.begin(); it != inputKeys.end(); it++) { // all Keys released ?
            if (!it->released) {
                allKeysReleased = 0;
            }
        }

        if (allKeysReleased) { // clear list

            inputKeys.clear();
            allKeysReleased = 0;
        }
        else { // check already existing Keys

            for (auto it = inputKeys.begin(); it != inputKeys.end(); it++) { // Key already exist?
                if (it->note == key.note) {
                    it->velocity = key.velocity;
                    reorder = 1;
                    return;
                }
            }
        }
    }

    inputKeys.push_back(key);
    reorder = 1;
}
void Arpeggiator::keyReleased(Key &key) {
    midiUpdateDelayTimer = 0;

    if (inputKeys.empty()) {
        return;
    }
    for (auto it = inputKeys.begin(); it != inputKeys.end(); it++) {
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
    reorder = 1;
}

void Arpeggiator::pressKey(Key &key) {

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

    key.born = micros();

    // uint32_t lifespan = (60000000 / (clock.bpm * 24)) * ticksToNextStep * arpPlayedKeysParallel.value; // in micros
    key.lifespan = (float)((60000000 / (clock.bpm * 24)) * ticksToNextStep) *
                   (1.0f / ((float)arpRatched.value + 1.0f)); // in micros

    key.retriggerAmounts = arpPlayedKeysParallel.value - 1;
    key.ratchedAmounts = arpRatched.value;
}

void Arpeggiator::serviceRoutine() {
    checkLatch();
    release();
    ratched();

    if (!arpEnable.value)
        return;
    if (arpStepDelayed)
        nextStep();
}

void Arpeggiator::ratched() {

    if (ratchedKeys.empty())
        return;

    for (auto it = ratchedKeys.begin(); it != ratchedKeys.end();) {
        if ((micros() - it->born) > it->lifespan) {
            it->born = micros();
            pressKey(*it);
            it = ratchedKeys.erase(it); // delete key
        }
        else {
            it++;
        }
    }
}

void Arpeggiator::release() {

    if (pressedKeys.empty())
        return;

    for (auto it = pressedKeys.begin(); it != pressedKeys.end();) {
        // find keys to be released , check sustain-> half lifespan
        if (((micros() - it->born) > it->lifespan && arpSustain) ||
            (((micros() - it->born) > (it->lifespan / 2)) && !arpSustain)) {

            voiceHandler->freeNote(*it); // free Note

            if (it->ratchedAmounts) {
                it->ratchedAmounts--;
                ratchedKeys.push_back(*it);
            }

            else if (it->retriggerAmounts) {
                it->retriggerAmounts--;
                retriggerKeys.push_back(*it);
            }

            it = pressedKeys.erase(it); // delete key
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
        if (inputKeys.empty())
            return;
        for (auto it = inputKeys.begin(); it != inputKeys.end();) {
            if (it->released) {
                it = inputKeys.erase(it); // delete key
            }
            else {
                it++;
            }
        }
        reorder = 1;
    }
}

void Arpeggiator::restart() {

    stepArp = 0;
    restarted = 1;
    midiUpdateDelayTimer = 0;

    pressedKeys.clear();
    retriggerKeys.clear();
    ratchedKeys.clear();
    reorder = 1;

    if (arpEnable.value)
        voiceHandler->reset(layerID); // free Note
}

void Arpeggiator::continueRestart() {

    midiUpdateDelayTimer = 0;

    pressedKeys.clear();
    retriggerKeys.clear();
    ratchedKeys.clear();
    reorder = 1;

    if (arpEnable.value)
        voiceHandler->reset(layerID); // free Note
}

void Arpeggiator::reset() {
    inputKeys.clear();
    restart();
}

void Arpeggiator::orderKeys() {
    // println(micros(), " - ordered");
    orderedKeys.clear();

    if (inputKeys.empty())
        return;

    for (auto it = inputKeys.begin(); it != inputKeys.end(); it++) {
        orderedKeys.push_back(*it);
    }
    std::sort(orderedKeys.begin(), orderedKeys.end(), compareByNote);
    reorder = 0;
}

void Arpeggiator::nextStep() {
    if (!arpEnable.value)
        return;

    if (midiUpdateDelayTimer < MIDIARPUPDATEDELAY) {
        arpStepDelayed = 1;
        // println(micros(), " - step delayed");
        return;
    }
    else {
        // println(micros(), " - step not delayed");
        arpStepDelayed = 0;
    }

    if (reorder)
        orderKeys();

    if (orderedKeys.empty())
        return;

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

    lifetime(key); // calculate lifespan

    // println(micros(), " - new key: ", key.note);

    // TODO not right when new notes are pressed, must be cleared and instead pull "X" complete new ARP notes
    if (!retriggerKeys.empty()) {
        // repress all retrigger Notes
        for (Key retKey : retriggerKeys) {
            retKey.lifespan = key.lifespan;
            retKey.born = key.born;
            retKey.ratchedAmounts = key.ratchedAmounts;
            pressKey(retKey);
        }
        retriggerKeys.clear();
    }

    // press new key
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
        stepArp = (int32_t)(orderedKeys.size()) - 1;
        restarted = 0;
    }
    else {
        // if arp size increased by one, a note would be repeated, so instead, we increase further
        do {
            stepArp = changeIntLoop(stepArp, -1, 0, (int32_t)(orderedKeys.size()) - 1);
        } while (orderedKeys[stepArp].note == arpKey.note && (int32_t)(orderedKeys.size()) > 1);
    }
}

void Arpeggiator::mode_down2() {

    if (stepArp == 0 && direction == 0) {
        decreaseArpOct();
    }
    if (restarted) {
        decreaseArpOct();
        stepArp = (int32_t)(orderedKeys.size()) - 1;
        restarted = 0;
        direction = 0;
    }
    else {
        // if arp size increased by one, a note would be repeated, so instead, we increase further
        do {
            if (direction == 0) {
                int16_t nextArpStep = (int32_t)stepArp - 2;
                if (nextArpStep == -1) {
                    stepArp = 0;
                }
                else if (nextArpStep < -1) {
                    stepArp = (int32_t)(orderedKeys.size()) - 1;
                }
                else {
                    stepArp = nextArpStep;
                    if (!(nextArpStep == 0 && (int32_t)(orderedKeys.size()) % 2 == 0))
                        direction = 1;
                }
            }
            else {
                stepArp = changeInt(stepArp, 1, 0, (int32_t)(orderedKeys.size()) - 1, true);

                direction = 0;
            }
        } while (orderedKeys[stepArp].note == arpKey.note && (int32_t)(orderedKeys.size()) > 1);
    }
}

void Arpeggiator::mode_down3() {
    if (stepArp == 0) {
        decreaseArpOct();
    }
    if (restarted) {
        decreaseArpOct();
        stepArp = (int32_t)(orderedKeys.size()) - 1;
        restarted = 0;
        direction = 0;
    }
    else {
        // if arp size increased by one, a note would be repeated, so instead, we increase further
        if (direction < 2) {
            do {
                if (stepArp == 0) {
                    stepArp = (int32_t)(orderedKeys.size()) - 1;
                    direction = 0 - 1;
                }
                else {
                    stepArp = changeInt(stepArp, -1, 0, (int32_t)(orderedKeys.size()) - 1, true);
                }
            } while (orderedKeys[stepArp].note == arpKey.note && (int32_t)(orderedKeys.size()) > 1);
            direction++;
        }
        else {
            do {
                if (stepArp == 0) {
                    stepArp = (int32_t)(orderedKeys.size()) - 1;
                }
                else {
                    if (stepArp == (int32_t)(orderedKeys.size()) - 1) {
                        stepArp = 0;
                    }
                    else {
                        stepArp = changeInt(stepArp, 1, 0, (int32_t)(orderedKeys.size()) - 1, true);
                    }
                }
            } while (orderedKeys[stepArp].note == arpKey.note && (int32_t)(orderedKeys.size()) > 1);
            direction = 0;
        }
    }
}
void Arpeggiator::mode_ordr() {
    if (stepArp == (int32_t)(orderedKeys.size()) - 1) {
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
            stepArp = changeIntLoop(stepArp, 1, 0, (int32_t)(orderedKeys.size()) - 1);
        } while (orderedKeys[stepArp].note == arpKey.note && (int32_t)(orderedKeys.size()) > 1);
    }
}
void Arpeggiator::mode_up2() {
    if (stepArp == (int32_t)(orderedKeys.size()) - 1 && direction > 0) {
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
                int16_t nextArpStep = (int32_t)stepArp + 2;
                if (nextArpStep == (int32_t)(int32_t)(orderedKeys.size())) {
                    stepArp = (int32_t)(orderedKeys.size()) - 1;
                    if (orderedKeys[stepArp].note == arpKey.note)
                        stepArp = 0;
                }
                else if (nextArpStep > (int32_t)(int32_t)(orderedKeys.size())) {
                    stepArp = 0;
                }
                else {
                    stepArp = nextArpStep;
                    if (!(nextArpStep == (int32_t)(int32_t)(orderedKeys.size()) - 1 &&
                          (int32_t)(orderedKeys.size()) % 2 == 0))
                        direction = 0;
                }
            }
            else {
                stepArp = changeInt(stepArp, -1, 0, (int32_t)(orderedKeys.size()) - 1, true);
                direction = 1;
            }
        } while (orderedKeys[stepArp].note == arpKey.note && (int32_t)(orderedKeys.size()) > 1);
    }
}
void Arpeggiator::mode_up3() {
    if (stepArp == (int32_t)(orderedKeys.size()) - 1) {
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
                if (stepArp == (int32_t)(orderedKeys.size()) - 1) {
                    stepArp = 0;
                    direction = 2 + 1;
                }
                else {
                    stepArp = changeInt(stepArp, 1, 0, (int32_t)(orderedKeys.size()) - 1, true);
                }
            } while (orderedKeys[stepArp].note == arpKey.note && (int32_t)(orderedKeys.size()) > 1);
            direction--;
        }
        else {
            do {

                if (stepArp == (int32_t)(orderedKeys.size()) - 1) {
                    stepArp = 0;
                }
                else {
                    if (stepArp == 0)
                        stepArp = (int32_t)(orderedKeys.size()) - 1;
                    else
                        stepArp = changeInt(stepArp, -1, 0, (int32_t)(orderedKeys.size()) - 1, true);
                }
            } while (orderedKeys[stepArp].note == arpKey.note && (int32_t)(orderedKeys.size()) > 1);
            direction = 2;
        }
    }
}
void Arpeggiator::mode_rnd() {
    if (randomCounter == (int32_t)(orderedKeys.size()) - 1) {
        increaseArpOct();
    }
    if (restarted) {
        increaseArpOct();
        randomCounter = 0;
        restarted = 0;
    }
    else {
        randomCounter = changeIntLoop(randomCounter, 1, 0, (int32_t)(orderedKeys.size()) - 1);
    }

    uint32_t randomNumber = std::rand() & 0x0000FFFF;

    stepArp = randomNumber * (int32_t)(orderedKeys.size()) / 0x0000FFFF;
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
                stepArp = changeInt(stepArp, 1, 0, (int32_t)(orderedKeys.size()) - 1);
                if (stepArp == (int32_t)(orderedKeys.size()) - 1) {
                    direction = 0;
                }
            }
            // going down
            else {
                stepArp = changeInt(stepArp, -1, 0, (int32_t)(orderedKeys.size()) - 1);
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
        } while (orderedKeys[stepArp].note == arpKey.note && (int32_t)(orderedKeys.size()) > 1);
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
                stepArp = changeInt(stepArp, 1, 0, (int32_t)(orderedKeys.size()) - 1);
                if (stepArp == (int32_t)(orderedKeys.size()) - 1) {
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
                stepArp = changeInt(stepArp, -1, 0, (int32_t)(orderedKeys.size()) - 1);
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
        } while (orderedKeys[stepArp].note == arpKey.note && (int32_t)(orderedKeys.size()) > 1 && !stepRepeat);
    }
}
void Arpeggiator::mode_downup() {
    if (restarted) {
        decreaseArpOct();
        direction = 0;
        octaveDirection = 0;
        stepArp = (int32_t)(orderedKeys.size()) - 1;
        restarted = 0;
    }
    else {

        do {

            // going up
            if (direction > 0) {
                stepArp = changeInt(stepArp, 1, 0, (int32_t)(orderedKeys.size()) - 1);
                if (stepArp == (int32_t)(orderedKeys.size()) - 1) {

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
                stepArp = changeInt(stepArp, -1, 0, (int32_t)(orderedKeys.size()) - 1);
                if (stepArp == 0) {
                    direction = 1;
                }
            }
        } while (orderedKeys[stepArp].note == arpKey.note && (int32_t)(orderedKeys.size()) > 1);
    }
}

void Arpeggiator::mode_downrupr() {
    if (restarted) {
        decreaseArpOct();
        direction = 0;
        octaveDirection = 0;
        stepArp = (int32_t)(orderedKeys.size()) - 1;
        restarted = 0;
        stepRepeat = 1;
    }
    else {

        do {

            // going up
            if (direction > 0) {
                stepArp = changeInt(stepArp, 1, 0, (int32_t)(orderedKeys.size()) - 1);
                if (stepArp == (int32_t)(orderedKeys.size()) - 1) {
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
                stepArp = changeInt(stepArp, -1, 0, (int32_t)(orderedKeys.size()) - 1);
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
        } while (orderedKeys[stepArp].note == arpKey.note && (int32_t)(orderedKeys.size()) > 1 && !stepRepeat);
    }
}
#endif
