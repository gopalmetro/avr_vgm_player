
void commandMessage() {
  switch (statusCommand)
    case 0x09:
    //do stuff
    break;
}

void manageChannels() {
  // MIDI Channels 1, 2 and 3 are identical
  switch (MIDIChannel)
    case 0x00:
    case 0x01:
    case 0x02:
    // do stuff
    break;
}
