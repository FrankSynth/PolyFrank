#ifdef POLYCONTROL
#include "liveData.hpp"

LiveData liveData;

uint16_t clockTicksPerStep[23] = {1,  2,  3,  4,  6,  8,   9,   12,  16,  18,  24, 32,
                                  36, 48, 64, 72, 96, 128, 144, 192, 256, 288, 384};

void VoiceHandler::playNote(Key &key) {

    uint8_t numberVoices = 0;

    if (livemodeVoiceMode.value == 0) { // 1Voice
        numberVoices = 1;
    }
    else if (livemodeVoiceMode.value == 1) { // 2Voices
        numberVoices = 2;
    }
    else if (livemodeVoiceMode.value == 2) { // 4Voices
        numberVoices = 4;
    }
    else if (livemodeVoiceMode.value == 3) { // 8Voices
        numberVoices = 8;
    }
    else if (livemodeVoiceMode.value == 4) { // AUTO
        // TODO Auto number of voice select
        numberVoices = 1;
    }

    if (livemodeMergeLayer.value == 1) {
        getNextVoicesAB(numberVoices);
    }

    else {
        if (key.layerID == 0) {
            getNextVoicesA(numberVoices);
        }
        else if (key.layerID == 1) {
            getNextVoicesB(numberVoices);
        }
    }

    for (voiceStateStruct *v : foundVoices) {

        v->status = PLAY;
        v->note = key.note;
        v->velocity = key.velocity;
        v->playID = playIDCounter;

        // println("PLAY VOICE | note :", v->note, "  playIDCount :", v->playID, "  Voice ID :", v->voiceID);
        // TODO send Note, Gate, Velocity

        playIDCounter++; // increase playID
    }
}

void VoiceHandler::freeNote(Key &key) {

    if (livemodeMergeLayer.value == 1) {

        findVoices(key.note, voicesA);
        for (voiceStateStruct *v : foundVoices) {
            if (sustainA) {
                v->status = SUSTAIN;
            }
            else {
                sendGateOff(v);
            }
        }

        findVoices(key.note, voicesB);
        for (voiceStateStruct *v : foundVoices) {
            if (sustainA) {
                v->status = SUSTAIN;
            }
            else {
                sendGateOff(v);
            }
        }
    }

    else {
        if (key.layerID == 0) {
            findVoices(key.note, voicesA);
            for (voiceStateStruct *v : foundVoices) {
                if (sustainA) {
                    v->status = SUSTAIN;
                }
                else {
                    sendGateOff(v);
                }
            }
        }
        else if (key.layerID == 1) {
            findVoices(key.note, voicesB);
            for (voiceStateStruct *v : foundVoices) {
                if (sustainB) {
                    v->status = SUSTAIN;
                }
                else {
                    sendGateOff(v);
                }
            }
        }
    }
}

void VoiceHandler::sendGateOff(voiceStateStruct *v) {

    v->status = FREE;
    // TODO send Gate off
}

void VoiceHandler::sustainOff(uint8_t layer) {

    if (livemodeMergeLayer.value == 1) {
        for (voiceStateStruct v : voicesA) {
            if (v.status == SUSTAIN) {
                sendGateOff(&v);
            }
        }
        sustainA = 0;
        for (voiceStateStruct v : voicesB) {
            if (v.status == SUSTAIN) {
                sendGateOff(&v);
            }
        }
        sustainB = 0;
    }

    else {
        if (layer == 0) {
            for (voiceStateStruct v : voicesA) {
                if (v.status == SUSTAIN) {
                    sendGateOff(&v);
                }
            }
            sustainA = 0;
        }
        else if (layer == 1) {
            for (voiceStateStruct v : voicesB) {
                if (v.status == SUSTAIN) {
                    sendGateOff(&v);
                }
            }
            sustainB = 0;
        }
    }
}
void VoiceHandler::sustainOn(uint8_t layer) {
    if (layer == 0) {
        sustainA = 1;
    }
    if (layer == 1) {
        sustainB = 1;
    }
}

void VoiceHandler::findVoices(uint8_t note, voiceStateStruct *voices) {
    foundVoices.clear();

    for (uint8_t i = 0; i < NUMBERVOICES; i++) {
        if (voices[i].note == note && voices[i].status == PLAY) {
            foundVoices.push_back(&voices[i]);
        }
    }
}

void VoiceHandler::getNextVoicesA(uint8_t numberVoices) {
    foundVoices.clear();

    for (uint8_t v = 0; v < numberVoices; v++) {
        searchNextVoice(voicesA);
    }
}

void VoiceHandler::getNextVoicesB(uint8_t numberVoices) {

    foundVoices.clear();

    for (uint8_t v = 0; v < numberVoices; v++) {
        searchNextVoice(voicesB);
    }
}

void VoiceHandler::getNextVoicesAB(uint8_t numberVoices) {

    foundVoices.clear();

    // to keep in Sync search in voiceA und take same voiceID from B
    for (uint8_t v = 0; v < numberVoices; v++) {
        searchNextVoice(voicesA);
    }
    for (voiceStateStruct *v : foundVoices) {
        foundVoices.push_back(&voicesB[v->voiceID]);
    }
}

