#ifdef POLYCONTROL
#include "voiceHandler.hpp"
#include "com/com.hpp"
#include "layer/layer.hpp"

extern COMinterChip layerCom;
extern std::vector<Layer *> allLayers;

void VoiceHandler::playNote(const Key &key) {

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

    if (livemodeMergeLayer.value == 1) {
        getNextVoicesAB(numberVoices);
    }
    else {
        getNextVoices(numberVoices, key.layerID);
    }

    for (voiceStateStruct *v : foundVoices) {
        playIDCounter++; // increase playID

        v->status = PLAY;
        v->note = key.note;
        v->velocity = key.velocity;
        v->playID = playIDCounter;

        layerCom.sendNewNote(v->layerID, v->voiceID, v->note, v->velocity);

        if (allLayers[v->layerID]->lfoA.dAlignLFOs == 1 && allLayers[v->layerID]->lfoA.dGateTrigger == 1) {
            layerCom.sendRetrigger(v->layerID, allLayers[v->layerID]->lfoA.id, VOICEALL);
        }
        if (allLayers[v->layerID]->lfoB.dAlignLFOs == 1 && allLayers[v->layerID]->lfoB.dGateTrigger == 1) {
            layerCom.sendRetrigger(v->layerID, allLayers[v->layerID]->lfoB.id, VOICEALL);
        }

        // println("PLAY VOICE | note :", v->note, "  playIDCount :", v->playID, "  Voice ID :", v->voiceID);
    }
}

void VoiceHandler::freeNote(const Key &key) {

    if (livemodeMergeLayer.value == 1) { // layer merged
        for (size_t i = 0; i < 2; i++) { // loop over 2 Layer
            findVoices(key.note, voices[i]);
            for (voiceStateStruct *v : foundVoices) {
                if (sustain[i]) {
                    v->status = SUSTAIN;
                }
                else {
                    sendGateOff(v);
                }
            }
        }
    }

    else {
        findVoices(key.note, voices[key.layerID]);
        for (voiceStateStruct *v : foundVoices) {
            if (sustain[key.layerID]) {
                v->status = SUSTAIN;
            }
            else {
                sendGateOff(v);
            }
        }
    }
}

void VoiceHandler::reset(uint8_t layer) {
    if (livemodeMergeLayer.value == 1) {
        if (layer == 0) {
            for (uint8_t i = 0; i < 2; i++) {
                for (uint8_t x = 0; x < NUMBERVOICES; x++) {
                    sendGateOff(&voices[i][x]);
                }
                sustain[i] = 0;
            }
        }
    }

    else {
        for (uint8_t i = 0; i < NUMBERVOICES; i++) {
            sendGateOff(&voices[layer][i]);
        }
        sustain[layer] = 0;
    }
}

void VoiceHandler::setSustain(uint8_t value, uint8_t layer) {
    if (value >= 64) {
        sustainOn(layer);
    }
    if (value < 64) {
        sustainOff(layer);
    }
}

void VoiceHandler::sendGateOff(voiceStateStruct *v) {

    v->status = FREE;

    layerCom.sendCloseGate(v->layerID, v->voiceID);
}

void VoiceHandler::sustainOff(uint8_t layer) {

    if (livemodeMergeLayer.value == 1) {
        if (layer == 0) {
            for (uint8_t i = 0; i < 2; i++) {
                for (uint8_t x = 0; x < NUMBERVOICES; x++) {
                    if (voices[i][x].status == SUSTAIN) {
                        sendGateOff(&voices[i][x]);
                    }
                }
                sustain[i] = 0;
            }
        }
    }

    else {
        for (uint8_t i = 0; i < NUMBERVOICES; i++) {
            if (voices[layer][i].status == SUSTAIN) {
                sendGateOff(&voices[layer][i]);
            }
        }
        sustain[layer] = 0;
    }
}
void VoiceHandler::sustainOn(uint8_t layer) {
    sustain[layer] = 1;
}

void VoiceHandler::findVoices(uint8_t note, voiceStateStruct *voices) {
    foundVoices.clear();

    for (uint8_t i = 0; i < NUMBERVOICES; i++) {
        if (voices[i].note == note && voices[i].status == PLAY) {
            foundVoices.push_back(&voices[i]);
        }
    }
}

void VoiceHandler::getNextVoices(uint8_t numberVoices, uint8_t layer) {
    foundVoices.clear();

    for (uint8_t v = 0; v < numberVoices; v++) {
        searchNextVoice(voices[layer]);
    }
}

void VoiceHandler::getNextVoicesAB(uint8_t numberVoices) {

    foundVoices.clear();

    for (uint8_t v = 0; v < numberVoices; v++) {
        searchNextVoiceAB();
    }
}

void VoiceHandler::searchNextVoice(voiceStateStruct *voiceLayer) {

    uint8_t oldestVoiceID = 0xFF;

    // find oldest FREE Voice

    for (uint8_t i = 0; i < NUMBERVOICES; i++) {
        if (voiceLayer[i].status == FREE) {
            if (oldestVoiceID != 0xFF) { // compare
                if (voiceLayer[i].playID < voiceLayer[oldestVoiceID].playID) {
                    oldestVoiceID = voiceLayer[i].voiceID;
                }
            }
            else { // first found
                oldestVoiceID = voiceLayer[i].voiceID;
            }
        }
    }

    // no free voice found, take the oldest played one
    if (oldestVoiceID == 0xFF) {
        for (uint8_t i = 0; i < NUMBERVOICES; i++) {
            // found oldest NOTE
            if (voiceLayer[i].status != SELECT) {
                if (oldestVoiceID != 0xFF) { // compare
                    if (voiceLayer[i].playID < voiceLayer[oldestVoiceID].playID) {
                        oldestVoiceID = voiceLayer[i].voiceID;
                    }
                }
                else { // first found
                    oldestVoiceID = voiceLayer[i].voiceID;
                }
            }
        }
    }
    if (oldestVoiceID == 0xFF) {
        PolyError_Handler("ERROR | LOGIC | VoiceHandler -> call for too many voices?");
        return;
    }

    voiceLayer[oldestVoiceID].status = SELECT;
    foundVoices.push_back(&voiceLayer[oldestVoiceID]);
}

void VoiceHandler::searchNextVoiceAB() {

    voiceStateStruct *nextVoice = nullptr;

    // find oldest FREE Voice
    for (uint8_t i = 0; i < 2; i++) {
        if (allLayers[i]->layerState.value == 1) { // check layerState
            for (uint8_t x = 0; x < NUMBERVOICES; x++) {
                if (voices[i][x].status == FREE) {
                    if (voices[i][x].playID < nextVoice->playID) {
                        nextVoice = &voices[i][x];
                    }
                }
            }
        }
    }

    // no voice free voice found, take the oldest played one
    if (nextVoice == nullptr) {

        for (uint8_t i = 0; i < 2; i++) {
            if (allLayers[i]->layerState.value == 1) { // check layerState
                // find oldest NOTE
                for (uint8_t x = 0; x < NUMBERVOICES; x++) {
                    if (voices[i][x].status == SELECT) { // not already selected
                        if (voices[i][x].playID < nextVoice->playID) {
                            nextVoice = &voices[i][x];
                        }
                    }
                }
            }
        }
    }
    if (nextVoice == nullptr) {
        PolyError_Handler("ERROR | LOGIC | VoiceHandler -> call for to many voices?");
        return;
    }

    nextVoice->status = SELECT;
    foundVoices.push_back(nextVoice);
}

#endif
