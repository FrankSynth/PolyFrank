#ifdef POLYCONTROL
#include "voiceHandler.hpp"
#include "com/com.hpp"

extern COMinterChip layerCom[2];

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

        layerCom[v->layerID].sendNewNote(v->voiceID, v->note, v->velocity);

        // println("PLAY VOICE | note :", v->note, "  playIDCount :", v->playID, "  Voice ID :", v->voiceID);

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

    layerCom[v->layerID].sendCloseGate(v->voiceID);
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

#endif