void VoiceHandler::searchNextVoice(voiceStateStruct *voices) {

    uint8_t oldestVoiceID = 0xFF;

    // find oldest FREE Voice

    for (uint8_t i = 0; i < NUMBERVOICES; i++) {
        if (voices[i].status == FREE) {
            if (voices[i].playID <= voicesA[oldestVoiceID].playID) {
                oldestVoiceID = voices[i].voiceID;
            }
        }
    }

    // no voice free voice found, take the oldest played one
    if (oldestVoiceID == 0xFF) {
        for (uint8_t i = 0; i < NUMBERVOICES; i++) {
            // found oldest NOTE
            if (voices[i].status != SELECT) {
                if (voices[i].playID <= voicesA[oldestVoiceID].playID) {
                    oldestVoiceID = voices[i].voiceID;
                }
            }
        }
    }
    if (oldestVoiceID == 0xFF) {
        PolyError_Handler("ERROR | LOGIC | VoiceHandler -> call for to much voices?");
    }

    voices[oldestVoiceID].status = SELECT;
    foundVoices.push_back(&voices[oldestVoiceID]);
}

// functions
void Arpeggiator::keyPressed(Key key) {
    allKeysReleased = 1;
    if (!inputKeys.empty()) {                                                                // inputKey list empty?
        for (std::list<Key>::iterator it = inputKeys.begin(); it != inputKeys.end(); it++) { // all Keys released ?
            if (!it->released) {
                allKeysReleased = 0;
                break;
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
void Arpeggiator::keyReleased(Key key) {

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

void Arpeggiator::pressAndLifetime(Key key) {
    key.releaseMe = micros() + 100; // in micros //TODO Lifespan berechnen
    voiceHandler->playNote(key);    // play new note
    pressedKeys.push_back(key);     // add to pressed List

    // TODO multiple KEYS pressed
    voiceHandler->freeNote(pressedKeys.front()); // free oldest key
    pressedKeys.pop_front();                     // kick from list
}

void Arpeggiator::releaseAndRatchet() {

    // TODO Ratched
    for (std::list<Key>::iterator it = pressedKeys.begin(); it != pressedKeys.end();) {
        if (it->releaseMe < micros()) {  // find keys to be released //TODO überkauf checken
            voiceHandler->freeNote(*it); // free Note
            pressedKeys.erase(it);       // delete key

            println("ReleaseKey");
        }
        else {
            it++;
        }
    }
}

void Arpeggiator::orderKeys() { // TODO Key Order
    orderedKeys.clear();

    for (std::list<Key>::iterator it = inputKeys.begin(); it != inputKeys.end(); it++) {
        orderedKeys.push_back(*it);
    }
}
void Arpeggiator::nextStep() {

    if (!arpEnable.value) {
        return;
    }
    keyListPosition++;
    keyListPosition %= orderedKeys.size();
    pressAndLifetime(orderedKeys[keyListPosition]);
}

void LiveData::controlChange(uint8_t channel, uint8_t cc, uint8_t value) {
    // TODO zu Midi Modul weiterleiten
}

// functions
void LiveData::keyPressed(uint8_t channel, uint8_t note, uint8_t velocity) {

    Key key;
    key.note = note;
    key.velocity = velocity;

    // check Midi channel

    if (channel == globalSettings.midiLayerAChannel.value) {                         // check midi Channel
        if ((livemodeKeysplit.value && key.note >= 44) || !livemodeKeysplit.value) { // Key split ? + upper half
            key.layerID = 0;

            if (!arpA.arpEnable.value) {    // arp aus?
                voiceHandler.playNote(key); // directMode
            }
            arpA.keyPressed(key);
        }
    }

    if (channel == globalSettings.midiLayerBChannel.value) {
        if ((livemodeKeysplit.value && key.note < 44) || !livemodeKeysplit.value) {
            key.layerID = 1;
            if (!arpB.arpEnable.value) {
                voiceHandler.playNote(key);
            }
            arpB.keyPressed(key);
        }
    }
}
void LiveData::keyReleased(uint8_t channel, uint8_t note) {

    Key key;
    key.note = note;

    if (channel == globalSettings.midiLayerAChannel.value) { // check midi Channel
        key.layerID = 0;

        if (!arpA.arpEnable.value) {    // arp aus?
            voiceHandler.freeNote(key); // directMode
        }
        arpA.keyReleased(key);
    }

    if (channel == globalSettings.midiLayerBChannel.value) {
        key.layerID = 1;

        if (!arpB.arpEnable.value) {
            voiceHandler.freeNote(key);
        }
        arpB.keyReleased(key);
    }
}

void LiveData::clockTick() {
    clock.tick();
}

void LiveData::update() {

    arpA.releaseAndRatchet();
    arpB.releaseAndRatchet();

    clockHandling();
}

void LiveData::clockHandling() {

    // TODO polyrythm
    // TODO LFO

    if (clock.ticked) {
        for (uint8_t i = 0; i < NUMBERLAYER; i++) {
            if (!(clock.counter % clockTicksPerStep[arps[i]->arpSteps.value])) {
                arps[i]->nextStep();

                // println("nextStep");
            }
        }
        clock.ticked = 0;
    }
}

#endif
