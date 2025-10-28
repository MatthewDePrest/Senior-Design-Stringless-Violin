function frequencyToNote(freq) {
  const A4 = 440;
  const NOTES = [
    "C", "C#", "D", "D#", "E", "F",
    "F#", "G", "G#", "A", "A#", "B"
  ];

  const semitonesFromA4 = 12 * Math.log2(freq / A4);
  const nearestSemitone = Math.round(semitonesFromA4);
  const noteIndex = (nearestSemitone + 9 + 1200) % 12; // +1200 because javascript % doesn't work with negatives
  const midiNumber = 69 + nearestSemitone;
  const octave = Math.floor(midiNumber / 12) - 1;

  return `${NOTES[noteIndex]}${octave}`;
}

// Example usage:
console.log(frequencyToNote(440));    // → "A4"
console.log(frequencyToNote(265));    // → "C4"
console.log(frequencyToNote(322));    // → "E4"
console.log(frequencyToNote(880));    // → "A5"
console.log(frequencyToNote(36.71));  // → "D1"
